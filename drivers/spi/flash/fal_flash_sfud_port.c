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
 * @file        fal_flash_sfud_port.c
 *
 * @brief       This file provides functions for fal flash sfud.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <sfud.h>
#include <spi_flash_sfud.h>
#include <fal/fal.h>

static sfud_flash *sfud_norflash0 = OS_NULL;

static int fal_sfud_init(fal_flash_t *flash)
{
    sfud_flash_t sfud_flash0 = NULL;
    sfud_flash0 = (sfud_flash_t)os_sfud_flash_find(OS_EXTERN_FLASH_BUS_NAME);

    if (NULL == sfud_flash0)
    {
        return -1;
    }

    sfud_norflash0 = sfud_flash0;
    return 0;
}

static int read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    if (sfud_read(sfud_norflash0, page_addr * OS_EXTERN_FLASH_PAGE_SIZE, page_nr * OS_EXTERN_FLASH_PAGE_SIZE, buff) != SFUD_SUCCESS)
    {
        return -1;
    }

    return 0;
}

static int write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    if (sfud_write(sfud_norflash0, page_addr * OS_EXTERN_FLASH_PAGE_SIZE, page_nr * OS_EXTERN_FLASH_PAGE_SIZE, buff) != SFUD_SUCCESS)
    {
        return -1;
    }

    return 0;
}

static int erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    if (sfud_erase(sfud_norflash0, page_addr * OS_EXTERN_FLASH_PAGE_SIZE, page_nr * OS_EXTERN_FLASH_PAGE_SIZE) != SFUD_SUCCESS)
    {
        return -1;
    }

    return 0;
}

FAL_FLASH_DEFINE sfud_flash0 =
{
    .name = OS_EXTERN_FLASH_NAME,
    .capacity   = OS_EXTERN_FLASH_SIZE,
    .block_size = OS_EXTERN_FLASH_BLOCK_SIZE,
    .page_size  = OS_EXTERN_FLASH_PAGE_SIZE,
    .ops  =
    {
        .init        = fal_sfud_init,
        .read_page   = read,
        .write_page  = write,
        .erase_block = erase,
    },
};

