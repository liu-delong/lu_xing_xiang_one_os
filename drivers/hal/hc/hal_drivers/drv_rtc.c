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
 * @brief       This file implements RTC driver for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <sys/time.h>
#include "time.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.rtc"
#include <drv_log.h>

#include "hc_rtc.h"
#include "hc_lpm.h"
#include "hc_gpio.h"
#include "drv_rtc.h"

static struct os_device rtc;

uint8_t Hex_To_ByteDec(uint8_t PuB_Dat)
{
    uint8_t dat;
    dat = (PuB_Dat / 16) * 10 + PuB_Dat % 16;
    return dat;
}

uint8_t ByteDec_To_Hex(uint8_t PuB_Dat)
{
    uint8_t dat;
    dat = (PuB_Dat / 10) * 16 + PuB_Dat % 10;
    return dat;
}

static int set_current_time(time_t time_stamp) 
{
    stc_rtc_time_t time;
    struct tm *p_tm;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return OS_ERROR;
    }

    time.u8Second = ByteDec_To_Hex(p_tm->tm_sec);
    time.u8Minute = ByteDec_To_Hex(p_tm->tm_min);
    time.u8Hour   = ByteDec_To_Hex(p_tm->tm_hour);
    time.u8Day    = ByteDec_To_Hex(p_tm->tm_mday);
    time.u8Month  = ByteDec_To_Hex(p_tm->tm_mon) + 1;
    time.u8Year   = ByteDec_To_Hex(p_tm->tm_year - 100);

    if(OS_EOK != Rtc_SetTime(&time)) 
    {
        LOG_EXT_E("set rtc time error.");
        return OS_ERROR;
    }

    return OS_EOK;
}

static time_t  get_current_time(void) 
{
    stc_rtc_time_t time;
    struct tm       tm_new;

    if(OS_EOK != Rtc_ReadDateTime(&time))
    {
        LOG_EXT_E("get rtc time error.");
        //return OS_ERROR;
    }

    tm_new.tm_sec  = Hex_To_ByteDec(time.u8Second);
    tm_new.tm_min  = Hex_To_ByteDec(time.u8Minute);
    tm_new.tm_hour = Hex_To_ByteDec(time.u8Hour);
    tm_new.tm_mday = Hex_To_ByteDec(time.u8Day);
    tm_new.tm_mon  = Hex_To_ByteDec(time.u8Month) - 1;
    tm_new.tm_year = Hex_To_ByteDec(time.u8Year) + 100;

    LOG_EXT_D("get rtc time.");
    return mktime(&tm_new);
}

os_err_t  os_rtc_init(os_device_t *dev)
{
    stc_rtc_initstruct_t RtcInitStruct;
    
    DDL_ZERO_STRUCT(RtcInitStruct);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralRtc,TRUE);
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);

    RtcInitStruct.rtcAmpm = RtcPm;
    RtcInitStruct.rtcClksrc = RtcClkRcl;
    RtcInitStruct.rtcTime.u8Second = 0x55;
    RtcInitStruct.rtcTime.u8Minute = 0x45;
    RtcInitStruct.rtcTime.u8Hour   = 0x16;
    RtcInitStruct.rtcTime.u8Day    = 0x17;
    RtcInitStruct.rtcTime.u8DayOfWeek = 0x03;
    RtcInitStruct.rtcTime.u8Month  = 0x04;
    RtcInitStruct.rtcTime.u8Year   = 0x19;
    RtcInitStruct.rtcCompen = RtcCompenEnable;
    RtcInitStruct.rtcCompValue = 0;
    Rtc_Init(&RtcInitStruct);
    EnableNvic(RTC_IRQn, IrqLevel3, FALSE);
    Rtc_Cmd(TRUE);

    return OS_EOK;
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);
    
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = get_current_time();
        LOG_EXT_D("RTC: get rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (set_current_time(*(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        LOG_EXT_D("RTC: set rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;
    }

    return result;
}

const static struct os_device_ops rtc_ops = {
    .init = os_rtc_init,
    .control = os_rtc_control,
};

static os_err_t os_hw_rtc_register(os_device_t *device, const char *name, os_uint32_t flag)
{
    OS_ASSERT(device != OS_NULL);

    Rtc_Cmd(TRUE);

    device->ops         = &rtc_ops;
    device->type        = OS_DEVICE_TYPE_RTC;
    device->cb_table[OS_DEVICE_CB_TYPE_RX].cb = OS_NULL;
    device->cb_table[OS_DEVICE_CB_TYPE_TX].cb = OS_NULL;
    device->user_data   = OS_NULL;

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
