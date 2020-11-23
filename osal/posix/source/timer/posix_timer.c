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
 * @file        posix_timer.c
 *
 * @brief       This file provides posix time related operations.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <pthread.h>
#include <os_timer.h>
#include <shell.h>
#include <stdio.h>
#include <time.h>
#include "posix_timer.h"


#define timer_abs(a, b) (((a) > (b)) ? ((a)-(b)) : ((b)-(a)))
typedef void (*sigev_notify_function)(union sigval); /* Notification function. */

typedef struct _timer_posix 
{
    void (*sigev_notify_function)(union sigval);
    os_timer_t          *timer;
    union sigval        sigev_value;
    struct itimerspec   value;
    unsigned long       overrun_count;
} timer_posix_t;

void timer_timeout_handle(void *parameter)
{
    sigev_notify_function func = NULL;
    timer_posix_t *tp          = (timer_posix_t*)parameter;
    os_uint64_t ns             = 0;
    os_tick_t tick             = 0;

    OS_ASSERT(tp);
    func = tp->sigev_notify_function;
    OS_ASSERT(func);
    func(tp->sigev_value);
   
    ns   = tp->value.it_interval.tv_sec * NANOSECOND_PER_SECOND + tp->value.it_interval.tv_nsec;
    tick = (double)ns / NANOSECOND_PER_TICK;
    tp->overrun_count++;
    OS_ASSERT(tp->timer);
    os_timer_control(tp->timer, OS_TIMER_CTRL_SET_TIME, &tick);
}

/**
 * @brief Create a per-process timer.
 *
 * This API does not accept SIGEV_SIGNAL as valid signal event notification
 * type.
 *
 * Only support the CLOCK_MONOTONIC
 *
 * See IEEE 1003.1
 */
int timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid)
{
    timer_posix_t *tp    = NULL;
    os_timer_t *os_timer = NULL;

    /* OS_ASSERT(CLOCK_REALTIME == clockid); */
    OS_ASSERT(timerid);
    OS_ASSERT(evp);

    if ((SIGEV_SIGNAL == evp->sigev_notify) || (CLOCK_MONOTONIC != clockid)) {
        return -1;
    }

    tp = (timer_posix_t*)os_malloc(sizeof(timer_posix_t));

    if (!tp) {
        return -1;
    }

    /* memset the tp data for init */
    char *pv = (char*)tp;
    for (long i = 0; i < sizeof(timer_posix_t); i++) {
        *pv++ = 0x00;
    }

    tp->overrun_count         = 0;
    tp->sigev_notify_function = evp->sigev_notify_function;
    tp->sigev_value           = evp->sigev_value;
    os_timer = os_timer_create("posix_timer",
                                timer_timeout_handle,
                                tp,
                                0,
                                OS_TIMER_FLAG_DEACTIVATED|OS_TIMER_FLAG_SOFT_TIMER); // prioritize the soft timer
    tp->timer = os_timer;
    *timerid  = (timer_t)tp;

    return 0;
}

int timer_delete(timer_t timerid)
{
    timer_posix_t *tp = (timer_posix_t*)timerid;

    OS_ASSERT(tp);
    os_timer_stop(tp->timer);
    os_timer_destroy(tp->timer);
    os_free(tp);

    return 0;
}

/**
 * @brief Get overrun count, os_timer is synchronized,
 *        so timer do not occure overrun, anyway return 0
 *
 * @param timerid, timerid
 *
 * @return 0, no error
 */
int timer_getoverrun(timer_t timerid)
{
    timer_posix_t *tp = (timer_posix_t*)timerid;

    OS_ASSERT(tp);
    return 0;
}

int timer_gettime(timer_t timerid, struct itimerspec *value)
{
    timer_posix_t *tp = (timer_posix_t*)timerid;
    int tick          = os_tick_get();

    OS_ASSERT(tp);
    OS_ASSERT(value);

    if (!tp->timer->timeout_tick) {
        value->it_interval.tv_sec  = 0;
        value->it_interval.tv_nsec = 0;
        value->it_value.tv_sec     = 0;
        value->it_value.tv_nsec    = 0;
        return 0;
    }

    os_uint64_t nsec        = timer_abs(tp->timer->timeout_tick, tick) * NANOSECOND_PER_TICK;
    value->it_interval      = tp->value.it_interval;
    value->it_value.tv_sec  = nsec / NANOSECOND_PER_SECOND;
    value->it_value.tv_nsec = nsec % NANOSECOND_PER_SECOND;

    return 0;
}

/**
 * @brief Set the timer expiration time
 *
 * @param timerid
 * @param flags, only support the relative time, do not support TIMER_ABSTIME
 * @param value, the expire time to be set
 * @param ovalue, save the previous expire time
 *
 * @return =0, no error or error
 */
int timer_settime(timer_t timerid, int flags, const struct itimerspec *value, struct itimerspec *ovalue)
{
    timer_posix_t *tp = (timer_posix_t*)timerid;

    OS_ASSERT(tp);
    OS_ASSERT(value);

    if (flags) { // don't support TIMER_ABSTIME
        return -1;
    }

    if (ovalue) { // save the prev itimerspec value
        *ovalue = tp->value;
    }

    os_uint64_t ns       = value->it_value.tv_sec * NANOSECOND_PER_SECOND + value->it_value.tv_nsec;
    tp->value            = *value;
    tp->timer->init_tick = (double)ns / NANOSECOND_PER_TICK;

    os_timer_control(tp->timer, OS_TIMER_CTRL_SET_PERIODIC, NULL);
    os_timer_start(tp->timer);

    return 0;
}

