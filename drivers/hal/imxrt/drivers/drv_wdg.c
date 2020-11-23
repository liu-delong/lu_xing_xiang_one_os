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
 * @brief       This file implements wdt driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <os_task.h>
#include <os_device.h>
#include <stdlib.h>
#include <string.h>
#include <board.h>
#include <os_memory.h>
#include "peripherals.h"

#ifdef OS_USING_WDG

#if !defined(WDOG1_PERIPHERAL) && !defined(RTWDOG_PERIPHERAL)
#error "Please define at least one BSP_USING_WDOGx"
#endif

#include <drv_log.h>
#include "drv_wdg.h"

#if defined(RTWDOG_PERIPHERAL)
#include "fsl_rtwdog.h"

void RTWDOG_IRQHandler(void)
{
    RTWDOG_ClearStatusFlags(RTWDOG, RTWDOG_CS_FLG_MASK);
    os_kprintf("enter RTWDOG_IRQHANDLER\n");

    return;
}

static os_err_t imxrt_wdog3_close(os_watchdog_t *wdt)
{
    os_uint32_t level;
    RTWDOG_Type *base;
    base = (RTWDOG_Type *)wdt->parent.user_data;

    level = os_hw_interrupt_disable();
    RTWDOG_Unlock(base);
    RTWDOG_Disable(base);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t imxrt_wdog3_open(os_watchdog_t *wdt, os_uint16_t oflag)
{
    os_uint32_t level;
    RTWDOG_Type *base;
    base = (RTWDOG_Type *)wdt->parent.user_data;

    level = os_hw_interrupt_disable();
    RTWDOG_Unlock(base);
    RTWDOG_Enable(base);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t imxrt_wdog3_init(os_watchdog_t *wdt)
{
    RTWDOG_Type *base;
    base = (RTWDOG_Type *)wdt->parent.user_data;

    RTWDOG_Init(base, &RTWDOG_config);
    imxrt_wdog3_close(wdt);

    return OS_EOK;
}

static os_err_t imxrt_wdog3_refresh(os_watchdog_t *wdt)
{
    
    RTWDOG_Type *base;
    base = (RTWDOG_Type *)wdt->parent.user_data;

    os_uint32_t level;

    level = os_hw_interrupt_disable();
    RTWDOG_Refresh(base);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

/**
 * @function control rtwdog
 *
 * @param
 *    wdt  whick wdog used
 *    cmd  control wdog options
 *    args argument of conrtol
 * @retval os_err_t the status of control result
 *
 * @attention rtwdog unit is not seconds because seconds for system is to inaccurate
 *
 */
static os_err_t imxrt_wdog3_control(os_watchdog_t *wdt, int cmd, void *args)
{
    os_err_t ret = OS_EOK;
    OS_ASSERT(wdt != NULL);

    RTWDOG_Type *base;
    base = (RTWDOG_Type *)wdt->parent.user_data;
    
    switch(cmd)
    {
        case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
            *(uint16_t *)args = base->TOVAL;
            break;
        case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
            os_kprintf("please use MCUXpresso config tools to set rtwdog timeout.\n");
            ret = OS_ERROR;
            break;
        case OS_DEVICE_CTRL_WDT_GET_TIMELEFT:
            *(uint16_t *)args = base->TOVAL - base->CNT;
            break;
        case OS_DEVICE_CTRL_WDT_KEEPALIVE:
            imxrt_wdog3_refresh(wdt);
            break;
        case OS_DEVICE_CTRL_WDT_START:
            imxrt_wdog3_open(wdt, *(os_uint32_t *)args);
            break;
        case OS_DEVICE_CTRL_WDT_STOP:
            imxrt_wdog3_close(wdt);
            break;
        default:
            ret = OS_EINVAL;
            break;
    }

    return ret;
}

static struct os_watchdog_ops imxrt_wdog3_ops =
{
    .init = imxrt_wdog3_init,
    .control = imxrt_wdog3_control,
};

static int imxrt_rtwdt_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_watchdog_t *pWdog = NULL;
    os_err_t ret = OS_EOK;

    pWdog = (os_watchdog_t *)os_calloc(1, sizeof(os_watchdog_t));;
    pWdog->ops = &imxrt_wdog3_ops;

    ret = os_hw_watchdog_register(pWdog, "rtwdog", OS_DEVICE_FLAG_RDWR, RTWDOG);
    if (ret != OS_EOK)
    {
        os_kprintf("imxrt rtwdog register failed %d\n", ret);
    }
    
    return ret;
}

OS_DRIVER_INFO imxrt_rtwdt_driver = {
    .name   = "RTWDOG_Type",
    .probe  = imxrt_rtwdt_probe,
};

OS_DRIVER_DEFINE(imxrt_rtwdt_driver, "1");

#endif

#if defined(WDOG1_PERIPHERAL)
#include "fsl_wdog.h"

void WDOG1_IRQHandler(void)
{
    WDOG_ClearInterruptStatus(WDOG1, WDOG_WICR_WTIS_MASK);
    
    os_kprintf("enter WDOG1_IRQHandler\n");
    
#if 0 // do something before reset. (it wil reset after @interruptTimeValue*0.5s automatically)
    WDOG_Refresh(WDOG1);
#endif

    return;
}

static os_err_t imxrt_wdog_close(os_watchdog_t *wdt)
{
    os_uint32_t level;
    WDOG_Type *base;
    base = (WDOG_Type *)wdt->parent.user_data;

    level = os_hw_interrupt_disable();
    WDOG_Disable(base);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t imxrt_wdog_open(os_watchdog_t *wdt, os_uint16_t oflag)
{
    WDOG_Type *base;
    base = (WDOG_Type *)wdt->parent.user_data;
    os_uint32_t level;

    level = os_hw_interrupt_disable();
    WDOG_Enable(base);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t imxrt_wdog_init(os_watchdog_t *wdt)
{
    WDOG_Type *base;
    base = (WDOG_Type *)wdt->parent.user_data;

    WDOG_Init(base, &WDOG1_config);
    imxrt_wdog_close(wdt);

    return OS_EOK;
}

static os_err_t imxrt_wdog_refresh(os_watchdog_t *wdt)
{
    WDOG_Type *base;
    base = (WDOG_Type *)wdt->parent.user_data;

    os_uint32_t level;

    level = os_hw_interrupt_disable();
    WDOG_Refresh(base);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

/**
 * @function control wdog
 *
 * @param
 *    wdt  whick wdog used
 *    cmd  control wdog options
 *    args argument of conrtol
 * @retval os_err_t the status of control result
 *
 * @attention wdog1/wdog2 is can not get left time(register not exist)  and wdogs unit is seconds
 *
 */
static os_err_t imxrt_wdog_control(os_watchdog_t *wdt, int cmd, void *args)
{
    os_err_t ret = OS_EOK;

    OS_ASSERT(wdt != NULL);

    WDOG_Type *base;
    base = (WDOG_Type *)wdt->parent.user_data;

    switch(cmd)
    {
        case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
            *(uint16_t *)args = (base->WCR >> 8)  / 2;
            break;
        case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
            OS_ASSERT(*(uint16_t *)args != 0);
            WDOG_SetTimeoutValue(base, (*(uint16_t *)args) * 2);
            imxrt_wdog_close(wdt);
            break;
        case OS_DEVICE_CTRL_WDT_KEEPALIVE:
            imxrt_wdog_refresh(wdt);
            break;
        case OS_DEVICE_CTRL_WDT_START:
            imxrt_wdog_open(wdt, *(os_uint32_t *)args);
            break;
        case OS_DEVICE_CTRL_WDT_STOP:
            imxrt_wdog_close(wdt);
            break;
        default:
            ret = OS_EINVAL;
            break;
    }

    return ret;
}

static struct os_watchdog_ops imxrt_wdog_ops =
{
    .init = imxrt_wdog_init,
    .control = imxrt_wdog_control,
};

static int imxrt_wdt_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_watchdog_t *pWdog = NULL;
    os_err_t ret = OS_EOK;

    pWdog = (os_watchdog_t *)os_calloc(1, sizeof(os_watchdog_t));;
    pWdog->ops = &imxrt_wdog_ops;

    ret = os_hw_watchdog_register(pWdog, "wdog1", OS_DEVICE_FLAG_RDWR, WDOG1);

    if (ret != OS_EOK)
    {
        os_kprintf("imxrt wdog1 register failed %d\n", ret);
    }

    return ret;
}

OS_DRIVER_INFO imxrt_wdt_driver = {
    .name   = "WDOG_Type",
    .probe  = imxrt_wdt_probe,
};

OS_DRIVER_DEFINE(imxrt_wdt_driver, "1");

#endif

#endif /* BSP_USING_WDT */
