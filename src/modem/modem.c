/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <anj/log/log.h>
#include <anj/utils.h>

#include <stm32u3xx_hal.h>
#include <usart.h>

#include "modem.h"
#include "modem_constants.h"
#include "modem_rx.h"
#include "modem_tx.h"

#include "circ_buf.h"

#define modem_log(...) anj_log(modem, __VA_ARGS__)

// Note: we rely on this using utils from Anjay Lite
ANJ_STATIC_ASSERT(sizeof(size_t) == sizeof(uint32_t), size_t_is_4_bytes);

// Implementation limitations (besides the ones described in net.c):
// - no other URCs besides +QIURC: "recv" are handled; other URCs that might be
//   worth handling:
//   - "closed" - in case the remote end (in TCP) closes the connection
//   - "pdpdeact" - in case the PDP context is deactivated
// - there's no detection of other spurious messages that might be sent by the
//   modem, e.g. when the connection with the network is lost
// - no support for HW flow control; this might be helpful in case we'd like to
//   avoid extensive buffering in our driver
// - no protection whatsoever against calling different methods for different
//   sockets in case some "stateful" operation is in progress, e.g. asynchronous
//   handling of AT+QIOPEN requires multiple calls
// - no timeout handling for any of the methods

static bool line_starts_with(const char *str, size_t line_len) {
    size_t i = 0;
    while (str[i] != '\0' && i < line_len) {
        if (modem_rx_seek(i) != str[i]) {
            return false;
        }
        i++;
    }
    return str[i] == '\0';
}

static bool line_equals(const char *str, size_t line_len) {
    for (size_t i = 0; i < line_len; i++) {
        if (str[i] == '\0' || modem_rx_seek(i) != str[i]) {
            return false;
        }
    }

    return str[line_len] == '\0';
}

// NOTE: implemented as macro to correctly report the line number
#define warn_and_advance(Len)                                                 \
    do {                                                                      \
        size_t To_seek = ANJ_MIN(Len, 64);                                    \
        char Buf[64 + 1];                                                     \
        size_t I = 0;                                                         \
        for (; I < To_seek; I++) {                                            \
            Buf[I] = modem_rx_seek(I);                                        \
        }                                                                     \
        Buf[I] = '\0';                                                        \
        if (Len > 64) {                                                       \
            modem_log(L_WARNING, "skipping RX (len: %u): %s...",              \
                      (unsigned) Len, Buf);                                   \
        } else {                                                              \
            modem_log(L_WARNING, "skipping RX (len: %u): %s", (unsigned) Len, \
                      Buf);                                                   \
        }                                                                     \
        modem_rx_advance(Len);                                                \
    } while (0)

#define RECV_QIURC_LENS_BUF_SIZE 15
static volatile uint8_t recv_qiurc_buf_storage[MODEM_SOCKET_RECV_MAX + 1];
static circ_buf_t recv_qiurc_buf = {
    .len = sizeof(recv_qiurc_buf_storage),
    .storage = recv_qiurc_buf_storage
};

static size_t recv_qiurc_lens[RECV_QIURC_LENS_BUF_SIZE];
static size_t recv_qiurc_lens_start = 0;
static size_t recv_qiurc_lens_end = 0;
static size_t recv_qiurc_lens_count = 0;

