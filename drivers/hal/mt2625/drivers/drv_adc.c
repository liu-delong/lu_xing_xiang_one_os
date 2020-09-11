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
 * @brief       This file implements adc driver for stm32.
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
#include "hal_adc.h"
#include "hal_platform.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.adc"
#include <drv_log.h>

struct mt_adc
{
    struct os_adc_device adc;
    ADC_HandleTypeDef *hadc;
};

static os_uint32_t adc_sample_to_voltage(uint32_t source_code)
{
    os_uint32_t voltage = (source_code * 1400) / 4095;
    
    return voltage;
}


/**
 ***********************************************************************************************************************
 * @brief           mt_adc_poll_convert_then_read: start adc convert in poll
 *
 * @details         channel and order config in stm32cubeMX,"channell" is mapping of rank configed in cube,
 *
 * @attention       Attention_description_Optional
 *
 ***********************************************************************************************************************
 */
static os_err_t mt_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{  
    struct mt_adc *dev_adc;
    hal_adc_status_t ret;
    os_uint32_t value = 0;
    
    OS_ASSERT(dev != OS_NULL);

    dev_adc = os_container_of(dev, struct mt_adc, adc);

    ret = hal_adc_get_data_polling(dev_adc->hadc->channel, (uint32_t *)&value);
    if (ret != HAL_ADC_STATUS_OK)
    {
        LOG_EXT_E("read [%s], ret[%d]", dev->parent.parent.name, ret);
        return OS_ERROR;
    }
    
    *buff = adc_sample_to_voltage(value);
        
    os_kprintf("[%s]-[%d],  value[%d]\r\n", __FILE__,  __LINE__, *buff);

    return OS_EOK;
}

static os_err_t mt_adc_open(struct os_adc_device *dev)
{
    struct mt_adc *dev_adc;
    int ret;

    dev_adc = os_container_of(dev, struct mt_adc, adc);

    ret = hal_gpio_init(dev_adc->hadc->gpio_pio);

    ret |= hal_pinmux_set_function(dev_adc->hadc->gpio_pio, 5);
    ret |= hal_adc_init();
    if (ret != 0)
    {
        LOG_EXT_E("read adc, ret[%d]", dev->parent.parent.name, ret);
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t mt_adc_close(struct os_adc_device *dev)
{
    hal_adc_status_t ret;

    ret = hal_adc_deinit();    
    if (ret != 0)
    {
        LOG_EXT_E("read adc, ret[%d]", dev->parent.parent.name, ret);
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t mt_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    if (!enable)
    {
        return mt_adc_close(dev);
    }
    else
    {
        return mt_adc_open(dev);
    }
}

static os_err_t mt_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}

static const struct os_adc_ops mt_adc_ops = {
    .adc_enabled            = mt_adc_enabled,
    .adc_control            = mt_adc_control,
    .adc_read               = mt_adc_read,
};

static int mt_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    os_err_t    result  = 0;

    struct mt_adc *mt_adc = os_calloc(1, sizeof(struct mt_adc));

    OS_ASSERT(mt_adc);

    mt_adc->hadc = (ADC_HandleTypeDef *)dev->info;
    
    struct os_adc_device *adc = &mt_adc->adc;

    adc->ops   = &mt_adc_ops;
    
    result = os_hw_adc_register(adc, dev->name, OS_DEVICE_FLAG_RDWR, NULL);
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO mt_adc_driver = {
    .name   = "ADC_HandleTypeDef",
    .probe  = mt_adc_probe,
};

OS_DRIVER_DEFINE(mt_adc_driver,"2");
