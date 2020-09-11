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
 * @brief       This file implements watchdog driver for FM33A0xx
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "os_dbg_ext.h"

#ifdef BSP_USING_WDT

static struct os_watchdog_ops ops;
static os_watchdog_t          watchdog;

static os_err_t wdt_init(os_watchdog_t *wdt)
{
    return OS_EOK;
}

os_int32_t fm_iwdt_set(os_int32_t para)
{
    os_int32_t ret = OS_EOK;

    if ((para >= 0) && (para <= 3)) 
    {
        IWDT_IWDTCFG_IWDTOVP_Set(para);
    }
    else
    {
        LOG_EXT_D("para error; arg: 0-125ms, 1-500ms, 2-2s, 3-8s", );
        ret = OS_ERROR;
    }

    return ret;
}

static os_err_t wdt_control(os_watchdog_t *wdt, os_int32_t cmd, void *arg)
{
    os_int32_t ret = OS_EOK;
    switch (cmd)
    {
        /* feed the watchdog */
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        IWDT_Clr();
        break;
        /* set watchdog timeout */
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        ret = fm_iwdt_set(*(os_uint32_t *)arg);
        break;
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        *((os_uint32_t *)arg) = IWDT_IWDTCFG_IWDTOVP_Get();
        LOG_EXT_D("arg: 0-125ms, 1-500ms, 2-2s, 3-8s", );
        break;
    case OS_DEVICE_CTRL_WDT_START:
    	RCC_PERCLK_SetableEx(IWDTCLK, ENABLE);
	IWDT_Clr();							
        break;
    default:
        LOG_EXT_D("This command is not supported.");
        ret = OS_ERROR;
    }
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           os_wdt_init:init watchdog and register.
 *
 * @param[in]       none
 *
 * @return          Return wdt init status.
 * @retval          OS_EOK          init success.
 * @retval          Others          init failed.
 ***********************************************************************************************************************
 */
int os_wdt_init(void)
{
    ops.init     = &wdt_init;
    ops.control  = &wdt_control;
    watchdog.ops = &ops;
    
    /* register watchdog device */
    if (os_hw_watchdog_register(&watchdog, "wdt", OS_DEVICE_FLAG_DEACTIVATE, OS_NULL) != OS_EOK)
    {
        LOG_EXT_D("wdt device register failed.");
        return OS_ERROR;
    }
    LOG_EXT_D("wdt device register success.");
    return OS_EOK;
}
OS_BOARD_INIT(os_wdt_init);

#endif /* BSP_USING_WDT */
