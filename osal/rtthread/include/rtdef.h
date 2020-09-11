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
 * @file        rtdef.h
 *
 * @brief       RT-Thread adaper API macro definition header file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __RT_DEF_H__
#define __RT_DEF_H__

#include <rtconfig.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_timer.h>
#include <os_task.h>
#include <os_ipc.h>
#include <os_sem.h>
#include <os_mutex.h>
#include <os_event.h>
#include <os_mq.h>
#include <os_mailbox.h>
#include <os_memory.h>
#include <os_libc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RT-Thread basic data type definitions */
typedef signed   char                   rt_int8_t;      /* 8bit integer type */
typedef signed   short                  rt_int16_t;     /* 16bit integer type */
typedef signed   long                   rt_int32_t;     /* 32bit integer type */
typedef signed   long long              rt_int64_t;     /* 64bit integer type */
typedef unsigned char                   rt_uint8_t;     /* 8bit unsigned integer type */
typedef unsigned short                  rt_uint16_t;    /* 16bit unsigned integer type */
typedef unsigned long                   rt_uint32_t;    /* 32bit unsigned integer type */
typedef unsigned long long              rt_uint64_t;    /* 64bit unsigned integer type */
typedef int                             rt_bool_t;      /* Boolean type */

typedef long                            rt_base_t;      /* Nbit CPU related date type */
typedef unsigned long                   rt_ubase_t;     /* Nbit unsigned CPU related data type */

typedef rt_base_t                       rt_err_t;       /* Type for error number */
typedef rt_uint32_t                     rt_time_t;      /* Type for time stamp */
typedef rt_uint32_t                     rt_tick_t;      /* Type for tick count */
typedef rt_base_t                       rt_flag_t;      /* Type for flags */
typedef rt_ubase_t                      rt_size_t;      /* Type for size number */
typedef rt_ubase_t                      rt_dev_t;       /* Type for device */
typedef rt_base_t                       rt_off_t;       /* Type for offset */

/* Boolean type definitions */
#define RT_TRUE                         1               /* Boolean true  */
#define RT_FALSE                        0               /* Boolean fails */

/* Maximum value of base type */
#define RT_UINT8_MAX                    0xff            /* Maxium number of UINT8 */
#define RT_UINT16_MAX                   0xffff          /* Maxium number of UINT16 */
#define RT_UINT32_MAX                   0xffffffff      /* Maxium number of UINT32 */
#define RT_TICK_MAX                     RT_UINT32_MAX   /* Maxium number of tick */

/* Compiler Related Definitions */
#if defined(__CC_ARM) || defined(__CLANG_ARM)           /* ARM Compiler */
    #include <stdarg.h>

    #define SECTION(x)                  __attribute__((section(x)))
    #define RT_UNUSED                   __attribute__((unused))
    #define RT_USED                     __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define RT_WEAK                     __attribute__((weak))
    #define rt_inline                   static __inline

    /* Module compiling */
    #ifdef RT_USING_MODULE
        #define RTT_API                 __declspec(dllimport)
    #else
        #define RTT_API                 __declspec(dllexport)
    #endif
#elif defined (__IAR_SYSTEMS_ICC__)                     /* for IAR Compiler */
    #include <stdarg.h>
    
    #define SECTION(x)                  @ x
    #define RT_UNUSED
    #define RT_USED                     __root
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)                    PRAGMA(data_alignment=n)
    #define RT_WEAK                     __weak
    #define rt_inline                   static inline
    #define RTT_API
#elif defined (__GNUC__)                                /* GNU GCC Compiler */
    #ifdef RT_USING_NEWLIB
        #include <stdarg.h>
    #else
        /* The version of GNU GCC must be greater than 4.x */
        typedef __builtin_va_list       __gnuc_va_list;
        typedef __gnuc_va_list          va_list;
        #define va_start(v,l)           __builtin_va_start(v,l)
        #define va_end(v)               __builtin_va_end(v)
        #define va_arg(v,l)             __builtin_va_arg(v,l)
    #endif

    #define SECTION(x)                  __attribute__((section(x)))
    #define RT_UNUSED                   __attribute__((unused))
    #define RT_USED                     __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define RT_WEAK                     __attribute__((weak))
    #define rt_inline                   static __inline
    #define RTT_API
