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
 * @file        drv_flash_lpc55s69.c
 *
 * @brief        This file provides flash read/write/erase functions for lpc55s69.
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"

#include "drv_flash.h"
#include <fsl_common.h>
#include <fsl_iap.h>

#define LOG_TAG "drv.flash"
#include <drv_log.h>

flash_config_t nxp_flashConfig;

/**
 ***********************************************************************************************************************
 * @brief           nxp_flash_read:Read data from flash,and this operation's units is word.
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
int nxp_flash_read(os_uint32_t addr, os_uint8_t *buf, size_t size)
{
    os_uint32_t status;

    if ((addr + size) > NXP_FLASH_END_ADDRESS)
    {
        LOG_EXT_E("read outrange flash size! addr is (0x%p)\r\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    if (addr % 512 != 0)
    {
        LOG_EXT_E("read addr must be 512-byte alignment\r\n");
        return OS_EINVAL;
    }
    status = FLASH_Read(&nxp_flashConfig, addr, buf, size);

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
int nxp_flash_write(os_uint32_t addr, const os_uint8_t *buf, size_t size)
{
    os_uint32_t status;

    if ((addr + size) > NXP_FLASH_END_ADDRESS)
    {
        LOG_EXT_E("ERROR: write outrange flash size! addr is (0x%p)\r\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    if (addr % 512 != 0)
    {
        LOG_EXT_E("write addr must be 512-byte alignment\r\n");
        return OS_EINVAL;
    }

    status = FLASH_Program(&nxp_flashConfig, addr, (os_uint8_t *)buf, size);
    
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
int nxp_flash_erase(os_uint32_t addr, size_t size)
{
    os_uint32_t status;

    if ((addr + size) > NXP_FLASH_END_ADDRESS)
    {
        LOG_EXT_E("ERROR: write outrange flash size! addr is (0x%p)\r\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    if (addr % 512 != 0)
    {
        LOG_EXT_E("erase addr must be 512-byte alignment\r\n");
        return OS_EINVAL;
    }
    if (size % 512 != 0)
    {
        LOG_EXT_I("erase size must be 512-byte or multiple!\r\n");
        return 0;
    }
    status = FLASH_Erase(&nxp_flashConfig, addr, size, kFLASH_ApiEraseKey);
    
    return size;

}

#include "fal_drv_flash.c"
