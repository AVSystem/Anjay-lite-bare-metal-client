/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <anj/core.h>
#include <anj/dm/device_object.h>
#include <anj/dm/security_object.h>
#include <anj/dm/server_object.h>
#include <anj/log/log.h>

#include <adc.h>
#include <gpdma.h>
#include <gpio.h>
#include <platform.h>
#include <stm32u3xx_hal.h>
#include <stm32u3xx_nucleo.h>
#include <usart.h>

#include "modem/modem.h"

#include "temperature_obj.h"

#define app_log(...) anj_log(app, __VA_ARGS__)

#ifndef CONFIG_ENDPOINT_NAME
#    define CONFIG_ENDPOINT_NAME "anjay-lite-bare-metal-client"
#endif // CONFIG_ENDPOINT_NAME

static int install_security_obj(anj_t *anj,
                                anj_dm_security_obj_t *security_obj) {
    anj_dm_security_instance_init_t security_inst = {
        .ssid = 1,
        .server_uri = "coap://eu.iot.avsystem.cloud:5683",
        .security_mode = ANJ_DM_SECURITY_NOSEC
    };
    anj_dm_security_obj_init(security_obj);
    if (anj_dm_security_obj_add_instance(security_obj, &security_inst)
            || anj_dm_security_obj_install(anj, security_obj)) {
        return -1;
    }
    return 0;
}

static int install_server_obj(anj_t *anj, anj_dm_server_obj_t *server_obj) {
    anj_dm_server_instance_init_t server_inst = {
        .ssid = 1,
        .lifetime = 50,
        .binding = "U",
        .bootstrap_on_registration_failure = &(bool) { false },
    };
    anj_dm_server_obj_init(server_obj);
    if (anj_dm_server_obj_add_instance(server_obj, &server_inst)
            || anj_dm_server_obj_install(anj, server_obj)) {
        return -1;
    }
    return 0;
}

static int install_device_obj(anj_t *anj, anj_dm_device_obj_t *device_obj) {
    anj_dm_device_object_init_t device_obj_conf = {
        .firmware_version = "0.1"
    };
    return anj_dm_device_obj_install(anj, device_obj, &device_obj_conf);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_GPDMA1_Init();
    MX_ADC1_Init();
    MX_LPUART1_UART_Init();

    /* Init BSP */
    BSP_LED_Init(LED_GREEN);
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
    init_com1();

    BSP_LED_On(LED_GREEN);

    // wait 200ms to ensure that all inits have finished
    HAL_Delay(500);

    app_log(L_DEBUG, "RCC reset source = %" PRIu32, HAL_RCC_GetResetSource());
    app_log(L_INFO, "Application startup...");

    // reset modem
    // power off
    HAL_GPIO_WritePin(MODEM_PWR_GPIO_Port, MODEM_PWR_Pin, GPIO_PIN_RESET);
    app_log(L_DEBUG, "Powering modem off...");
    HAL_Delay(1500);

    // power on
    HAL_GPIO_WritePin(MODEM_PWR_GPIO_Port, MODEM_PWR_Pin, GPIO_PIN_SET);
    app_log(L_DEBUG, "Powering modem on...");
    // NOTE: consider a better method of waiting for the modem to be ready
    HAL_Delay(9000);

    if (modem_bringup()) {
        app_log(L_ERROR,
                "Failed to bring up the modem, application will restart...");
        // HACK: sometimes modems fail to login to network
        // AT+QIACT doesn't pass and returns with error
        NVIC_SystemReset();
    }
    app_log(L_DEBUG, "Modem bringup successful!");

    anj_t anj;
    anj_dm_security_obj_t security_obj;
    anj_dm_server_obj_t server_obj;
    anj_dm_device_obj_t device_obj;
    {
        anj_configuration_t config = {
            .endpoint_name = CONFIG_ENDPOINT_NAME
        };

        app_log(L_INFO, "Initializing Anjay Lite, endpoint name: %s",
                config.endpoint_name);

        if (anj_core_init(&anj, &config)) {
            app_log(L_ERROR, "Failed to initialize Anjay Lite");
            return -1;
        }

        if (install_security_obj(&anj, &security_obj)
                || install_server_obj(&anj, &server_obj)
                || install_device_obj(&anj, &device_obj)) {
            app_log(L_ERROR, "Failed to install mandatory objects");
            return -1;
        }

        if (anj_dm_add_obj(&anj, temperature_sensor_init())) {
            app_log(L_ERROR, "Failed to install temperature object error");
            return -1;
        }
    }
    app_log(L_INFO, "Anjay Lite initialized");

    while (1) {
        anj_core_step(&anj);
        HAL_Delay(10);
        check_button_state();
        temperature_sensor_update(&anj);
    }
}
