/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * \@file        drv_gpio.h
 *
 * \@brief       This file provides operation functions declaration for gpio.
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __GPIO_H
#define __GPIO_H

#define __FM_PORT(port) GPIO##port##_BASE

#define GET_PIN(PORTx, PIN) \
    (os_base_t)((16 * (((os_base_t)__FM_PORT(PORTx) - (os_base_t)GPIOA_BASE) / (0x020UL))) + PIN)

#define PIN_BASE(__pin)   (GPIOx_Type *)(((__pin) / 16) * 0x020UL + (os_base_t)GPIOA_BASE)
#define PIN_OFFSET(__pin) (1 << (((os_uint32_t)__pin) % 16))

#define __FM_PIN(index, gpio, gpio_index)    \
{                                                                              \
    index, GPIO##gpio, GPIO_Pin_##gpio_index     \
}

#define __FM_PIN_RESERVE                          \
{                                                                             \
    -1, 0, 0                                                                \
}

/* STM32 GPIO driver */
struct pin_index
{
    os_int32_t    index;
    GPIOx_Type *gpio;
    os_uint32_t   pin;
};


int os_hw_pin_init(void);

#endif    // __GPIO_H