static int urc_buffer_handler(void) {
    // incoming lines are in form:
    // +QIURC: "recv",0,<n>\r\n
    // <raw n bytes>

    // NOTE: this method may be called in case the whole payload has not been
    // written to the RX buffer yet, so do not consume the buffer eagerly.
    size_t line_len;
    if (modem_rx_seek_line_length(&line_len)) {
        return 1;
    }

    static const char header[] = "+QIURC: \"recv\",0,";
    static const char header_len = sizeof(header) - 1;
    if (!line_starts_with(header, line_len)) {
        warn_and_advance(line_len);
        return 1;
    }

    char msg_len_str[4]; // 1500 bytes max: assume 4 digits (no nullterm needed)
    size_t msg_len_str_len = line_len - header_len;
    if (msg_len_str_len > sizeof(msg_len_str)) {
        warn_and_advance(line_len);
        return -1;
    }
    for (size_t i = 0; i < msg_len_str_len; i++) {
        msg_len_str[i] = modem_rx_seek(header_len + i);
    }

    size_t msg_len;
    if (anj_string_to_uint32_value((uint32_t *) &msg_len, msg_len_str,
                                   msg_len_str_len)) {
        warn_and_advance(line_len);
        return -1;
    }

    // check if the whole message has been received; account for the header line
    // and for the following \r\n before the actual payload
    if (modem_rx_buf_avail() < line_len + msg_len + 2) {
        modem_log(L_TRACE, "no full line and payload in RX buffer");
        return 1;
    }

    // we're sure that the whole message has been received, so we can start
    // consuming the RX buffer
    modem_rx_advance(line_len + 2);

    if (msg_len > circ_buf_free(&recv_qiurc_buf)
            || recv_qiurc_lens_count >= RECV_QIURC_LENS_BUF_SIZE) {
        modem_log(L_WARNING,
                  "Dropping recv urc because the buffer is too short");
        warn_and_advance(msg_len);
        return -1;
    }

    // NOTE: change this when adding support for TCP
    bool is_udp = true;
    if (is_udp && msg_len == MODEM_SOCKET_RECV_MAX) {
        // BG96 TCP/IP AT Commands Manual states that up to 1500 bytes can be
        // received, but we don't know whether messages above that, in case of
        // UDP, are dropped, or truncated. Assume that they could be truncated,
        // so let's drop them.
        warn_and_advance(msg_len);
        return -1;
    }

    // read the actual payload
    for (size_t i = 0; i < msg_len; i++) {
        circ_buf_push(&recv_qiurc_buf, modem_rx_pop());
    }
    recv_qiurc_lens[recv_qiurc_lens_end] = msg_len;
    recv_qiurc_lens_end = (recv_qiurc_lens_end + 1) % RECV_QIURC_LENS_BUF_SIZE;
    recv_qiurc_lens_count++;

    return 0;
}

int modem_socket_try_recv(uint8_t *buf, size_t buf_len, size_t *out_msg_len) {
    if (recv_qiurc_lens_count == 0) {
        return urc_buffer_handler() < 0 ? -1 : 1;
    }

    size_t msg_len = recv_qiurc_lens[recv_qiurc_lens_start];
    recv_qiurc_lens_start =
            (recv_qiurc_lens_start + 1) % RECV_QIURC_LENS_BUF_SIZE;
    recv_qiurc_lens_count--;

    if (buf_len < msg_len) {
        modem_log(L_ERROR, "Buffer for message to receive to small");
        circ_buf_advance_n(&recv_qiurc_buf, msg_len);
        return -1;
    }

    for (size_t i = 0; i < msg_len; i++) {
        buf[i] = circ_buf_pop(&recv_qiurc_buf);
    }
    *out_msg_len = msg_len;

    return 0;
}

typedef struct {
    const char *response;
    int return_code;
} response_t;

static const response_t ok_or_error[] = { { "OK", 0 }, { "ERROR", -1 } };

static int match_responses(const response_t *responses,
                           size_t responses_len,
                           int on_unexpected) {
    size_t line_len;
    if (modem_rx_seek_line_length(&line_len)) {
        return 1;
    }
    for (size_t i = 0; i < responses_len; i++) {
        if (line_equals(responses[i].response, line_len)) {
            modem_rx_advance(line_len);
            return responses[i].return_code;
        }
    }

    warn_and_advance(line_len);
    return on_unexpected;
}

static int match_responses_strict(const response_t *responses,
                                  size_t responses_len) {
    return match_responses(responses, responses_len, -1);
}

static int match_responses_lenient(const response_t *responses,
                                   size_t responses_len) {
    return match_responses(responses, responses_len, 1);
}

