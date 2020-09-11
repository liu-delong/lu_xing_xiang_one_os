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
 * @file        hwtimer.c
 *
 * @brief       this file implements hwtimer related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_device.h>
#include <os_errno.h>
#include <drv_cfg.h>
#include <string.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>

static os_clockevent_t *gs_best_ce = OS_NULL;

static os_list_node_t gs_clockevent_list = OS_LIST_INIT(gs_clockevent_list);

os_clockevent_t *os_clockevent_best(void)
{
    return gs_best_ce;
}

static void os_clockevent_enqueue(os_clockevent_t *ce)
{
    os_list_node_t *entry = &gs_clockevent_list;
    os_clockevent_t *tmp;

    os_list_for_each_entry(tmp, &gs_clockevent_list, os_clockevent_t, list)
    {
        if (tmp->rating < ce->rating)
            break;
        entry = &tmp->list;
    }
    os_list_add(entry, &ce->list);
}

static os_bool_t event_flag;

static void os_clockevent_valid_handler(os_clockevent_t *ce)
{
    event_flag = OS_TRUE;
}

static os_bool_t os_clockevent_valid(os_clockevent_t *ce)
{
    volatile int i = 1000000;

    event_flag = OS_FALSE;

    ce->event_handler = os_clockevent_valid_handler;

    ce->ops->start(ce, 1 & ce->prescaler_mask, 1);

    while (!event_flag && i--);

    ce->ops->stop(ce);

    ce->event_handler = OS_NULL;

    return event_flag;
}

void os_clockevent_select_best(void)
{
    os_clockevent_t *ce;

    if (gs_best_ce != OS_NULL)
    {
        os_device_close(&gs_best_ce->parent);
    }

    os_list_for_each_entry(ce, &gs_clockevent_list, os_clockevent_t, list)
    {
        if (os_clockevent_valid(ce))
        {
            gs_best_ce = ce;
            break;
        }
        else
        {
            os_kprintf("invalid clockevent %s.\r\n", ce->name);
        }
    }

    OS_ASSERT(gs_best_ce != NULL);
    OS_ASSERT(os_device_open(&gs_best_ce->parent, 0) == OS_EOK);
}

void os_clockevent_register_isr(os_clockevent_t *ce, void (*event_handler)(os_clockevent_t *ce))
{
    if (ce == gs_best_ce && ce->event_handler != OS_NULL)
    {
        os_kprintf("best ce handler replace from %p to %p\r\n", ce->event_handler, event_handler);
    }

    ce->event_handler = event_handler;
}

os_uint64_t os_clockevent_read(os_clockevent_t *ce)
{
    return ce->ops->read(ce);
}

static os_bool_t os_clockevent_auto_period(os_clockevent_t *ce)
{
    return ((ce->feature == OS_CLOCKEVENT_FEATURE_PERIOD)
            && ce->period_count != 0 
            && (ce->period_count & ~ce->count_mask) == 0);
}

static int os_clockevent_calc_param(os_clockevent_t *ce)
{
    os_uint64_t nsec;
    os_uint64_t evt;
    os_uint64_t count;
    os_uint32_t prescaler;
    os_int32_t  trig_isr = 0;

    nsec = os_clocksource_gettime();

    /* reserve 5000 nsec */
    if (ce->next_nsec <= (nsec + 5000))
    {
        if (ce->period_nsec == 0)
        {
            ce->count = 0;
            return 1;
        }
        else
        {
            while (ce->next_nsec <= nsec)
            {
                ce->next_nsec += ce->period_nsec;
                trig_isr++;
            }
        }
    }

    if (os_clockevent_auto_period(ce))
    {
        prescaler = 1 & ce->prescaler_mask;
        count     = ce->period_count;
    }
    else
    {
        nsec = ce->next_nsec - nsec;
        nsec = max(nsec, ce->min_nsec);
        evt  = nsec * ce->mult >> ce->shift;
    
        if ((evt & ~ce->count_mask) == 0)
        {
            prescaler = 1 & ce->prescaler_mask;
            count     = evt;
        }
        else
        {
            if (((evt & ce->count_mask) > (ce->count_mask / 2)) || ((evt >> ce->count_bits) > 1))
            {
                prescaler = (evt >> ce->count_bits) & ce->prescaler_mask;
                count     = ce->count_mask;
            }
            else
            {
                prescaler = 1 & ce->prescaler_mask;
                count     = ce->count_mask / 2;
            }
        }
    }

    if (count == 0)
    {
        count = 1;
    }

    ce->prescaler = prescaler;
    ce->count     = count;

    OS_ASSERT(count != 0);

    return trig_isr;
}

static int os_clockevent_next(os_clockevent_t *ce, os_bool_t force_trig)
{
    os_int32_t trig_isr = os_clockevent_calc_param(ce);

    if (ce->count != 0)
    {
        ce->ops->start(ce, ce->prescaler, ce->count);
    }
    else if (force_trig)
    {
        ce->prescaler = 1 & ce->prescaler_mask;
        ce->count     = 1;
        ce->ops->start(ce, ce->prescaler, ce->count);
    }
    
    return trig_isr;
}

