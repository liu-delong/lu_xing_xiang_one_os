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
 * @file        adc_config.h
 *
 * @brief       Provide configuration information used by ADC
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_ADC_H__
#define __DRV_ADC_H__

#if defined(BSP_USING_ADC1)

#include <os_task.h>
#include "define_all.h"  
#include "drv_gpio.h"
#include "drv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FM_ADC_TEMP_FACTOR    100
#define FM_ADC_REF_VOLT             5

typedef struct _tag_FM_ADC_IO_CFG
{
    os_uint32_t channel;
    GPIOx_Type *GPIOx;
    os_uint32_t GPIOx_pin;
     void (*io_func)(GPIOx_Type *GPIOx_Num, os_uint32_t GPIOx_PinNum) ;
    os_uint32_t adc_in;
     void (*ans_set)(uint32_t ADC_value) ;
}FM_ADC_IO_CFG;


typedef struct _tag_FM_HandleTypeDef
{
    ANAC_Type *Instance;
    os_uint32_t clk_value;
}FM_HandleTypeDef;

#ifdef BSP_USING_ADC1
#ifndef ADC1_CONFIG
#define ADC1_CONFIG                          \
    {                                                         \
       .Instance     = ANAC,                         \
       .clk_value    = RCC_PERCLKCON2_ADCCKSEL_RCHFDIV16,      \
    }
#endif /* ADC1_CONFIG */
#endif /* BSP_USING_ADC1 */

#ifdef __cplusplus
}
#endif

#endif  /* BSP_USING_ADC1 */

#endif /* __DRV_ADC_H__ */

