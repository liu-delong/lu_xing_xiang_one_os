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
 * @file        rtconfig.h
 *
 * @brief       RT-Thread adaper macro definition header file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __RT_CONFIG_H__
#define __RT_CONFIG_H__

#include <oneos_config.h>

#define RT_THREAD_PRIORITY_MAX          OS_TASK_PRIORITY_MAX
#define RT_TICK_PER_SECOND              OS_TICK_PER_SECOND
#define RT_ALIGN_SIZE                   OS_ALIGN_SIZE
#define RT_NAME_MAX                     OS_NAME_MAX

#if defined(OS_USING_SEMAPHORE)
#define RT_USING_SEMAPHORE
#endif

#if defined(OS_USING_MUTEX)
#define RT_USING_MUTEX
#endif 

#if defined(OS_USING_EVENT)
#define RT_USING_EVENT
#endif

#if defined(OS_USING_MAILBOX)
#define RT_USING_MAILBOX
#endif

#if defined(OS_USING_MESSAGEQUEUE)
#define RT_USING_MESSAGEQUEUE
#endif

#if defined(OS_USING_CONSOLE)
#define RT_USING_CONSOLE
#endif

#if defined(OS_CPU_CACHE_LINE_SZ)
#define RT_CPU_CACHE_LINE_SZ    OS_CPU_CACHE_LINE_SZ
#endif

#if defined(OS_USING_HEAP)
#define RT_USING_HEAP
#endif

#if defined(OS_USING_MEM_SLAB)
#define RT_USING_SLAB
#endif

#if defined(OS_USING_MEM_HEAP)
#define RT_USING_MEMHEAP
#endif

#if defined(OS_USING_MEM_POOL)
#define RT_USING_MEMPOOL
#endif

#if defined(OS_USING_LIBC)
#define RT_USING_LIBC
#endif

#if defined(OS_USING_MINILIBC)
#define RT_USING_MINILIBC
#endif


#if defined(OS_USING_MODULE)        
#define RT_USING_MODULE
#endif

#if defined(OS_USING_NEWLIB)
#define RT_USING_NEWLIB
#endif

#if defined(OS_USING_DLOG)
#define RT_USING_ULOG
#define ULOG_ASSERT_ENABLE
#endif

#if defined(OS_USING_HOOK)
#define RT_USING_HOOK
#endif

#if defined(OS_USING_IDLE_HOOK)
#define RT_USING_IDLE_HOOK
#endif

#if defined (RT_USING_HOOK)
#ifndef RT_USING_IDLE_HOOK
#define RT_USING_IDLE_HOOK
#endif
#endif

#if defined(OS_USING_LWP)
#define RT_USING_LWP
#endif

#if defined(OS_DEBUG)
#define RT_DEBUG
#endif

#define RT_DEBUG_COLOR

#endif /* __RT_CONFIG_H__ */

