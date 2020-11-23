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

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.adc"
#include <drv_log.h>

//#define ADC_REGULAR_TEST
#define STM32_ADC_MAX_NUM 3

#ifdef ADC_REGULAR_TEST
os_uint16_t adc_test_buf[30];
#endif

struct stm32_adc_info
{
    os_uint8_t adc_num;
    os_uint8_t channel_num_max;
    os_uint8_t regular_channel_num;
    
    os_uint16_t *adc_convbuf;
    os_uint32_t *adc_valuebuf;
};

struct stm32_adc
{
    struct os_adc_device adc;
    ADC_HandleTypeDef *hadc[STM32_ADC_MAX_NUM];
};

static struct stm32_adc_info adc_info = {0, 0, 0, OS_NULL, OS_NULL};
struct stm32_adc *st_adc = OS_NULL;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    os_interrupt_enter();
    os_hw_adc_isr(&st_adc->adc, OS_ADC_EVENT_CONVERT_DONE);
    os_interrupt_leave();
}

__weak HAL_StatusTypeDef HAL_ADCEx_MultiModeStart_DMA(ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length)
{
    return HAL_ADC_Start_DMA(hadc, pData, Length);
}

__weak HAL_StatusTypeDef HAL_ADCEx_MultiModeStop_DMA(ADC_HandleTypeDef* hadc)
{
    return HAL_ADC_Stop_DMA(hadc);
}

/**
 ***********************************************************************************************************************
 * @brief           stm32_adc_poll_convert_then_read: start adc convert in poll
 *
 * @details         channel and order config in stm32cubeMX,"channell" is mapping of rank configed in cube,
 *
 * @attention       Attention_description_Optional
 *
 ***********************************************************************************************************************
 */
static os_err_t stm32_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{  
    os_uint8_t adc_mode = 0;
    struct stm32_adc *dev_adc;
    
    OS_ASSERT(dev != OS_NULL);

    dev_adc = os_container_of(dev, struct stm32_adc, adc);
    
    for (os_uint8_t i = 0; i < STM32_ADC_MAX_NUM;i++)
    {
        if (dev_adc->hadc[i] != OS_NULL)
        {
            if (dev_adc->hadc[i]->DMA_Handle != OS_NULL)
            {
                adc_mode = 1;
            }
#ifdef ADC_FLAG_OVR
            if (__HAL_ADC_GET_FLAG(dev_adc->hadc[i], ADC_FLAG_OVR))
            {
                LOG_EXT_E("adc conversion time or clock is too fast!\n");
                return OS_ERROR;
            }
#endif
        }
    }

    if (adc_mode == 1)
    {
        if ((adc_info.adc_num == 0x01) && (channel > adc_info.regular_channel_num - 1))
        {
            LOG_EXT_E("adc channel is invalid! check adc channel and dma config\n");
            return OS_ENOSYS;
        }
        else if (channel > adc_info.regular_channel_num * 2 - 1)
        {
            LOG_EXT_E("adc channel is invalid! check adc channel and dma config\n");
            return OS_ENOSYS;
        }
        *buff = (os_int32_t)((os_uint64_t)adc_info.adc_convbuf[channel] * dev->mult >> dev->shift);
        return OS_EOK;
    }
    else
    {
        if ((channel < adc_info.adc_num) && (dev_adc->hadc[channel] != OS_NULL))
        {
            HAL_ADC_Start(dev_adc->hadc[channel]);
            if (HAL_ADC_PollForConversion(dev_adc->hadc[channel], 500) != HAL_OK)
            {
                return OS_ERROR;
            }
            *buff = (os_int32_t)((os_uint64_t)HAL_ADC_GetValue(dev_adc->hadc[channel]) * dev->mult >> dev->shift);
            HAL_ADC_Stop(dev_adc->hadc[channel]);
            return OS_EOK;
        }
        else
        {
            LOG_EXT_E("adc channel is invalid! check adc channel and dma config\n");
            return OS_ENOSYS; 
        }
    }
}

static os_err_t stm32_adc_open(struct os_adc_device *dev)
{
    HAL_StatusTypeDef  ret = HAL_OK;
    struct stm32_adc *dev_adc;

    dev_adc = os_container_of(dev, struct stm32_adc, adc);
    
    adc_info.regular_channel_num = adc_info.adc_num * adc_info.channel_num_max;
#ifdef ADC_REGULAR_TEST
    adc_info.adc_convbuf = adc_test_buf;
#else
    if (adc_info.adc_convbuf == OS_NULL)
    {
       adc_info.adc_convbuf = os_calloc(1, 2 * adc_info.regular_channel_num); 
    }
#endif
    
    for (os_int8_t i = STM32_ADC_MAX_NUM - 1; i >= 0;i--)
    {
        if (dev_adc->hadc[i] != OS_NULL)
        {
            if (dev_adc->hadc[i]->DMA_Handle != OS_NULL)
            {
                if (adc_info.adc_num != 0x01)
                {
                    ret = HAL_ADCEx_MultiModeStart_DMA(dev_adc->hadc[i], (uint32_t *)adc_info.adc_convbuf, adc_info.regular_channel_num);
                }
                else
                {
                    ret = HAL_ADC_Start_DMA(dev_adc->hadc[i], (uint32_t *)adc_info.adc_convbuf, adc_info.regular_channel_num);
                } 
            }
            else
            {
                ret = HAL_ADC_Start(dev_adc->hadc[i]); 
            }
            
            if (ret != HAL_OK)
            {
                return OS_ERROR;
            } 
        }
    }   
    return OS_EOK;
}

