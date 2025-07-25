/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#ifndef _TEMPERATURE_OBJ_H_
#define _TEMPERATURE_OBJ_H_

#include <anj/core.h>
#include <anj/defs.h>

/**
 * @brief Start ADC DMA Conversion for sensor measurements and take first sample
 * as current min and max values
 */
const anj_dm_obj_t *temperature_sensor_init();

/**
 * @brief Updates the sensor value and adjusts min/max tracked values.
 *
 * Measures internal MCU core temperature using builtin
 * temperature sensor channel. Also updates the minimum and maximum
 * recorded values based on the new reading.
 *
 * @param obj Pointer to the Temperature Object.
 */
void temperature_sensor_update(anj_t *anj);

#endif // _TEMPERATURE_OBJ_H_
