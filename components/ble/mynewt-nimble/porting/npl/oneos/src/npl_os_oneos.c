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
 * @file        npl_os_oneos.c
 *
 * @brief       The function invoke oneos system api.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <os_kernel.h>
#include "nimble/nimble_npl.h"

#ifndef OS_USING_HEAP
#error "NimBLE npl depend on 'OS_USING_HEAP'"
#endif

static void npl_oneos_callout_init(struct ble_npl_callout *co, struct ble_npl_eventq *evq, ble_npl_event_fn *ev_cb, void *ev_arg);
static ble_npl_error_t npl_oneos_callout_reset(struct ble_npl_callout *co, ble_npl_time_t ticks);
static ble_npl_time_t npl_oneos_callout_remaining_ticks(struct ble_npl_callout *co, ble_npl_time_t now);
static ble_npl_error_t npl_oneos_time_ms_to_ticks(uint32_t ms, ble_npl_time_t *out_ticks);
static ble_npl_error_t npl_oneos_time_ticks_to_ms(ble_npl_time_t ticks, uint32_t *out_ms);

void *ble_npl_get_current_task_id(void)
{
    return os_task_self();
}

void ble_npl_eventq_init(struct ble_npl_eventq *evq)
{
    evq->q = os_mq_create("npl_evq", sizeof(struct ble_npl_event *), 32, OS_IPC_FLAG_FIFO);
    return;
}

struct ble_npl_event *ble_npl_eventq_get(struct ble_npl_eventq *evq, ble_npl_time_t timeout)
{
    struct ble_npl_event *ev = NULL;
    os_mq_t *os_mq = (os_mq_t *)evq->q;
    void *buffer = &ev;
    os_size_t size = sizeof(struct ble_npl_event *);
    os_size_t recv_size;

    os_mq_recv(os_mq, buffer, size, timeout, &recv_size);
    if (ev)
    {
        ev->queued = false;
    }

    return ev;
}

void ble_npl_eventq_put(struct ble_npl_eventq *evq, struct ble_npl_event *ev)
{
    int ret;

    if (ev->queued)
    {
        return;
    }

    ev->queued = true;
    ret = os_mq_send((os_mq_t *)evq->q, &ev, sizeof(struct ble_npl_event *), OS_IPC_WAITING_NO);

    OS_ASSERT(ret == OS_EOK);
    return;
}

void ble_npl_eventq_remove(struct ble_npl_eventq *evq, struct ble_npl_event *ev)
{
    if (!ev->queued)
    {
        return;
    }

    int i;
    int count;
    os_base_t level;
    int ret;

    struct ble_npl_event *tmp_ev = NULL;
    os_mq_t *os_mq = (os_mq_t *)evq->q;
    void *buffer = &tmp_ev;
    os_size_t size = sizeof(struct ble_npl_event *);
    os_size_t recv_size;

    // os_mq_control((os_mq_t *)evq->q, (os_ipc_cmd_t)OS_IPC_CMD_RESET, OS_NULL);
    level = os_hw_interrupt_disable();

    count = ((os_mq_t *)evq->q)->entry;
    for (i = 0; i < count; i++)
    {
        ret = os_mq_recv(os_mq, buffer, size, OS_IPC_WAITING_NO, &recv_size);
        if (OS_EEMPTY == ret)
        {
            break;
        }

        if (tmp_ev == ev)
        {
            continue;
        }

        ret = os_mq_send(os_mq, buffer, size, OS_IPC_WAITING_NO);
        OS_ASSERT(ret == OS_EOK);
    }
    os_hw_interrupt_enable(level);

    ev->queued = false;
    return;
}

bool ble_npl_eventq_is_empty(struct ble_npl_eventq *evq)
{
    int count;
    os_base_t level;

    level = os_hw_interrupt_disable();
    count = ((os_mq_t *)evq->q)->entry;
    os_hw_interrupt_enable(level);

    return count ? true : false;
}

void ble_npl_event_run(struct ble_npl_event *ev)
{
    ev->fn(ev);
}

void ble_npl_event_init(struct ble_npl_event *ev, ble_npl_event_fn *fn,
                        void *arg)
{
    memset(ev, 0, sizeof(*ev));
    ev->fn = fn;
    ev->arg = arg;
}

bool ble_npl_event_is_queued(struct ble_npl_event *ev)
{
    return ev->queued;
}

void *ble_npl_event_get_arg(struct ble_npl_event *ev)
{
    return ev->arg;
}

