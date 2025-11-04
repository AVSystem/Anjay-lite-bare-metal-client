/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay Lite LwM2M SDK
 * All rights reserved.
 *
 * Licensed under AVSystem Anjay Lite LwM2M Client SDK - Non-Commercial License.
 * See the attached LICENSE file for details.
 */

#define _DEFAULT_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <anj/core.h>
#include <anj/defs.h>
#include <anj/dm/core.h>
#include <anj/utils.h>

#include <adc.h>
#include <stm32u3xx_ll_adc.h>

#include "temperature_obj.h"

#define TEMPERATURE_OID 3303
#define TEMPERATURE_RESOURCES_COUNT 6

enum {
    RID_MIN_MEASURED_VALUE = 5601,
    RID_MAX_MEASURED_VALUE = 5602,
    RID_RESET_MIN_MAX_MEASURED_VALUES = 5605,
    RID_SENSOR_VALUE = 5700,
    RID_SENSOR_UNIT = 5701,
    RID_APPLICATION_TYPE = 5750,
};

enum {
    RID_MIN_MEASURED_VALUE_IDX = 0,
    RID_MAX_MEASURED_VALUE_IDX,
    RID_RESET_MIN_MAX_MEASURED_VALUES_IDX,
    RID_SENSOR_VALUE_IDX,
    RID_SENSOR_UNIT_IDX,
    RID_APPLICATION_TYPE_IDX,
    _RID_LAST
};

ANJ_STATIC_ASSERT(_RID_LAST == TEMPERATURE_RESOURCES_COUNT,
                  temperature_resource_count_mismatch);

static const anj_dm_res_t RES[TEMPERATURE_RESOURCES_COUNT] = {
    [RID_MIN_MEASURED_VALUE_IDX] = {
        .rid = RID_MIN_MEASURED_VALUE,
        .type = ANJ_DATA_TYPE_DOUBLE,
        .kind = ANJ_DM_RES_R
    },
    [RID_MAX_MEASURED_VALUE_IDX] = {
        .rid = RID_MAX_MEASURED_VALUE,
        .type = ANJ_DATA_TYPE_DOUBLE,
        .kind = ANJ_DM_RES_R
    },
    [RID_RESET_MIN_MAX_MEASURED_VALUES_IDX] = {
        .rid = RID_RESET_MIN_MAX_MEASURED_VALUES,
        .kind = ANJ_DM_RES_E
    },
    [RID_SENSOR_VALUE_IDX] = {
        .rid = RID_SENSOR_VALUE,
        .type = ANJ_DATA_TYPE_DOUBLE,
        .kind = ANJ_DM_RES_R
    },
    [RID_SENSOR_UNIT_IDX] = {
        .rid = RID_SENSOR_UNIT,
        .type = ANJ_DATA_TYPE_STRING,
        .kind = ANJ_DM_RES_R
    },
    [RID_APPLICATION_TYPE_IDX] = {
        .rid = RID_APPLICATION_TYPE,
        .type = ANJ_DATA_TYPE_STRING,
        .kind = ANJ_DM_RES_RW
    }
};

#define TEMP_OBJ_SENSOR_UNITS_VAL "C"
#define TEMP_OBJ_APPL_TYPE_MAX_SIZE 32

typedef struct {
    double sensor_value;
    double min_sensor_value;
    double max_sensor_value;
    char application_type[TEMP_OBJ_APPL_TYPE_MAX_SIZE];
    char application_type_cached[TEMP_OBJ_APPL_TYPE_MAX_SIZE];
} temp_obj_ctx_t;

static inline temp_obj_ctx_t *get_ctx(void);
static float read_current_temperature(void);

void temperature_sensor_update(anj_t *anj) {
    temp_obj_ctx_t *ctx = get_ctx();

    double prev_temp_value = ctx->sensor_value;
    ctx->sensor_value = read_current_temperature();

    if (prev_temp_value != ctx->sensor_value) {
        anj_core_data_model_changed(anj,
                                    &ANJ_MAKE_RESOURCE_PATH(TEMPERATURE_OID, 0,
                                                            RID_SENSOR_VALUE),
                                    ANJ_CORE_CHANGE_TYPE_VALUE_CHANGED);
    }
    if (ctx->sensor_value < ctx->min_sensor_value) {
        ctx->min_sensor_value = ctx->sensor_value;
        anj_core_data_model_changed(
                anj,
                &ANJ_MAKE_RESOURCE_PATH(TEMPERATURE_OID, 0,
                                        RID_MIN_MEASURED_VALUE),
                ANJ_CORE_CHANGE_TYPE_VALUE_CHANGED);
    }
    if (ctx->sensor_value > ctx->max_sensor_value) {
        ctx->max_sensor_value = ctx->sensor_value;
        anj_core_data_model_changed(
                anj,
                &ANJ_MAKE_RESOURCE_PATH(TEMPERATURE_OID, 0,
                                        RID_MAX_MEASURED_VALUE),
                ANJ_CORE_CHANGE_TYPE_VALUE_CHANGED);
    }
}

