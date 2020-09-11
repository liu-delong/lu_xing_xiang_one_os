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
 * @file        cputime.h
 *
 * \@brief      This file provides struct definition and cputime functions declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef CLOCKEVENT_H__
#define CLOCKEVENT_H__

#include <drivers.h>
#include <os_types.h>
#include <os_hw.h>
#include <os_irq.h>
#include <timer/timer.h>

typedef enum
{
    OS_CLOCKEVENT_FEATURE_ONESHOT,
    OS_CLOCKEVENT_FEATURE_PERIOD,
} os_clockevent_featrue_t;

enum os_clockevent_state {
    OS_CLOCKEVENT_STATE_DETACHED,
    OS_CLOCKEVENT_STATE_SHUTDOWN,
    OS_CLOCKEVENT_STATE_PERIODIC,
    OS_CLOCKEVENT_STATE_ONESHOT,
    OS_CLOCKEVENT_STATE_ONESHOT_STOPPED,
};

typedef struct os_clockevent os_clockevent_t;

struct os_clockevent_ops
{
    void (*start)(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count);
    void (*stop)(os_clockevent_t *ce);
    os_uint64_t (*read)(void *clock);
};

#define OS_CLOCKEVENT_NAME_LENGTH  (16)

struct os_clockevent {
    os_device_t parent;
    
    char name[OS_CLOCKEVENT_NAME_LENGTH];

    void (*event_handler)(os_clockevent_t *ce);
    
    const struct os_clockevent_ops *ops;

    os_list_node_t list;

    os_uint32_t rating;
    os_uint32_t freq;

    os_uint32_t mult;       /* count = time(ns) * mult >> shift */
    os_uint32_t shift;

    os_uint64_t mask;
    os_uint64_t min_nsec;
    os_uint64_t max_nsec;

    os_uint64_t next_nsec;
    os_uint64_t period_nsec;
    os_uint64_t period_count;

    os_uint32_t prescaler;
    os_uint32_t prescaler_mask;
    
    os_uint64_t count;
    os_uint64_t count_mask;

    os_uint8_t  prescaler_bits;
    os_uint8_t  count_bits;
    
    os_uint8_t  feature;
};

os_clockevent_t *os_clockevent_best(void);

os_uint64_t os_clockevent_read(os_clockevent_t *ce);

void os_clockevent_select_best(void);
void os_clockevent_register_isr(os_clockevent_t *ce, void (*event_handler)(os_clockevent_t *ce));
void os_clockevent_start_oneshot(os_clockevent_t *ce, os_uint64_t nsec);
void os_clockevent_start_period(os_clockevent_t *ce, os_uint64_t nsec);
void os_clockevent_stop(os_clockevent_t *ce);
void os_clockevent_isr(os_clockevent_t *ce);
void os_clockevent_register(const char *name, os_clockevent_t *ce);

#endif  /* CLOCKEVENT_H__ */

