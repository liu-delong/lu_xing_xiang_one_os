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
 * @file        spi_flash_at45dbxx.h
 *
 * @brief       this file implements adc related definitions and declarations
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef SPI_FLASH_AT45DBXX_H
#define SPI_FLASH_AT45DBXX_H

#include <board.h>
#include <drv_cfg.h>
#include <os_workqueue.h>
#include <drv_spi.h>

struct spi_flash_at45dbxx
{
    struct os_device              flash_device;
    struct os_device_blk_geometry geometry;
    struct os_spi_device         *os_spi_device;
};

#endif /* SPI_FLASH_AT45DBXX_H */
