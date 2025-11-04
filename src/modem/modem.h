/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay Lite LwM2M SDK
 * All rights reserved.
 *
 * Licensed under AVSystem Anjay Lite LwM2M Client SDK - Non-Commercial License.
 * See the attached LICENSE file for details.
 */

#ifndef MODEM_ASYNC_H
#define MODEM_ASYNC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    int step;
} modem_socket_open_ctx_t;

typedef struct {
    int step;
} modem_socket_send_ctx_t;

int modem_bringup(void);
int modem_send_command(const char *command);

int modem_socket_open_init(modem_socket_open_ctx_t *ctx,
                           const char *hostname,
                           const char *port);
int modem_socket_open_continue(modem_socket_open_ctx_t *ctx);

int modem_socket_try_recv(uint8_t *buf, size_t buf_len, size_t *out_msg_len);

int modem_socket_send_init(modem_socket_send_ctx_t *ctx, size_t len);
int modem_socket_send_continue(modem_socket_send_ctx_t *ctx,
                               size_t len,
                               const uint8_t *buf);

int modem_socket_close_init(void);
int modem_socket_close_continue(void);

#endif // MODEM_ASYNC_H
