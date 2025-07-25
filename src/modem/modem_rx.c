/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stm32u3xx_hal.h>
#include <usart.h>

#include "circ_buf.h"
#include "modem_constants.h"
#include "modem_rx.h"

static volatile uint8_t rx_buf_storage[MODEM_RX_BUF];
static uint8_t byte;

static circ_buf_t rx_buf = {
    .len = sizeof(rx_buf_storage),
    .storage = rx_buf_storage
};

// NOTE: since project has been generated with USE_HAL_UART_REGISTER_CALLBACKS
// disabled, we can only override the callback that handles all UARTs. In case
// we'd like to support multiple UARTs, the CubeMX-generated code should be
// regenerated.
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &hlpuart1) {
        if (circ_buf_free(&rx_buf) == 0) {
            // RX buffer overflow
            assert(false);
            // pop the oldest byte to not crash the program in release builds
            circ_buf_pop(&rx_buf);
        }
        circ_buf_push(&rx_buf, byte);
        HAL_UART_Receive_IT(huart, &byte, 1);
    }
}

int modem_rx_start(void) {
    // start chain of interrupts
    if (HAL_UART_Receive_IT(&hlpuart1, &byte, 1) != HAL_OK)
        return -1;

    return 0;
}

static inline bool is_newline(char chr) {
    return chr == '\r' || chr == '\n';
}

bool modem_rx_pop_newlines(void) {
    while (circ_buf_avail(&rx_buf) > 0) {
        if (!is_newline(circ_buf_seek(&rx_buf, 0))) {
            return false;
        }
        circ_buf_advance(&rx_buf);
    }
    return true;
}

int modem_rx_seek_line_length(size_t *out_line_len) {
    if (modem_rx_pop_newlines()) {
        return -1;
    }

    for (size_t i = 0; i < circ_buf_avail(&rx_buf); i++) {
        if (is_newline(circ_buf_seek(&rx_buf, i))) {
            // now first *out_line_len chars in buffer
            // are valid characters of a line
            *out_line_len = i;
            return 0;
        }
    }

    return -1;
}

void modem_rx_advance(size_t len) {
    circ_buf_advance_n(&rx_buf, len);
}

char modem_rx_seek(size_t at) {
    return circ_buf_seek(&rx_buf, at);
}

char modem_rx_pop(void) {
    return circ_buf_pop(&rx_buf);
}

void modem_rx_buf_flush(void) {
    return circ_buf_flush(&rx_buf);
}

size_t modem_rx_buf_avail(void) {
    return circ_buf_avail(&rx_buf);
}