void ble_npl_event_set_arg(struct ble_npl_event *ev, void *arg)
{
    ev->arg = arg;
}

ble_npl_error_t ble_npl_mutex_init(struct ble_npl_mutex *mu)
{
    if (!mu)
    {
        return BLE_NPL_INVALID_PARAM;
    }

    mu->handle = os_mutex_create("npl_mtx", OS_IPC_FLAG_FIFO, OS_TRUE);
    OS_ASSERT(mu->handle);

    return BLE_NPL_OK;
}

ble_npl_error_t ble_npl_mutex_pend(struct ble_npl_mutex *mu, ble_npl_time_t timeout)
{
    if (!mu)
    {
        return BLE_NPL_INVALID_PARAM;
    }

    os_mutex_t *os_mutex = (os_mutex_t *)(mu->handle);
    os_err_t ret;

    ret = os_mutex_recursive_lock(os_mutex, timeout);
    return (ret == OS_EOK) ? BLE_NPL_OK : BLE_NPL_TIMEOUT;
}

ble_npl_error_t ble_npl_mutex_release(struct ble_npl_mutex *mu)
{
    if (!mu)
    {
        return BLE_NPL_INVALID_PARAM;
    }

    os_mutex_t *os_mutex = (os_mutex_t *)(mu->handle);
    os_err_t ret;

    ret = os_mutex_recursive_unlock(os_mutex);
    return (ret == OS_EOK) ? BLE_NPL_OK : BLE_NPL_ERROR;
}

ble_npl_error_t ble_npl_sem_init(struct ble_npl_sem *sem, uint16_t tokens)
{
    if (!sem)
    {
        return BLE_NPL_INVALID_PARAM;
    }

    sem->handle = os_sem_create("npl_sem", tokens, OS_IPC_FLAG_FIFO);
    OS_ASSERT(sem->handle);

    return BLE_NPL_OK;
}

ble_npl_error_t ble_npl_sem_pend(struct ble_npl_sem *sem, ble_npl_time_t timeout)
{
    if (!sem)
    {
        return BLE_NPL_INVALID_PARAM;
    }

    os_sem_t *os_sem = (os_sem_t *)(sem->handle);
    os_err_t ret;

    ret = os_sem_wait(os_sem, timeout);
    return (ret == OS_EOK) ? BLE_NPL_OK : BLE_NPL_TIMEOUT;
}

ble_npl_error_t ble_npl_sem_release(struct ble_npl_sem *sem)
{
    int ret;

    if (!sem)
    {
        return BLE_NPL_INVALID_PARAM;
    }

    os_sem_t *os_sem = (os_sem_t *)(sem->handle);
    ret = os_sem_post(os_sem);

    return ret == OS_EOK ? BLE_NPL_OK : BLE_NPL_ERROR;
}

uint16_t ble_npl_sem_get_count(struct ble_npl_sem *sem)
{
    int count;
    os_base_t level;

    OS_ASSERT(sem->handle);

    level = os_hw_interrupt_disable();
    count = ((os_sem_t *)(sem->handle))->count;
    os_hw_interrupt_enable(level);

    return count;
}

void ble_npl_callout_init(struct ble_npl_callout *co, struct ble_npl_eventq *evq,
                          ble_npl_event_fn *ev_cb, void *ev_arg)
{
    npl_oneos_callout_init(co, evq, ev_cb, ev_arg);
}

ble_npl_error_t ble_npl_callout_reset(struct ble_npl_callout *co, ble_npl_time_t ticks)
{
    return npl_oneos_callout_reset(co, ticks);
}

void ble_npl_callout_stop(struct ble_npl_callout *co)
{
    if (co->handle)
    {
        os_timer_t *os_timer = (os_timer_t *)(co->handle);

        OS_ASSERT(os_timer);
        os_timer_stop(os_timer);
    }
}

bool ble_npl_callout_is_active(struct ble_npl_callout *co)
{
    return (((os_timer_t *)(co->handle))->parent.flag & OS_TIMER_FLAG_ACTIVATED) ? true : false;
}

ble_npl_time_t ble_npl_callout_get_ticks(struct ble_npl_callout *co)
{
    return ((os_timer_t *)(co->handle))->timeout_tick;
}

ble_npl_time_t ble_npl_callout_remaining_ticks(struct ble_npl_callout *co,
                                               ble_npl_time_t time)
{
    return npl_oneos_callout_remaining_ticks(co, time);
}

