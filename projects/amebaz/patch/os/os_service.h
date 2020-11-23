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
 * @file        os_service.h
 *
 * @brief       This file provide OS interfaces for ameba SDK.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _FREERTOS_SERVICE_H_
#define _FREERTOS_SERVICE_H_

#include "dlist.h"

#if defined(CONFIG_PLATFORM_8195A)
#include "platform/platform_stdlib.h"
extern VOID RtlUdelayOS(u32 us);
#elif defined(CONFIG_PLATFORM_8711B)
#include "platform/platform_stdlib.h"
#else
#include <string.h>
#endif

#if (defined CONFIG_GSPI_HCI || defined CONFIG_SDIO_HCI) || defined(CONFIG_LX_HCI)
/* For SPI interface transfer and us delay implementation */
#if !defined(CONFIG_PLATFORM_8195A) && !defined(CONFIG_PLATFORM_8711B)
#include <rtwlan_bsp.h>
#endif
#endif

#if !defined(CONFIG_PLATFORM_8195A) && !defined(CONFIG_PLATFORM_8711B)
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;
typedef unsigned long long u64;
typedef unsigned int       uint;
typedef signed int         sint;

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

#define IN
#define OUT
#define VOID        void
#define NDIS_OID    uint
#define NDIS_STATUS uint
#ifndef PVOID
typedef void *PVOID;
#endif

typedef unsigned int     __kernel_size_t;
typedef int              __kernel_ssize_t;
typedef __kernel_size_t  SIZE_T;
typedef __kernel_ssize_t SSIZE_T;

#endif

#define FIELD_OFFSET(s, field) ((SSIZE_T) & ((s *)(0))->field)

/* os types */
typedef char          osdepCHAR;
typedef float         osdepFLOAT;
typedef double        osdepDOUBLE;
typedef long          osdepLONG;
typedef short         osdepSHORT;
typedef unsigned long osdepSTACK_TYPE;
typedef long          osdepBASE_TYPE;
typedef unsigned long osdepTickType;

typedef void *            _timerHandle;
typedef void *            _sema;
typedef void *            _mutex;
typedef void *            _lock;
typedef void *            _queueHandle;
typedef void *            _xqueue;
typedef struct timer_list _timer;

typedef struct sk_buff _pkt;
typedef unsigned char  _buffer;

#ifndef __LIST_H
#warning "DLIST_NOT_DEFINE!!!!!!"
struct list_head
{
    struct list_head *next, *prev;
};
#endif

struct __queue {
    struct list_head queue;
    _lock            lock;
};

typedef struct __queue   _queue;
typedef struct list_head _list;
typedef unsigned long    _irqL;

typedef void *_thread_hdl_;
typedef void  thread_return;
typedef void *thread_context;

#define ATOMIC_T atomic_t
#define HZ       configTICK_RATE_HZ

#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
/* emulate a modern version */
#define LINUX_VERSION_CODE KERNEL_VERSION(2, 6, 17)

static __inline _list *get_next(_list *list)
{
    return list->next;
}

static __inline _list *get_list_head(_queue *queue)
{
    return (&(queue->queue));
}

#define LIST_CONTAINOR(ptr, type, member)                                                                              \
    ((type *)((char *)(ptr) - (SIZE_T)((char *)&((type *)ptr)->member - (char *)ptr)))
//#define container_of(p,t,n) (t*)((p)-&(((t*)0)->n))
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - (SIZE_T)(&((type *)0)->member)))
#define TASK_PRORITY_LOW                1
#define TASK_PRORITY_MIDDLE             2
#define TASK_PRORITY_HIGH               3
#define TASK_PRORITY_SUPER              4

#define TIMER_MAX_DELAY 0xFFFFFFFF

void save_and_cli(void);
void restore_flags(void);
void cli(void);

#ifndef mdelay
#define mdelay(t) ((t / portTICK_RATE_MS) > 0) ? (vTaskDelay(t / portTICK_RATE_MS)) : (vTaskDelay(1))
#endif

#ifndef udelay
#define udelay(t) ((t / (portTICK_RATE_MS * 1000)) > 0) ? vTaskDelay(t / (portTICK_RATE_MS * 1000)) : (vTaskDelay(1))
#endif

#define __init
#define __exit
#define __devinit
#define __devexit

#define KERN_ERR
#define KERN_INFO
#define KERN_NOTICE

#undef GFP_KERNEL
#define GFP_KERNEL 1
#define GFP_ATOMIC 1

#define SET_MODULE_OWNER(some_struct)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#define SET_NETDEV_DEV(dev, obj)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#define register_netdev(dev) (0)
#define unregister_netdev(dev)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#define netif_queue_stopped(dev) (0)
#define netif_wake_queue(dev)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#define printk printf

#define DBG_ERR(fmt, args...) printf("\n\r[%s] " fmt, __FUNCTION__, ##args)
#if WLAN_INTF_DBG
#define DBG_TRACE(fmt, args...) printf("\n\r[%s] " fmt, __FUNCTION__, ##args)
#define DBG_INFO(fmt, args...)  printf("\n\r[%s] " fmt, __FUNCTION__, ##args)
#else
#define DBG_TRACE(fmt, args...)
#define DBG_INFO(fmt, args...)
#endif
#define HALT()                                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        cli();                                                                                                         \
        for (;;)                                                                                                       \
            ;                                                                                                          \
    } while (0)
#undef ASSERT
#define ASSERT(x)                                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((x) == 0)                                                                                                  \
            printf("\n\rAssert(" #x ") failed on line %d in file %s", __LINE__, __FILE__);                             \
        HALT();                                                                                                        \
    } while (0)

#undef DBG_ASSERT
#define DBG_ASSERT(x, msg)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((x) == 0)                                                                                                  \
            printf("\n\r%s, Assert(" #x ") failed on line %d in file %s", msg, __LINE__, __FILE__);                    \
    } while (0)

/* for 8195A, it is defined in ..system../basic_types.h */
#if !defined(CONFIG_PLATFORM_8195A) && !defined(CONFIG_PLATFORM_8711B)
typedef struct {
    volatile int counter;
} atomic_t;
#endif

#undef atomic_read
#define atomic_read(v) ((v)->counter)

#undef atomic_set
#define atomic_set(v, i) ((v)->counter = (i))

#define time_after(a, b)  ((long)(b) - (long)(a) < 0)
#define time_before(a, b) time_after(b, a)

#define time_after_eq(a, b)  ((long)(a) - (long)(b) >= 0)
#define time_before_eq(a, b) time_after_eq(b, a)

extern void rtw_init_listhead(_list *list);
extern u32  rtw_is_list_empty(_list *phead);
extern void rtw_list_insert_head(_list *plist, _list *phead);
extern void rtw_list_insert_tail(_list *plist, _list *phead);
extern void rtw_list_delete(_list *plist);

#if CONFIG_PLATFORM_8711B
extern u32 random_seed;
#endif

#endif /* _FREERTOS_SERVICE_H_ */