void os_clockevent_start_oneshot(os_clockevent_t *ce, os_uint64_t nsec)
{
    os_base_t level;
    
    OS_ASSERT(ce != NULL);

    nsec = max(nsec, ce->min_nsec);

    ce->next_nsec    = os_clocksource_gettime() + nsec;
    ce->period_nsec  = 0;
    ce->period_count = 0;

    level = os_hw_interrupt_disable();
    os_clockevent_next(ce, OS_TRUE);
    os_hw_interrupt_enable(level);
}

void os_clockevent_start_period(os_clockevent_t *ce, os_uint64_t nsec)
{
    os_base_t level;
    
    OS_ASSERT(ce != NULL);

    nsec = max(nsec, ce->min_nsec);

    ce->next_nsec    = os_clocksource_gettime() + nsec;
    ce->period_nsec  = nsec;
    ce->period_count = nsec * ce->mult >> ce->shift;

    if (ce->period_count == 0)
    {
        ce->period_count = 1;
    }

    level = os_hw_interrupt_disable();
    os_clockevent_next(ce, OS_TRUE);
    os_hw_interrupt_enable(level);
}

void os_clockevent_stop(os_clockevent_t *ce)
{
    OS_ASSERT(ce != NULL);

    ce->next_nsec   = 0;
    ce->period_nsec = 0;

    if (ce->ops->stop != NULL)
    {
        ce->ops->stop(ce);
    }
}

void os_clockevent_isr(os_clockevent_t *ce)
{
    if (os_clockevent_auto_period(ce))
    {
        os_interrupt_enter();
        if (ce->event_handler)
            ce->event_handler(ce);
        os_interrupt_leave();
    }
    else
    {
        ce->ops->stop(ce);
        if (os_clockevent_next(ce, OS_FALSE) != 0)
        {
            os_interrupt_enter();
            if (ce->event_handler)
                ce->event_handler(ce);
            os_interrupt_leave();
        }
    }
}

os_err_t os_clockevent_close(os_device_t *dev)
{
    OS_ASSERT(dev != OS_NULL);

    os_clockevent_t *ce;

    ce = (os_clockevent_t *)dev;

    os_clockevent_stop(ce);
    os_clockevent_register_isr(ce, OS_NULL);
    return OS_EOK;
}

#ifdef OS_USING_DEVICE_OPS
const struct os_device_ops _clockevent_ops =
{
    .close = os_clockevent_close;
};
#endif

void os_clockevent_register(const char *name, os_clockevent_t *ce)
{
    OS_ASSERT(ce != OS_NULL);
    OS_ASSERT(ce->ops != OS_NULL);
    OS_ASSERT(ce->feature == OS_CLOCKEVENT_FEATURE_ONESHOT || ce->feature == OS_CLOCKEVENT_FEATURE_PERIOD);
    
    memcpy(ce->name, name, min(strlen(name) + 1, OS_CLOCKEVENT_NAME_LENGTH));
    ce->name[OS_CLOCKEVENT_NAME_LENGTH - 1] = 0;

    os_uint64_t msec = ce->mask / ce->freq;
    if (!msec)
        msec = 1;
    else if (msec > 600000 && ce->mask > (~0u))
        msec = 600000;

    ce->max_nsec = msec * NSEC_PER_MSEC;

    calc_mult_shift(&ce->mult, &ce->shift, NSEC_PER_SEC, ce->freq, msec / MSEC_PER_SEC);

#ifdef OS_USING_DEVICE_OPS
    ce->parent.ops   = &_clockevent_ops;
#else
    ce->parent.close = os_clockevent_close;
#endif

    ce->parent.type  = OS_DEVICE_TYPE_CLOCKEVENT;
    os_device_register(&ce->parent, name, OS_DEVICE_FLAG_STANDALONE);

    os_clockevent_enqueue(ce);
}

#if defined(OS_USING_SHELL) && defined(OS_CLOCKEVENT_SHOW)

#include <shell.h>

void os_clockevent_show(void)
{
    os_base_t level;
    os_clockevent_t *ce;

    os_kprintf("clockevent:\r\n\r\n");

    level = os_hw_interrupt_disable();

    os_list_for_each_entry(ce, &gs_clockevent_list, os_clockevent_t, list)
    {
        os_kprintf("name:%s\r\n"
                   "rating:%u\r\n"
                   "freq:%u\r\n"
                   "mask:%Lx    %Lu\r\n"
                   "min_nsec:%Lu max_nsec:%Lu\r\n"
                   "next_nsec:%Lu period_nsec:%Lu\r\n"
                   "event_handler:%p\r\n\r\n",
                   ce->name,
                   ce->rating,
                   ce->freq,
                   ce->mask, ce->mask,
                   ce->min_nsec,ce->max_nsec,
                   ce->next_nsec, ce->period_nsec,
                   ce->event_handler);
    }

    os_kprintf("best clockevent is %s\r\n\r\n", gs_best_ce->name);

    os_hw_interrupt_enable(level);
}

SH_CMD_EXPORT(list_clockevent, os_clockevent_show, "list_clockevent");

#endif /* OS_USING_SHELL */

