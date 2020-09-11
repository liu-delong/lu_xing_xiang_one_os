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
 * @file        drv_flash.c
 *
 * @brief       The driver file for flash.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"

#ifdef BSP_USING_ON_CHIP_FLASH
#include "drv_flash.h"
#include "flash_api.h"

#if defined(PKG_USING_FAL)
#include "fal.h"
#endif

#define LOG_TAG "drv.flash"
#include <drv_log.h>

int ameba_flash_read(os_uint32_t addr, os_uint8_t *buf, os_size_t size)
{
    flash_t flash;

    if ((addr + size) > AMEBA_FLASH_END_ADDRESS)
    {
        LOG_EXT_E("read outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return OS_EINVAL;
    }

    flash_stream_read(&flash, addr, size, buf);

    return size;
}

int ameba_flash_write(os_uint32_t addr, const os_uint8_t *buf, os_size_t size)
{
    flash_t flash;

    if ((addr + size) > AMEBA_FLASH_END_ADDRESS)
    {
        LOG_EXT_E("ERROR: write outrange flash size! addr is (0x%p)\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    if (addr % 4 != 0)
    {
        LOG_EXT_E("write addr must be 4-byte alignment");
        return OS_EINVAL;
    }

    if (size < 1)
    {
        return OS_ERROR;
    }

    ameba_flash_erase(addr, size);
    /* flash_erase_sector(&flash, addr); */
    flash_burst_write(&flash, addr, size, buf);

    return size;
}

int ameba_flash_erase(os_uint32_t addr, os_size_t size)
{
    int     i   = 0;
    flash_t flash;
    
    os_uint32_t sector_num = 0;
    os_uint32_t sector_addr;

    if ((addr + size) > AMEBA_FLASH_END_ADDRESS)
    {
        LOG_EXT_E("ERROR: erase outrange flash size! addr is (0x%p)\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    sector_addr = OS_ALIGN_DOWN(addr, AMEBA_FLASH_SECTOR_SIZE);
    sector_num  = (size + AMEBA_FLASH_SECTOR_SIZE - 1) / AMEBA_FLASH_SECTOR_SIZE;

    for (i = 0; i < sector_num; i++)
        flash_erase_sector(&flash, sector_addr + AMEBA_FLASH_SECTOR_SIZE * i);

    LOG_EXT_D("erase done: addr (0x%p), size %d", (void *)addr, size);
    return size;
}

#if defined(PKG_USING_FAL)

static int fal_flash_read(long offset, os_uint8_t *buf, size_t size);
static int fal_flash_write(long offset, const os_uint8_t *buf, size_t size);
static int fal_flash_erase(long offset, size_t size);

const struct fal_flash_dev ameba_onchip_flash = {OS_ONCHIP_FLASH_NAME,
                                                 AMEBA_FLASH_START_ADRESS,
                                                 AMEBA_FLASH_SIZE,
                                                 AMEBA_FLASH_SECTOR_SIZE,
                                                 {NULL, fal_flash_read, fal_flash_write, fal_flash_erase}};

static int fal_flash_read(long offset, os_uint8_t *buf, size_t size)
{
    return ameba_flash_read(offset, buf, size);
}

static int fal_flash_write(long offset, const os_uint8_t *buf, size_t size)
{
    return ameba_flash_write(offset, buf, size);
}

static int fal_flash_erase(long offset, size_t size)
{
    return ameba_flash_erase(offset, size);
}

#endif
#endif /* BSP_USING_ON_CHIP_FLASH */
