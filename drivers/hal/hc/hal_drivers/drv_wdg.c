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
 * @brief       This file implements watchdog driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>

#define DRV_EXT_TAG "drv.wdt"
#define DRV_EXT_LVL DBG_EXT_INFO
#include <drv_log.h>
#include <bus/bus.h>
#include "hc_wdt.h"
#include "drv_wdt.h"

static os_watchdog_t hc32_watchdog;

#ifdef OS_USING_WDG

static os_err_t hc32_wdt_init(os_watchdog_t *wdt)
{
    Sysctrl_SetPeripheralGate(SysctrlPeripheralWdt, TRUE);

    Wdt_Init(WdtResetEn, WdtT6s55);
    return OS_EOK;
}

static os_err_t hc32_wdt_control(os_watchdog_t *wdt, int cmd, void *arg)
{
    os_uint32_t timeout = 0;
    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        Wdt_Feed();
        return OS_EOK;

    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        timeout = *(os_uint32_t *)arg;
        if (timeout == 13)
        {
            Wdt_WriteWdtLoad(WdtT13s1);
        }
        else if (timeout == 26)
        {
            Wdt_WriteWdtLoad(WdtT26s2);
        }
        else if (timeout == 52)
        {
            Wdt_WriteWdtLoad(WdtT52s4);
        }
        return OS_EOK;

    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        break;

    case OS_DEVICE_CTRL_WDT_START:
        Wdt_Start();
        LOG_EXT_I("wdt start.");
        return OS_EOK;
    }

    return OS_ENOSYS;
}

const static struct os_watchdog_ops ops =
{
    .init     = &hc32_wdt_init,
    .control  = &hc32_wdt_control,
};

int os_hw_wdt_init(void)
{
    os_err_t ret = OS_EOK;

    hc32_watchdog.ops = &ops;

    ret = os_hw_watchdog_register(&hc32_watchdog, "iwdg", OS_DEVICE_FLAG_DEACTIVATE, OS_NULL);

    if (ret != OS_EOK)
    {
        LOG_EXT_E("os device register failed %d\n", ret);
    }

    return ret;
}

OS_DEVICE_INIT(os_hw_wdt_init);
#endif
