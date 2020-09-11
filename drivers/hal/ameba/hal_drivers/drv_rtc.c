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
 * @brief       The driver file for rtc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <sys/time.h>
#include "rtc_api.h"

#ifdef BSP_USING_ONCHIP_RTC

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.rtc"
#include <drv_log.h>

static struct os_device rtc;

static time_t get_rtc_timestamp(void)
{
    return rtc_read();
}

static os_err_t set_rtc_time_stamp(time_t time_stamp)
{
    rtc_write(time_stamp);
    return OS_EOK;
}

static void os_rtc_init(void)
{
    rtc_init();
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);

    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = get_rtc_timestamp();
        LOG_EXT_D("RTC: get rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (set_rtc_time_stamp(*(os_uint32_t *)args))
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
const static struct os_device_ops rtc_ops =
{
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    os_rtc_control
};
#endif

static os_err_t os_hw_rtc_register(os_device_t *device, const char *name, os_uint32_t flag)
{
    OS_ASSERT(device != OS_NULL);

    os_rtc_init();

#ifdef OS_USING_DEVICE_OPS
    device->ops = &rtc_ops;
#else
    device->init    = OS_NULL;
    device->open    = OS_NULL;
    device->close   = OS_NULL;
    device->read    = OS_NULL;
    device->write   = OS_NULL;
    device->control = os_rtc_control;
#endif
    device->type        = OS_DEVICE_TYPE_RTC;
    device->rx_indicate = OS_NULL;
    device->tx_complete = OS_NULL;
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

#endif /* BSP_USING_ONCHIP_RTC */
