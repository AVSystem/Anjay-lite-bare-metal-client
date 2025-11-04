/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay Lite LwM2M SDK
 * All rights reserved.
 *
 * Licensed under AVSystem Anjay Lite LwM2M Client SDK - Non-Commercial License.
 * See the attached LICENSE file for details.
 */

#include <anj/compat/time.h>

#include "stm32u3xx_hal.h"

// NOTE: consider adding support for NTP sync for real time and to support
// rollover of HAL_GetTick() after about 49 days

anj_time_monotonic_t anj_time_monotonic_now(void) {
    return anj_time_monotonic_new(HAL_GetTick(), ANJ_TIME_UNIT_MS);
}

anj_time_real_t anj_time_real_now(void) {
    return anj_time_real_new(HAL_GetTick(), ANJ_TIME_UNIT_MS);
}
