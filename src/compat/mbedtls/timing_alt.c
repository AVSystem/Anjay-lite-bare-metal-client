/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay Lite LwM2M SDK
 * All rights reserved.
 *
 * Licensed under AVSystem Anjay Lite LwM2M Client SDK - Non-Commercial License.
 * See the attached LICENSE file for details.
 */

/*
 * This functions are mostly copied from Mbed TLS librars
 */

#include <anj/compat/time.h>

#include "mbedtls/timing.h"
#include "timing_alt.h"

/* This function is an external definition of function used inside MbedTLS
 * library.
 */

// to avoid the implicit declaration warning
unsigned long mbedtls_timing_hardclock(void);
unsigned long mbedtls_timing_hardclock(void) {
    return anj_time_monotonic_to_scalar(anj_time_monotonic_now(),
                                        ANJ_TIME_UNIT_US);
}

unsigned long mbedtls_timing_get_timer(struct mbedtls_timing_hr_time *val,
                                       int reset) {
    if (reset) {
        val->time_ms =
                anj_time_real_to_scalar(anj_time_real_now(), ANJ_TIME_UNIT_MS);
        return 0;
    } else {
        unsigned long delta;
        anj_time_real_t now = anj_time_real_now();
        anj_time_real_t val_now =
                anj_time_real_new(val->time_ms, ANJ_TIME_UNIT_MS);
        anj_time_duration_t diff = anj_time_real_diff(now, val_now);

        delta = anj_time_duration_to_scalar(diff, ANJ_TIME_UNIT_MS);
        return delta;
    }
}

void mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms) {
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;

    ctx->int_ms = int_ms;
    ctx->fin_ms = fin_ms;

    if (fin_ms) {
        (void) mbedtls_timing_get_timer(&ctx->timer, 1);
    }
}

int mbedtls_timing_get_delay(void *data) {
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;
    unsigned long elapsed_ms;

    if (ctx->fin_ms == 0) {
        return -1;
    }

    elapsed_ms = mbedtls_timing_get_timer(&ctx->timer, 0);

    if (elapsed_ms >= ctx->fin_ms) {
        return 2;
    }

    if (elapsed_ms >= ctx->int_ms) {
        return 1;
    }

    return 0;
}

uint32_t
mbedtls_timing_get_final_delay(const mbedtls_timing_delay_context *data) {
    return data->fin_ms;
}
