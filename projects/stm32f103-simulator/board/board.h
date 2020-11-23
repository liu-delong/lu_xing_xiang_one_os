/*
 * Copyright (c) 2012-2020, CMCC IOT
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <stm32f1xx_hal.h>
#include <drv_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STM32_FLASH_START_ADRESS       ((uint32_t)0x08000000)
#define STM32_FLASH_SIZE               (256 * 2048)
#define STM32_FLASH_END_ADDRESS        ((uint32_t)(STM32_FLASH_START_ADRESS + STM32_FLASH_SIZE))

#define STM32_SRAM1_SIZE               (16)
#define STM32_SRAM1_START              (0x20000000)
#define STM32_SRAM1_END                (STM32_SRAM1_START + STM32_SRAM1_SIZE * 1024)

#define STM32_SRAM2_SIZE               (32)
#define STM32_SRAM2_START              (0x10000000)
#define STM32_SRAM2_END                (STM32_SRAM2_START + STM32_SRAM2_SIZE * 1024)

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define HEAP_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end;
#define HEAP_BEGIN    (&__bss_end)
#endif

#define HEAP_END                       STM32_SRAM1_END

void SystemClock_Config(void);
void SystemClock_MSI_ON(void);
void SystemClock_MSI_OFF(void);
void SystemClock_80M(void);
void SystemClock_24M(void);
void SystemClock_2M(void);
void SystemClock_ReConfig(uint8_t mode);

extern const struct push_button key_table[];
extern const int key_table_size;

extern const led_t led_table[];
extern const int led_table_size;

#ifdef __cplusplus
}
#endif

#endif

