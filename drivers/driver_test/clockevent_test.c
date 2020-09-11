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
 * @file        hwtimer_test.c
 *
 * @brief       The test file for hwtimer.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_device.h>
#include <os_errno.h>
#include <os_clock.h>
#include <stdlib.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>
#include <shell.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "clockevent_test"
#include <drv_log.h>

static int timeout_count = 0;
static int timeout_callback_print = 1;

static void timeout_callback(os_clockevent_t * ce)
{
    timeout_count++;

    if (timeout_callback_print)
    {
        LOG_EXT_I("clockevent timeout callback");
    }
}

static int clockevent_test(int argc, char *argv[])
{
    int i;
    int ret;

    os_clockevent_t *ce;

    if (argc != 2)
    {
        LOG_EXT_I("usage: clockevent_test <dev>");
        LOG_EXT_I("       clockevent_test tim1");
        return -1;
    }

    ce = (os_clockevent_t *)os_device_find(argv[1]);
    if (ce == OS_NULL)
    {
        LOG_EXT_I("invalid device %s", argv[1]);
        return -2;
    }

    ret = os_device_open((os_device_t *)ce, 0);
    if (ret != OS_EOK)
    {
        LOG_EXT_I("device %s open failed:%d", argv[1], ret);
        return -3;
    }

    os_task_t *self = os_task_self();
    os_uint8_t task_prio = self->current_priority;
    os_uint8_t high_prio = 1;
    os_task_control(self, OS_TASK_CTRL_CHANGE_PRIORITY, &high_prio);
    
    os_clockevent_register_isr(ce, timeout_callback);

    /* sync tick */
    os_task_msleep(1);

    /* 1.oneshot mode */
    for (i = 0; i < 3; i++)
    {
        LOG_EXT_I("clockevent onshot start %d", i);        
        os_clockevent_start_oneshot(ce, NSEC_PER_SEC);
        os_task_msleep(2000);
    }

    LOG_EXT_I("clockevent onshot before stop.");
    
    os_clockevent_stop(ce);
    os_task_msleep(2000);

    LOG_EXT_I("clockevent onshot after stop.");

    /* 2.period mode */
    LOG_EXT_I("clockevent period start...");    
    os_clockevent_start_period(ce, NSEC_PER_SEC);
    os_task_msleep(10300);

    LOG_EXT_I("clockevent period before stop.");
    
    os_clockevent_stop(ce);
    os_task_msleep(2000);
    
    LOG_EXT_I("clockevent period after stop.");

    os_device_close((os_device_t *)ce);

    os_task_control(self, OS_TASK_CTRL_CHANGE_PRIORITY, &task_prio);
    
    return 0;
}

static int clockevent_test_period(int argc, char *argv[])
{
    int ret;
    os_clockevent_t *ce;
    os_uint64_t period_nsec;

    os_uint64_t start, now;
    os_tick_t tick_start, tick_now;

    if (argc != 3)
    {
        LOG_EXT_I("usage: clockevent_test_period <dev> <period_nsec>");
        LOG_EXT_I("       clockevent_test_period tim1 50000");
        return -1;
    }

    period_nsec = strtol(argv[2], OS_NULL, 0);

    ce = (os_clockevent_t *)os_device_find(argv[1]);
    if (ce == OS_NULL)
    {
        LOG_EXT_I("invalid device %s", argv[1]);
        return -2;
    }

    ret = os_device_open((os_device_t *)ce, 0);
    if (ret != OS_EOK)
    {
        LOG_EXT_I("device %s open failed:%d", argv[1], ret);
        return -3;
    }

    os_task_t *self = os_task_self();
    os_uint8_t task_prio = self->current_priority;
    os_uint8_t high_prio = 1;
    os_task_control(self, OS_TASK_CTRL_CHANGE_PRIORITY, &high_prio);
    
    os_clockevent_register_isr(ce, timeout_callback);

    /* period mode */
    LOG_EXT_I("clockevent period start %Lu", period_nsec);
    
    timeout_count = 0;
    timeout_callback_print = 0;

    /* sync tick */
    os_task_msleep(1);

    tick_start = os_tick_get();
    start = os_clocksource_gettime();
    
    os_clockevent_start_period(ce, period_nsec);
    os_task_msleep(10000);
    os_clockevent_stop(ce);

    tick_now = os_tick_get();
    now = os_clocksource_gettime();
    
    LOG_EXT_I("clockevent period stop %d", timeout_count);

    LOG_EXT_I("msleep start tick: %d", tick_start);
    LOG_EXT_I("msleep now   tick: %d             delta tick: %d",
              tick_now, tick_now - tick_start);
    LOG_EXT_I("msleep start time: %d.%09d", (int)(start / NSEC_PER_SEC), (int)(start % NSEC_PER_SEC));
    LOG_EXT_I("msleep now   time: %d.%09d, delta time: %d us",
              (int)(now / NSEC_PER_SEC), (int)(now % NSEC_PER_SEC), (int)((now - start) / 1000));

    timeout_callback_print = 1;

    os_device_close((os_device_t *)ce);

    os_task_control(self, OS_TASK_CTRL_CHANGE_PRIORITY, &task_prio);
    
    return 0;
}

SH_CMD_EXPORT(clockevent_test, clockevent_test, "clockevent_test");
SH_CMD_EXPORT(clockevent_test_period, clockevent_test_period, "clockevent_test_period");