int modem_send_command(const char *command) {
    int res;
    if ((res = modem_tx_append_str(command))) {
        return res;
    }
    if ((res = modem_tx_append_str("\r\n"))) {
        return res;
    }
    return modem_tx_start();
}

// NOTE: we observed that when connection times out application
// fails to properly close and reopen socket, it might be worth to
// debug this implementation (some issues might also be related to modem rx
// buffer handling)
int modem_socket_open_init(modem_socket_open_ctx_t *ctx,
                           const char *hostname,
                           const char *port) {
    // sanity flush of the buffer to clear out trash that might
    // cause issues in future
    modem_rx_buf_flush();
    const char *to_write[] = { "AT+QIOPEN=1,0,\"UDP\",\"", hostname, "\",",
                               port, ",0,1\r\n" };
    for (size_t i = 0; i < ANJ_ARRAY_SIZE(to_write); i++) {
        if (modem_tx_append_str(to_write[i])) {
            return -1;
        }
    }
    if (modem_tx_start()) {
        return -1;
    }
    ctx->step = 0;
    return 0;
}

int modem_socket_open_continue(modem_socket_open_ctx_t *ctx) {
    switch (ctx->step) {
    case 0: {
        int res = match_responses_strict(ok_or_error,
                                         ANJ_ARRAY_SIZE(ok_or_error));
        if (res) {
            return res;
        }
        ctx->step = 1;
        return 1;
    }
    case 1: {
        static const response_t responses[] = {
            { "+QIOPEN: 0,0", 0 },
        };
        return match_responses_strict(responses, ANJ_ARRAY_SIZE(responses));
    }
    default: { return -1; }
    }
}

int modem_socket_send_init(modem_socket_send_ctx_t *ctx, size_t len) {
    if (len > MODEM_SOCKET_SEND_MAX) {
        // modem cannot send messages longer than 1460 bytes in one go
        return -1;
    }

    char len_buf[6];
    len_buf[anj_uint32_to_string_value(len_buf, len)] = '\0';

    const char *to_write[] = { "AT+QISEND=0,", len_buf, "\r\n" };

    for (size_t i = 0; i < ANJ_ARRAY_SIZE(to_write); i++) {
        if (modem_tx_append_str(to_write[i])) {
            return -1;
        }
    }

    if (modem_tx_start()) {
        return -1;
    }
    ctx->step = 0;
    return 0;
}

int modem_socket_send_continue(modem_socket_send_ctx_t *ctx,
                               size_t len,
                               const uint8_t *buf) {
    switch (ctx->step) {
    case 0: {
        bool emptied = modem_rx_pop_newlines();
        if (emptied) {
            return 1;
        }
        char prompt = modem_rx_seek(0);
        if (prompt != '>') {
            modem_log(L_WARNING, "unexpected character instead of prompt: 0x%x",
                      (unsigned) prompt);

            return urc_buffer_handler() < 0 ? -1 : 1;
        }
        modem_rx_advance(1); // skip the prompt

        // transmit data
        int res;
        if ((res = modem_tx_append(buf, len))) {
            return res;
        }
        // append Ctrl-Z to signal end of transmission
        if ((res = modem_tx_append(&(const uint8_t) { 0x1A }, 1))) {
            return res;
        }
        ctx->step = 1;
        return modem_tx_start() ? -1 : 1;
    }
    case 1: {
        static const response_t responses[] = {
            { "SEND OK", 0 },
            // For some reason BG96 might send a space character after the
            // prompt, so let's handle it here...
            { " ", 1 }
        };
        return match_responses_strict(responses, ANJ_ARRAY_SIZE(responses));
    }
    default: { return -1; }
    }
}

// NOTE: we observed that when connection times out application
// fails to properly close and reopen socket, it might be worth to
// debug this implementation (some issues might also be related to modem rx
// buffer handling)
int modem_socket_close_init(void) {
    // sanity flush of the buffer to clear out trash that might
    // cause issues in future
    modem_rx_buf_flush();

    return modem_send_command("AT+QICLOSE=0");
}