static int res_read(anj_t *anj,
                    const anj_dm_obj_t *obj,
                    anj_iid_t iid,
                    anj_rid_t rid,
                    anj_riid_t riid,
                    anj_res_value_t *out_value) {
    (void) anj;
    (void) obj;
    (void) iid;
    (void) riid;

    temp_obj_ctx_t *temp_obj_ctx = get_ctx();

    switch (rid) {
    case RID_SENSOR_VALUE:
        out_value->double_value = temp_obj_ctx->sensor_value;
        break;
    case RID_MIN_MEASURED_VALUE:
        out_value->double_value = temp_obj_ctx->min_sensor_value;
        break;
    case RID_MAX_MEASURED_VALUE:
        out_value->double_value = temp_obj_ctx->max_sensor_value;
        break;
    case RID_SENSOR_UNIT:
        out_value->bytes_or_string.data = TEMP_OBJ_SENSOR_UNITS_VAL;
        break;
    case RID_APPLICATION_TYPE:
        out_value->bytes_or_string.data = temp_obj_ctx->application_type;
        break;
    default:
        return ANJ_DM_ERR_NOT_FOUND;
    }
    return 0;
}

static int res_write(anj_t *anj,
                     const anj_dm_obj_t *obj,
                     anj_iid_t iid,
                     anj_rid_t rid,
                     anj_riid_t riid,
                     const anj_res_value_t *value) {
    (void) anj;
    (void) obj;
    (void) iid;
    (void) riid;

    temp_obj_ctx_t *temp_obj_ctx = get_ctx();

    switch (rid) {
    case RID_APPLICATION_TYPE:
        return anj_dm_write_string_chunked(value,
                                           temp_obj_ctx->application_type,
                                           TEMP_OBJ_APPL_TYPE_MAX_SIZE, NULL);
    default:
        return ANJ_DM_ERR_NOT_FOUND;
    }
    return 0;
}

static int res_execute(anj_t *anj,
                       const anj_dm_obj_t *obj,
                       anj_iid_t iid,
                       anj_rid_t rid,
                       const char *execute_arg,
                       size_t execute_arg_len) {
    (void) anj;
    (void) obj;
    (void) iid;
    (void) execute_arg;
    (void) execute_arg_len;

    temp_obj_ctx_t *temp_obj_ctx = get_ctx();

    switch (rid) {
    case RID_RESET_MIN_MAX_MEASURED_VALUES: {
        temp_obj_ctx->min_sensor_value = temp_obj_ctx->sensor_value;
        temp_obj_ctx->max_sensor_value = temp_obj_ctx->sensor_value;

        anj_core_data_model_changed(
                anj,
                &ANJ_MAKE_RESOURCE_PATH(TEMPERATURE_OID, 0,
                                        RID_MIN_MEASURED_VALUE),
                ANJ_CORE_CHANGE_TYPE_VALUE_CHANGED);
        anj_core_data_model_changed(
                anj,
                &ANJ_MAKE_RESOURCE_PATH(TEMPERATURE_OID, 0,
                                        RID_MAX_MEASURED_VALUE),
                ANJ_CORE_CHANGE_TYPE_VALUE_CHANGED);
        break;
    }
    default:
        return ANJ_DM_ERR_NOT_FOUND;
    }

    return 0;
}

static int transaction_begin(anj_t *anj, const anj_dm_obj_t *obj) {
    (void) anj;
    (void) obj;
    temp_obj_ctx_t *temp_obj_ctx = get_ctx();
    memcpy(temp_obj_ctx->application_type_cached,
           temp_obj_ctx->application_type, TEMP_OBJ_APPL_TYPE_MAX_SIZE);
    return 0;
}

static void transaction_end(anj_t *anj,
                            const anj_dm_obj_t *obj,
                            anj_dm_transaction_result_t result) {
    (void) anj;
    (void) obj;
    temp_obj_ctx_t *temp_obj_ctx = get_ctx();
    if (result) {
        // Restore cached data
        memcpy(temp_obj_ctx->application_type,
               temp_obj_ctx->application_type_cached,
               TEMP_OBJ_APPL_TYPE_MAX_SIZE);
    }
}

