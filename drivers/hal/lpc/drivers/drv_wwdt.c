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
 * @file        drv_wwdt.c
 *
 * @brief       This file implements wwdt driver for nxp
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_irq.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <drv_cfg.h>
#include <drv_wwdt.h>
#include "peripherals.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.wwdt"
#include <drv_log.h>

struct nxp_wwdt
{
    os_watchdog_t wdt;
    
    struct nxp_wwdt_info *wwdt_info;
};

struct nxp_wwdt *pnxp_wwdt = OS_NULL;

void WWDT_IRQHANDLER(void)
{
    uint32_t wdtStatus = WWDT_GetStatusFlags(pnxp_wwdt->wwdt_info->wwdt_base);

    /*this timeoutflag can be set,but in IRQ function cannot check it! it's sure in demo and test!*/
    if (wdtStatus & kWWDT_TimeoutFlag)
    {
        WWDT_Disable(pnxp_wwdt->wwdt_info->wwdt_base);
        WWDT_ClearStatusFlags(pnxp_wwdt->wwdt_info->wwdt_base, kWWDT_TimeoutFlag);
        WWDT_Enable(pnxp_wwdt->wwdt_info->wwdt_base);
    }

    if (wdtStatus & kWWDT_WarningFlag)
    {
        WWDT_ClearStatusFlags(pnxp_wwdt->wwdt_info->wwdt_base, kWWDT_WarningFlag);
        if (pnxp_wwdt->wdt.parent.cb_table[OS_DEVICE_CB_TYPE_RX].cb != OS_NULL)
        {
            pnxp_wwdt->wdt.parent.cb_table[OS_DEVICE_CB_TYPE_RX].cb(&pnxp_wwdt->wdt.parent, 0x00);
        }
    }
    SDK_ISR_EXIT_BARRIER;
}

static os_err_t nxp_wwdt_init(os_watchdog_t *wdt)
{
    return OS_EOK;
}

static os_err_t nxp_wwdt_close(os_watchdog_t *wdt)
{
    os_uint32_t level;

    struct nxp_wwdt *nxp_wwdt = (struct nxp_wwdt *)wdt;

    level = os_hw_interrupt_disable();
    WWDT_Disable(nxp_wwdt->wwdt_info->wwdt_base);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t nxp_wwdt_open(os_watchdog_t *wdt, os_uint16_t oflag)
{
    os_uint32_t level;
    
    struct nxp_wwdt *nxp_wwdt = (struct nxp_wwdt *)wdt;

    level = os_hw_interrupt_disable();
#ifdef WWDT_IRQN
    EnableIRQ(WWDT_IRQN);
#endif
    WWDT_Enable(nxp_wwdt->wwdt_info->wwdt_base);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t nxp_wwdt_refresh(os_watchdog_t *wdt)
{
    os_uint32_t level;
    
    struct nxp_wwdt *nxp_wwdt = (struct nxp_wwdt *)wdt;

    level = os_hw_interrupt_disable();
    WWDT_Refresh(nxp_wwdt->wwdt_info->wwdt_base);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t nxp_wwdt_control(os_watchdog_t *wdt, int cmd, void *args)
{
    OS_ASSERT(wdt != NULL);
    
    struct nxp_wwdt *nxp_wwdt = (struct nxp_wwdt *)wdt;

    switch(cmd)
    {
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
    {
        
    }
    break;
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
    {
        *(uint16_t *)args = nxp_wwdt->wwdt_info->wwdt_config->timeoutValue;
    }
    break;
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
    {
        nxp_wwdt_refresh(wdt);
    }
    break;
    case OS_DEVICE_CTRL_WDT_START:
    {
        nxp_wwdt_open(wdt, *(os_uint32_t *)args);
    }
    break;
    case OS_DEVICE_CTRL_WDT_STOP:
    {
        nxp_wwdt_close(wdt);
    }
    break;
    default:
        return OS_EINVAL;
    }

    return OS_EOK;
}

static struct os_watchdog_ops nxp_wwdt_ops =
{
    .init = nxp_wwdt_init,
    .control = nxp_wwdt_control,
};

static int nxp_wwdt_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct nxp_wwdt *nxp_wwdt = os_calloc(1, sizeof(struct nxp_wwdt));;

    OS_ASSERT(nxp_wwdt);

    pnxp_wwdt = nxp_wwdt;

    nxp_wwdt->wwdt_info = (struct nxp_wwdt_info *)dev->info;
    nxp_wwdt->wdt.ops = &nxp_wwdt_ops;

    if (os_hw_watchdog_register(&nxp_wwdt->wdt, dev->name, OS_DEVICE_FLAG_DEACTIVATE, OS_NULL) != OS_EOK)
    {
        LOG_EXT_E("wwdt device register failed.");
        return OS_ERROR;
    }
    LOG_EXT_D("wwdt device register success.");
    return OS_EOK;
}

OS_DRIVER_INFO nxp_wwdt_driver = {
    .name   = "WWDT_Type",
    .probe  = nxp_wwdt_probe,
};

OS_DRIVER_DEFINE(nxp_wwdt_driver, "1");


