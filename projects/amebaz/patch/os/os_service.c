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
 * @file        os_service.c
 *
 * @brief       This file provide OS interfaces for ameba SDK.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <osdep_service.h>

#include <os_service.h>
#ifdef SECTION
#undef SECTION
#endif
#include <os_hw.h>
#include <os_kernel.h>
#include <os_util.h>
#include <stdio.h>

//#define ONEOS_SERVICE_DEBUG
#define ONEOS_SERVICE_DEBUG_LEVEL 2

#ifdef ONEOS_SERVICE_DEBUG
#define DEBUG_LOG(_level, fmt, args...)                                                                                \
    if ((_level) >= ONEOS_SERVICE_DEBUG_LEVEL)                                                                         \
        os_kprintf(fmt, args)
#else
#define DEBUG_LOG(level, fmt, args...)
#endif

#ifndef USE_MUTEX_FOR_SPINLOCK
#define USE_MUTEX_FOR_SPINLOCK 1
#endif

extern _LONG_CALL_ void DelayUs(u32 us);
extern _LONG_CALL_ void DelayMs(u32 ms);

extern uint32_t pmu_yield_os_check(void);
extern uint32_t pmu_set_sysactive_time(uint32_t timeout_ms);

void save_and_cli(void)
{
    DEBUG_LOG(1, "L:%d fun:%s runing...\n", __LINE__, __FUNCTION__);
    os_enter_critical();
}

void restore_flags(void)
{
    DEBUG_LOG(1, "L:%d fun:%s runing...\n", __LINE__, __FUNCTION__);
    os_exit_critical();
}

void cli(void)
{
    DEBUG_LOG(1, "L:%d fun:%s runing...\n", __LINE__, __FUNCTION__);
    os_hw_interrupt_disable();
}

/* Not needed on 64bit architectures */
static unsigned int __div64_32(u64 *n, unsigned int base)
{
    u64          rem = *n;
    u64          b   = base;
    u64          res, d = 1;
    unsigned int high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= base)
    {
        high /= base;
        res = (u64)high << 32;
        rem -= (u64)(high * base) << 32;
    }

    while ((u64)b > 0 && b < rem)
    {
        b = b + b;
        d = d + d;
    }

    do
    {
        if (rem >= b)
        {
            rem -= b;
            res += d;
        }
        b >>= 1;
        d >>= 1;
    } while (d);

    *n = res;
    return rem;
}

/********************* os depended service ********************/

u8 *_oneos_malloc(u32 sz)
{
    void *pbuf;

    DEBUG_LOG(2, "L:%d fun:%s sz:%d\n", __LINE__, __FUNCTION__, sz);
    pbuf = os_malloc(sz);
    return pbuf;
}

u8 *_oneos_zmalloc(u32 sz)
{
    void *pbuf = os_malloc(sz);

    DEBUG_LOG(2, "L:%d fun:%s sz:%d\n", __LINE__, __FUNCTION__, sz);
    if (pbuf != OS_NULL)
        memset(pbuf, 0, sz);

    return pbuf;
}

void _oneos_mfree(u8 *pbuf, u32 sz)
{
    DEBUG_LOG(2, "L:%d fun:%s\n", __LINE__, __FUNCTION__);
    os_free(pbuf);
}

static void _oneos_memcpy(void *dst, void *src, u32 sz)
{
    DEBUG_LOG(1, "L:%d fun:%s dst:0x%08x src:0x%08x sz:%d\n", __LINE__, __FUNCTION__, dst, src, sz);
    memcpy(dst, src, sz);
}

static int _oneos_memcmp(void *dst, void *src, u32 sz)
{
    // under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0
    DEBUG_LOG(1, "L:%d fun:%s dst:0x%08x src:0x%08x sz:%d\n", __LINE__, __FUNCTION__, dst, src, sz);
    if (!(memcmp(dst, src, sz)))
        return 1;

    return 0;
}

static void _oneos_memset(void *pbuf, int c, u32 sz)
{
    DEBUG_LOG(1, "L:%d fun:%s buf:0x%08x c:%c sz:%d\n", __LINE__, __FUNCTION__, pbuf, c, sz);
    memset(pbuf, c, sz);
}

static void _oneos_init_sema(_sema *sema, int init_val)
{
    char       name[OS_NAME_MAX];
    static int ameba_sem = 0;

    DEBUG_LOG(3, "L:%d fun:%s begin val:%d\n", __LINE__, __FUNCTION__, init_val);
    memset(name, 0, OS_NAME_MAX);
    snprintf(name, OS_NAME_MAX, "sem-%03d", ameba_sem);
    *sema = os_sem_create(name, init_val, OS_IPC_FLAG_FIFO);
    if (*sema != OS_NULL)
        ameba_sem++;
    DEBUG_LOG(3, "L:%d fun:%s end sema:0x%08x num:%d\n", __LINE__, __FUNCTION__, *sema, ameba_sem);
}

static void _oneos_free_sema(_sema *sema)
{
    OS_ASSERT(*sema != OS_NULL);

    DEBUG_LOG(2, "L:%d fun:%s sema:0x%08x\n", __LINE__, __FUNCTION__, *sema);
    os_sem_destroy(*sema);
    *sema = OS_NULL;
}

