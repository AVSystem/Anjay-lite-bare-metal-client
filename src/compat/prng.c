/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay Lite LwM2M SDK
 * All rights reserved.
 *
 * Licensed under AVSystem Anjay Lite LwM2M Client SDK - Non-Commercial License.
 * See the attached LICENSE file for details.
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <anj/compat/rng.h>
#include <anj/compat/time.h>
#include <anj/log.h>
#include <anj/utils.h>

#include <rng.h>

int anj_rng_generate(uint8_t *buffer, size_t size) {
    uint32_t random_number;
    for (size_t i = 0; i < size / sizeof(random_number); i++) {
        if (HAL_RNG_GenerateRandomNumber(&hrng, &random_number) != HAL_OK) {
            return -1;
        }
        memcpy(buffer, &random_number, sizeof(random_number));
        buffer += sizeof(random_number);
    }

    size_t last_chunk_size = size % sizeof(random_number);
    if (last_chunk_size) {
        if (HAL_RNG_GenerateRandomNumber(&hrng, &random_number) != HAL_OK) {
            return -1;
        }
        memcpy(buffer, &random_number, last_chunk_size);
    }

    return 0;
}
