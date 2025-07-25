/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#include <anj/compat/log_impl_decls.h>
#include <anj/utils.h>

#include <stm32u3xx_nucleo.h>

void anj_log_handler_output(const char *output, size_t len) {
#ifdef ANJ_LOG_FULL
    // HACK: proper support for colored output per level would require
    // implementing a custom impl header (ANJ_LOG_ALT_IMPL_HEADER), for now
    // let's infer it from the first character of the output
    const char *color = NULL;
    if (len > 0) {
        switch (output[0]) {
        case 'T': {               // L_TRACE
            color = "\033[0;36m"; // cyan
            break;
        }
        case 'D': {               // L_DEBUG
            color = "\033[0;32m"; // green
            break;
        }
        case 'W': {               // L_WARNING
            color = "\033[0;33m"; // yellow
            break;
        }
        case 'E': {               // L_ERROR
            color = "\033[0;31m"; // red
            break;
        }
        default: { // L_INFO and unexpected input
            break; // no color
        }
        }
    }
    if (color) {
        HAL_UART_Transmit(&hcom_uart[COM1], (const uint8_t *) color,
                          sizeof("\033[0;XXm") - 1, COM_POLL_TIMEOUT);
    }
#endif // ANJ_LOG_FULL
    HAL_UART_Transmit(&hcom_uart[COM1], (const uint8_t *) output, len,
                      COM_POLL_TIMEOUT);
#ifdef ANJ_LOG_FULL
    if (color) {
        // reset color
        static const char reset_color[] = "\033[0m";
        HAL_UART_Transmit(&hcom_uart[COM1], (const uint8_t *) reset_color,
                          sizeof(reset_color) - 1, COM_POLL_TIMEOUT);
    }
#endif // ANJ_LOG_FULL
    static const char newline[] = "\r\n";
    HAL_UART_Transmit(&hcom_uart[COM1], (const uint8_t *) newline,
                      sizeof(newline) - 1, COM_POLL_TIMEOUT);
}