static void _oneos_up_sema(_sema *sema)
{
    if (*sema == OS_NULL)
    {
        os_kprintf("err!! up sema is NULL\n");
        return;
    }
    DEBUG_LOG(2, "L:%d fun:%s sema:0x%08x\n", __LINE__, __FUNCTION__, *sema);
    os_sem_post(*sema);
}

static void _oneos_up_sema_from_isr(_sema *sema)
{
    if (*sema == OS_NULL)
    {
        os_kprintf("err!! up sema from isr is NULL\n");
        return;
    }
    DEBUG_LOG(2, "L:%d fun:%s sema:0x%08x\n", __LINE__, __FUNCTION__, *sema);
    os_sem_post(*sema);
}

static u32 _oneos_down_sema(_sema *sema, u32 timeout)
{
    os_int32_t tick;

    OS_ASSERT(*sema != OS_NULL);

    DEBUG_LOG(2, "L:%d fun:%s sema:0x%08x timeout:%d\n", __LINE__, __FUNCTION__, *sema, timeout);

    if (timeout >= OS_TICK_MAX / 2)
        tick = OS_IPC_WAITING_FOREVER;
    else
        tick = rtw_ms_to_systime(timeout);

    if (os_sem_wait(*sema, tick) != OS_EOK)
        return OS_FALSE;

    return OS_TRUE;
}

static void _oneos_mutex_init(_mutex *pmutex)
{
    char       name[OS_NAME_MAX];
    static int ameba_mutex = 0;

    DEBUG_LOG(3, "L:%d fun:%s begin\n", __LINE__, __FUNCTION__);
    memset(name, 0, OS_NAME_MAX);
    snprintf(name, OS_NAME_MAX, "mux-%03d", ameba_mutex);
    *pmutex = os_mutex_create(name, OS_IPC_FLAG_FIFO, 0);
    if (*pmutex != OS_NULL)
        ameba_mutex++;
    DEBUG_LOG(3, "L:%d fun:%s end pmutex:0x%08x\n", __LINE__, __FUNCTION__, *pmutex);
}

static void _oneos_mutex_free(_mutex *pmutex)
{
    OS_ASSERT(*pmutex != OS_NULL);

    DEBUG_LOG(2, "L:%d fun:%s pmutex:0x%08x\n", __LINE__, __FUNCTION__, *pmutex);
    os_mutex_destroy(*pmutex);
    *pmutex = OS_NULL;
}

static void _oneos_mutex_get(_lock *plock)
{
    OS_ASSERT(*plock != OS_NULL);

    DEBUG_LOG(2, "L:%d fun:%s pmutex:0x%08x\n", __LINE__, __FUNCTION__, *plock);
    os_mutex_lock(*plock, OS_IPC_WAITING_FOREVER);
}

static int _oneos_mutex_get_timeout(_lock *plock, u32 timeout_ms)
{
    os_int32_t tick;

    OS_ASSERT(*plock != OS_NULL);

    DEBUG_LOG(2, "L:%d fun:%s plock:0x%08x timeout_ms:%d\n", __LINE__, __FUNCTION__, *plock, timeout_ms);
    if (timeout_ms >= OS_TICK_MAX / 2)
        tick = OS_IPC_WAITING_FOREVER;
    else
        tick = rtw_ms_to_systime(timeout_ms);

    return os_mutex_lock(*plock, tick);
}

static void _oneos_mutex_put(_lock *plock)
{
    if (*plock == OS_NULL)
    {
        os_kprintf("err!! mutex put is null\n");
        return;
    }
    DEBUG_LOG(2, "L:%d fun:%s pmutex:0x%08x\n", __LINE__, __FUNCTION__, *plock);
    os_mutex_unlock(*plock);
}

static void _oneos_enter_critical(_lock *plock, _irqL *pirqL)
{
    DEBUG_LOG(1, "L:%d fun:%s *pirqL:0x%08x\n", __LINE__, __FUNCTION__, *pirqL);
    os_enter_critical();
}

static void _oneos_exit_critical(_lock *plock, _irqL *pirqL)
{
    DEBUG_LOG(1, "L:%d fun:%s *pirqL:0x%08x\n", __LINE__, __FUNCTION__, *pirqL);
    os_exit_critical();
}

// static rt_base_t level;
static void _oneos_enter_critical_from_isr(_lock *plock, _irqL *pirqL)
{
    DEBUG_LOG(1, "L:%d fun:%s *pirqL:0x%08x\n", __LINE__, __FUNCTION__, *pirqL);
    *pirqL = os_hw_interrupt_disable();
}

static void _oneos_exit_critical_from_isr(_lock *plock, _irqL *pirqL)
{
    DEBUG_LOG(1, "L:%d fun:%s *pirqL:0x%08x\n", __LINE__, __FUNCTION__, *pirqL);
    os_hw_interrupt_enable(*pirqL);
}

static int _oneos_enter_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
    OS_ASSERT(*pmutex != OS_NULL);

    DEBUG_LOG(1, "L:%d fun:%s *pirqL:0x%08x\n", __LINE__, __FUNCTION__, *pirqL);
    if (os_mutex_lock(*pmutex, OS_IPC_WAITING_FOREVER) != OS_EOK)
        return OS_FALSE;

    return OS_TRUE;
}