int modem_socket_close_continue(void) {
    return match_responses_strict(ok_or_error, ANJ_ARRAY_SIZE(ok_or_error));
}

static int bringup_command_ex(const char *command,
                              const response_t *responses,
                              size_t responses_count,
                              uint32_t timeout_ms,
                              size_t attempts,
                              uint32_t delay_ms) {
    modem_log(L_DEBUG, "running bringup command: %s", command);
    HAL_Delay(200);

    for (size_t i = 0; i < attempts; i++) {
        modem_rx_buf_flush(); // clear leftovers from previous commands
        if (i > 0) {
            HAL_Delay(delay_ms);
        }
        if (modem_send_command(command)) {
            modem_log(L_ERROR, "failed to send command: %s", command);
            return -1;
        }
        uint32_t deadline = HAL_GetTick() + timeout_ms;
        while (HAL_GetTick() < deadline) {
            int res = match_responses_lenient(responses, responses_count);
            if (res > 0) {
                continue;
            }
            if (res < 0) {
                modem_log(L_ERROR, "received an error response, code: %d", res);
                return -1;
            }
            return 0;
        }
    }
    modem_log(L_ERROR, "bringup command timed out: %s", command);
    return -1;
}

static int bringup_command(const char *command) {
    return bringup_command_ex(command, ok_or_error, ANJ_ARRAY_SIZE(ok_or_error),
                              5000, 1, 0);
}

#ifndef CONFIG_APN
#    define CONFIG_APN "internet"
#endif // CONFIG_APN

int modem_bringup(void) {
    if (modem_rx_start()) {
        return -1;
    }

    // test if modem is responding at all; this also might help with autobaud
    if (bringup_command_ex("AT", ok_or_error, ANJ_ARRAY_SIZE(ok_or_error), 1000,
                           3, 1000)) {
        return -1;
    }
    // disable echo
    if (bringup_command("ATE0")) {
        return -1;
    }
    // print current config of various options
    if (bringup_command("AT&V")) {
        return -1;
    }
    // disable SMS URCs
    if (bringup_command("AT+CNMI=0,0,0,0,0")) {
        return -1;
    }
    // configure APN
    if (bringup_command_ex("AT+CGDCONT=1,\"IP\",\"" CONFIG_APN "\"",
                           ok_or_error, ANJ_ARRAY_SIZE(ok_or_error), 10000, 1,
                           0)) { //
        return -1;
    }
    // enable full functionality
    if (bringup_command("AT+CFUN=1")) {
        return -1;
    }
    // activate PDP context
    if (bringup_command_ex("AT+QIACT=1", ok_or_error,
                           ANJ_ARRAY_SIZE(ok_or_error), 20000, 1, 0)) {
        return -1;
    }
    // disable network registration URCs
    if (bringup_command("AT+CREG=0")) {
        return -1;
    }
    // disable sleep mode
    if (bringup_command("AT+QSCLK=0")) {
        return -1;
    }
    // disable PSM
    if (bringup_command("AT+CPSMS=0")) {
        return -1;
    }
    // disable eDRX
    if (bringup_command("AT+CEDRXS=0")) {
        return -1;
    }
    // configure modem to send URCs over UART1
    if (bringup_command("AT+QURCCFG=\"urcport\",\"uart1\"")) {
        return -1;
    }
    static const response_t creg_responses[] = {
        { "+CREG: 0,1", 0 }, // registered, home network
        { "+CREG: 0,5", 0 }, // registered, roaming,
        { "OK", 1 }, // if for some reason OK is returned early, do not treat it
                     // as success
        { "ERROR", -1 }, // error shall cause return early
    };
    // wait for modem to report proper network registration status
    if (bringup_command_ex("AT+CREG?", creg_responses,
                           ANJ_ARRAY_SIZE(creg_responses), 1000, 5, 1000)) {
        return -1;
    }
    // log PDP context status
    if (bringup_command("AT+QIACT?")) {
        return -1;
    }
    HAL_Delay(200);
    modem_rx_buf_flush();
    return 0;
}
