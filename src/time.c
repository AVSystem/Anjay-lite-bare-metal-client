/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#include <anj/compat/time.h>

#include "stm32u3xx_hal.h"

// NOTE: consider adding support for NTP sync for real time and to support
// rollover of HAL_GetTick() after about 49 days

uint64_t anj_time_now(void) {
    return HAL_GetTick();
}

uint64_t anj_time_real_now(void) {
    return anj_time_now();
}