static void _oneos_exit_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
    if (*pmutex == OS_NULL)
    {
        os_kprintf("err!! critical mutex is null\n");
        return;
    }
    DEBUG_LOG(1, "L:%d fun:%s *pirqL:0x%08x\n", __LINE__, __FUNCTION__, *pirqL);
    os_mutex_unlock(*pmutex);
}

static void _oneos_spinlock_init(_lock *plock)
{
#if USE_MUTEX_FOR_SPINLOCK
    char       name[OS_NAME_MAX];
    static int ameba_spin = 0;

    DEBUG_LOG(3, "L:%d fun:%s begin\n", __LINE__, __FUNCTION__);
    memset(name, 0, OS_NAME_MAX);
    snprintf(name, OS_NAME_MAX, "spn-03d", ameba_spin);
    *plock = os_mutex_create(name, OS_IPC_FLAG_FIFO, 0);
    if (*plock != OS_NULL)
        ameba_spin++;
    DEBUG_LOG(3, "L:%d fun:%s end plock:0x%08x ameba_spin:%d\n", __LINE__, __FUNCTION__, *plock, ameba_spin);
#endif
}

static void _oneos_spinlock_free(_lock *plock)
{
#if USE_MUTEX_FOR_SPINLOCK
    // OS_ASSERT(*plock != OS_NULL);
    if (*plock == OS_NULL)
        return;

    DEBUG_LOG(2, "L:%d fun:%s plock:0x%08x\n", __LINE__, __FUNCTION__, *plock);
    os_mutex_destroy(*plock);
    *plock = NULL;
#endif
}

static void _oneos_spinlock(_lock *plock)
{
#if USE_MUTEX_FOR_SPINLOCK
    OS_ASSERT(*plock != OS_NULL);

    DEBUG_LOG(2, "L:%d fun:%s plock:0x%08x\n", __LINE__, __FUNCTION__, *plock);
    os_mutex_lock(*plock, OS_IPC_WAITING_FOREVER);
#endif
}

static void _oneos_spinunlock(_lock *plock)
{
#if USE_MUTEX_FOR_SPINLOCK
    if (*plock == OS_NULL)
    {
        os_kprintf("err!! spinunlock is null\n");
        return;
    }
    DEBUG_LOG(2, "L:%d fun:%s plock:0x%08x\n", __LINE__, __FUNCTION__, *plock);
    os_mutex_unlock(*plock);
#endif
}

static void _oneos_spinlock_irqsave(_lock *plock, _irqL *irqL)
{
#if USE_MUTEX_FOR_SPINLOCK
    if (*plock == OS_NULL)
        os_kprintf("err!! spinlock irqsave null\n");

    DEBUG_LOG(2, "L:%d fun:%s plock:0x%08x irqL:0x%08x\n", __LINE__, __FUNCTION__, *plock, *irqL);
    *irqL = 0xdeadbeff;
    while (os_mutex_lock(*plock, 0) != OS_EOK)
    {
        os_kprintf("spinlock_irqsave failed!\n");
    }
#endif
}

static void _oneos_spinunlock_irqsave(_lock *plock, _irqL *irqL)
{
#if USE_MUTEX_FOR_SPINLOCK
    if (*plock == OS_NULL)
        os_kprintf("err!! spinunlock irqsave null\n");

    DEBUG_LOG(2, "L:%d fun:%s plock:0x%08x irqL:0x%08x\n", __LINE__, __FUNCTION__, *plock, *irqL);
    os_mutex_unlock(*plock);
#endif
}

static int _oneos_init_xqueue(_xqueue *queue, const char *name, u32 message_size, u32 number_of_messages)
{
    DEBUG_LOG(3, "L:%d fun:%s begin name:%s size:%d msgs:%d\n", __LINE__, __FUNCTION__, name, message_size, number_of_messages);

    *queue = os_mq_create(name, message_size, number_of_messages, OS_IPC_FLAG_FIFO);
    if (*queue == OS_NULL)
    {
        os_kprintf("err!! create xqueue fail\n");
        return -1;
    }
    DEBUG_LOG(3, "L:%d fun:%s end queue:0x%08x\n", __LINE__, __FUNCTION__, *queue);
    return 0;
}

static int _oneos_push_to_xqueue(_xqueue *queue, void *message, u32 timeout_ms)
{
    OS_ASSERT(*queue != OS_NULL);

    DEBUG_LOG(2, "L:%d fun:%s queue:0x%08x timeout_ms:%d\n", __LINE__, __FUNCTION__, *queue, timeout_ms);
    if (os_mq_send(*queue, message, ((os_mq_t *)queue)->msg_size, 0) != OS_EOK)
    {
        return -1;
    }
    return 0;
}

static int _oneos_pop_from_xqueue(_xqueue *queue, void *message, u32 timeout_ms)
{
    os_uint32_t tick;
    os_size_t   recv_size;

    OS_ASSERT(*queue != OS_NULL);

    DEBUG_LOG(2, "L:%d fun:%s queue:0x%08x msg:0x%08x timeout_ms:%d\n", __LINE__, __FUNCTION__, *queue, message, timeout_ms);
    if(timeout_ms >= OS_TICK_MAX / 2) 
        tick = OS_IPC_WAITING_FOREVER;
    else
        tick = rtw_ms_to_systime(timeout_ms);

    if (os_mq_recv(*queue, message, ((os_mq_t *)queue)->msg_size, tick, &recv_size) != OS_EOK)
    {
        return -1;
    }
    return 0;
}

