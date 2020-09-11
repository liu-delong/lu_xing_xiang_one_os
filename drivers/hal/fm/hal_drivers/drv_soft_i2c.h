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
 * @file        drv_soft_i2c.h
 *
 * @brief       This file implements soft I2C for fm33a0xx.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_I2C__
#define __DRV_I2C__

#include <os_task.h>
#include <os_hw.h>
#include <drv_cfg.h>
#include "board.h"

struct fm_soft_i2c_config
{
    os_uint8_t  scl;
    os_uint8_t  sda;
    const char *bus_name;
};

struct fm_i2c
{
    struct os_i2c_bit_ops    ops;
    struct os_i2c_bus_device i2c2_bus;
};

#ifdef BSP_USING_I2C1
#define I2C1_BUS_CONFIG                                  \
    {                                                    \
        .scl = BSP_I2C1_SCL_PIN,                         \
        .sda = BSP_I2C1_SDA_PIN,                         \
        .bus_name = "i2c1",                              \
    }
#endif

int os_hw_i2c_init(void);

#endif /* __DRV_I2C__ */
