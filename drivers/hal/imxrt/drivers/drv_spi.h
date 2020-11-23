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
 * @brief       This file implements spi driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#ifndef DRV_SPI_H__
#define DRV_SPI_H__

#include <os_task.h>
#include <os_device.h>
#include "peripherals.h"
#include "spi.h"

struct imxrt_sw_spi_cs
{
    os_uint32_t pin;
};

typedef struct 
{
    struct os_spi_bus spi_bus;
    LPSPI_Type *base;
} imxrt_spi_t;

struct nxp_lpspi_info {
    LPSPI_Type *spi_base;
    const lpspi_master_config_t *config;
};

int rt_hw_spi_bus_init(void);
os_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, os_base_t pin);

#endif /* DRV_SPI_H__ */
