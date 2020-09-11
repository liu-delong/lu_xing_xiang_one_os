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
 * @brief       This file implements adc driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <os_task.h>

#ifdef BSP_USING_ADC

#if !defined(BSP_USING_ADC1) && !defined(BSP_USING_ADC2)
#error "Please define at least one BSP_USING_ADCx"
#endif

#if (defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
#error "Please don't define 'FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL'!"
#endif

#define LOG_TAG             "drv.adc"
#include <drv_log.h>
#include "drv_adc.h"
#include "fsl_adc.h"
#include <os_device.h>

static os_err_t imxrt_hp_adc_enabled(struct rt_adc_device *device, os_uint32_t channel, os_bool_t enabled)
{
    return OS_EOK;
}

static os_err_t imxrt_hp_adc_convert(struct rt_adc_device *device, os_uint32_t channel, os_uint32_t *value)
{
    adc_channel_config_t adc_channel;
    ADC_Type *base;
    base = (ADC_Type *)(device->parent.user_data);

    adc_channel.channelNumber = channel;
    adc_channel.enableInterruptOnConversionCompleted = false;

    ADC_SetChannelConfig(base, 0, &adc_channel);

    while (0U == ADC_GetChannelStatusFlags(base, 0))
    {
        continue;
    }

    *value = ADC_GetChannelConversionValue(base, 0);

    return OS_EOK;
}

static struct rt_adc_ops imxrt_adc_ops =
{
    .enabled = imxrt_hp_adc_enabled,
    .convert = imxrt_hp_adc_convert,
};

#if defined(BSP_USING_ADC1)

static adc_config_t ADC1_config_value;
static struct rt_adc_device adc1_device;

#endif  /* BSP_USING_ADC1 */

#if defined(BSP_USING_ADC2)

static adc_config_t ADC2_config_value;
static struct rt_adc_device adc2_device;

#endif  /* BSP_USING_ADC2 */

int rt_hw_adc_init(void)
{
    int result = OS_EOK;

#if defined(BSP_USING_ADC1)

    ADC_GetDefaultConfig(&ADC1_config_value);
    ADC_Init(ADC1, &ADC1_config_value);

#if !(defined(FSL_FEATURE_ADC_SUPPORT_HARDWARE_TRIGGER_REMOVE) && FSL_FEATURE_ADC_SUPPORT_HARDWARE_TRIGGER_REMOVE)
    ADC_EnableHardwareTrigger(ADC1, false);
#endif
    ADC_DoAutoCalibration(ADC1);

    result = rt_hw_adc_register(&adc1_device, "adc1", &imxrt_adc_ops, ADC1);

    if (result != OS_EOK)
    {
        LOG_E("register adc1 device failed error code = %d\n", result);
    }

#endif /* BSP_USING_ADC1 */

#if defined(BSP_USING_ADC2)

    ADC_GetDefaultConfig(&ADC2_config_value);
    ADC_Init(ADC2, &ADC2_config_value);

#if !(defined(FSL_FEATURE_ADC_SUPPORT_HARDWARE_TRIGGER_REMOVE) && FSL_FEATURE_ADC_SUPPORT_HARDWARE_TRIGGER_REMOVE)
    ADC_EnableHardwareTrigger(ADC2, false);
#endif
    ADC_DoAutoCalibration(ADC2);

    result = rt_hw_adc_register(&adc2_device, "adc2", &imxrt_adc_ops, ADC2);

    if (result != OS_EOK)
    {
        LOG_E("register adc2 device failed error code = %d\n", result);
    }

#endif /* BSP_USING_ADC2 */

    return result;
}

INIT_DEVICE_EXPORT(rt_hw_adc_init);

#endif /* BSP_USING_ADC */

