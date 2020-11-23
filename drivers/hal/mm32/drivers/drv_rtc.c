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
 * @file        drv_rtc.c
 *
 * @brief       This file implements RTC driver for mm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <sys/time.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.rtc"
#include <drv_log.h>

#include "bsp.h"

#ifdef OS_USING_RTC

#define BKUP_REG_DATA 0x5050
// #define RTC_INT_ENABLE

struct mm32_rtc {

    os_device_t rtc;

    RTC_HandleTypeDef *hrtc;
};

static time_t mm32_rtc_get_timestamp(RTC_HandleTypeDef *hrtc)
{
    time_t tim;
    tim = RTC_GetCounter();
    LOG_EXT_D("get rtc time.");
    return tim;
}

static os_err_t mm32_rtc_set_time_stamp(RTC_HandleTypeDef *hrtc, time_t time_stamp)
{
    RTC_SetCounter(time_stamp);
    LOG_EXT_D("set rtc time.");   
    return OS_EOK;
}

static void mm32_rtc_init(RTC_HandleTypeDef *hrtc)
{   

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    if (BKP_ReadBackupRegister(BKP_DR1) != BKUP_REG_DATA)
    {
        BKP_DeInit();	
#if defined(BSP_RTC_USING_LSI)
        RCC_LSEConfig(RCC_LSE_OFF);	
        RCC_LSICmd(ENABLE);

        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)	
        {
            //wait for LSI ready
        }       
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);		
#elif defined(BSP_RTC_USING_LSE)
        RCC_LSEConfig(RCC_LSE_ON);	
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	
        {
            //wait for LSE ready
        }       
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		
#endif


        RCC_RTCCLKCmd(ENABLE);	
        RTC_WaitForLastTask();	
        RTC_WaitForSynchro();		
#ifdef RTC_INT_ENABLE
        RTC_ITConfig(RTC_IT_SEC, ENABLE);		
        RTC_WaitForLastTask();	
#endif

        RTC_EnterConfigMode();
        RTC_SetPrescaler(32767); 
        RTC_WaitForLastTask();	
        RTC_WaitForSynchro();	
        RTC_ExitConfigMode(); 

        BKP_WriteBackupRegister(BKP_DR1, BKUP_REG_DATA);	
    }
    else
    {
        RTC_WaitForSynchro();	
        RTC_ITConfig(RTC_IT_SEC, ENABLE);	
        RTC_WaitForLastTask();	
    }		
#ifdef RTC_INT_ENABLE
		NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;	    
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
    NVIC_Init(&NVIC_InitStructure);	
#endif		
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);

    struct mm32_rtc *m_rtc = (struct mm32_rtc *)dev;

    switch (cmd)
    {
        case OS_DEVICE_CTRL_RTC_GET_TIME:
            *(os_uint32_t *)args = mm32_rtc_get_timestamp(m_rtc->hrtc);
            LOG_EXT_D("RTC: get rtc_time %x\n", *(os_uint32_t *)args);
            result = OS_EOK;
            break;

        case OS_DEVICE_CTRL_RTC_SET_TIME:
            if (mm32_rtc_set_time_stamp(m_rtc->hrtc, *(os_uint32_t *)args))
            {
                result = OS_ERROR;
            }
            LOG_EXT_D("RTC: set rtc_time %x\n", *(os_uint32_t *)args);
            result = OS_EOK;
            break;
    }

    return result;
}
#ifdef RTC_INT_ENABLE
void RTC_IRQHandler(void)
{		
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
    {   
        RTC_ClearITPendingBit(RTC_IT_SEC);
        os_kprintf("rtc pps.\r\n");
    }
    if(RTC_GetITStatus(RTC_IT_ALR) != RESET) 
    {
        RTC_ClearITPendingBit(RTC_IT_ALR);	
        os_kprintf("rtc alarm.\r\n");
    }
    RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_OW);
    RTC_WaitForLastTask();
}
#endif
const static struct os_device_ops rtc_ops = {
    .control = os_rtc_control,
};

static int mm32_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct mm32_rtc *m_rtc;

    m_rtc = os_calloc(1, sizeof(struct mm32_rtc));

    OS_ASSERT(m_rtc);

    m_rtc->hrtc = (RTC_HandleTypeDef *)dev->info;

    mm32_rtc_init(m_rtc->hrtc);

    os_device_default(&m_rtc->rtc, OS_DEVICE_TYPE_RTC);

    m_rtc->rtc.ops     = &rtc_ops;

    return os_device_register(&m_rtc->rtc, dev->name, OS_DEVICE_FLAG_RDWR);
}

OS_DRIVER_INFO mm32_rtc_driver = {
    .name   = "RTC_Type",
    .probe  = mm32_rtc_probe,
};

OS_DRIVER_DEFINE(mm32_rtc_driver, "1");

#endif
