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
 * @file        drv_usart.h
 *
 * @brief        This file provides functions declaration for usart driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include <os_types.h>
#include <drv_cfg.h>
#include <ingenic_usart.h>
#include <ingenic_gpio.h>

struct ingenic_usart_info {
    os_uint32_t hw_base;
    os_uint32_t irqno;
};

struct ingenic_uart
{
    struct os_serial_device serial_dev;
    struct ingenic_usart_info *usart_info;
    os_uint8_t *buffer;
    os_size_t count;
    os_size_t size;
    os_int32_t state;
};

int os_hw_usart_init(void);

#endif

