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
 * @brief       This file implements watchdog driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>

#define DRV_EXT_TAG "drv.wwdt"
#define DRV_EXT_LVL DBG_EXT_INFO
#include <drv_log.h>
#include <bus/bus.h>
#include "hal_wdt.h"
#include "bsp.h"

struct mt_wdg {
    os_watchdog_t wdg;
    
    WDG_HandleTypeDef *hwdg;
};

os_uint32_t mt_reset_status_get()
{
    return hal_wdt_get_reset_status();
}


static os_err_t mt_wdt_init(os_watchdog_t *wdt)
{
    hal_wdt_status_t ret;
    
    ret = hal_wdt_deinit();
    if(HAL_WDT_STATUS_OK != ret) 
    {
         LOG_EXT_E("mt_wdt_initerror, ret[%d]", ret);
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t mt_wdt_control(os_watchdog_t *wdt, int cmd, void *arg)
{
    hal_wdt_config_t wdt_config;
    hal_wdt_status_t ret;

    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        ret = hal_wdt_feed(HAL_WDT_FEED_MAGIC);
        
        if (ret != HAL_WDT_STATUS_OK)
        {
             LOG_EXT_E("feed wtd errorret[%d]", ret);
             return OS_ERROR;
        }
        break;
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        wdt_config.mode = HAL_WDT_MODE_RESET;
        wdt_config.seconds = *(os_uint32_t *)arg;
        ret = hal_wdt_init(&wdt_config);
        if(HAL_WDT_STATUS_OK != ret) 
        {
             LOG_EXT_E("config watch dog timeout seconds[%d], ret[%d]", wdt_config.seconds, ret);
            return OS_ERROR;
        }
        break;
        
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        return OS_ERROR;
        break;
        
    case OS_DEVICE_CTRL_WDT_START:
        hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);;
        LOG_EXT_I("wdt start.");
        break;
    default :
        LOG_EXT_E("no para.");
        return OS_ENOSYS;
    }

    return OS_EOK;
}

const static struct os_watchdog_ops ops = 
{
    .init     = &mt_wdt_init,
    .control  = &mt_wdt_control,
};

static int mt_wdt_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct mt_wdg *wdg;

    wdg = os_calloc(1, sizeof(struct mt_wdg));

    OS_ASSERT(wdg);

    wdg->hwdg = (WDG_HandleTypeDef *)dev->info;
    wdg->wdg.ops = &ops;

    if (os_hw_watchdog_register(&wdg->wdg, dev->name, OS_DEVICE_FLAG_DEACTIVATE, OS_NULL) != OS_EOK)
    {
        LOG_EXT_E("wwdt device register failed.");
        return OS_ERROR;
    }
    LOG_EXT_D("wwdt device register success.");
    return OS_EOK;
}

OS_DRIVER_INFO mt_wdt_driver = {
    .name   = "WDG_HandleTypeDef",
    .probe  = mt_wdt_probe,
};

OS_DRIVER_DEFINE(mt_wdt_driver, "1");

