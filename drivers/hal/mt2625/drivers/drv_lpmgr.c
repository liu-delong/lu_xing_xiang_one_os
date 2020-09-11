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
 * @file        drv_lpmgr.c
 *
 * @brief       This file implements low power manager for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <lpmgr.h>
#include <board.h>

#include "hal_platform.h"
#include "hal_rtc.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_rtc_internal.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.lpmgr"
#include <drv_log.h>

typedef struct __tag_lpm_info_s
{
    uint32_t sleep_tick;
    uint32_t rtc_sw_flag;
    uint32_t rtc_handle;
}lpm_info_s;

static void uart_console_reconfig(void)
{
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    os_device_control(os_console_get_device(), OS_DEVICE_CTRL_CONFIG, &config);
}

uint32_t mt_tick_to_ms(os_tick_t tick)
{
    return tick * (1000 / OS_TICK_PER_SECOND);
}

void mt_rtc_timer_callback(void *user_data)
{
    os_kprintf("[%s]-[%d], RTC callback!!\r\n", __FILE__, __LINE__);
}

static int mt_rtc_sw_timer(lpm_info_s *lpm_info)
{
    rtc_sw_timer_status_t status;
    uint32_t life_time_100ms = 0;

    /*******创建rtc定时*********/
    life_time_100ms = mt_tick_to_ms(lpm_info->sleep_tick) / 100;
    os_kprintf("\n\n========rtc_sw_timer_create timeout[%d s]=========\r\n", life_time_100ms /10);
    status = rtc_sw_timer_create(&lpm_info->rtc_handle, life_time_100ms, false, mt_rtc_timer_callback);
    status |= rtc_sw_timer_start(lpm_info->rtc_handle);

    return status;
}

/**
 ***********************************************************************************************************************
 * @brief           Put device into sleep mode.
 *
 * @param[in]       lpm             Low power manager structure.
 * @param[in]       mode            Low power mode.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void sleep(struct lpmgr *lpm, uint8_t mode)
{
    hal_sleep_mode_t hal_mode = SYS_SLEEP_MODE_NONE;
    uint32_t ret;
    bool lock_ret;
    bool ret2;
    uint32_t  info_high = 0;
    uint32_t  info_low = 0;

    lpm_info_s *lpm_info = lpm->parent.user_data;
    OS_ASSERT(lpm_info != NULL);
    
    switch (mode)
    {
//    case SYS_SLEEP_MODE_NONE:
//        hal_mode = HAL_SLEEP_MODE_NONE;
//        break;

//    case SYS_SLEEP_MODE_IDLE:
//        hal_mode = HAL_SLEEP_MODE_IDLE;
//        break;

    case SYS_SLEEP_MODE_LIGHT:
        hal_mode = HAL_SLEEP_MODE_LIGHT_SLEEP;
        break;

    case SYS_SLEEP_MODE_DEEP:
        hal_mode = HAL_SLEEP_MODE_DEEP_SLEEP;
        break;

    case SYS_SLEEP_MODE_SHUTDOWN:
        os_kprintf("power off: %d\n", mode);
        hal_sleep_manager_enter_power_off_mode();
        return;

    default:
        os_kprintf("invalid mode: %d\n", mode);
        return ;
    }
    
    /* Low power consumption call MT2625 library interface */
    if ((hal_mode == HAL_SLEEP_MODE_LIGHT_SLEEP) || 
        (hal_mode == HAL_SLEEP_MODE_DEEP_SLEEP))
    {
        lock_ret = hal_sleep_manager_is_sleep_lock_locked(HAL_SLEEP_LOCK_ALL);
        if (lock_ret == true)
        {
            ret2 = hal_sleep_manager_get_sleep_lock_status(HAL_SLEEP_LOCK_ALL, &info_high, &info_low);
            os_kprintf("[%s]-[%d], HAL_SLEEP_LOCK_ALL:info_high[%d], info_low[%d], ret2[%d]\r\n", __FILE__, __LINE__, info_high, info_low, ret2);
            return ;
        }
        
        if (hal_mode == HAL_SLEEP_MODE_DEEP_SLEEP)
        {
            /* deep/deeper mode need RTC timer to wake up  */
            lock_ret = hal_sleep_manager_is_sleep_lock_locked(HAL_SLEEP_LOCK_DEEP);
            if ((lock_ret == false) && (lpm_info->rtc_sw_flag == 0))
            {
                ret = mt_rtc_sw_timer(lpm_info);
                if (ret != 0)
                {
                    os_kprintf("creat rtc se timer fail: %d\n", ret);
                    return ;
                }

                lpm_info->rtc_sw_flag = 1;
                //os_kprintf("[%s]-[%d], creat rtc timer success!\r\n", __FILE__, __LINE__);
            }
            else
            {
                ret2 = hal_sleep_manager_get_sleep_lock_status(HAL_SLEEP_LOCK_DEEP, &info_high, &info_low);
                os_kprintf("[%s]-[%d], HAL_SLEEP_LOCK_DEEP:info_high[%d], info_low[%d], ret2[%d], lpm_info->rtc_sw_flag[%d]\r\n", 
                    __FILE__, __LINE__, info_high, info_low, ret2, lpm_info->rtc_sw_flag);
                //return;
                /* enter sleep_deep mode fail, continue try enter sleep_light mode */
            }
        }
        
        os_kprintf("[%s]-[%d], ===enter=== hal_mode[%d], sleep_tick[%d], cur_tick[%d]\r\n", __FILE__, __LINE__, hal_mode, lpm_info->sleep_tick, os_tick_get());
        os_enter_critical();
        portSUPPRESS_TICKS_AND_SLEEP(lpm_info->sleep_tick);  
        os_exit_critical();
        os_kprintf("[%s]-[%d], ===wake up=== hal_mode[%d], sleep_tick[%d], cur_tick[%d]\r\n", __FILE__, __LINE__, hal_mode, lpm_info->sleep_tick, os_tick_get());
    }
    else
    {
        os_kprintf("[%s]-[%d]\r\n", __FILE__, __LINE__);
        hal_sleep_manager_enter_sleep_mode(hal_mode);
    }

    return ;
}

