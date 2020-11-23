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
#include <drv_log.h>
#include <hc_gpio.h>
#include <hc_adc.h>
#include <hc_bgr.h>

#define HC32_ADC_MAX_CHANNEL 1

struct adc_channel_info{
	en_gpio_port_t port;
	en_gpio_pin_t pin;
	en_adc_sqr_chmux_t mux;
	en_adc_samp_ch_sel_t chn;
};

struct hc32_adc
{
    struct os_adc_device adc;
	
    struct adc_channel_info channel[HC32_ADC_MAX_CHANNEL];
};

struct hc32_adc adcs = {
	.channel = {
#if (HC32_ADC_MAX_CHANNEL >= 1)
		{GpioPortA,GpioPin0,AdcSQRCH0MUX,AdcExInputCH0},
#if (HC32_ADC_MAX_CHANNEL >= 2)
		{GpioPortA,GpioPin1,AdcSQRCH1MUX,AdcExInputCH1},
#if (HC32_ADC_MAX_CHANNEL >= 3)
		{GpioPortA,GpioPin2,AdcSQRCH2MUX,AdcExInputCH2},
#endif
#endif
#endif
	},
};

static os_err_t hc32_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{  
    struct hc32_adc *dev_adc;
    
	if(channel >= HC32_ADC_MAX_CHANNEL)
	{		
		LOG_EXT_E("adc channel %d cannot find!\n", channel);
		return OS_ERROR;
	}
		
    dev_adc = os_container_of(dev, struct hc32_adc, adc);
    

	Adc_SQR_Start();

	while(FALSE == Adc_GetIrqStatus(AdcMskIrqSqr));

	*buff = (os_int32_t)((os_uint64_t)Adc_GetSqrResult(dev_adc->channel[channel].mux) * dev->mult >> dev->shift);
	
    Adc_SQR_Stop();
	
    return OS_EOK;
}

static os_err_t hc32_adc_open(struct os_adc_device *dev)
{
	Adc_SQR_Start();
    return OS_EOK;
}

static os_err_t hc32_adc_close(struct os_adc_device *dev)
{
	Adc_SQR_Stop();
    
    return OS_EOK;
}

static os_err_t hc32_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    if (!enable)
    {
        return hc32_adc_close(dev);
    }
    else
    {
        return hc32_adc_open(dev);
    }
}

static os_err_t hc32_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}

static const struct os_adc_ops hc32_adc_ops = {
    .adc_enabled            = hc32_adc_enabled,
    .adc_control            = hc32_adc_control,
    .adc_read               = hc32_adc_read,
};

static inline void __os_hw_adc_init(void)
{
	
	os_uint32_t chn;
    stc_adc_cfg_t              stcAdcCfg;
    stc_adc_sqr_cfg_t          stcAdcSqrCfg;

	Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
	for(chn=0;chn<HC32_ADC_MAX_CHANNEL;chn++)
	{
		Gpio_SetAnalogMode(adcs.channel[chn].port, adcs.channel[chn].pin);
	}
    
    DDL_ZERO_STRUCT(stcAdcCfg);
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralAdcBgr, TRUE); 
    
    Bgr_BgrEnable();
    
    stcAdcCfg.enAdcMode         = AdcScanMode;
    stcAdcCfg.enAdcClkDiv       = AdcMskClkDiv1;
    stcAdcCfg.enAdcSampCycleSel = AdcMskSampCycle12Clk;      
    stcAdcCfg.enAdcRefVolSel    = AdcMskRefVolSelAVDD;      
    stcAdcCfg.enAdcOpBuf        = AdcMskBufDisable;
    stcAdcCfg.enInRef           = AdcMskInRefDisable;
    stcAdcCfg.enAdcAlign        = AdcAlignRight;
    Adc_Init(&stcAdcCfg);

    
    stcAdcSqrCfg.bSqrDmaTrig = FALSE;
    stcAdcSqrCfg.enResultAcc = AdcResultAccDisable;
    stcAdcSqrCfg.u8SqrCnt    = HC32_ADC_MAX_CHANNEL;
    Adc_SqrModeCfg(&stcAdcSqrCfg);

	for(chn=0;chn<HC32_ADC_MAX_CHANNEL;chn++){
		
	    Adc_CfgSqrChannel(adcs.channel[chn].mux, adcs.channel[chn].chn);

	}	    

}

 int os_hw_adc_init(void)
{
    os_err_t result = OS_ERROR;
	struct os_adc_device *dev_adc = &adcs.adc;

	__os_hw_adc_init();

    dev_adc->ops    = &hc32_adc_ops;
    dev_adc->max_value = (1UL << 12) - 1;
    dev_adc->ref_low   = 0;                 /* ref 0 - 3.3v */
    dev_adc->ref_hight = 3300;
    
    result = os_hw_adc_register(dev_adc,
                                "adc",
                                OS_DEVICE_FLAG_RDWR,
                                NULL);
    
    return result;
}

	
//OS_DEVICE_INIT(os_hw_adc_init);

