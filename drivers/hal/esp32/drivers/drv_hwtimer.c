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
 * @file        drv_hwtimer.c
 *
 * @brief       This file implements timer driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_irq.h>
#include <os_memory.h>
#include <timer/timer.h>
#include "drv_common.h"
#include "driver/timer.h"

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#endif

#ifdef OS_USING_CLOCKEVENT
#include <timer/clockevent.h>
#endif

#include <drv_hwtimer.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.hwtimer"
#include <drv_log.h>

#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_FREQ           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds


struct esp32_timer {
    union _clock {
        os_clocksource_t   cs;
        os_clockevent_t    ce;
    } clock;

    struct esp32_timer_info *info;
    os_uint32_t freq;
    os_list_node_t list;
    void *user_data;
};

static os_list_node_t esp32_timer_list = OS_LIST_INIT(esp32_timer_list);

static os_uint64_t esp32_timer_read(void *clock)
{
    struct esp32_timer *timer = (struct esp32_timer *)clock;
    os_uint64_t value;

    timer_get_counter_value(timer->info->group, timer->info->id, &value);
    return value;
}

#ifdef OS_USING_CLOCKEVENT
void IRAM_ATTR esp32_timer_isr(void *para)
{
    struct esp32_timer *timer = (struct esp32_timer *)para;
    
    os_clockevent_isr((os_clockevent_t *)timer);
}

static void esp32_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct esp32_timer *timer = (struct esp32_timer *)ce;
    timer_group_t group = timer->info->group;
    timer_idx_t id = timer->info->id;

    timer_config_t config = {
        .divider = prescaler,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = true,
    };

    OS_ASSERT(prescaler != 0);
    OS_ASSERT(count != 0);

    timer_init(group, id, &config);
    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(group, id, 0x00000000ULL);
    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(group, id, count);
    timer_enable_intr(group, id);
    timer_isr_register(group, id, esp32_timer_isr, (void *)timer, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(group, id);
}

static void esp32_timer_stop(os_clockevent_t *ce)
{
    struct esp32_timer *timer = (struct esp32_timer *)ce;
    OS_ASSERT(ce != OS_NULL);

    timer_pause(timer->info->group, timer->info->id);
}

static const struct os_clockevent_ops esp32_timer_ops =
{
    .start = esp32_timer_start,
    .stop  = esp32_timer_stop,
    .read  = esp32_timer_read,
};
#endif

static int esp32_timer_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct esp32_timer *timer;
    timer_group_t group;
    timer_idx_t id;

    timer_config_t config_default = {
        .divider = 1,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = true,
    }; // default clock source is APB

    timer = os_calloc(1, sizeof(struct esp32_timer));
    OS_ASSERT(timer);
    
    timer->freq = TIMER_BASE_CLK;
    timer->info = dev->info;

#ifdef OS_USING_CLOCKSOURCE
    if (os_clocksource_best() == OS_NULL)
    {
        group = timer->info->group;
        id = timer->info->id;

        timer->clock.cs.rating  = 640;
        timer->clock.cs.freq    = timer->freq;
        timer->clock.cs.mask    = 0xffffffffull;
        timer->clock.cs.read    = esp32_timer_read;

        timer_init(group, id, &config_default);
        timer_set_counter_value(group, id, 0x00000000ULL);
        timer_set_alarm_value(group, id, 80000);//1ms interval
        timer_disable_intr(group, id);
        timer_start(group, id);

        os_clocksource_register(dev->name, &timer->clock.cs);
    }
    else
#endif
    {
#ifdef OS_USING_CLOCKEVENT
        timer_init(timer->info->group, timer->info->id, &config_default);

        timer->clock.ce.rating  = 640;
        timer->clock.ce.freq    = timer->freq;
        timer->clock.ce.mask    = 0xffffffffull;
        
        timer->clock.ce.prescaler_mask = 0xfffful;
        timer->clock.ce.prescaler_bits = 16;
        
        timer->clock.ce.count_mask = ~0ull;
        timer->clock.ce.count_bits = 64;

        timer->clock.ce.feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

        timer->clock.ce.min_nsec = NSEC_PER_SEC / timer->clock.ce.freq;
        
        timer->clock.ce.ops     = &esp32_timer_ops;
        os_clockevent_register(dev->name, &timer->clock.ce);
#endif
    }
    os_list_add(&esp32_timer_list, &timer->list);

    return OS_EOK;
}

OS_DRIVER_INFO esp32_timer_driver = {
    .name   = "ESP32_TIMER_DRIVER",
    .probe  = esp32_timer_probe,
};

OS_DRIVER_DEFINE(esp32_timer_driver, "1");

