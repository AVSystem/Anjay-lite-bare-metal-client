# Copyright 2025 AVSystem <avsystem@avsystem.com>
# AVSystem Anjay LwM2M SDK
# ALL RIGHTS RESERVED
cmake_minimum_required(VERSION 3.22)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME anjay-lite-bare-metal-client)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: ${CMAKE_BUILD_TYPE}")

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms
	USE_NUCLEO_64
	USE_HAL_DRIVER
	STM32U385xx
    $<$<CONFIG:Debug>:DEBUG>
)

# STM32CubeMX generated include paths
set(MX_Include_Dirs
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Inc
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/deps/ST/BSP/STM32U3xx_Nucleo
    ${CMAKE_SOURCE_DIR}/deps/ST/CMSIS/Device/ST/STM32U3xx/Include
    ${CMAKE_SOURCE_DIR}/deps/ST/CMSIS/Include
)

# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Src/gpio.c
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Src/platform.c
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Src/adc.c
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Src/gpdma.c
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Src/usart.c
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Src/stm32u3xx_it.c
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Src/stm32u3xx_hal_msp.c
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Src/sysmem.c
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Src/syscalls.c
    ${CMAKE_SOURCE_DIR}/deps/ST/startup_stm32u385xx.s
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_SOURCE_DIR}/deps/ST/Core/Src/system_stm32u3xx.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_cortex.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_rcc.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_rcc_ex.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_flash.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_flash_ex.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_gpio.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_adc.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_adc_ex.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_dma.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_dma_ex.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_exti.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_pwr.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_pwr_ex.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_gtzc.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_icache.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_tim.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_tim_ex.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_usart.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_usart_ex.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_uart_ex.c
    ${CMAKE_SOURCE_DIR}/deps/ST/STM32U3xx_HAL_Driver/Src/stm32u3xx_hal_uart.c
    ${CMAKE_SOURCE_DIR}/deps/ST/BSP/STM32U3xx_Nucleo/stm32u3xx_nucleo.c
)

# Interface library for includes and symbols
add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)

# anjay-lite library
file(
    GLOB_RECURSE
    ANJAY_LITE_SOURCES
    "${CMAKE_SOURCE_DIR}/deps/anjay-lite/src/*.c"
)
add_library(anjay_lite ${ANJAY_LITE_SOURCES})

target_include_directories(
    anjay_lite PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/anjay-lite/include_public
    ${CMAKE_SOURCE_DIR}/config
)

target_compile_options(anjay_lite PRIVATE
    -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.
)

# Main executable
file(
    GLOB_RECURSE
    APPLICATION_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.c"
)
add_executable(${CMAKE_PROJECT_NAME} ${APPLICATION_SOURCES})

# Add STM32CubeMX generated application sources to the main executable
target_sources(
    ${CMAKE_PROJECT_NAME} PRIVATE
    ${MX_Application_Src}
)

target_link_libraries(
    ${CMAKE_PROJECT_NAME} PRIVATE
    STM32_Drivers
    stm32cubemx
    anjay_lite
)

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES
    ADDITIONAL_CLEAN_FILES
    ${CMAKE_PROJECT_NAME}.map
)

set(
    CONFIG_APN
    "internet"
    CACHE STRING
    "APN name to use in AT+CGDCONT command"
)
set(
    CONFIG_ENDPOINT_NAME
    "anjay-lite-bare-metal-client"
    CACHE STRING
    "LwM2M endpoint name"
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    CONFIG_APN="${CONFIG_APN}"
    CONFIG_ENDPOINT_NAME="${CONFIG_ENDPOINT_NAME}"
)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Winit-self
    -Wsign-conversion
    -Wcast-qual
    -Wvla
    -Wno-variadic-macros
    -Wno-long-long
    -Wshadow
    -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.
)
