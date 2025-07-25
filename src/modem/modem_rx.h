/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#ifndef MODEM_RX_H
#define MODEM_RX_H

#include <stdbool.h>
#include <stddef.h>

int modem_rx_start(void);
bool modem_rx_pop_newlines(void);
int modem_rx_seek_line_length(size_t *out_line_len);
void modem_rx_advance(size_t len);
char modem_rx_seek(size_t at);
char modem_rx_pop(void);
void modem_rx_buf_flush(void);
size_t modem_rx_buf_avail(void);

#endif // MODEM_RX_H
