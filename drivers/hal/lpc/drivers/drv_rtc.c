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
 * @brief       This file implements rtc driver for nxp.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <sys/time.h>
#include <drv_rtc.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.rtc"
#include <drv_log.h>

struct nxp_rtc
{
    os_device_t rtc;
    
    struct nxp_rtc_info *rtc_info;
};

static time_t nxp_rtc_get_timestamp(struct nxp_rtc *nxp_rtc)
{
    struct tm       tm_new;
    rtc_datetime_t rtc_datestruct = {0};

    RTC_GetDatetime(nxp_rtc->rtc_info->rtc_base, &rtc_datestruct);

    tm_new.tm_sec  = rtc_datestruct.second; 
    tm_new.tm_min  = rtc_datestruct.minute; 
    tm_new.tm_hour = rtc_datestruct.hour;
    
    tm_new.tm_mday = rtc_datestruct.day; 
    tm_new.tm_mon  = rtc_datestruct.month - 1; 
    tm_new.tm_year = rtc_datestruct.year - 1900;

    LOG_EXT_D("get rtc time.");
    return mktime(&tm_new);
}

static os_err_t nxp_rtc_set_time_stamp(struct nxp_rtc *nxp_rtc, time_t time_stamp)
{
    struct tm *p_tm;
    rtc_datetime_t rtc_dateStruct = {0};
    
    p_tm = localtime(&time_stamp);
    
    rtc_dateStruct.second = p_tm->tm_sec ;
    rtc_dateStruct.minute = p_tm->tm_min ;
    rtc_dateStruct.hour   = p_tm->tm_hour;

    rtc_dateStruct.day    = p_tm->tm_mday;
    rtc_dateStruct.month  = p_tm->tm_mon  + 1;
    rtc_dateStruct.year   = p_tm->tm_year + 1900;
    
    RTC_StopTimer(nxp_rtc->rtc_info->rtc_base);
    
    RTC_SetDatetime(nxp_rtc->rtc_info->rtc_base, &rtc_dateStruct);

    RTC_StartTimer(nxp_rtc->rtc_info->rtc_base);
    
    return OS_EOK;
}

static void nxp_rtc_init(struct nxp_rtc *nxp_rtc)
{
    RTC_StartTimer(nxp_rtc->rtc_info->rtc_base);
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);

    struct nxp_rtc *nxp_rtc = (struct nxp_rtc *)dev;
    
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = nxp_rtc_get_timestamp(nxp_rtc);
        LOG_EXT_D("RTC: get rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (nxp_rtc_set_time_stamp(nxp_rtc, *(os_uint32_t *)args))
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
    .control = os_rtc_control,
};

static int nxp_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct nxp_rtc *nxp_rtc;

    nxp_rtc = os_calloc(1, sizeof(struct nxp_rtc));

    OS_ASSERT(nxp_rtc);

    nxp_rtc->rtc_info = (struct nxp_rtc_info *)dev->info;

    nxp_rtc_init(nxp_rtc);

    os_device_default(&nxp_rtc->rtc, OS_DEVICE_TYPE_RTC);

    nxp_rtc->rtc.ops     = &rtc_ops;

    return os_device_register(&nxp_rtc->rtc, dev->name, OS_DEVICE_FLAG_RDWR);
}

OS_DRIVER_INFO nxp_rtc_driver = {
    .name   = "RTC_Type",
    .probe  = nxp_rtc_probe,
};

OS_DRIVER_DEFINE(nxp_rtc_driver, "1");