#elif defined (__ADSPBLACKFIN__)                        /* for VisualDSP++ Compiler */
    #include <stdarg.h>
    
    #define SECTION(x)                  __attribute__((section(x)))
    #define RT_UNUSED                   __attribute__((unused))
    #define RT_USED                     __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define RT_WEAK                     __attribute__((weak))
    #define rt_inline                   static inline
    #define RTT_API
#elif defined (_MSC_VER)
    #include <stdarg.h>
    
    #define SECTION(x)
    #define RT_UNUSED
    #define RT_USED
    #define ALIGN(n)                    __declspec(align(n))
    #define RT_WEAK
    #define rt_inline                   static __inline
    #define RTT_API
#elif defined (__TI_COMPILER_VERSION__)
    #include <stdarg.h>

    /* 
     * The way that TI compiler set section is different from other(at least
     * GCC and MDK) compilers. See ARM Optimizing C/C++ Compiler 5.9.3 for more
     * details.
     */
    #define SECTION(x)
    #define RT_UNUSED
    #define RT_USED
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)
    #define RT_WEAK
    #define rt_inline                   static inline
    #define RTT_API
#else
    #error not supported tool chain
#endif


/* Initialization export */
#define INIT_BOARD_EXPORT(fn)           OS_BOARD_INIT(fn)
#define INIT_PREV_EXPORT(fn)            OS_PREV_INIT(fn)
#define INIT_DEVICE_EXPORT(fn)          OS_DEVICE_INIT(fn)
#define INIT_COMPONENT_EXPORT(fn)       OS_CMPOENT_INIT(fn)
#define INIT_ENV_EXPORT(fn)             OS_ENV_INIT(fn)
#define INIT_APP_EXPORT(fn)             OS_APP_INIT(fn)

/* RT-Thread error code definitions */
#define RT_EOK                          OS_EOK                      /* There is no error */
#define RT_ERROR                        (-OS_ERROR)                 /* A generic error happens */
#define RT_ETIMEOUT                     (-OS_ETIMEOUT)              /* Timed out */
#define RT_EFULL                        (-OS_EFULL)                 /* The resource is full */
#define RT_EEMPTY                       (-OS_EEMPTY)                /* The resource is empty */
#define RT_ENOMEM                       (-OS_ENOMEM)                /* No memory */
#define RT_ENOSYS                       (-OS_ENOSYS)                /* No system */
#define RT_EBUSY                        (-OS_EBUSY)                 /* Busy */
#define RT_EIO                          (-OS_EIO)                   /* IO error */
#define RT_EINTR                        (-OS_EINTR)                 /* Interrupted system call */
#define RT_EINVAL                       (-OS_EINVAL)                /* Invalid argument */


#define RT_ALIGN(size, align)           OS_ALIGN_UP(size, align)
#define RT_ALIGN_DOWN(size, align)      OS_ALIGN_DOWN(size, align)

#define RT_NULL                         OS_NULL

/* Double List structure */
struct rt_list_node
{
    struct rt_list_node *next;                          /* Point to next node. */
    struct rt_list_node *prev;                          /* Point to prev node. */
};
typedef struct rt_list_node rt_list_t;                  /* Type for lists. */

/* Single List structure */
struct rt_slist_node
{
    struct rt_slist_node *next;                         /* Point to next node. */
};
typedef struct rt_slist_node rt_slist_t;                /* Type for single list. */


#define RT_TIMER_FLAG_DEACTIVATED       OS_TIMER_FLAG_DEACTIVATED       /* Timer is deactive */
#define RT_TIMER_FLAG_ACTIVATED         OS_TIMER_FLAG_ACTIVATED         /* Timer is active */

