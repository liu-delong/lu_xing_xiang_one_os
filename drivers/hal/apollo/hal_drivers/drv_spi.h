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
 * @brief       This file provides operation functions declaration for spi.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SPI_H_
#define __SPI_H_

#include <os_types.h>

struct am_spi_cs
{
    os_uint32_t pin;
};

int      os_hw_spi_init(void);
os_err_t os_hw_spi_device_attach(const char *bus_name, const char *device_name, os_base_t pin);

#endif /* __SPI_H_ */
