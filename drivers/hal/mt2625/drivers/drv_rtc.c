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
 * @brief       This file implements RTC driver for stm32.
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

#include "hal_rtc.h"
#include "bsp.h"

struct mt_rtc {
    os_device_t rtc;
    RTC_HandleTypeDef *hrtc;
};

static int set_current_time(time_t time_stamp) 
{
    hal_rtc_time_t time;
    struct tm *p_tm;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return OS_ERROR;
    }

    time.rtc_sec = p_tm->tm_sec;
    time.rtc_min = p_tm->tm_min;
    time.rtc_hour  = p_tm->tm_hour;
    time.rtc_day    = p_tm->tm_mday;
    time.rtc_mon   = p_tm->tm_mon + 1;
    time.rtc_year    = p_tm->tm_year - 100;
    time.rtc_week = p_tm->tm_wday + 1;

    // Set the RTC current time.
    if(HAL_RTC_STATUS_OK != hal_rtc_set_time(&time)) 
    {
        LOG_EXT_E("set rtc time error.");
        return OS_ERROR;
    }

    return OS_EOK;
}


static time_t  get_current_time(void) 
{
    hal_rtc_time_t time;
    struct tm       tm_new;

    if(HAL_RTC_STATUS_OK != hal_rtc_get_time(&time)) {
        LOG_EXT_E("get rtc time error.");
        return OS_ERROR;
    }

    tm_new.tm_sec  = time.rtc_sec;
    tm_new.tm_min  = time.rtc_min;
    tm_new.tm_hour = time.rtc_hour;
    tm_new.tm_mday = time.rtc_day;
    tm_new.tm_mon  = time.rtc_mon- 1;
    tm_new.tm_year = time.rtc_year + 100;

    LOG_EXT_D("get rtc time.");
    return mktime(&tm_new);

}

os_err_t  os_rtc_init(os_device_t *dev)
{
    /* RTC has been initialized in BootLoader */
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

#ifdef OS_USING_DEVICE_OPS
const static struct os_device_ops rtc_ops = {
    .init = os_rtc_init,
    .control = os_rtc_control,
};
#endif

static int mt_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct mt_rtc *st_rtc;

    st_rtc = os_calloc(1, sizeof(struct mt_rtc));

    OS_ASSERT(st_rtc);

    st_rtc->hrtc = (RTC_HandleTypeDef *)dev->info;

    os_device_default(&st_rtc->rtc, OS_DEVICE_TYPE_RTC);
    
#ifdef OS_USING_DEVICE_OPS
    st_rtc->rtc.ops     = &rtc_ops;
#else
    st_rtc->rtc.init = os_rtc_init;
    st_rtc->rtc.control = os_rtc_control;
#endif

    return os_device_register(&st_rtc->rtc, dev->name, OS_DEVICE_FLAG_RDWR);
}

OS_DRIVER_INFO mt_rtc_driver = {
    .name   = "RTC_HandleTypeDef",
    .probe  = mt_rtc_probe,
};

OS_DRIVER_DEFINE(mt_rtc_driver, "1");

