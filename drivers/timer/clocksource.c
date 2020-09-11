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
 * @file        cputime.c
 *
 * @brief       This file provides functions for cputime calculation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_device.h>
#include <os_errno.h>
#include <os_hw.h>
#include <os_clock.h>
#include <drv_cfg.h>
#include <string.h>
#include <timer/clocksource.h>

static os_clocksource_t *gs_best_cs = OS_NULL;

static os_list_node_t gs_clocksource_list = OS_LIST_INIT(gs_clocksource_list);

os_clocksource_t *os_clocksource_best(void)
{
    return gs_best_cs;
}

static void os_clocksource_update_cs(os_clocksource_t *cs)
{
    os_uint64_t nsec;
    os_uint64_t count;

    count = cs->read(cs);

    nsec = ((count - cs->last_update_count) & cs->mask) * cs->mult >> cs->shift;

    if (nsec > (cs->max_nsec / 2))
    {
        cs->last_update_count = count;
        cs->last_update_nsec += nsec;
    }
}

void os_clocksource_update(void)
{
    os_base_t level;
    os_clocksource_t *cs;

    level = os_hw_interrupt_disable();

    os_list_for_each_entry(cs, &gs_clocksource_list, os_clocksource_t, list)
    {
        os_clocksource_update_cs(cs);
    }

    os_hw_interrupt_enable(level);
}

static os_uint64_t os_clocksource_gettime_cs(os_clocksource_t *cs)
{
    os_base_t   level;
    os_uint64_t nsec;
    os_uint64_t count;

    level = os_hw_interrupt_disable();
    
    count = cs->read(cs) - cs->last_update_count;
    nsec  = cs->last_update_nsec;

    os_hw_interrupt_enable(level);

    nsec += (count & cs->mask) * cs->mult >> cs->shift;

    return nsec;
}

os_uint64_t os_clocksource_gettime(void)
{
    if (gs_best_cs != OS_NULL)
    {
        return os_clocksource_gettime_cs(gs_best_cs);
    }
    else
    {
        return (os_uint64_t)os_tick_get() * NSEC_PER_SEC / OS_TICK_PER_SECOND;
    }
}

static void __os_clocksource_ndelay(os_uint64_t nsec)
{
    os_uint64_t i;
    for (i = 0; i < nsec; i++);
}

void os_clocksource_ndelay(os_uint64_t nsec)
{
    os_uint64_t count_tmp, count_delta, count_half;

    os_clocksource_t *cs = gs_best_cs;

    if (cs == OS_NULL)
    {
        __os_clocksource_ndelay(nsec);
        return;
    }

    count_tmp   = cs->read(cs);
    count_delta = nsec * cs->mult_t >> cs->shift_t;
    count_half  = (cs->mask >> 1) + 1;

    while (count_delta > 0)
    {
        if (count_delta > cs->mask)
        {
            count_tmp   += count_half;
            count_delta -= count_half;
        }
        else if (count_delta > count_half)
        {
            count_tmp   += count_delta >> 1;
            count_delta -= count_delta >> 1;
        }
        else
        {
            count_tmp   += count_delta;
            count_delta  = 0;
        }

        while (((count_tmp  - cs->read(cs)) & cs->mask) <= count_half);
    }
}

static void os_clocksource_select(void)
{
    if (gs_best_cs != OS_NULL)
    {
        os_device_close(&gs_best_cs->parent);
    }

    gs_best_cs = os_list_first_entry_or_null(&gs_clocksource_list, os_clocksource_t, list);
    
    OS_ASSERT(gs_best_cs != OS_NULL);
    OS_ASSERT(os_device_open(&gs_best_cs->parent, 0) == OS_EOK);
}

static void os_clocksource_enqueue(os_clocksource_t *cs)
{
    os_list_node_t *entry = &gs_clocksource_list;
    os_clocksource_t *tmp;

    os_list_for_each_entry(tmp, &gs_clocksource_list, os_clocksource_t, list)
    {
        if (tmp->rating < cs->rating)
            break;
        entry = &tmp->list;
    }
    os_list_add(entry, &cs->list);
}

static os_bool_t os_clocksource_valid(os_clocksource_t *cs)
{
    volatile int i = 10000;
    os_uint64_t start, end;

    start = cs->read(cs);
    
    while (i-- && start == (end = cs->read(cs)));

    return (start != end);
}

void os_clocksource_register(const char *name, os_clocksource_t *cs)
{
    OS_ASSERT(cs != OS_NULL);
    OS_ASSERT(cs->read != OS_NULL);

    memcpy(cs->name, name, min(strlen(name) + 1, OS_CLOCKSOURCE_NAME_LENGTH));
    cs->name[OS_CLOCKSOURCE_NAME_LENGTH - 1] = 0;

    os_uint64_t msec = MSEC_PER_SEC * cs->mask / cs->freq;
    if (!msec)
        msec = 1;
    else if (msec > 600000 && cs->mask > (~0u))
        msec = 600000;

    cs->max_nsec = msec * NSEC_PER_MSEC;
    cs->min_nsec = NSEC_PER_SEC / cs->freq;

    calc_mult_shift(&cs->mult, &cs->shift, cs->freq, NSEC_PER_SEC, msec / MSEC_PER_SEC);
    calc_mult_shift(&cs->mult_t, &cs->shift_t, NSEC_PER_SEC, cs->freq, msec / MSEC_PER_SEC);

    if (!os_clocksource_valid(cs))
    {
        os_kprintf("invalid clocksource %s.\r\n", name);
        return;
    }

    cs->parent.type = OS_DEVICE_TYPE_CLOCKSOURCE;
    os_device_register(&cs->parent, name, OS_DEVICE_FLAG_STANDALONE);

    os_clocksource_enqueue(cs);
    os_clocksource_select();
}

void os_hw_us_delay(os_uint32_t us)
{
    os_clocksource_ndelay(us * NSEC_PER_USEC);
}

#if defined(OS_USING_SHELL) && defined(OS_CLOCKSOURCE_SHOW)

#include <shell.h>

static void os_clocksource_show(void)
{
    os_base_t level;
    os_clocksource_t *cs;

    os_kprintf("clocksource:\r\n\r\n");

    level = os_hw_interrupt_disable();

    os_list_for_each_entry(cs, &gs_clocksource_list, os_clocksource_t, list)
    {
        os_kprintf("name:%s\r\n"
                   "rating:%u\r\n"
                   "freq:%u\r\n"
                   "mask:%Lx    %Lu\r\n"
                   "min_nsec:%Lu max_nsec:%Lu\r\n"
                   "last_update_nsec: %Lu\r\n"
                   "last_update_count:%Lu\r\n"
                   "current nsec: %Lu\r\n\r\n",
                   cs->name,
                   cs->rating,
                   cs->freq,
                   cs->mask, cs->mask,
                   cs->min_nsec,cs->max_nsec,
                   cs->last_update_nsec,
                   cs->last_update_count,
                   os_clocksource_gettime_cs(cs));
    }

    os_kprintf("best clocksource is %s\r\n\r\n", gs_best_cs->name);

    os_hw_interrupt_enable(level);
}

SH_CMD_EXPORT(list_clocksource, os_clocksource_show, "list_clocksource");

#endif /* OS_USING_SHELL */

