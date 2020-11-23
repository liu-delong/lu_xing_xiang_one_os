/**
*******************************************************************************
****************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        drv_rtc.c
 *
 * @brief       This file implements RTC driver for FM33A0xx.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version

*******************************************************************************
****************************************
 */

#include "board.h"
#include <sys/time.h>
#ifdef BSP_USING_ONCHIP_RTC

#include <drv_log.h>
#include "drv_rtc.h"


static struct os_device rtc;

os_rtc_para rtc_para;

os_uint32_t RTC_BAK0_Read(void)
{
    return (RTC_BAK->BAK0);
}

void RTC_BAK0_Write(os_uint32_t value)
{
    RTC_BAK->BAK0 = value;
}


void RTC_Init(void)
{
    RCC_PERCLK_SetableEx(RTCCLK, ENABLE);

    RTC_FSEL_FSEL_Set(RTC_FSEL_FSEL_PLL1HZ);
    RTC_Trim_Proc(0);						
    RTC_PR1SEN_PR1SEN_Setable(DISABLE);		

    RTC_STAMPEN_STAMP0EN_Setable(DISABLE);
    RTC_STAMPEN_STAMP1EN_Setable(DISABLE);

    RTC_RTCIE_SetableEx(DISABLE, 0xFFFFFFFF);
}

os_err_t RTC_GetRTC(RTC_TimeDateTypeDef* para)
{
    os_int32_t n, i;
    os_err_t Result = OS_ERROR;

    RTC_TimeDateTypeDef TempTime1,TempTime2;

    for(n=0 ; n<3; n++)
    {
        RTC_TimeDate_GetEx(&TempTime1);
        RTC_TimeDate_GetEx(&TempTime2);

        for(i=0; i<7; i++)
        {
            if(((os_uint8_t*)(&TempTime1))[i] != ((os_uint8_t*)(&TempTime2))[i])
                break;
        }

        if(i == 7)
        {
            Result = OS_EOK;
            memcpy((os_uint8_t*)(para), (os_uint8_t*)(&TempTime1), 7);
            break;
        }
    }
    return Result;
}

os_err_t RTC_SetRTC(RTC_TimeDateTypeDef* para)
{
    os_int32_t n;
    os_err_t Result = OS_EOK;
    RTC_TimeDateTypeDef TempTime1;

    for(n=0 ; n<3; n++)
    {
        RTC_RTCWE_Write(RTC_WRITE_ENABLE);	
        RTC_TimeDate_SetEx(para);			
        RTC_RTCWE_Write(RTC_WRITE_DISABLE);	

        Result = RTC_GetRTC(&TempTime1);	
    }
    return Result;
}

static os_err_t get_rtc_timestamp(time_t *lt)
{
    struct tm       tm_new;
    RTC_TimeDateTypeDef para;
    os_err_t ret;

    ret = RTC_GetRTC(&para);
    if (ret == OS_ERROR)
    {
        LOG_EXT_D("get rtc time error");
        return OS_ERROR;
    }

    tm_new.tm_sec  = (int)para.Second;
    tm_new.tm_min  = (int)para.Minute;
    tm_new.tm_hour = (int)para.Hour;
    tm_new.tm_mday = (int)para.Date;
    tm_new.tm_mon  = (int)para.Month- 1;
    tm_new.tm_year = (int)para.Year+ 100;

    LOG_EXT_D("get rtc time.");

    *lt = mktime(&tm_new);

    return OS_EOK;
}


static os_err_t set_rtc_time_stamp(time_t time_stamp)
{
    RTC_TimeDateTypeDef para;
    struct tm *p_tm;
    os_err_t ret;

    p_tm = localtime(&time_stamp);

    para.Second = (os_uint8_t)p_tm->tm_sec;
    para.Minute = (os_uint8_t)p_tm->tm_min;
    para.Hour = (os_uint8_t)p_tm->tm_hour;
    para.Date = (os_uint8_t)p_tm->tm_mday;
    para.Month = (os_uint8_t)p_tm->tm_mon + 1;
    para.Year = (os_uint8_t)p_tm->tm_year - 100;
    para.Week = (os_uint8_t)p_tm->tm_wday;

    ret = RTC_SetRTC(&para);
    if (ret == OS_ERROR)
    {
        LOG_EXT_D("set rtc time, error");
        return OS_ERROR;
    }

    LOG_EXT_D("set rtc time.");

    RTC_BAK0_Write(BKUP_REG_DATA);

    return OS_EOK;
}

static void os_rtc_init(void)
{
}

static os_err_t os_rtc_config(struct os_device *dev)
{
    if (BKUP_REG_DATA == RTC_BAK0_Read())
    {
        RTC_Init();
    }
    return OS_EOK;
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    time_t time_stamp;
    OS_ASSERT(dev != OS_NULL);
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        result = get_rtc_timestamp(&time_stamp);
        if (result == OS_EOK)
        {
            *(os_uint32_t *)args = time_stamp;
            LOG_EXT_D("RTC: get rtc_time %x\n", *(os_uint32_t *)args);
        }
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        result = set_rtc_time_stamp(*(os_uint32_t *)args);
        if (result == OS_EOK)
        {
            LOG_EXT_D("RTC: set rtc_time %x\n", *(os_uint32_t *)args);
        }
        break;
    default:
        break;

    }

    return result;
}


const static struct os_device_ops rtc_ops =
{
    .control = os_rtc_control
};

static os_err_t os_hw_rtc_register(os_device_t *device, const char *name, os_uint32_t flag)
{
    OS_ASSERT(device != OS_NULL);

    os_rtc_init();
    if (os_rtc_config(device) != OS_EOK)
    {
        return OS_ERROR;
    }

    device->ops         = &rtc_ops;

    device->type        = OS_DEVICE_TYPE_RTC;
    device->cb_table[OS_DEVICE_CB_TYPE_RX].cb = OS_NULL;
    device->cb_table[OS_DEVICE_CB_TYPE_TX].cb = OS_NULL;
    device->user_data   = &rtc_para;

    /* register a character device */
    return os_device_register(device, name, flag);
}

int os_hw_rtc_init(void)
{
    os_err_t result;
    result = os_hw_rtc_register(&rtc, "rtc", OS_DEVICE_FLAG_RDWR);
    if (result != OS_EOK)
    {
        LOG_EXT_E("rtc register err code: %d", result);
        return result;
    }
    LOG_EXT_D("rtc init success");
    return OS_EOK;
}
OS_DEVICE_INIT(os_hw_rtc_init);

#endif /* BSP_USING_ONCHIP_RTC */

