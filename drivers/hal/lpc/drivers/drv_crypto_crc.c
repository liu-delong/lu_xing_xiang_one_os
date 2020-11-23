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
 * @file        drv_crypto.c
 *
 * @brief       This file implements stm32 crypto driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_device.h>
#include <stdlib.h>
#include <string.h>
#include "drv_crypto.h"
#include <board.h>
#include <os_memory.h>

static os_int32_t nxp_crc_config(struct hwcrypto_crc *ctx)
{
    struct nxp_hwcrypto_device *nxp_hw_dev = (struct nxp_hwcrypto_device *)ctx->parent.device->user_data;

    return 0;
}

static os_uint32_t nxp_crc_update(struct hwcrypto_crc *ctx, const os_uint8_t *in, os_size_t length)
{
    struct nxp_hwcrypto_device *nxp_hwcrypto = (struct nxp_hwcrypto_device *)ctx->parent.device->user_data;

    os_mutex_lock(&nxp_hwcrypto->mutex, OS_IPC_WAITING_FOREVER);
    CRC_WriteData(nxp_hwcrypto->crc_info->crc_base, in, length);
    os_mutex_unlock(&nxp_hwcrypto->mutex);

    if (nxp_hwcrypto->crc_info->crc_config->polynomial == kCRC_Polynomial_CRC_32)
        return CRC_Get32bitResult(nxp_hwcrypto->crc_info->crc_base);
    else
        return CRC_Get16bitResult(nxp_hwcrypto->crc_info->crc_base);
}

static const struct hwcrypto_crc_ops crc_ops = {
    .config = nxp_crc_config,
    .update = nxp_crc_update,
};

static os_err_t nxp_crc_crypto_create(struct os_hwcrypto_ctx *ctx)
{
    struct nxp_hwcrypto_device *nxp_hwcrypto = (struct nxp_hwcrypto_device *)ctx->device->user_data;
    
//    CRC_Init(nxp_hwcrypto->crc_info->crc_base, nxp_hwcrypto->crc_info->crc_config);

    ctx->contex = nxp_hwcrypto->crc_info->crc_base;
    ((struct hwcrypto_crc *)ctx)->ops = &crc_ops;
    
    return OS_EOK;
}

static void nxp_crc_crypto_destroy(struct os_hwcrypto_ctx *ctx)
{
    struct nxp_hwcrypto_device *nxp_hwcrypto = (struct nxp_hwcrypto_device *)ctx->device->user_data;
    
    CRC_Deinit(nxp_hwcrypto->crc_info->crc_base);
}

static os_err_t nxp_crc_crypto_clone(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src)
{
    if (des->contex && src->contex)
    {
        memcpy(des->contex, src->contex, sizeof(CRC_Type));
    }

    return OS_EOK;
}

static void nxp_crc_crypto_reset(struct os_hwcrypto_ctx *ctx)
{
    struct nxp_hwcrypto_device *nxp_hwcrypto = (struct nxp_hwcrypto_device *)ctx->device->user_data;
    
    CRC_Reset(nxp_hwcrypto->crc_info->crc_base);
}

static const struct os_hwcrypto_ops _ops = {
    .create  = nxp_crc_crypto_create,
    .destroy = nxp_crc_crypto_destroy,
    .copy    = nxp_crc_crypto_clone,
    .reset   = nxp_crc_crypto_reset,
};

static int nxp_crc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct nxp_hwcrypto_device *nxp_hwcrypto = os_calloc(1, sizeof(struct nxp_hwcrypto_device));

    OS_ASSERT(nxp_hwcrypto);

    struct os_hwcrypto_device *hwcrypto = &nxp_hwcrypto->hwcrypto;

    nxp_hwcrypto->crc_info = (struct nxp_crc_engine_info *)dev->info;

    hwcrypto->ops = &_ops;

//    hwcrypto->id = nxp_hwcrypto_uid();

    hwcrypto->user_data = hwcrypto;

    if (os_hwcrypto_crc_register(hwcrypto, dev->name) != OS_EOK)
    {
        os_kprintf("nxp crc probe failed %s.\r\n", dev->name);
        os_free(nxp_hwcrypto);
        return -1;
    }
    
    os_mutex_init(&nxp_hwcrypto->mutex, OS_HWCRYPTO_DEFAULT_NAME, OS_IPC_FLAG_FIFO, OS_FALSE);
    return 0;
}

OS_DRIVER_INFO nxp_crc_driver = {
    .name   = "CRC_ENGINE_Type",
    .probe  = nxp_crc_probe,
};

OS_DRIVER_DEFINE(nxp_crc_driver, "3");