static int _oneos_deinit_xqueue(_xqueue *queue)
{
    OS_ASSERT(*queue != OS_NULL);

    DEBUG_LOG(2, "L:%d fun:%s queue:0x%08x\n", __LINE__, __FUNCTION__, *queue);
    os_mq_destroy(*queue);
    return 0;
}

static u32 _oneos_get_current_time(void)
{
    return (u32)os_tick_get();
}

static u32 _oneos_systime_to_ms(u32 systime)
{
    return systime * 1000 / OS_TICK_PER_SECOND;
}

static u32 _oneos_systime_to_sec(u32 systime)
{
    return systime / OS_TICK_PER_SECOND;
}

static u32 _oneos_ms_to_systime(u32 ms)
{
    return os_tick_from_ms(ms);
}

static u32 _oneos_sec_to_systime(u32 sec)
{
    return sec * OS_TICK_PER_SECOND;
}

static void _oneos_msleep_os(int ms)
{
    DEBUG_LOG(2, "L:%d fun:%s ms:%d\n", __LINE__, __FUNCTION__, ms);
#if defined(CONFIG_PLATFORM_8195A)
    os_task_delay(os_tick_from_ms(ms));
#elif defined(CONFIG_PLATFORM_8711B)
    if (pmu_yield_os_check())
        os_task_delay(os_tick_from_ms(ms));
    else
        DelayMs(ms);
#endif
}

static void _oneos_usleep_os(int us)
{
#if defined(STM32F2XX) || defined(STM32F4XX) || defined(STM32F10X_XL)
    WLAN_BSP_UsLoop(us);
#elif defined(CONFIG_PLATFORM_8195A)

#elif defined(CONFIG_PLATFORM_8711B)
    DelayUs(us);
#endif
}

static void _oneos_mdelay_os(int ms)
{
    os_task_delay(os_tick_from_ms(ms));
}

static void _oneos_udelay_os(int us)
{
#if defined(STM32F2XX) || defined(STM32F4XX) || defined(STM32F10X_XL)
    WLAN_BSP_UsLoop(us);
#elif defined(CONFIG_PLATFORM_8195A)
    HalDelayUs(us);
#elif defined(CONFIG_PLATFORM_8711B)
    DelayUs(us);
#else
#error "Please implement hardware dependent micro second level sleep here"
#endif
}

static void _oneos_yield_os(void)
{
#if defined(CONFIG_PLATFORM_8195A)
    os_task_yield();
#elif defined(CONFIG_PLATFORM_8711B)
    if (pmu_yield_os_check())
        os_task_yield();
    else
        DelayMs(1);
#endif
}

static void _oneos_ATOMIC_SET(ATOMIC_T *v, int i)
{
    atomic_set(v, i);
}

static int _oneos_ATOMIC_READ(ATOMIC_T *v)
{
    return atomic_read(v);
}

static void _oneos_ATOMIC_ADD(ATOMIC_T *v, int i)
{
    save_and_cli();
    v->counter += i;
    restore_flags();
}

static void _oneos_ATOMIC_SUB(ATOMIC_T *v, int i)
{
    save_and_cli();
    v->counter -= i;
    restore_flags();
}

static void _oneos_ATOMIC_INC(ATOMIC_T *v)
{
    _oneos_ATOMIC_ADD(v, 1);
}

static void _oneos_ATOMIC_DEC(ATOMIC_T *v)
{
    _oneos_ATOMIC_SUB(v, 1);
}

static int _oneos_ATOMIC_ADD_RETURN(ATOMIC_T *v, int i)
{
    int temp;

    save_and_cli();
    temp = v->counter;
    temp += i;
    v->counter = temp;
    restore_flags();

    return temp;
}

static int _oneos_ATOMIC_SUB_RETURN(ATOMIC_T *v, int i)
{
    int temp;

    save_and_cli();
    temp = v->counter;
    temp -= i;
    v->counter = temp;
    restore_flags();

    return temp;
}

static int _oneos_ATOMIC_INC_RETURN(ATOMIC_T *v)
{
    return _oneos_ATOMIC_ADD_RETURN(v, 1);
}

static int _oneos_ATOMIC_DEC_RETURN(ATOMIC_T *v)
{
    return _oneos_ATOMIC_SUB_RETURN(v, 1);
}

static u64 _oneos_modular64(u64 n, u64 base)
{
    unsigned int __base = (base);
    unsigned int __rem;

    if (((n) >> 32) == 0)
    {
        __rem = (unsigned int)(n) % __base;
        (n)   = (unsigned int)(n) / __base;
    }
    else
        __rem = __div64_32(&(n), __base);

    return __rem;
}

static int _oneos_arc4random(void)
{
    u32                  res  = _oneos_get_current_time();
    static unsigned long seed = 0xDEADB00B;

#if CONFIG_PLATFORM_8711B
    if (random_seed)
    {
        seed        = random_seed;
        random_seed = 0;
    }
#endif

    seed = ((seed & 0x007F00FF) << 7) ^ ((seed & 0x0F80FF00) >> 8) ^ (res << 13) ^ (res >> 9);
    return (int)seed;
}

