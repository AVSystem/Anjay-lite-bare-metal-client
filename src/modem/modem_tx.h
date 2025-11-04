/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay Lite LwM2M SDK
 * All rights reserved.
 *
 * Licensed under AVSystem Anjay Lite LwM2M Client SDK - Non-Commercial License.
 * See the attached LICENSE file for details.
 */

#ifndef MODEM_TX_H
#define MODEM_TX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int modem_tx_append(const uint8_t *buf, size_t len);
int modem_tx_append_str(const char *str);
int modem_tx_start(void);

#endif // MODEM_TX_H
