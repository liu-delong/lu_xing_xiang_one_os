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
 * @file        drv_spi.h
 *
 * @brief       This file implements SPI driver for fm33a0xx.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_SPI_H_
#define __DRV_SPI_H_

#include <os_task.h>
#include "os_device.h"
#include <os_hw.h>
#include <drv_common.h>
#ifdef OS_USING_SPI

os_err_t os_hw_spi_device_attach(const char *bus_name, const char *device_name, os_base_t pin_num);

struct fm_hw_spi_cs
{
    GPIOx_Type* GPIOx;
    os_uint32_t GPIO_Pin;
};

struct fm_spi_config
{
    SPIx_Type *Instance;
    char *bus_name;
};

struct fm_spi_device
{
    os_uint32_t pin;
    char *bus_name;
    char *device_name;
};

typedef struct
{
    SPI_Master_SInitTypeDef Init;
    SPIx_Type *Instance;
} FM_HandleType;

/* stm32 spi dirver class */
struct fm_spi
{
    FM_HandleType handle;
    const struct fm_spi_config *config;
    struct os_spi_configuration *cfg;
    struct os_spi_bus spi_bus;
};

#ifdef BSP_USING_SPI1
#define SPI1_BUS_CONFIG                                  \
    {                                                    \
        .Instance = SPI1,                                \
        .bus_name = "spi1",                              \
    }
#endif

#ifdef BSP_USING_SPI2
#define SPI2_BUS_CONFIG                                  \
    {                                                    \
        .Instance = SPI2,                                \
        .bus_name = "spi2",                              \
    }
#endif

#endif
#endif /*__DRV_SPI_H_ */
