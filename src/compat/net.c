/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay Lite LwM2M SDK
 * All rights reserved.
 *
 * Licensed under AVSystem Anjay Lite LwM2M Client SDK - Non-Commercial License.
 * See the attached LICENSE file for details.
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <anj/compat/net/anj_net_api.h>
#include <anj/log.h>
#include <anj/utils.h>

#include "modem/modem.h"
#include "modem/modem_constants.h"

typedef enum {
    CURRENT_OP_NONE,
    CURRENT_OP_CONNECT,
    CURRENT_OP_SEND,
    CURRENT_OP_SHUTDOWN
} current_op_t;

typedef union {
    modem_socket_open_ctx_t open;
    modem_socket_send_ctx_t send;
} op_ctx_t;

typedef struct {
    bool used;
    anj_net_socket_state_t state;
    current_op_t current_op;
    op_ctx_t op_ctx;
} net_ctx_t;

// Current implementation limitations:
// - only one socket supported at a time, support for multiple sockets would
//   require:
//   - context storage as array
//   - any "locking" mechanism on modem communication layer to ensure that
//     multiple sockets do not interfere with each other
//   - a different way of handling incoming data, e.g. by using "buffer access"
//     mode URCs instead of "direct push" mode (per BG96 TCP/IP AT Commands
//     Manual)
// - only UDP sockets supported
// - only IPv4 sockets supported
static net_ctx_t ctx_storage;

static int net_create_ctx(net_ctx_t **ctx, const anj_net_config_t *config) {
    if (ctx_storage.used) {
        return -1;
    }
    // we support IPv4 only
    if (config->raw_socket_config.af_setting
            == ANJ_NET_AF_SETTING_FORCE_INET6) {
        return ANJ_NET_ENOTSUP;
    }
    ctx_storage.used = true;
    ctx_storage.state = ANJ_NET_SOCKET_STATE_CLOSED;
    ctx_storage.current_op = CURRENT_OP_NONE;
    *ctx = &ctx_storage;
    return 0;
}

static int
net_connect(net_ctx_t *ctx, const char *hostname, const char *port_str) {
    switch (ctx->current_op) {
    case CURRENT_OP_NONE: {
        int res = modem_socket_open_init(&ctx->op_ctx.open, hostname, port_str);
        if (res) {
            return -1;
        }
        ctx->current_op = CURRENT_OP_CONNECT;
        return ANJ_NET_EINPROGRESS;
    }
    case CURRENT_OP_CONNECT: {
        int res = modem_socket_open_continue(&ctx->op_ctx.open);
        if (res > 0) {
            return ANJ_NET_EINPROGRESS;
        }
        ctx->current_op = CURRENT_OP_NONE;
        if (res < 0) {
            return -1;
        }
        ctx->state = ANJ_NET_SOCKET_STATE_CONNECTED;
        return 0;
    }
    default: { return -1; }
    }
}

static int net_send(net_ctx_t *ctx,
                    size_t *bytes_sent,
                    const uint8_t *buf,
                    size_t length) {
    if (ctx->state != ANJ_NET_SOCKET_STATE_CONNECTED) {
        return -1;
    }
    // NOTE: change this when adding support for TCP
    // NOTE: technically, there's no limit for maximum length of TCP message
    // that Anjay Lite can send, but the modem still has the limit; in such case
    // the message should be sent in multiple parts
    bool is_udp = true;
    if (is_udp && length > MODEM_SOCKET_SEND_MAX) {
        // modem cannot send messages longer than 1460 bytes in one go
        return ANJ_NET_EMSGSIZE;
    }
    switch (ctx->current_op) {
    case CURRENT_OP_NONE: {
        int res = modem_socket_send_init(&ctx->op_ctx.send, length);
        if (res) {
            return -1;
        }
        ctx->current_op = CURRENT_OP_SEND;
        return ANJ_NET_EINPROGRESS;
    }
    case CURRENT_OP_SEND: {
        int res = modem_socket_send_continue(&ctx->op_ctx.send, length, buf);
        if (res > 0) {
            return ANJ_NET_EINPROGRESS;
        }
        ctx->current_op = CURRENT_OP_NONE;
        if (res < 0) {
            return -1;
        }
        *bytes_sent = length;
        return 0;
    }
    default: { return -1; }
    }
}

static int
net_recv(net_ctx_t *ctx, size_t *bytes_received, uint8_t *buf, size_t length) {
    if (ctx->state != ANJ_NET_SOCKET_STATE_CONNECTED
            && ctx->state != ANJ_NET_SOCKET_STATE_SHUTDOWN) {
        return -1;
    }
    if (ctx->current_op != CURRENT_OP_NONE) {
        // NOTE: check whether that should be EAGAIN or some fatal -1
        return ANJ_NET_EAGAIN;
    }
    int res = modem_socket_try_recv(buf, length, bytes_received);
    if (res > 0) {
        return ANJ_NET_EAGAIN;
    }
    if (res < 0) {
        return -1;
    }
    // NOTE: change this when adding support for TCP
    bool is_udp = true;
    if (*bytes_received == length && is_udp) {
        // if the whole buffer is filled, we don't know if if the buffer was
        // filled completely, the message might be truncated
        return ANJ_NET_EMSGSIZE;
    }
    return 0;
}