static const anj_dm_handlers_t TEMP_OBJ_HANDLERS = {
    .res_read = res_read,
    .res_write = res_write,
    .res_execute = res_execute,
    .transaction_begin = transaction_begin,
    .transaction_end = transaction_end,
};

static const anj_dm_obj_inst_t INST = {
    .iid = 0,
    .res_count = TEMPERATURE_RESOURCES_COUNT,
    .resources = RES
};

static const anj_dm_obj_t OBJ = {
    .oid = TEMPERATURE_OID,
    .version = "1.1",
    .insts = &INST,
    .handlers = &TEMP_OBJ_HANDLERS,
    .max_inst_count = 1
};

static temp_obj_ctx_t temperature_ctx = {
    .application_type = "Sensor_1",
};

/**
 * Buffer for ADC DMA
 *
 * We have ADC configured in continuous conversion mode with automatic DMA
 * requests There are 2 channels configured:
 *  - Vref - internal adc reference voltage measurement (Rank 1)
 *  - VSENSE - internal cortex temperature sensor (Rank 2)
 *
 * we dont care about precise measurement, so we don't do any kind of software
 * aggregating on data so we store just latests sample
 *
 * adc_dma_buff[0] - vref internal channel
 * adc_dma_buff[1] - temperature sensor channel
 *
 * in case you increase sample size buffer will contain alternating values:
 * even indexes - vref internal channel
 * odd indexes - temperature sensor channel
 */

#define ADC_BUFF_SAMPLES 1
#define ADC_CHANNELS_COUNT 2
static uint16_t adc_dma_buff[ADC_CHANNELS_COUNT * ADC_BUFF_SAMPLES];

const anj_dm_obj_t *temperature_sensor_init(void) {
    /**
     * calibrate adc and start DMA requests
     */
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *) adc_dma_buff,
                      sizeof(adc_dma_buff) / sizeof(uint16_t));

    // wait 100ms to acquire current temperature
    HAL_Delay(100);

    temperature_ctx.sensor_value = read_current_temperature();
    temperature_ctx.min_sensor_value = temperature_ctx.sensor_value;
    temperature_ctx.max_sensor_value = temperature_ctx.sensor_value;

    return &OBJ;
}

/**
 * Factory calibration of internal temperature sensor
 */
#define CAL1_TEMP TEMPSENSOR_CAL1_TEMP
#define CAL2_TEMP TEMPSENSOR_CAL2_TEMP

#define CAL1_VALUE (*TEMPSENSOR_CAL1_ADDR)
#define CAL2_VALUE (*TEMPSENSOR_CAL2_ADDR)

#define CAL_VREF TEMPSENSOR_CAL_VREF

static float read_current_temperature(void) {
    /**
     * copy latest raw adc measurements
     */
    uint32_t vref_raw = adc_dma_buff[0];
    int32_t temp_raw = adc_dma_buff[1];

    /**
     * Factory calibration was based on 3.0V Vref+,
     * so we need to take current vref into consideration
     */
    // calculate analog reference voltage from raw measurement
    uint32_t vdda =
            __HAL_ADC_CALC_VREFANALOG_VOLTAGE(vref_raw, ADC_RESOLUTION12b);
    // calculate temperature sensor voltage in reference to current analog
    // voltage (mV)
    int32_t temp_measurment_mV =
            __HAL_ADC_CALC_DATA_TO_VOLTAGE(vdda, temp_raw, ADC_RESOLUTION12b);

    // Calibration values were potentially measured with different vref than
    // ours current so we need to apply scaling
    uint32_t cal1_scaled = CAL1_VALUE * CAL_VREF / vdda;
    uint32_t cal2_scaled = CAL2_VALUE * CAL_VREF / vdda;

    // convert calibration values into voltages (mV)
    float cal1_val_mV = __HAL_ADC_CALC_DATA_TO_VOLTAGE(
            (int32_t) vdda, (int32_t) cal1_scaled, ADC_RESOLUTION12b);
    float cal2_val_mV = __HAL_ADC_CALC_DATA_TO_VOLTAGE(
            (int32_t) vdda, (int32_t) cal2_scaled, ADC_RESOLUTION12b);

    // calculate coefficient for temperature conversion from voltage to
    // temperature
    float prescaler =
            ((float) (CAL2_TEMP - CAL1_TEMP)) / (cal2_val_mV - cal1_val_mV);

    // convert measured temperature voltage into temperature
    return prescaler * (temp_measurment_mV - cal1_val_mV) + CAL1_TEMP;
}

static inline temp_obj_ctx_t *get_ctx(void) {
    return &temperature_ctx;
}