#define RT_TIMER_FLAG_ONE_SHOT          OS_TIMER_FLAG_ONE_SHOT          /* One shot timer */
#define RT_TIMER_FLAG_PERIODIC          OS_TIMER_FLAG_PERIODIC          /* Periodic timer */

#define RT_TIMER_FLAG_HARD_TIMER        OS_TIMER_FLAG_HARD_TIMER        /* Hard timer,the timer's callback 
                                                                           function will be called in tick isr. */
#define RT_TIMER_FLAG_SOFT_TIMER        OS_TIMER_FLAG_SOFT_TIMER        /* Soft timer,the timer's callback function
                                                                           will be called in timer thread. */

#define RT_TIMER_CTRL_SET_TIME          OS_TIMER_CTRL_SET_TIME          /* Set timer control command */
#define RT_TIMER_CTRL_GET_TIME          OS_TIMER_CTRL_GET_TIME          /* Get timer control command */
#define RT_TIMER_CTRL_SET_ONESHOT       OS_TIMER_CTRL_SET_ONESHOT       /* Change timer to one shot */
#define RT_TIMER_CTRL_SET_PERIODIC      OS_TIMER_CTRL_SET_PERIODIC      /* Change timer to periodic */


/* Timer structure */
struct rt_timer
{
    struct os_timer     oneos_timer;                                    /* OneOS timer control block */
    os_bool_t           is_static;                                      /* flag for static or dynamic creation */
};
typedef struct rt_timer *rt_timer_t;


/* Thread state definitions */
#define RT_THREAD_INIT                  OS_TASK_INIT                    /* Initialized status */
#define RT_THREAD_READY                 OS_TASK_READY                   /* Ready status */
#define RT_THREAD_SUSPEND               OS_TASK_SUSPEND                 /* Suspend status */
#define RT_THREAD_RUNNING               OS_TASK_RUNNING                 /* Running status */
#define RT_THREAD_BLOCK                 OS_TASK_BLOCK                   /* Blocked status */
#define RT_THREAD_CLOSE                 OS_TASK_CLOSE                   /* Closed status */
#define RT_THREAD_STAT_MASK             OS_TASK_STAT_MASK

#define RT_THREAD_STAT_SIGNAL           OS_TASK_STAT_SIGNAL             /* Task hold signals */
#define RT_THREAD_STAT_SIGNAL_READY     OS_TASK_STAT_SIGNAL_READY
#define RT_THREAD_STAT_SIGNAL_WAIT      OS_TASK_STAT_SIGNAL_WAIT        /* Task is waiting for signals */
#define RT_THREAD_STAT_SIGNAL_PENDING   OS_TASK_STAT_SIGNAL_PENDING     /* Signals is held and it has not been procressed */
#define RT_THREAD_STAT_SIGNAL_MASK      OS_TASK_STAT_SIGNAL_MASK

/* Thread control command definitions */
#define RT_THREAD_CTRL_STARTUP          OS_TASK_CTRL_STARTUP            /* Startup thread. */
#define RT_THREAD_CTRL_CLOSE            OS_TASK_CTRL_CLOSE              /* Close thread. */
#define RT_THREAD_CTRL_CHANGE_PRIORITY  OS_TASK_CTRL_CHANGE_PRIORITY    /* Change thread priority. */

/* Thread structure */
struct rt_thread
{
    struct os_task          oneos_task;
    os_bool_t               is_static;
};
typedef struct rt_thread *rt_thread_t;

/* IPC flags and control command definitions */
#define RT_IPC_FLAG_FIFO                OS_IPC_FLAG_FIFO                /* FIFOed IPC. @ref IPC. */
#define RT_IPC_FLAG_PRIO                OS_IPC_FLAG_PRIO                /* PRIOed IPC. @ref IPC. */

#define RT_IPC_CMD_UNKNOWN              OS_IPC_CMD_UNKNOWN              /* Unknown IPC command */
#define RT_IPC_CMD_RESET                OS_IPC_CMD_RESET                /* Reset IPC object */

#define RT_WAITING_FOREVER              -1                              /* Block forever until get resource. */
#define RT_WAITING_NO                   0                               /* Non-block. */

