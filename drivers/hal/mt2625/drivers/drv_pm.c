/*
 * Copyright (c) 2012-2020, CMCC IOT
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */


#include "os_kernel.h"
#include "drv_cfg.h"
#include "os_hw.h"

#if defined(CM_USING_PM) && defined(BSP_USING_SLEEP_MANAGER)

#include "drv_pm.h"
#include "hal_platform.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_sleep_manager_platform.h"
#include "hal_wdt.h"
#include "hal_gpt.h"

// #define DRV_DEBUG

#ifdef DRV_DEBUG

#define DBG_EXT_TAG               "drv.pm"
#define DBG_EXT_LVL               DBG_LOG
#else
#define DBG_EXT_LVL               DBG_ERROR
#endif /* DRV_DEBUG */
#include <os_dbg_ext.h>

static volatile uint32_t globe_sleep_time = 0;

static hal_sleep_mode_t pm_get_mode(uint8_t mode)
{
    hal_sleep_mode_t mt2625_pm_mode;
    switch (mode)
    {
    case PM_SLEEP_MODE_NONE:
        mt2625_pm_mode = HAL_SLEEP_MODE_IDLE;
        break;

    case PM_SLEEP_MODE_IDLE:
        mt2625_pm_mode = HAL_SLEEP_MODE_IDLE;
        break;

    case PM_SLEEP_MODE_LIGHT:
        mt2625_pm_mode = HAL_SLEEP_MODE_LIGHT_SLEEP;
        break;

    case PM_SLEEP_MODE_DEEP:
        mt2625_pm_mode = HAL_SLEEP_MODE_DEEP_SLEEP;
        break;

    case PM_SLEEP_MODE_STANDBY:
        mt2625_pm_mode = HAL_SLEEP_MODE_DEEPER_SLEEP;
        break;

    default:
        break;
    }

    return mt2625_pm_mode;
}

extern void os_gpt0_pause(void);
extern void os_gpt0_resume(bool update, uint32_t new_compare);

/**
 * This function will put STM32L4xx into sleep mode.
 *
 * @param pm pointer to power manage structure
 */
static void pm_enter_sleep(struct cm_pm *pm, uint8_t mode)
{
    hal_sleep_manager_status_t ret;
    os_base_t level;
    os_uint32_t reload_time;
    hal_sleep_mode_t mt2625_pm_mode, mt2625_pm_sleep_mode;
    uint32_t lock_status;
    static hal_sleep_mode_t current_sleep_mode = HAL_SLEEP_MODE_NONE;

    lock_status = hal_sleep_manager_is_sleep_lock_locked(HAL_SLEEP_LOCK_ALL);
    if (lock_status == 1)
    {
        // sleep_manager_set_sleep_mode(HAL_SLEEP_MODE_IDLE);
        hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_IDLE);
        return;
    }
    level = os_hw_interrupt_disable();

    mt2625_pm_mode = pm_get_mode(mode);
    if ((mt2625_pm_mode == HAL_SLEEP_MODE_NONE) || 
        (globe_sleep_time < 2)) // 20ms
    {
        return;
    }

    mt2625_pm_sleep_mode = mt2625_pm_mode;

    // if (sleep_manager_get_sleep_mode() == mt2625_pm_mode)
    // {
    //     return;
    // }

    if (mt2625_pm_mode != HAL_SLEEP_MODE_IDLE)
    {
        os_gpt0_pause();
    #ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
        hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
    #endif
    }

    if (mt2625_pm_mode == HAL_SLEEP_MODE_DEEP_SLEEP ||
        mt2625_pm_mode == HAL_SLEEP_MODE_DEEPER_SLEEP)
    {
        if (hal_sleep_manager_is_sleep_lock_locked(HAL_SLEEP_LOCK_DEEP) == false)
        {
            extern bool tryDeepSleep(void);
            tryDeepSleep();
        }
        goto __exit;
    }
    else if (mt2625_pm_mode == HAL_SLEEP_MODE_LIGHT_SLEEP)
    {
        reload_time = (globe_sleep_time - 1) * (1000 / CM_TICK_PER_SECOND);

        extern bool tryLightSleep(uint32_t try_sleep_ms);
        tryLightSleep(reload_time);
        goto __exit;
    }
    else
    {
        // sleep_manager_set_sleep_mode(HAL_SLEEP_MODE_IDLE);
        hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_IDLE);
    }

__exit:
    if (mt2625_pm_mode != HAL_SLEEP_MODE_IDLE)
    {
        os_gpt0_resume(false, 0);
    #ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
        hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
    #endif
    }
    os_hw_interrupt_enable(level);
}

static void pm_rum_config(struct cm_pm *pm, uint8_t mode)
{
    static char *run_str[] = PM_RUN_MODE_NAMES;

    LOG_EXT_I("switch to <%s> mode", run_str[mode]);
}

/**
 * This function start the timer of pm
 *
 * @param pm Pointer to power manage structure
 * @param timeout How many OS Ticks that MCU can sleep
 */
static void pm_timer_start(struct cm_pm *pm, os_uint32_t timeout)
{
    OS_ASSERT(pm != OS_NULL);
    OS_ASSERT(timeout > 0);

    globe_sleep_time = timeout;
}

/**
 * This function stop the timer of pm
 *
 * @param pm Pointer to power manage structure
 */
static void pm_timer_stop(struct cm_pm *pm)
{
    OS_ASSERT(pm != OS_NULL);
}

/**
 * This function calculate how many OS Ticks that MCU have suspended
 *
 * @param pm Pointer to power manage structure
 *
 * @return OS Ticks
 */
static os_tick_t pm_timer_get_tick(struct cm_pm *pm)
{
    /* 8 ticks: Time compensation for function call */
    return os_tick_from_ms(globe_sleep_time) + 8;
}

int drv_pm_hw_init(void)
{
    os_uint8_t timer_mask = 0;
    static const struct cm_pm_ops _ops =
    {
        pm_enter_sleep,
        pm_rum_config,
        pm_timer_start,
        pm_timer_stop,
        pm_timer_get_tick
    };

    /* initialize timer mask */
    timer_mask = 1UL << PM_SLEEP_MODE_LIGHT;

    /* initialize system pm module */
    cm_system_pm_init(&_ops, timer_mask, OS_NULL);

    return 0;
}

OS_BOARD_INIT(drv_pm_hw_init);

static void _get_sleep_mode(void)
{
    extern hal_sleep_mode_t sleep_manager_get_sleep_mode(void);
    os_kprintf("=> 0x%x\n", sleep_manager_get_sleep_mode());
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(sleep_mode, _get_sleep_mode, "get sleepmode");
#endif

#endif /* CM_USING_PM && BSP_USING_SLEEP_MANAGER */
