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

#define __MM32_PORT(port) GPIO##port##_BASE

#define GET_PIN(PORTx, PIN)                                                                                            \
    (os_base_t)((16 * (((os_base_t)__MM32_PORT(PORTx) - (os_base_t)GPIOA_BASE) / (0x0400UL))) + PIN)

#define PIN_BASE(__pin)   (GPIO_TypeDef *)(((__pin) / 16) * 0x0400UL + (os_base_t)GPIOA_BASE)
#define PIN_OFFSET(__pin) (1 << ((__pin) % 16))

#define __MM32_PIN(index, gpio, gpio_index)                                                                           \
    {                                                                                                                  \
        index, GPIO##gpio, GPIO_PIN_##gpio_index                                                                       \
    }


#define __MM32_PIN_RESERVE                                                                                            \
    {                                                                                                                  \
        -1, 0, 0                                                                                                       \
    }

/* MM32 GPIO driver */
struct pin_index
{
    os_int32_t index;
    os_uint32_t rcc;
    GPIO_TypeDef *gpio;
    os_uint32_t pin;
    os_uint8_t port_source;
    os_uint8_t pin_source;
};

struct pin_pull_state
{
    os_int8_t pull_state;
};

struct pin_irq_map
{
    os_uint16_t            pinbit;
    os_uint32_t            irqbit;
    enum IRQn              irqno;
};


int os_hw_pin_init(void);

#endif /* __DRV_GPIO_H__ */