#ifdef RT_USING_SEMAPHORE
/**
 ***********************************************************************************************************************
 * @struct      rt_semaphore
 *
 * @brief       Semaphore structure
 ***********************************************************************************************************************
 */
struct rt_semaphore
{
    struct os_semaphore     oneos_sem;                                  /* OneOS semaphore control block */
    os_bool_t               is_static;                                  /* flag for static or dynamic creation */
};
typedef struct rt_semaphore *rt_sem_t;
#endif /* RT_USING_SEMAPHORE */

#ifdef RT_USING_MUTEX
/**
 ***********************************************************************************************************************
 * @struct      rt_mutex
 *
 * @brief       Mutual exclusion (mutex) structure
 ***********************************************************************************************************************
 */
struct rt_mutex
{
    struct os_mutex     oneos_mutex;                                    /* OneOS mutex control block */
    os_bool_t           is_static;                                      /* flag for static or dynamic creation */
};
typedef struct rt_mutex *rt_mutex_t;
#endif /* RT_USING_MUTEX */

#ifdef RT_USING_EVENT
/* Flag defintions in event */
#define RT_EVENT_FLAG_AND               OS_EVENT_OPTION_AND             /* Logic and */
#define RT_EVENT_FLAG_OR                OS_EVENT_OPTION_OR              /* Logic or */
#define RT_EVENT_FLAG_CLEAR             OS_EVENT_OPTION_CLEAR           /* Clear flag */

/**
 ***********************************************************************************************************************
 * @struct      rt_event
 *
 * @brief       Event structure
 ***********************************************************************************************************************
 */
struct rt_event
{
    struct os_event     oneos_event;                                    /* OneOS event control block */
    os_bool_t           is_static;                                      /* flag for static or dynamic creation */
};
typedef struct rt_event *rt_event_t;
#endif /* RT_USING_EVENT */

#ifdef RT_USING_MAILBOX
/**
 ***********************************************************************************************************************
 * @struct      rt_mailbox
 *
 * @brief       Mailbox structure
 ***********************************************************************************************************************
 */
struct rt_mailbox
{
    struct os_mailbox    oneos_mailbox;                                 /* OneOS mailbox control block */
    os_bool_t            is_static;                                     /* flag for static or dynamic creation */
    void                *alloc_pool;
};
typedef struct rt_mailbox *rt_mailbox_t;
#endif /* RT_USING_MAILBOX */

#ifdef RT_USING_MESSAGEQUEUE
/**
 ***********************************************************************************************************************
 * @struct      rt_messagequeue
 *
 * @brief       Message queue structure
 ***********************************************************************************************************************
 */
struct rt_messagequeue
{
    struct os_mq        oneos_mq;                                       /* OneOS messagequeue control block */
    os_bool_t           is_static;                                      /* flag for static or dynamic creation */
    void               *alloc_pool;
};
typedef struct rt_messagequeue *rt_mq_t;
#endif /* RT_USING_MESSAGEQUEUE */

#ifdef RT_USING_MEMHEAP
/**
 ***********************************************************************************************************************
 * @struct      rt_memheap
 *
 * @brief       Base structure of memory heap object
 ***********************************************************************************************************************
 */
struct rt_memheap
{
    struct os_memheap   oneos_memheap;                                  /* OneOS memheap control block */
};
#endif /* RT_USING_MEMHEAP */


#ifdef RT_USING_MEMPOOL
/**
 ***********************************************************************************************************************
 * @struct      rt_mempool
 *
 * @brief       Base structure of Memory pool object
 ***********************************************************************************************************************
 */
struct rt_mempool
{
    struct os_mempool       oneos_mp;                                   /* OneOS mempool control block */
    os_bool_t               is_static;                                  /* flag for static or dynamic creation */

};
typedef struct rt_mempool *rt_mp_t;
#endif /* RT_USING_MEMPOOL */

#ifdef __cplusplus
}
#endif

#endif /* __RT_DEF_H__ */