// Note: since there's no differentiation between shutdown and close operations
// on BG96, do the defacto close operation here.
static int net_shutdown(net_ctx_t *ctx) {
    switch (ctx->current_op) {
    case CURRENT_OP_SHUTDOWN: {
        int res = modem_socket_close_continue();
        if (res > 0) {
            return ANJ_NET_EINPROGRESS;
        }
        ctx->state = ANJ_NET_SOCKET_STATE_SHUTDOWN;
        ctx->current_op = CURRENT_OP_NONE;
        if (res < 0) {
            return -1;
        }
        return 0;
    }
    default: {
        int res = modem_socket_close_init();
        if (res) {
            return -1;
        }
        ctx->current_op = CURRENT_OP_SHUTDOWN;
        return ANJ_NET_EINPROGRESS;
    }
    }
    return 0;
}

static int net_close(net_ctx_t *ctx) {
    if (ctx->state == ANJ_NET_SOCKET_STATE_CLOSED) {
        return 0; // already closed
    }
    if (ctx->state == ANJ_NET_SOCKET_STATE_SHUTDOWN) {
        ctx->state = ANJ_NET_SOCKET_STATE_CLOSED;
        return 0;
    }
    // Caller might have not called shutdown before, so do it here.
    return net_shutdown(ctx);
}

static int net_get_inner_mtu(net_ctx_t *ctx, int32_t *out_value) {
    // This is not necessarily the MTU of the underlying network, but its the
    // maximum message size that the modem can handle in a single AT+QISEND
    // command.
    //
    // "<send_length> Integer type. The length of data to be sent, which cannot
    // exceed 1460 bytes."
    (void) ctx;
    *out_value = 1460;
    return 0;
}

static int net_get_state(net_ctx_t *ctx, anj_net_socket_state_t *out_value) {
    *out_value = ctx->state;
    return 0;
}

static int net_reuse_last_port(net_ctx_t *ctx) {
    (void) ctx;
    return ANJ_NET_ENOTSUP; // not implemented
}

static int net_cleanup_ctx(net_ctx_t **ctx) {
    int close_result = 0;
    if ((*ctx)->state != ANJ_NET_SOCKET_STATE_CLOSED) {
        close_result = net_close(*ctx);
        if (close_result == ANJ_NET_EINPROGRESS) {
            return close_result;
        }
    }
    (*ctx)->used = false;
    *ctx = NULL;
    return close_result;
}

static int net_queue_mode_rx_off(anj_net_ctx_t *ctx) {
    (void) ctx;
    return ANJ_NET_OK;
}

#ifdef ANJ_NET_WITH_UDP
int anj_udp_create_ctx(anj_net_ctx_t **ctx, const anj_net_config_t *config) {
    return net_create_ctx((net_ctx_t **) ctx, config);
}

int anj_udp_cleanup_ctx(anj_net_ctx_t **ctx) {
    return net_cleanup_ctx((net_ctx_t **) ctx);
}

int anj_udp_connect(anj_net_ctx_t *ctx,
                    const char *hostname,
                    const char *port) {
    return net_connect((net_ctx_t *) ctx, hostname, port);
}

int anj_udp_send(anj_net_ctx_t *ctx,
                 size_t *bytes_sent,
                 const uint8_t *buf,
                 size_t length) {
    return net_send((net_ctx_t *) ctx, bytes_sent, buf, length);
}

int anj_udp_recv(anj_net_ctx_t *ctx,
                 size_t *bytes_received,
                 uint8_t *buf,
                 size_t length) {
    return net_recv((net_ctx_t *) ctx, bytes_received, buf, length);
}

int anj_udp_shutdown(anj_net_ctx_t *ctx) {
    return net_shutdown((net_ctx_t *) ctx);
}

int anj_udp_close(anj_net_ctx_t *ctx) {
    return net_close((net_ctx_t *) ctx);
}

int anj_udp_get_state(anj_net_ctx_t *ctx, anj_net_socket_state_t *out_value) {
    return net_get_state((net_ctx_t *) ctx, out_value);
}

int anj_udp_get_inner_mtu(anj_net_ctx_t *ctx, int32_t *out_value) {
    return net_get_inner_mtu((net_ctx_t *) ctx, out_value);
}

int anj_udp_reuse_last_port(anj_net_ctx_t *ctx) {
    return net_reuse_last_port((net_ctx_t *) ctx);
}

int anj_udp_queue_mode_rx_off(anj_net_ctx_t *ctx) {
    return net_queue_mode_rx_off(ctx);
}
#endif // ANJ_NET_WITH_UDP
