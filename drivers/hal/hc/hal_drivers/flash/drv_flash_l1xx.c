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
 * @file        drv_flash_l1xx.c
 *
 * @brief       The file of flash drv for hc32l1xx
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "hc_flash.h"

#ifdef BSP_USING_ON_CHIP_FLASH
#include "drv_flash.h"

#if defined(OS_USING_FAL)
#include "fal.h"
#endif

/* Use Flash_WriteByte/Flash_SectorErase function from bootloader */
#ifdef OS_USE_BOOTLOADER
extern PTR_WRITE pfun_write;
extern PTR_ERASE pfun_erase;
#endif

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
int hc32_flash_read(os_uint32_t addr, os_uint8_t *buf, os_size_t size)
{
    size_t i;

    if ((addr + size) > HC32_FLASH_END_ADDRESS)
    {
        printf("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return OS_EINVAL;
    }

    for(i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(os_uint8_t *) addr;
    }

    return size;
}

/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */

int hc32_flash_write(os_uint32_t addr, const uint8_t *buf, os_size_t size)
{
    size_t i;
    uint8_t *p = (uint8_t *)buf;

    if ((addr + size) > HC32_FLASH_END_ADDRESS)
    {
        printf("ERROR: write outrange flash size! addr is (0x%p)\n", (void*)(addr + size));
        return OS_EINVAL;
    }

    for(i = 0; i < size; i++)
    {
#ifdef OS_USE_BOOTLOADER
        pfun_write(addr + i, p[i]);
#else
        Flash_WriteByte(addr + i, p[i]);
#endif
    }

    for(i = 0; i < size; i++)
    {
        if(p[i] != *((uint8_t *)(addr + i)))
        {
            return 0;
        }
    }

    return size;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
int hc32_flash_erase(os_uint32_t addr, os_size_t size)
{
    os_uint16_t sector_start = 0;
    os_uint16_t sector_end = 0;
    os_uint16_t sector_cnt = 0;

    if ((addr + size) > HC32_FLASH_END_ADDRESS)
    {
        printf("ERROR: erase outrange flash size! addr is (0x%p)\n", (void*)(addr + size));
        return OS_EINVAL;
    }

    sector_start = addr / HC32_SECTOR_SIZE;
    sector_end = (addr + size) / HC32_SECTOR_SIZE + (1 ? 0 : (addr % HC32_SECTOR_SIZE));

    for (sector_cnt = 0; sector_cnt < (sector_end - sector_start); sector_cnt++)
    {
#ifdef OS_USE_BOOTLOADER
        pfun_erase((sector_start + sector_cnt) * HC32_SECTOR_SIZE);
#else
        Flash_SectorErase((sector_start + sector_cnt) * HC32_SECTOR_SIZE);
#endif
    }

    return size;
}

#if defined(OS_USING_FAL)

static int fal_flash_read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    int count = hc32_flash_read(HC32_FLASH_START_ADRESS + page_addr * HC32_FLASH_PAGE_SIZE, buff, page_nr * HC32_FLASH_PAGE_SIZE);

    return (count == page_nr * HC32_FLASH_PAGE_SIZE) ? 0 : -1;

}

static int fal_flash_write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    int count = hc32_flash_write(HC32_FLASH_START_ADRESS + page_addr * HC32_FLASH_PAGE_SIZE, buff, page_nr * HC32_FLASH_PAGE_SIZE);

    return (count == page_nr * HC32_FLASH_PAGE_SIZE) ? 0 : -1;

}

static int fal_flash_erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int count =  hc32_flash_erase(HC32_FLASH_START_ADRESS + page_addr * HC32_FLASH_PAGE_SIZE, page_nr * HC32_FLASH_PAGE_SIZE);

    return (count == page_nr * HC32_FLASH_PAGE_SIZE) ? 0 : -1;

}

FAL_FLASH_DEFINE hc32_onchip_flash =
{
    .name = "onchip_flash",
    .capacity = HC32_FLASH_SIZE,
    .block_size = HC32_FLASH_BLOCK_SIZE,
    .page_size  = HC32_FLASH_PAGE_SIZE,

    .ops  =
    {
        .init  = NULL,
        .read_page  = fal_flash_read,
        .write_page = fal_flash_write,
        .erase_block = fal_flash_erase,
    },
};

#endif
#endif /* BSP_USING_ON_CHIP_FLASH */