static os_err_t stm32_adc_close(struct os_adc_device *dev)
{
    HAL_StatusTypeDef  ret = HAL_OK;
    
    struct stm32_adc *dev_adc;

    dev_adc = os_container_of(dev, struct stm32_adc, adc);

    for (os_uint8_t i = 0; i < STM32_ADC_MAX_NUM;i++)
    {
        if (dev_adc->hadc[i] != OS_NULL)
        {
            if (dev_adc->hadc[i]->DMA_Handle != OS_NULL)
            {
                if (adc_info.adc_num != 0x01)
                {
                    ret = HAL_ADCEx_MultiModeStop_DMA(dev_adc->hadc[i]);
                }
                else
                {
                    ret = HAL_ADC_Stop_DMA(dev_adc->hadc[i]);
                }        
            }
            else
            {
                ret = HAL_ADC_Stop(dev_adc->hadc[i]);
            }
            if (ret != HAL_OK)
            {
                return OS_ERROR;
            }
        }
    }
    
    return OS_EOK;
}

static os_err_t stm32_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    if (!enable)
    {
        return stm32_adc_close(dev);
    }
    else
    {
        return stm32_adc_open(dev);
    }
}

static os_err_t stm32_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}

static os_err_t stm32_adc_probe_check(struct os_adc_device *adc, ADC_HandleTypeDef *hadc)
{
    os_uint32_t value = 0;

#ifdef ADC_DATAALIGN_RIGHT
    if (hadc->Init.DataAlign != ADC_DATAALIGN_RIGHT)
    {
        LOG_EXT_E("adc dataalign must be right-align!\n");
        return OS_ERROR;
    }
#endif
    
#ifdef ADC_RESOLUTION_12B    
    switch(hadc->Init.Resolution)
    {
#ifdef ADC_RESOLUTION_10B
    case ADC_RESOLUTION_10B:
        value = (1UL << 10) - 1;
        break;
#endif
#ifdef ADC_RESOLUTION_12B
    case ADC_RESOLUTION_12B:
        value = (1UL << 12) - 1;
        break;
#endif
#ifdef ADC_RESOLUTION_14B
    case ADC_RESOLUTION_14B:
        value = (1UL << 14) - 1;
        break;
#endif
#ifdef ADC_RESOLUTION_16B
    case ADC_RESOLUTION_16B:
        value = (1UL << 16) - 1;
        break;
#endif
    default:
        LOG_EXT_E("adc resolution just support 10/12/14/16bits!\n");
        return OS_ERROR;
    }
#else
    value = (1UL << 12) - 1;
#endif

    if (adc->max_value == 0)
    {
        adc->max_value = value;
    }
    else
    {
        if (adc->max_value != value)
        {
            LOG_EXT_E("all adc must use same resolution in 10/12/14/16bits!\n");
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

static const struct os_adc_ops stm32_adc_ops = {
    .adc_enabled            = stm32_adc_enabled,
    .adc_control            = stm32_adc_control,
    .adc_read               = stm32_adc_read,
};

static int stm32_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;

    if (st_adc == OS_NULL)
    {
        st_adc = os_calloc(1, sizeof(struct stm32_adc)); 
        OS_ASSERT(st_adc);
        
        st_adc->hadc[0] = OS_NULL;
        st_adc->hadc[1] = OS_NULL;
        st_adc->hadc[2] = OS_NULL;
        adc_info.adc_num = 0;
        adc_info.regular_channel_num = 0;

        struct os_adc_device *dev_adc = &st_adc->adc;
        dev_adc->ops    = &stm32_adc_ops;
        dev_adc->max_value = 0;
        dev_adc->ref_low   = 0;                 /* ref 0 - 3.3v */
        dev_adc->ref_hight = 3300;
        
        result = stm32_adc_probe_check(&st_adc->adc, (ADC_HandleTypeDef *)dev->info);
        result = os_hw_adc_register(dev_adc,
                                    "adc",
                                    OS_DEVICE_FLAG_RDWR,
                                    NULL);
    }
    
    for (os_uint8_t i = 0; i < STM32_ADC_MAX_NUM;i++)
    {
        if (st_adc->hadc[i] == OS_NULL)
        {
            st_adc->hadc[i] = (ADC_HandleTypeDef *)dev->info;
            adc_info.adc_num++;
#if defined(__STM32F0xx_H) && defined(ADC_CHSELR_CHSEL0)
            for (int j = 0; j < 32;j ++)
            {
                if ((st_adc->hadc[i]->Instance->CHSELR >> j) & 0x01)
                {
                    adc_info.channel_num_max++;
                }
            }
#else
            if (adc_info.channel_num_max < st_adc->hadc[i]->Init.NbrOfConversion)
            {
                adc_info.channel_num_max = st_adc->hadc[i]->Init.NbrOfConversion;
            }
#endif
            result = stm32_adc_probe_check(&st_adc->adc, st_adc->hadc[i]);
            OS_ASSERT(result == OS_EOK);
            break;
        } 
    }

    return OS_EOK;
}

OS_DRIVER_INFO stm32_adc_driver = {
    .name   = "ADC_HandleTypeDef",
    .probe  = stm32_adc_probe,
};

OS_DRIVER_DEFINE(stm32_adc_driver,"2");
