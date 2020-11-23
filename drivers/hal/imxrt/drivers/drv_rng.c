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
 * @brief       This file implements imxrt crypto driver.
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
#include <board.h>
#include <os_memory.h>
#include "fsl_trng.h"
#include "drv_rng.h"
#include "hw_rng.h"
#include "peripherals.h"

static os_uint32_t imxrt_rng_rand(struct hwcrypto_rng *ctx)
{
    os_uint32_t gen_random = 0;
    TRNG_Type *HW_TypeDef = (TRNG_Type *)(ctx->parent.contex);

    struct imxrt_hwcrypto_device *imxrt_hw_dev = (struct imxrt_hwcrypto_device *)ctx->parent.device->user_data;

    os_mutex_lock(&imxrt_hw_dev->mutex, OS_IPC_WAITING_FOREVER);

    if (TRNG_GetRandomData(HW_TypeDef, &gen_random, sizeof(os_uint32_t)) != kStatus_Success)
    {
        os_kprintf("rng get data fail.\r\n");
        gen_random = 0;
    }

    os_mutex_unlock(&imxrt_hw_dev->mutex);

    return gen_random;
}

static const struct hwcrypto_rng_ops imxrt_rng_ops = {
    .update = imxrt_rng_rand,
};

static os_err_t imxrt_rng_create(struct os_hwcrypto_ctx *ctx)
{
    struct imxrt_hwcrypto_device *imxrt_hw_dev = (struct imxrt_hwcrypto_device *)ctx->device->user_data;
    trng_config_t userConfig;
    
    TRNG_Type *hrng = (TRNG_Type *)(*(os_uint32_t *)imxrt_hw_dev->handle);
    OS_ASSERT(hrng != OS_NULL);
    
    if (TRNG_GetDefaultConfig(&userConfig) != kStatus_Success)
    {
        os_kprintf("rng get cfg fail.\r\n");
        return OS_ERROR;
    }

    if (TRNG_Init(hrng, &userConfig) != kStatus_Success)
    {
        os_kprintf("rng init fail.\r\n");
        return OS_ERROR;
    }
    
    ctx->contex = hrng;
    ((struct hwcrypto_rng *)ctx)->ops = &imxrt_rng_ops;

    return OS_EOK;
}

static void imxrt_rng_destroy(struct os_hwcrypto_ctx *ctx)
{
    struct imxrt_hwcrypto_device *imxrt_hw_dev = (struct imxrt_hwcrypto_device *)ctx->device->user_data;
        
    TRNG_Type *hrng = (TRNG_Type *)(*(os_uint32_t *)imxrt_hw_dev->handle);
    OS_ASSERT(hrng != OS_NULL);
    
    TRNG_Deinit(hrng);

    return;
}

static os_err_t imxrt_rng_clone(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src)
{
    if (des->contex && src->contex)
    {
        memcpy(des->contex, src->contex, sizeof(TRNG_Type));
    }

    return OS_EOK;
}

static void imxrt_rng_reset(struct os_hwcrypto_ctx *ctx)
{
    // do nothing.

    return;
}

static const struct os_hwcrypto_ops os_rng_ops = {
    .create  = imxrt_rng_create,
    .destroy = imxrt_rng_destroy,
    .copy    = imxrt_rng_clone,
    .reset   = imxrt_rng_reset,
};

static int imxrt_rng_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    struct imxrt_hwcrypto_device *imxrt_hwcrypto = os_calloc(1, sizeof(struct imxrt_hwcrypto_device));
    OS_ASSERT(imxrt_hwcrypto);
    imxrt_hwcrypto->handle = (void *)dev->info;

    struct os_hwcrypto_device *hwcrypto = &imxrt_hwcrypto->hwcrypto;
    hwcrypto->ops = &os_rng_ops;
    hwcrypto->id = imxrt_hwcrypto_uid();
    hwcrypto->user_data = hwcrypto;

    if (os_hwcrypto_rng_register(hwcrypto, dev->name) != OS_EOK)
    {
        os_kprintf("imxrt rng probe failed %s.\r\n", dev->name);
        os_free(imxrt_hwcrypto);
        return -1;
    }
    
    os_mutex_init(&imxrt_hwcrypto->mutex, OS_HWCRYPTO_DEFAULT_NAME, OS_IPC_FLAG_FIFO, OS_FALSE);
    return 0;
}

OS_DRIVER_INFO imxrt_rng_driver = {
    .name   = "TRNG_Type",
    .probe  = imxrt_rng_probe,
};

OS_DRIVER_DEFINE(imxrt_rng_driver, "3");

