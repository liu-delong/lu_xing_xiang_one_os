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
 * @brief        This file provides flash read/write/erase functions for FM33A0xx.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"

#ifdef BSP_USING_ON_CHIP_FLASH
#include "drv_flash.h"

#if defined(PKG_USING_FAL)
#include "fal.h"
#endif

#define LOG_TAG "drv.flash"
#include <drv_log.h>


/* Flash operation verification code (for reliability, specific data can be customized by the user, stored in ee or flash, read and assign to OperateKey before operating flash) */
#define FLASHOPKEY	0x12ABF00F


os_uint8_t Flash_Erase_Sector( os_uint16_t SectorNum, os_uint32_t OperateKey )
{
	os_uint16_t i;
	os_uint8_t Result = 0;
	os_uint32_t *PFlash;
	
	PFlash = (os_uint32_t *)(os_uint32_t)(SectorNum*512);
	if( OperateKey == FLASHOPKEY )
	{
		RCC_PERCLK_SetableEx(FLSEPCLK, ENABLE);
	}
	FLASH_Erase_Sector( SectorNum*512 );
	RCC_PERCLK_SetableEx(FLSEPCLK, DISABLE);
	
	for( i=0; i<128; i++ )
	{
		if( PFlash[i] != 0xFFFFFFFF ) 
		{
			Result = 1;
			break;
		}
	}
	
	return Result;
}

os_uint8_t Flsah_Write_String( os_uint32_t prog_addr, const os_uint8_t* prog_data, os_uint16_t Len, os_uint32_t OperateKey )
{
	os_uint16_t i;
	os_uint8_t Result = 0;
	os_uint8_t *PFlash;
	
	if( OperateKey == FLASHOPKEY )
	{
		RCC_PERCLK_SetableEx(FLSEPCLK, ENABLE);
	}
	FLASH_Prog_ByteString( prog_addr, prog_data, Len);
	RCC_PERCLK_SetableEx(FLSEPCLK, DISABLE);
	
	PFlash = (os_uint8_t*)prog_addr;
	for( i=0;i<Len;i++ )
	{
		if( PFlash[i] != prog_data[i] ) 
		{
			Result = 1;
			break;
		}
	}	
	
	return Result;
}

/**
 ***********************************************************************************************************************
 * @brief           fm_flash_read:Read data from flash,and this operation's units is word.
 *
 * @param[in]       addr            flash address.
 * @param[out]      buf             buffer to store read data.
 * @param[in]       size            read bytes size.
 *
 * @return          Return read size or status.
 * @retval          size            read bytes size.
 * @retval          Others          read failed.
 ***********************************************************************************************************************
 */
int fm_flash_read(os_uint32_t addr, os_uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > FM_FLASH_END_ADDRESS)
    {
        LOG_EXT_E("read outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return OS_EINVAL;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(os_uint8_t *)addr;
    }

    return size;
}

/**
 ***********************************************************************************************************************
 * @brief           Write data to flash.This operation's units is word.
 *
 * @attention       This operation must after erase.
 *
 * @param[in]       addr            flash address.
 * @param[in]       buf             the write data buffer.
 * @param[in]       size            write bytes size.
 *
 * @return          Return write size or status.
 * @retval          size            write bytes size.
 * @retval          Others          write failed.
 ***********************************************************************************************************************
 */
int fm_flash_write(os_uint32_t addr, const os_uint8_t *buf, size_t size)
{
    if ((addr + size) > FM_FLASH_END_ADDRESS)
    {
        LOG_EXT_E("ERROR: write outrange flash size! addr is (0x%p)\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    if (addr % 4 != 0)
    {
        LOG_EXT_E("write addr must be 4-byte alignment");
        return OS_EINVAL;
    }

    Flsah_Write_String(addr, buf, size, FLASHOPKEY);

    return size;
}

/**
 ***********************************************************************************************************************
 * @brief           Erase data on flash.This operation is irreversible and it's units is different which on many chips.
 *
 * @param[in]       addr            Flash address.
 * @param[in]       size            Erase bytes size.
 *
 * @return          Return erase result or status.
 * @retval          size            Erase bytes size.
 * @retval          Others          Erase failed.
 ***********************************************************************************************************************
 */
int fm_flash_erase(os_uint32_t addr, size_t size)
{
    os_uint16_t SectorNumstart, SectorNumend;
    os_uint16_t i;

    if ((addr + size) > FM_FLASH_END_ADDRESS)
    {
        LOG_EXT_E("ERROR: erase outrange flash size! addr is (0x%p)\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    SectorNumstart = addr /FM_FLASH_PAGE_SIZE;
    SectorNumend = (addr + size + FM_FLASH_PAGE_SIZE - 1) /FM_FLASH_PAGE_SIZE;

    for (i = SectorNumstart; i <= SectorNumend; i++)
    {
        Flash_Erase_Sector(i, FLASHOPKEY);
    }
    
    LOG_EXT_D("erase done: addr (0x%p), size %d", (void *)addr, size);
    return size;
}

#if defined(PKG_USING_FAL)

static int fal_flash_read(long offset, os_uint8_t *buf, size_t size);
static int fal_flash_write(long offset, const os_uint8_t *buf, size_t size);
static int fal_flash_erase(long offset, size_t size);

const struct fal_flash_dev fm_onchip_flash = {"onchip_flash",
                                                 FM_FLASH_START_ADRESS,
                                                 FM_FLASH_SIZE,
                                                 FM_FLASH_PAGE_SIZE,
                                                 {NULL, fal_flash_read, fal_flash_write, fal_flash_erase}};

static int fal_flash_read(long offset, os_uint8_t *buf, size_t size)
{
    return fm_flash_read(fm_onchip_flash.addr + offset, buf, size);
}

static int fal_flash_write(long offset, const os_uint8_t *buf, size_t size)
{
    return fm_flash_write(fm_onchip_flash.addr + offset, buf, size);
}

static int fal_flash_erase(long offset, size_t size)
{
    return fm_flash_erase(fm_onchip_flash.addr + offset, size);
}

#endif
#endif /* BSP_USING_ON_CHIP_FLASH */