void ble_npl_callout_set_arg(struct ble_npl_callout *co, void *arg)
{
    co->ev.arg = arg;
}

ble_npl_time_t ble_npl_time_get(void)
{
    return os_tick_get();
}

ble_npl_error_t ble_npl_time_ms_to_ticks(uint32_t ms, ble_npl_time_t *out_ticks)
{
    return npl_oneos_time_ms_to_ticks(ms, out_ticks);
}

ble_npl_error_t ble_npl_time_ticks_to_ms(ble_npl_time_t ticks, uint32_t *out_ms)
{
    return npl_oneos_time_ticks_to_ms(ticks, out_ms);
}

ble_npl_time_t ble_npl_time_ms_to_ticks32(uint32_t ms)
{
    return ((ms * OS_TICK_PER_SECOND) / 1000);
}

uint32_t ble_npl_time_ticks_to_ms32(ble_npl_time_t ticks)
{
    return ((ticks * 1000) / OS_TICK_PER_SECOND);
}

void ble_npl_time_delay(ble_npl_time_t ticks)
{
    os_task_delay(ticks);
}

uint32_t ble_npl_hw_enter_critical(void)
{
    return os_hw_interrupt_disable();
}

void ble_npl_hw_exit_critical(uint32_t ctx)
{
    os_hw_interrupt_enable(ctx);
}

#include "cmsis_compiler.h"
bool ble_npl_hw_is_in_critical(void)
{
    /*
     * XXX Currently OneOS does not support an API for finding out if interrupts
     *     are currently disabled, hence in a critical section in this context.
     *     So for now, we use this func to get state when using arm cortex-M.
    -*/
    return __get_PRIMASK();
}

static void os_callout_timer_cb(void *parameter)
{
    struct ble_npl_callout *co;

    co = (struct ble_npl_callout *)parameter;
    OS_ASSERT(co);

    if (co->evq)
    {
        ble_npl_eventq_put(co->evq, &co->ev);
    }
    else
    {
        co->ev.fn(&co->ev);
    }
}

static void npl_oneos_callout_init(struct ble_npl_callout *co, struct ble_npl_eventq *evq,
                                   ble_npl_event_fn *ev_cb, void *ev_arg)
{
    memset(co, 0, sizeof(*co));

    co->handle = os_timer_create("co", os_callout_timer_cb, co, 0, (OS_TIMER_FLAG_ONE_SHOT | OS_TIMER_FLAG_SOFT_TIMER));
    co->evq = evq;
    ble_npl_event_init(&co->ev, ev_cb, ev_arg);
}

static ble_npl_error_t npl_oneos_callout_reset(struct ble_npl_callout *co, ble_npl_time_t ticks)
{
    if (ticks == 0)
    {
        ticks = 1;
    }

    os_timer_stop((os_timer_t *)co->handle);
    os_timer_control((os_timer_t *)co->handle, OS_TIMER_CTRL_SET_TIME, &ticks);
    os_timer_start((os_timer_t *)co->handle);

    return BLE_NPL_OK;
}

static ble_npl_time_t npl_oneos_callout_remaining_ticks(struct ble_npl_callout *co,
                                                        ble_npl_time_t now)
{
    ble_npl_time_t rt;
    uint32_t exp;
    os_base_t level;

    level = os_hw_interrupt_disable();
    exp = ((os_timer_t *)co->handle)->timeout_tick;
    os_hw_interrupt_enable(level);

    if (exp > now)
    {
        rt = exp - now;
    }
    else
    {
        rt = 0;
    }

    return rt;
}

static ble_npl_error_t npl_oneos_time_ms_to_ticks(uint32_t ms, ble_npl_time_t *out_ticks)
{
    uint64_t ticks;

    ticks = os_tick_from_ms(ms);
    if (ticks > UINT32_MAX)
    {
        return BLE_NPL_EINVAL;
    }
    *out_ticks = ticks;

    return BLE_NPL_OK;
}

static ble_npl_error_t npl_oneos_time_ticks_to_ms(ble_npl_time_t ticks, uint32_t *out_ms)
{
    uint64_t ms;

    ms = ((uint64_t)ticks * 1000) / OS_TICK_PER_SECOND;
    if (ms > UINT32_MAX)
    {
        return BLE_NPL_EINVAL;
    }
    *out_ms = ms;

    return BLE_NPL_OK;
}