static int _oneos_get_random_bytes(void *buf, u32 len)
{
#if 1
    unsigned int  ranbuf;
    unsigned int *lp;
    int           i, count;
    count = len / sizeof(unsigned int);
    lp    = (unsigned int *)buf;

    for (i = 0; i < count; i++)
    {
        lp[i] = _oneos_arc4random();
        len -= sizeof(unsigned int);
    }

    if (len > 0)
    {
        ranbuf = _oneos_arc4random();
        _oneos_memcpy(&lp[i], &ranbuf, len);
    }
    return 0;
#else
    unsigned long ranbuf, *lp;
    lp = (unsigned long *)buf;
    while (len > 0)
    {
        ranbuf = _oneos_arc4random();
        *lp++  = ranbuf; /* this op need the pointer is 4Byte-align! */
        len -= sizeof(ranbuf);
    }
    return 0;
#endif
}

static u32 _oneos_GetFreeHeapSize(void)
{
    unsigned int total, used, maxused;
    os_memory_info(&total, &used, &maxused);
    // os_kprintf("memory stat: %d, %d, %d\r\n", total, used, maxused);
    return total - maxused;
}

#ifndef RT_USING_LWIP
#define RT_LWIP_TCPTHREAD_PRIORITY 10
#endif
static int _oneos_create_task(struct task_struct *ptask,
                              const char         *name,
                              u32                 stack_size,
                              u32                 priority,
                              thread_func_t       func,
                              void               *thctx)
{
    os_task_t *tid;

    DEBUG_LOG(3,
              "L:%d fun:%s begin name:%s stack_size:%d priority:%d func:0x%08x thctx:0x%08x\n",
              __LINE__,
              __FUNCTION__,
              name,
              stack_size,
              priority,
              func,
              thctx);

    os_memset(ptask, 0, sizeof(struct task_struct));
    ptask->task_name        = name;
    ptask->blocked          = 0;
    ptask->callback_running = 0;

    _oneos_init_sema(&ptask->wakeup_sema, 0);
    if (ptask->wakeup_sema == OS_NULL)
    {
        os_kprintf("L:%d create wakeup sem fail\n");
        goto _thread_err;
    }
    _oneos_init_sema(&ptask->terminate_sema, 0);
    if (ptask->terminate_sema == OS_NULL)
    {
        os_kprintf("L:%d terminate sem fail\n");
        goto _thread_err;
    }

    stack_size = (stack_size * 3);

    priority = RT_LWIP_TCPTHREAD_PRIORITY + priority;
    if (priority >= OS_TASK_PRIORITY_MAX)
        priority = OS_TASK_PRIORITY_MAX - 1;

    tid = os_task_create(name, func, thctx, stack_size, priority, 10);
    if (tid == OS_NULL)
    {
        os_kprintf("L:%d thread create fail\n");
        goto _thread_err;
    }

    ptask->task = tid;
    os_task_startup(tid);

    DEBUG_LOG(3,
              "L:%d fun:%s end stack_size:%d priority:%d wakeup:0x%08x terminate:0x%08x\n",
              __LINE__,
              __FUNCTION__,
              stack_size,
              priority,
              ptask->wakeup_sema,
              ptask->terminate_sema);

    return OS_TRUE;

_thread_err:
    if (ptask->wakeup_sema)
        _oneos_free_sema(&ptask->wakeup_sema);
    if (ptask->terminate_sema)
        _oneos_free_sema(&ptask->terminate_sema);
    os_memset(ptask, 0, sizeof(struct task_struct));
    return OS_FALSE;
}

static void _oneos_delete_task(struct task_struct *ptask)
{
    if (!ptask->task)
    {
        os_kprintf("_oneos_delete_task(): ptask is NULL!\n");
        return;
    }

    DEBUG_LOG(2, "L:%d fun:%s name:%s\n", __LINE__, __FUNCTION__, ptask->task_name);
    ptask->blocked = 1;

    _oneos_up_sema(&ptask->wakeup_sema);
    _oneos_down_sema(&ptask->terminate_sema, TIMER_MAX_DELAY);

    _oneos_free_sema(&ptask->wakeup_sema);
    _oneos_free_sema(&ptask->terminate_sema);

    ptask->task = 0;
}

void _oneos_wakeup_task(struct task_struct *ptask)
{
    DEBUG_LOG(2, "L:%d fun:%s name:%s\n", __LINE__, __FUNCTION__, ptask->task_name);
    _oneos_up_sema(&ptask->wakeup_sema);
}

static void _oneos_thread_enter(char *name)
{
    DEBUG_LOG(3, "L:%d fun:%s name:%s\n", __LINE__, __FUNCTION__, name);
}

static void _oneos_thread_exit(void)
{
    DEBUG_LOG(3, "L:%d fun:%s\n", __LINE__, __FUNCTION__);
}

