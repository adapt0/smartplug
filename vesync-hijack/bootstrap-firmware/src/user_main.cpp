/////////////////////////////////////////////////////////////////////////////
/** @file
User application entry point

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "task_blink.h"
#include "task_upgrade.h"
#include <esp_common.h>
#include <uart.h>

/// Hardware pin assignments
enum Pins {
    PIN_MOD_LED   = 2,  ///< ESP8266 module LED
};

/////////////////////////////////////////////////////////////////////////////
/// RF calibration sector location. Take from esp8266-rtos-sdk examples
///
/// SDK just reversed 4 sectors, used for rf init data and paramters.
/// We add this function to force users to set rf cal sector, since
/// we don't know which sector is free in user's application.
/// sector map for last several sectors : ABCCC
/// A : rf cal
/// B : rf init data
/// C : sdk parameters
/// @returns rf cal sector
extern "C" uint32 user_rf_cal_sector_set() {
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map) {
    case FLASH_SIZE_4M_MAP_256_256:
        rf_cal_sec = 128 - 5;
        break;

    case FLASH_SIZE_8M_MAP_512_512:
        rf_cal_sec = 256 - 5;
        break;

    case FLASH_SIZE_16M_MAP_512_512:
    case FLASH_SIZE_16M_MAP_1024_1024:
        rf_cal_sec = 512 - 5;
        break;

    case FLASH_SIZE_32M_MAP_512_512:
    case FLASH_SIZE_32M_MAP_1024_1024:
        rf_cal_sec = 1024 - 5;
        break;

    default:
        rf_cal_sec = 0;
        break;
    }

    return rf_cal_sec;
}

/////////////////////////////////////////////////////////////////////////////
/// entry of user application, init user function here
extern "C" void ICACHE_FLASH_ATTR user_init() {
    // configure UART
    UART_ConfigTypeDef  uart_config;
    uart_config.baud_rate         = BIT_RATE_115200;
    uart_config.data_bits         = UART_WordLength_8b;
    uart_config.parity            = USART_Parity_None;
    uart_config.stop_bits         = USART_StopBits_1;
    uart_config.flow_ctrl         = USART_HardwareFlowControl_None;
    uart_config.UART_RxFlowThresh = 120;
    uart_config.UART_InverseMask  = UART_None_Inverse;
    UART_ParamConfig(UART0, &uart_config);

    printf("SDK version:%s\n", system_get_sdk_version());

    TaskBase::create<TaskBlink>("blink", PIN_MOD_LED);
    TaskBase::create<TaskUpgrade>("upgrade");
}
