/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <stm32u3xx_hal.h>
#include <usart.h>

#include "circ_buf.h"
#include "modem_constants.h"
#include "modem_tx.h"

static volatile uint8_t tx_buf_storage[MODEM_TX_BUF];
static volatile bool tx_ongoing;
static circ_buf_t tx_buf = {
    .len = sizeof(tx_buf_storage),
    .storage = tx_buf_storage
};

static void tx_char(UART_HandleTypeDef *uart) {
    uint8_t byte = circ_buf_pop(&tx_buf);
    HAL_UART_Transmit_IT(uart, &byte, 1);
}

// NOTE: since project has been generated with USE_HAL_UART_REGISTER_CALLBACKS
// disabled, we can only override the callback that handles all UARTs. In case
// we'd like to support multiple UARTs, the CubeMX-generated code should be
// regenerated.
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &hlpuart1) {
        if (circ_buf_avail(&tx_buf) > 0) {
            tx_char(huart);
        } else {
            tx_ongoing = false;
        }
    }
}

int modem_tx_append_str(const char *str) {
    return modem_tx_append((const uint8_t *) str, strlen(str));
}

int modem_tx_append(const uint8_t *buf, size_t len) {
    if (tx_ongoing) {
        return -1;
    }
    return circ_buf_append(&tx_buf, buf, len);
}

int modem_tx_start(void) {
    if (tx_ongoing) {
        return -1;
    }
    if (circ_buf_avail(&tx_buf) == 0) {
        return 0;
    }

    tx_ongoing = true;
    tx_char(&hlpuart1);

    return 0;
}
