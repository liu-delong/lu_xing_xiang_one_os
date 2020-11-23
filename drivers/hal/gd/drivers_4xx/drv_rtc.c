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
 * @brief       This file implements RTC driver for gd32.
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

#include "drv_rtc.h"

#define BKUP_REG_DATA 0x3C5A

struct gd32_rtc {
    os_device_t rtc;
    
    os_uint32_t hrtc;
};


static time_t gd32_rtc_get_timestamp(os_uint32_t hrtc)
{
    time_t ret;
    rtc_parameter_struct *rtc_initpara;

    rtc_current_time_get(rtc_initpara);

    struct tm       tm_new;

    tm_new.tm_sec  = rtc_initpara->second;
    tm_new.tm_min  = rtc_initpara->minute;
    tm_new.tm_hour = rtc_initpara->hour;
    tm_new.tm_mday = rtc_initpara->date;
    tm_new.tm_mon  = rtc_initpara->month;
    tm_new.tm_year = rtc_initpara->year;

    LOG_EXT_D("get rtc time.");
    ret = mktime(&tm_new);

    return ret;
}

static os_err_t gd32_rtc_set_time_stamp(os_uint32_t hrtc, time_t time_stamp)
{
    struct tm *p_tm;
    rtc_parameter_struct *rtc_initpara;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return OS_ERROR;
    }

    rtc_initpara->second = p_tm->tm_sec;
    rtc_initpara->minute = p_tm->tm_min;
    rtc_initpara->hour   = p_tm->tm_hour;
    rtc_initpara->date    = p_tm->tm_mday;
    rtc_initpara->month   = p_tm->tm_mon + 1;
    rtc_initpara->year    = p_tm->tm_year - 100;
    rtc_initpara->day_of_week = p_tm->tm_wday + 1;

    rtc_init(rtc_initpara);
    return OS_EOK;
}

static void gd32_rtc_init(os_uint32_t hrtc)
{
    /* enable PMU clock */
    rcu_periph_clock_enable(RCU_PMU);
    /* enable the access of the RTC registers */
    pmu_backup_write_enable();

    rcu_osci_on(RCU_LXTAL);
    rcu_osci_stab_wait(RCU_LXTAL);
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
    rcu_periph_clock_enable(RCU_RTC);
    rtc_register_sync_wait();
    
    rtc_interrupt_enable(RTC_INT_ALARM0); 
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);

    struct gd32_rtc *gd_rtc = (struct gd32_rtc *)dev;
    
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = gd32_rtc_get_timestamp(gd_rtc->hrtc);
        LOG_EXT_D("RTC: get rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (gd32_rtc_set_time_stamp(gd_rtc->hrtc, *(os_uint32_t *)args))
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

static int gd32_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct gd32_rtc *gd_rtc;

    gd_rtc = os_calloc(1, sizeof(struct gd32_rtc));

    OS_ASSERT(gd_rtc);

    gd_rtc->hrtc = (os_uint32_t)dev->info;

    gd32_rtc_init(gd_rtc->hrtc);

    os_device_default(&gd_rtc->rtc, OS_DEVICE_TYPE_RTC);
    
    gd_rtc->rtc.ops     = &rtc_ops;

    return os_device_register(&gd_rtc->rtc, dev->name, OS_DEVICE_FLAG_RDWR);
}

OS_DRIVER_INFO gd32_rtc_driver = {
    .name   = "RTC_Type",
    .probe  = gd32_rtc_probe,
};

OS_DRIVER_DEFINE(gd32_rtc_driver, "1");

