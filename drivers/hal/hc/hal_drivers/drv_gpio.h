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
 * @file        drv_gpio.h
 *
 * @brief       This file provides struct/macro declaration and functions declaration for STM32 gpio driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#include <drv_common.h>
#include "board.h"
#include "hc_sysctrl.h"
#include "hc_gpio.h"

#if defined(SOC_SERIES_HC32L19)

#define	GPIOPORTA (0)
#define	GPIOPORTB (1)
#define	GPIOPORTC (2)
#define	GPIOPORTD (3)
#define	GPIOPORTE (4)
#define	GPIOPORTF (5)

#else

	#error "please add surpport for SOC..."

#endif

#define __HC32_PORT(port) GPIOPORT##port

#define GET_PIN(PORTx, PIN)                                                                                            \
    (os_base_t)((16 * ((os_base_t)__HC32_PORT(PORTx) - (os_base_t)GPIOPORTA)) + PIN)

//#define PIN_BASE(__pin)   (GPIO_TypeDef *)(((__pin) / 16) * 0x0400UL + (os_base_t)GPIOA_BASE)
//#define PIN_OFFSET(__pin) (1 << ((__pin) % 16))

#define __INDEX_PIN(index, gpio, gpio_index)                                                                           \
    {                                                                                                                  \
        index, GpioPort##gpio, GpioPin##gpio_index, OS_NULL, OS_NULL, 0, 0                                                                       \
    }

#define __STM32_PIN_RESERVE                                                                                            \
    {                                                                                                                  \
        -1, 0, 0                                                                                                       \
    }

/* HC32 GPIO driver */
struct pin_index
{
    os_int32_t    index;
    en_gpio_port_t   port;
    en_gpio_pin_t   pin;
	void (*hdr)(void *args);
	void *args;
	os_uint16_t mode;
	os_uint8_t	irq_ref;
};

struct HC32_IRQ_STAT{
	IRQn_Type	irq;
	os_uint32_t ref;
};

struct pin_pull_state
{
    os_int8_t pd;
	os_int8_t pu;
};

struct pin_irq_map
{
    os_uint16_t pinbit;
    IRQn_Type   irqno;
};

int os_hw_pin_init(void);

#endif /* __DRV_GPIO_H__ */