static void run(struct lpmgr *lpm, uint8_t mode)
{
    /* Re-configure clock for peripheral */
    uart_console_reconfig();
}

#if 1

/**
 ***********************************************************************************************************************
 * @brief           Start the timer of pm.
 *
 * @param[in]       lpm             Low power manager structure.
 * @param[in]       timeout         How many OS ticks that MCU can sleep.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void lpm_timer_start(struct lpmgr *lpm, os_uint32_t timeout)
{
    OS_ASSERT(lpm != OS_NULL);
    OS_ASSERT(timeout > 0);
    lpm_info_s *lpm_info = lpm->parent.user_data;
    OS_ASSERT(lpm_info != OS_NULL);

    if (timeout != OS_TICK_MAX)
    {
        lpm_info->sleep_tick = timeout;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Stop the timer of pm.
 *
 * @param[in]       lpm             Low power manager structure.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void lpm_timer_stop(struct lpmgr *lpm)
{
    rtc_sw_timer_status_t ret_sw;
    OS_ASSERT(lpm != OS_NULL);
    lpm_info_s *lpm_info = lpm->parent.user_data;
    OS_ASSERT(lpm_info != OS_NULL);
    
    if (lpm_info->rtc_sw_flag == 1)
    {
        ret_sw = rtc_sw_timer_stop(lpm_info->rtc_handle);
        OS_ASSERT(ret_sw == RTC_SW_TIMER_STATUS_OK);
        lpm_info->rtc_sw_flag = 0;
        os_kprintf("[%s]-[%d], stop lpm timer!\r\n", __FILE__, __LINE__);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Calculate how many OS ticks that MCU has suspended.
 *
 * @param[in]       lpm             Low power manager structure.
 *
 * @return          OS ticks.
 ***********************************************************************************************************************
 */
static os_tick_t lpm_timer_get_tick(struct lpmgr *lpm)
{
    OS_ASSERT(lpm != OS_NULL);
    return 0;
}

#endif

/**
***********************************************************************************************************************
* @brief           Initialise low power manager.
*
* @param[in]		None.
*
* @return          0.
***********************************************************************************************************************
*/
int drv_lpmgr_hw_init(void)
{
    static const struct os_lpmgr_ops s_lpmgr_ops = {
        sleep,
        run,
        lpm_timer_start,
        lpm_timer_stop,
        lpm_timer_get_tick
    };

    os_uint8_t timer_mask = 0;
    lpm_info_s *lpm_info = NULL;

    os_kprintf("[%s]-[%d]\r\n", __FILE__, __LINE__);
    /* hal_sleep_manager_init() called in system_hardware_init() */

    /* Initialize timer mask */
    timer_mask = 1UL << SYS_SLEEP_MODE_DEEP;
    timer_mask |= 1UL << SYS_SLEEP_MODE_LIGHT;

    lpm_info = (lpm_info_s *)os_calloc(1, sizeof(lpm_info_s));
    lpm_info->rtc_handle = 0;
    lpm_info->rtc_sw_flag = 0;
    lpm_info->sleep_tick = 0;
    os_kprintf("[%s]-[%d], timer_mask[%d]\r\n", __FILE__, __LINE__, timer_mask);

    /* Initialize system lpmgr module */
    os_lpmgr_init(&s_lpmgr_ops, timer_mask, lpm_info);

    return 0;
}

OS_PREV_INIT(drv_lpmgr_hw_init);

