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
 * @brief       This file implements adc driver for FM33A0xx.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>

#if defined(BSP_USING_ADC1)
#include "drv_adc.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.adc"
#include "os_dbg_ext.h"

#include <string.h>
static FM_HandleTypeDef adc_config[] = {
#ifdef BSP_USING_ADC1
    ADC1_CONFIG,
#endif
};

struct fm_adc
{
    FM_HandleTypeDef    ADC_Handler;
    struct os_adc_device fm_adc_device;
};

#define ADC_NUM (sizeof(adc_config) / sizeof(adc_config[0]))

static struct fm_adc fm_adc_obj[ADC_NUM];

static FM_ADC_IO_CFG fm_adc_io_cfg[] = 
{
    {CH_IN1, GPIOC, GPIO_Pin_12, AnalogIO, GPIO_ANASEL_PC12ANS_ADC_IN1,GPIO_ANASEL_PC12ANS_Set},
    {CH_IN2, GPIOC, GPIO_Pin_13, AnalogIO, GPIO_ANASEL_PC13ANS_ADC_IN2,GPIO_ANASEL_PC12ANS_Set},
    {CH_IN3, GPIOD, GPIO_Pin_0,   AnalogIO, 0,  NULL},
    {CH_IN4, GPIOD, GPIO_Pin_1,   AnalogIO, 0,  NULL},
    {CH_IN5, GPIOF, GPIO_Pin_6,   AnalogIO, 0,  NULL},
    {CH_IN6, GPIOC, GPIO_Pin_15, AnalogIO, GPIO_ANASEL_PC15ANS_ACMP2_INP0,GPIO_ANASEL_PC15ANS_Set},
    {CH_IN7, GPIOB, GPIO_Pin_2,   AnalogIO, 0,  NULL},
    {CH_IN8, GPIOB, GPIO_Pin_3,   AnalogIO, 0,  NULL},
};

void fm_adc_io_func(os_uint32_t channel)
{
    os_uint16_t i = 0;
    os_uint16_t num = 0;
    FM_ADC_IO_CFG *fm_adc_io;

    num = sizeof(fm_adc_io_cfg)/sizeof(fm_adc_io_cfg[0]);
    for (i = 0; i < num; i++)
    {
        if (fm_adc_io_cfg[i].channel == channel)
        {
            fm_adc_io = &fm_adc_io_cfg[i];
            break;
        }
    }

    if (i == num)
    {
        return;
    }

    if (fm_adc_io->io_func != NULL) 
    {
        fm_adc_io->io_func(fm_adc_io->GPIOx, fm_adc_io->GPIOx_pin);
    }

    if (fm_adc_io->ans_set != NULL) 
    {
        fm_adc_io->ans_set(fm_adc_io->adc_in);
    }

}

static os_err_t fm_adc_enabled(struct os_adc_device *device, os_uint32_t channel, os_bool_t enabled)
{
    OS_ASSERT(device != OS_NULL);

    if (enabled)
    {
        RCC_PERCLK_SetableEx(ANACCLK, ENABLE);      /* Analog circuit bus clock enable */
        RCC_PERCLK_SetableEx(ADCCLK, ENABLE);	       /* ADC clock enbale */
    }
    else
    {
        RCC_PERCLK_SetableEx(ANACCLK, DISABLE);
        RCC_PERCLK_SetableEx(ADCCLK, DISABLE);	
    }

    return OS_EOK;
}

os_uint8_t ADC_Wait_Finish(void)
{	
	Do_DelayStart();	
	{
		if(SET == ANAC_ADCIF_ADC_IF_Chk()) return 0;		
	}While_DelayMsEnd(8*clkmode);   /* wait 8ms */
	
	return 1;   /* timeout */
}


static os_err_t fm_get_adc_value(struct os_adc_device *device, os_uint32_t channel, os_uint32_t *value)
{
    FM_HandleTypeDef *fm_adc_handler;
    os_uint32_t volt; 
    OS_ASSERT(device != OS_NULL);
    fm_adc_handler = device->parent.user_data;

    fm_adc_io_func(channel);

    RCC_PERCLKCON2_ADCCKSEL_Set(fm_adc_handler->clk_value);

    ANAC_ADC_Channel_SetEx(channel);			
    ANAC_ADCCON_ADC_IE_Setable(DISABLE);
    ANAC_ADCCON_ADC_EN_Setable(ENABLE);

    if(1 == ADC_Wait_Finish())
    {
        return OS_ERROR;
    }
    
    volt = ANAC_ADCDATA_Read();
    ANAC_ADCIF_ADC_IF_Clr();
    if (CH_PTAT == channel)
    {
        *value = FM_ADC_TEMP_FACTOR * ANAC_ADC_TemperatureCalc(volt, FM_ADC_REF_VOLT);
    }
    else 
    {
        *value = ANAC_ADC_VoltageCalc2(volt, FM_ADC_REF_VOLT);
    }

    return OS_EOK;
}

static const struct os_adc_ops stm_adc_ops = {
    .enabled = fm_adc_enabled,
    .convert = fm_get_adc_value,
};

/**
 ***********************************************************************************************************************
 * @brief           fm_adc_init:init fm33a0xx adc.
 *
 * @param[in]       none
 *
 * @return          [Return adc init status.
 * @retval          OS_EOK         init success.
 * @retval          Others         init failed.
 ***********************************************************************************************************************
 */
static int fm_adc_init(void)
{
    int result = OS_EOK;
    /* save adc name */
    char name_buf[5] = {'a', 'd', 'c', '0', 0};
    int  i           = 0;

    for (i = 0; i < sizeof(adc_config) / sizeof(adc_config[0]); i++)
    {
        /* ADC init */
        name_buf[3] = '0';
        fm_adc_obj[i].ADC_Handler = adc_config[i];
        
        if (fm_adc_obj[i].ADC_Handler.Instance == ANAC)
        {
            name_buf[3] = '1';
        }

        /* register ADC device */
        if (os_hw_adc_register(&fm_adc_obj[i].fm_adc_device,
                               name_buf,
                               &stm_adc_ops,
                               &fm_adc_obj[i].ADC_Handler) == OS_EOK)
        {
            LOG_EXT_D("%s init success", name_buf);
        }
        else
        {
            LOG_EXT_E("%s register failed", name_buf);
            result = OS_ERROR;
        }
    }

    return result;
}
OS_BOARD_INIT(fm_adc_init);

#endif /* BSP_USING_ADC */
