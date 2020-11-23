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
 * @file        drv_adc.c
 *
 * @brief       This file implements adc driver for nxp.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_stddef.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <string.h>
#include <os_irq.h>
#include <drv_adc.h>
#include "fsl_power.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.adc"
#include <drv_log.h>

//#define ADC_REGULAR_TEST

#ifdef ADC_REGULAR_TEST
os_uint16_t adc_test_buf[30];
#endif

struct nxp_adc
{
    struct os_adc_device adc;
    struct nxp_adc_info *info;
    lpadc_conv_result_t adc_result;
};

static os_err_t nxp_adc_read(struct os_adc_device *adc, os_uint32_t channel, os_int32_t *buff)
{
    struct nxp_adc *nxp_adc;
    os_int32_t adc_buff;
    
    nxp_adc = os_container_of(adc, struct nxp_adc, adc);
    
    while (!LPADC_GetConvResult(nxp_adc->info->adc_base, &nxp_adc->adc_result, channel));
    *buff = (os_int32_t)((os_uint64_t)((nxp_adc->adc_result.convValue) >> 3U) * adc->mult >> adc->shift);
    return OS_EOK;
}

static os_err_t nxp_adc_enabled(struct os_adc_device *adc, os_bool_t enable)
{
    struct nxp_adc *nxp_adc;
    
    nxp_adc = os_container_of(adc, struct nxp_adc, adc);
    
    if (enable == OS_TRUE)
    {
        POWER_DisablePD(kPDRUNCFG_PD_LDOGPADC);
        LPADC_DoSoftwareTrigger(nxp_adc->info->adc_base, 1U);
    }
    else
    {}
    
    return OS_EOK;
}

static os_err_t nxp_adc_control(struct os_adc_device *adc, int cmd, void *arg)
{
    return OS_EOK;
}

static const struct os_adc_ops nxp_adc_ops = {
    .adc_enabled            = nxp_adc_enabled,
    .adc_control            = OS_NULL,
    .adc_read               = nxp_adc_read,
};

static int nxp_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    
    struct nxp_adc_info *adc_info = (struct nxp_adc_info *)dev->info;
    
    struct nxp_adc *nxp_adc = os_calloc(1, sizeof(struct nxp_adc)); 
    OS_ASSERT(nxp_adc);
    
    nxp_adc->info = adc_info;
    
    struct os_adc_device *dev_adc = &nxp_adc->adc;
    dev_adc->ops    = &nxp_adc_ops;
    dev_adc->max_value = (1UL << 12) - 1;;
    dev_adc->ref_low   = 0;                 /* ref 0 - 3.3v */
    dev_adc->ref_hight = 3300;

    result = os_hw_adc_register(dev_adc,
                                "adc",
                                OS_DEVICE_FLAG_RDWR,
                                NULL);


    return OS_EOK;
}

OS_DRIVER_INFO nxp_adc_driver = {
    .name   = "ADC_Type",
    .probe  = nxp_adc_probe,
};

OS_DRIVER_DEFINE(nxp_adc_driver,"2");
