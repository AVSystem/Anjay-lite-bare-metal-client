/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#include "stm32u3xx_hal.h"
#include "stm32u3xx_nucleo.h"
#include <stdio.h>

void Error_Handler(void);

void SystemClock_Config(void);

void init_com1(void);
void check_button_state(void);

#define RCC_OSC32_IN_Pin GPIO_PIN_14
#define RCC_OSC32_IN_GPIO_Port GPIOC
#define RCC_OSC32_OUT_Pin GPIO_PIN_15
#define RCC_OSC32_OUT_GPIO_Port GPIOC
#define MODEM_PWR_Pin GPIO_PIN_0
#define MODEM_PWR_GPIO_Port GPIOA
#define DEBUG_JTMS_SWDIO_Pin GPIO_PIN_13
#define DEBUG_JTMS_SWDIO_GPIO_Port GPIOA
#define DEBUG_JTCK_SWCLK_Pin GPIO_PIN_14
#define DEBUG_JTCK_SWCLK_GPIO_Port GPIOA
#define DEBUG_JTDI_Pin GPIO_PIN_15
#define DEBUG_JTDI_GPIO_Port GPIOA
#define DEBUG_JTDO_SWO_Pin GPIO_PIN_3
#define DEBUG_JTDO_SWO_GPIO_Port GPIOB