_timerHandle _oneos_timerCreate(const signed char *pcTimerName,
                                osdepTickType      xTimerPeriodInTicks,
                                u32                uxAutoReload,
                                void              *pvTimerID,
                                TIMER_FUN          pxCallbackFunction)
{
    os_timer_t *timer;
    os_tick_t   time;
    os_uint8_t  flag;

    DEBUG_LOG(3,
              "L:%d fun:%s begin name:%s reload:%d tick:%d fun:0x%08x ID:0x%08x\n",
              __LINE__,
              __FUNCTION__,
              pcTimerName,
              uxAutoReload,
              xTimerPeriodInTicks,
              pxCallbackFunction,
              pvTimerID);

    if (uxAutoReload == 1)
        flag = (OS_TIMER_FLAG_SOFT_TIMER | OS_TIMER_FLAG_PERIODIC);
    else
        flag = (OS_TIMER_FLAG_SOFT_TIMER | OS_TIMER_FLAG_ONE_SHOT);

    if (xTimerPeriodInTicks >= (OS_TICK_MAX / 2))
        time = (OS_TICK_MAX / 2) - 1;
    else
        time = xTimerPeriodInTicks;

    timer = os_timer_create(pcTimerName, pxCallbackFunction, OS_NULL, time, flag);
    if (timer == OS_NULL)
    {
        os_kprintf("timer create fail\n");
        return OS_NULL;
    }
    timer->parameter = timer;
    DEBUG_LOG(3, "L:%d fun:%s end timer:0x%08x\n", __LINE__, __FUNCTION__, timer);

    return (_timerHandle)timer;
}

u32 _oneos_timerDelete(_timerHandle xTimer, osdepTickType xBlockTime)
{
    DEBUG_LOG(3, "L:%d fun:%s timer:0x%08x\n", __LINE__, __FUNCTION__, xTimer);

    OS_ASSERT(xTimer != OS_NULL);

    os_timer_destroy(xTimer);
    return OS_TRUE;
}

u32 _oneos_timerIsTimerActive(_timerHandle xTimer)
{
    os_timer_t *pxTimer = (os_timer_t *)xTimer;

    DEBUG_LOG(3, "L:%d fun:%s timer:0x%08x\n", __LINE__, __FUNCTION__, xTimer);
    if (!xTimer)
    {
        os_kprintf("err!! timer is active null\n");
        return 0;
    }

    return (pxTimer->parent.flag & OS_TIMER_FLAG_ACTIVATED) ? 1 : 0;
}

u32 _oneos_timerStop(_timerHandle xTimer, osdepTickType xBlockTime)
{
    OS_ASSERT(xTimer != OS_NULL);

    DEBUG_LOG(3, "L:%d fun:%s timer:0x%08x\n", __LINE__, __FUNCTION__, xTimer);
    if (os_timer_stop(xTimer) != OS_EOK)
    {
        // os_kprintf("timer stop fail\n");
        return OS_FALSE;
    }
    return OS_TRUE;
}

u32 _oneos_timerChangePeriod(_timerHandle xTimer, osdepTickType xNewPeriod, osdepTickType xBlockTime)
{
    int time;

    OS_ASSERT(xTimer != OS_NULL);

    DEBUG_LOG(3, "L:%d fun:%s timer:0x%08x new_tick:%d\n", __LINE__, __FUNCTION__, xTimer, xNewPeriod);
    if (xNewPeriod == 0)
        time = 1;
    else if (xNewPeriod >= (OS_TICK_MAX / 2))
        time = (OS_TICK_MAX / 2) - 1;
    else
        time = xNewPeriod;

    os_timer_stop(xTimer);
    os_timer_control(xTimer, OS_TIMER_CTRL_SET_TIME, (void *)&time);

    if (os_timer_start(xTimer) != OS_EOK)
    {
        os_kprintf("change time and timer start fail\n");
        return OS_FALSE;
    }

    return OS_TRUE;
}
void *_oneos_timerGetID(_timerHandle xTimer)
{
    return xTimer;
}

u32 _oneos_timerStart(_timerHandle xTimer, osdepTickType xBlockTime)
{
    OS_ASSERT(xTimer != OS_NULL);

    DEBUG_LOG(3, "L:%d fun:%s timer:0x%08x\n", __LINE__, __FUNCTION__, xTimer);
    if (os_timer_start(xTimer) != OS_EOK)
    {
        os_kprintf("change time and timer start fail\n");
        return OS_FALSE;
    }

    return OS_TRUE;
}

u32 _oneos_timerStartFromISR(_timerHandle xTimer, osdepBASE_TYPE *pxHigherPriorityTaskWoken)
{
    DEBUG_LOG(1, "L:%d fun:%s timer:0x%08x\n", __LINE__, __FUNCTION__, xTimer);

    if (xTimer == OS_NULL)
    {
        os_kprintf("timer start from isr null\n");
        return OS_FALSE;
    }

    if (os_timer_start(xTimer) != OS_EOK)
    {
        os_kprintf("timer start from isr fail\n");
        return OS_FALSE;
    }

    return OS_TRUE;
}

u32 _oneos_timerStopFromISR(_timerHandle xTimer, osdepBASE_TYPE *pxHigherPriorityTaskWoken)
{
    DEBUG_LOG(1, "L:%d fun:%s timer:0x%08x\n", __LINE__, __FUNCTION__, xTimer);

    if (xTimer == OS_NULL)
    {
        os_kprintf("timer start from isr null\n");
        return OS_FALSE;
    }

    if (os_timer_stop(xTimer) != OS_EOK)
    {
        os_kprintf("timer stop from isr fail\n");
        return OS_FALSE;
    }

    return OS_TRUE;
}

