/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        board.h
 *
 * @brief       Board resource definition
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BOARD_H_
#define __BOARD_H_

#include <drv_cfg.h>
#include <am_mcu_apollo.h>

#define SysTick_BASE        AM_REGADDRn(SYSTICK, 0, SYSTCSR)

extern os_uint32_t SystemCoreClock;

/* <o> Internal SRAM memory size[Kbytes] <8-256> */
/* <i>Default: 256 */
#define AM_SRAM_SIZE 64
#define AM_SRAM_END  (0x10000000 + AM_SRAM_SIZE * 1024)

/* GPIO */
#define APOLLO_PIN_NUMBERS 64

/* LED */
#define APOLLO_LED_TABLE                                                                                               \
    {                                                                                                                  \
        47, 48,                                                                                                        \
    }

/* USART0 */
#define AM_UART0_INST 0

//#define UART0_GPIO_RX     1
//#define UART0_GPIO_CFG_RX AM_HAL_PIN_1_UARTRX  
//#define UART0_GPIO_TX     0
//#define UART0_GPIO_CFG_TX AM_HAL_PIN_0_UARTTX

#define UART0_GPIO_RX     40
#define UART0_GPIO_CFG_RX AM_HAL_PIN_40_UARTRX
#define UART0_GPIO_TX     14
#define UART0_GPIO_CFG_TX AM_HAL_PIN_14_UARTTX

/* USART1 */
#define AM_UART1_INST 1

#define UART1_GPIO_RX     9
#define UART1_GPIO_CFG_RX AM_HAL_PIN_9_UART1RX
#define UART1_GPIO_TX     8
#define UART1_GPIO_CFG_TX AM_HAL_PIN_8_UART1TX

/* SPI0 */
#define AM_SPI0_IOM_INST 0

#define SPI0_GPIO_SCK      5
#define SPI0_GPIO_CFG_SCK  AM_HAL_PIN_5_M0SCK
#define SPI0_GPIO_MISO     6
#define SPI0_GPIO_CFG_MISO AM_HAL_PIN_6_M0MISO
#define SPI0_GPIO_MOSI     7
#define SPI0_GPIO_CFG_MOSI AM_HAL_PIN_7_M0MOSI

/* SPI1 */
#define AM_SPI1_IOM_INST 1

#define SPI1_GPIO_SCK      8
#define SPI1_GPIO_CFG_SCK  AM_HAL_PIN_8_M1SCK
#define SPI1_GPIO_MISO     9
#define SPI1_GPIO_CFG_MISO AM_HAL_PIN_9_M1MISO
#define SPI1_GPIO_MOSI     10
#define SPI1_GPIO_CFG_MOSI AM_HAL_PIN_10_M1MOSI
#define SPI1_GPIO_CE0      35
#define SPI1_GPIO_CFG_CE0  AM_HAL_PIN_35_M1nCE0

/* ADC */
#define ADC_CHANNEL_NUM 8

#define ADC_CHANNEL0_GPIO 12
#define ADC_CHANNEL0_PIN  AM_HAL_PIN_12_ADC0

#define ADC_CHANNEL0_CFG    (AM_HAL_ADC_SLOT_AVG_1      |\
                            AM_HAL_ADC_SLOT_CHSEL_VBATT |\
                            AM_HAL_ADC_SLOT_WINDOW_EN   |\
                            AM_HAL_ADC_SLOT_ENABLE)

#define ADC_CHANNEL7_CFG    (AM_HAL_ADC_SLOT_AVG_1      |\
                            AM_HAL_ADC_SLOT_CHSEL_TEMP  |\
                            AM_HAL_ADC_SLOT_WINDOW_EN   |\
                            AM_HAL_ADC_SLOT_ENABLE)
                           
/* I2CBB */
#define I2CBB_GPIO_SCL 45
#define I2CBB_GPIO_SDA 7

/* I2C0 */
#define I2C0_GPIO_SCL     5
#define I2C0_GPIO_CFG_SCK AM_HAL_PIN_5_M0SCL
#define I2C0_GPIO_SDA     6
#define I2C0_GPIO_CFG_SDA AM_HAL_PIN_6_M0SDA

/* I2C2 */
#define I2C2_GPIO_SCL     27
#define I2C2_GPIO_CFG_SCK AM_HAL_PIN_27_M2SCL
#define I2C2_GPIO_SDA     25
#define I2C2_GPIO_CFG_SDA AM_HAL_PIN_25_M2SDA

/* I2C3 */
#define I2C3_GPIO_SCL     42
#define I2C3_GPIO_CFG_SCK AM_HAL_PIN_42_M3SCL
#define I2C3_GPIO_SDA     43
#define I2C3_GPIO_CFG_SDA AM_HAL_PIN_43_M3SDA

/* I2C4 */
#define I2C4_GPIO_SCL     39
#define I2C4_GPIO_CFG_SCK AM_HAL_PIN_39_M4SCL
#define I2C4_GPIO_SDA     40
#define I2C4_GPIO_CFG_SDA AM_HAL_PIN_40_M4SDA

extern const struct push_button key_table[];
extern const int                key_table_size;

extern const led_t led_table[];
extern const int   led_table_size;

#endif /* __BOARD_H__ */
