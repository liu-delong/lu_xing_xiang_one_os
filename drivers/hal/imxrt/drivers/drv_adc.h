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
 * @file        drv_adc.h
 *
 * @brief       This file implements adc driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRV_ADC_H__
#define DRV_ADC_H__
#include <os_device.h>
#include "adc.h"

#if defined(ADC1_PERIPHERAL) || defined(ADC2_PERIPHERAL)
struct nxp_adc_info
{
    ADC_Type *adc_base;
    const adc_config_t *adc_config;
};
#endif

#ifdef ADC_ETC_PERIPHERAL
struct nxp_adc_etc_info
{
    ADC_ETC_Type *adc_base;
    const adc_etc_config_t *adc_etc_config;
};
#endif

typedef struct
{
    struct os_adc_device adc;
#if defined(ADC1_PERIPHERAL) || defined(ADC2_PERIPHERAL)
    struct nxp_adc_info *info;
#endif
}imxrt_adc;



int rt_hw_adc_init(void);

#endif /* DRV_ADC_H__ */

