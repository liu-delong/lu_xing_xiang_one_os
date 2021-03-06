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
 * @file        drv_crypto.h
 *
 * @brief       This file provides crypto function declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_CRYPTO_H__
#define __DRV_CRYPTO_H__

#include <board.h>
#include <os_hw.h>
#include <os_irq.h>
#include <os_device.h>
#include <os_mutex.h>
#include <hwcrypto.h>

#define TEMP_IMXRT_UID 0x1b7df9a4

struct imxrt_hwcrypto_device
{
    struct os_hwcrypto_device hwcrypto;
    
    struct os_mutex mutex;

    void *handle;
};


struct nxp_trng_info {
    TRNG_Type *trng_base;
    const trng_config_t *config;
};

static inline os_uint32_t imxrt_hwcrypto_uid(void)
{
    os_uint32_t uid = 0;

    uid = TEMP_IMXRT_UID;

    return uid;
}

#endif /* __DRV_CRYPTO_H__ */
