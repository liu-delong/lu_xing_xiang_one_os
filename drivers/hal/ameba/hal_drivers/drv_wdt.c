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
 * @file        drv_wdt.c
 *
 * @brief       The driver file for wdt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "watchdog.h"
#include "wdt_api.h"
#ifdef BSP_USING_WDT

#define DRV_EXT_TAG "drv.wdt"
#define DRV_EXT_LVL DBG_EXT_INFO
#include <drv_log.h>

struct ameba_wdt_obj
{
    os_uint32_t timeout;
};
static struct ameba_wdt_obj   ameba_wdt;
static struct os_watchdog_ops ops;
static os_watchdog_t          watchdog;

static os_err_t wdt_init(os_watchdog_t *wdt)
{
    return OS_EOK;
}

static os_err_t wdt_control(os_watchdog_t *wdt, int cmd, void *arg)
{
    switch (cmd)
    {
        /* feed the watchdog */
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        watchdog_refresh();
        break;
        /* set watchdog timeout */
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        watchdog_stop();
        watchdog_init(*((os_uint32_t *)arg) * 1000);
        break;
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        (*((os_uint32_t *)arg)) = ameba_wdt.timeout;
        break;
    case OS_DEVICE_CTRL_WDT_START:
        watchdog_start();
        break;
    default:
        LOG_EXT_W("This command is not supported.");
        return OS_ERROR;
    }
    return OS_EOK;
}

int os_wdt_init(void)
{
    ops.init     = &wdt_init;
    ops.control  = &wdt_control;
    watchdog.ops = &ops;
    /* register watchdog device */
    if (os_hw_watchdog_register(&watchdog, "wdt", OS_DEVICE_FLAG_DEACTIVATE, OS_NULL) != OS_EOK)
    {
        LOG_EXT_E("wdt device register failed.");
        return OS_ERROR;
    }
    LOG_EXT_D("wdt device register success.");
    return OS_EOK;
}
OS_DEVICE_INIT(os_wdt_init);

#endif /* BSP_USING_WDT */