u32 _oneos_timerResetFromISR(_timerHandle xTimer, osdepBASE_TYPE *pxHigherPriorityTaskWoken)
{
    DEBUG_LOG(1, "L:%d fun:%s timer:0x%08x\n", __LINE__, __FUNCTION__, xTimer);

    if (xTimer == OS_NULL)
    {
        os_kprintf("timer Reset from isr null\n");
        return OS_FALSE;
    }

    if (os_timer_stop(xTimer) != OS_EOK)
    {
        os_kprintf("timer Reset from isr stop fail\n");
        return OS_FALSE;
    }

    if (os_timer_start(xTimer) != OS_EOK)
    {
        os_kprintf("timer Reset from isr start fail\n");
        return OS_FALSE;
    }

    return OS_TRUE;
}

u32 _oneos_timerChangePeriodFromISR(_timerHandle    xTimer,
                                    osdepTickType   xNewPeriod,
                                    osdepBASE_TYPE *pxHigherPriorityTaskWoken)
{
    int time;

    DEBUG_LOG(1, "L:%d fun:%s timer:0x%08x\n", __LINE__, __FUNCTION__, xTimer);

    if (xTimer == OS_NULL)
    {
        os_kprintf("timer Change Period from isr null\n");
        return OS_FALSE;
    }

    if (xNewPeriod == 0)
        time = 1;
    else if (xNewPeriod >= (OS_TICK_MAX / 2))
        time = (OS_TICK_MAX / 2) - 1;
    else
        time = xNewPeriod;

    os_timer_stop(xTimer);
    os_timer_control(xTimer, OS_TIMER_CTRL_SET_TIME, (void *)&time);

    if (os_timer_start(xTimer) != OS_EOK)
    {
        os_kprintf("timer Change Period from isr start fail\n");
        return OS_FALSE;
    }

    return OS_TRUE;
}

u32 _oneos_timerReset(_timerHandle xTimer, osdepTickType xBlockTime)
{
    OS_ASSERT(xTimer != OS_NULL);

    DEBUG_LOG(2, "L:%d fun:%s timer:0x%08x\n", __LINE__, __FUNCTION__, xTimer);

    os_timer_stop(xTimer);
    if (os_timer_start(xTimer) != OS_EOK)
    {
        os_kprintf("timer reset fail\n");
        return OS_FALSE;
    }

    return OS_TRUE;
}

void _oneos_acquire_wakelock(void)
{
#if defined(CONFIG_PLATFORM_8195A)

#if defined(configUSE_WAKELOCK_PMU) && (configUSE_WAKELOCK_PMU == 1)
    pmu_acquire_wakelock(PMU_WLAN_DEVICE);
#endif

#elif defined(CONFIG_PLATFORM_8711B)

#if defined(configUSE_WAKELOCK_PMU) && (configUSE_WAKELOCK_PMU == 1)
    if (pmu_yield_os_check())
        pmu_acquire_wakelock(PMU_WLAN_DEVICE);
#endif

#endif
}

void _oneos_release_wakelock(void)
{
#if defined(CONFIG_PLATFORM_8195A)

#if defined(configUSE_WAKELOCK_PMU) && (configUSE_WAKELOCK_PMU == 1)
    pmu_release_wakelock(PMU_WLAN_DEVICE);
#endif

#elif defined(CONFIG_PLATFORM_8711B)

#if defined(configUSE_WAKELOCK_PMU) && (configUSE_WAKELOCK_PMU == 1)
    if (pmu_yield_os_check())
        pmu_release_wakelock(PMU_WLAN_DEVICE);
#endif

#endif
}

void _oneos_wakelock_timeout(uint32_t timeout)
{
#if defined(CONFIG_PLATFORM_8195A)

#elif defined(CONFIG_PLATFORM_8711B)
    if (pmu_yield_os_check())
        pmu_set_sysactive_time(/*PMU_WLAN_DEVICE, */ timeout);
    else
        printf("can't aquire wake during suspend flow!!\n");
#endif
}

u8 _oneos_get_scheduler_state(void)
{
    os_task_t *thread = os_task_self();
    u8         state  = thread->stat;
    switch (state)
    {
    case OS_TASK_INIT:
        state = OS_SCHEDULER_NOT_STARTED;
        break;
    case OS_TASK_RUNNING:
        state = OS_SCHEDULER_RUNNING;
        break;
    case OS_TASK_SUSPEND:
        state = OS_SCHEDULER_SUSPENDED;
        break;
    }
    os_kprintf("[func]:%s [line]:%d state:%d\n", __FUNCTION__, __LINE__, state);
    return state;
}

const struct osdep_service_ops osdep_service = {
    _oneos_malloc,                  /* rtw_vmalloc */
    _oneos_zmalloc,                 /* rtw_zvmalloc */
    _oneos_mfree,                   /* rtw_vmfree */
    _oneos_malloc,                  /* rtw_malloc */
    _oneos_zmalloc,                 /* rtw_zmalloc */
    _oneos_mfree,                   /* rtw_mfree */
    _oneos_memcpy,                  /* rtw_memcpy */
    _oneos_memcmp,                  /* rtw_memcmp */
    _oneos_memset,                  /* rtw_memset */
    _oneos_init_sema,               /* rtw_init_sema */
    _oneos_free_sema,               /* rtw_free_sema */
    _oneos_up_sema,                 /* rtw_up_sema */
    _oneos_up_sema_from_isr,        /* rtw_up_sema_from_isr */
    _oneos_down_sema,               /* rtw_down_sema */
    _oneos_mutex_init,              /* rtw_mutex_init */
    _oneos_mutex_free,              /* rtw_mutex_free */
    _oneos_mutex_get,               /* rtw_mutex_get */
    _oneos_mutex_get_timeout,       /* rtw_mutex_get_timeout */
    _oneos_mutex_put,               /* rtw_mutex_put */
    _oneos_enter_critical,          /* rtw_enter_critical */
    _oneos_exit_critical,           /* rtw_exit_critical */
    _oneos_enter_critical_from_isr, /* rtw_enter_critical_from_isr */
    _oneos_exit_critical_from_isr,  /* rtw_exit_critical_from_isr */
    NULL,                           /* rtw_enter_critical_bh */
    NULL,                           /* rtw_exit_critical_bh */
    _oneos_enter_critical_mutex,    /* rtw_enter_critical_mutex */
    _oneos_exit_critical_mutex,     /* rtw_exit_critical_mutex */
    _oneos_spinlock_init,           /* rtw_spinlock_init */
    _oneos_spinlock_free,           /* rtw_spinlock_free */
    _oneos_spinlock,                /* rtw_spin_lock */
    _oneos_spinunlock,              /* rtw_spin_unlock */
    _oneos_spinlock_irqsave,        /* rtw_spinlock_irqsave */
    _oneos_spinunlock_irqsave,      /* rtw_spinunlock_irqsave */
    _oneos_init_xqueue,             /* rtw_init_xqueue */
    _oneos_push_to_xqueue,          /* rtw_push_to_xqueue */
    _oneos_pop_from_xqueue,         /* rtw_pop_from_xqueue */
    _oneos_deinit_xqueue,           /* rtw_deinit_xqueue */
    _oneos_get_current_time,        /* rtw_get_current_time */
    _oneos_systime_to_ms,           /* rtw_systime_to_ms */
    _oneos_systime_to_sec,          /* rtw_systime_to_sec */
    _oneos_ms_to_systime,           /* rtw_ms_to_systime */
    _oneos_sec_to_systime,          /* rtw_sec_to_systime */
    _oneos_msleep_os,               /* rtw_msleep_os */
    _oneos_usleep_os,               /* rtw_usleep_os */
    _oneos_mdelay_os,               /* rtw_mdelay_os */
    _oneos_udelay_os,               /* rtw_udelay_os */
    _oneos_yield_os,                /* rtw_yield_os */

    _oneos_ATOMIC_SET,        /* ATOMIC_SET */
    _oneos_ATOMIC_READ,       /* ATOMIC_READ */
    _oneos_ATOMIC_ADD,        /* ATOMIC_ADD */
    _oneos_ATOMIC_SUB,        /* ATOMIC_SUB */
    _oneos_ATOMIC_INC,        /* ATOMIC_INC */
    _oneos_ATOMIC_DEC,        /* ATOMIC_DEC */
    _oneos_ATOMIC_ADD_RETURN, /* ATOMIC_ADD_RETURN */
    _oneos_ATOMIC_SUB_RETURN, /* ATOMIC_SUB_RETURN */
    _oneos_ATOMIC_INC_RETURN, /* ATOMIC_INC_RETURN */
    _oneos_ATOMIC_DEC_RETURN, /* ATOMIC_DEC_RETURN */

    _oneos_modular64,        /* rtw_modular64 */
    _oneos_get_random_bytes, /* rtw_get_random_bytes */
    _oneos_GetFreeHeapSize,  /* rtw_getFreeHeapSize */

    _oneos_create_task, /* rtw_create_task */
    _oneos_delete_task, /* rtw_delete_task */
    _oneos_wakeup_task, /* rtw_wakeup_task */

    _oneos_thread_enter, /* rtw_thread_enter */
    _oneos_thread_exit,  /* rtw_thread_exit */

    _oneos_timerCreate,              /* rtw_timerCreate */
    _oneos_timerDelete,              /* rtw_timerDelete */
    _oneos_timerIsTimerActive,       /* rtw_timerIsTimerActive */
    _oneos_timerStop,                /* rtw_timerStop */
    _oneos_timerChangePeriod,        /* rtw_timerChangePeriod */
    _oneos_timerGetID,               /* rtw_timerGetID */
    _oneos_timerStart,               /* rtw_timerStart */
    _oneos_timerStartFromISR,        /* rtw_timerStartFromISR */
    _oneos_timerStopFromISR,         /* rtw_timerStopFromISR */
    _oneos_timerResetFromISR,        /* rtw_timerResetFromISR */
    _oneos_timerChangePeriodFromISR, /* rtw_timerChangePeriodFromISR */
    _oneos_timerReset,               /* rtw_timerReset */

    _oneos_acquire_wakelock,   /* rtw_acquire_wakelock */
    _oneos_release_wakelock,   /* rtw_release_wakelock */
    _oneos_wakelock_timeout,   /* rtw_wakelock_timeout */
    _oneos_get_scheduler_state /* rtw_get_scheduler_state */
};

void vTaskDelay(const os_uint32_t xTicksToDelay)
{
    os_task_delay(xTicksToDelay);
}

void *pvPortMalloc(size_t xWantedSize)
{
    return os_malloc(xWantedSize);
}

void vPortFree(void *pv)
{
    os_free(pv);
}

uint32_t xTaskGetTickCount(void)
{
    return os_tick_get();
}
