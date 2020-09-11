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
 * @file        cmsis_event.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 event function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_version.h>
#include <os_clock.h>
#include <os_tick.h>
#include <string.h>
#include <board.h>

#include "cmsis_internal.h" 
#include "../../../kernel/source/os_kernel_internal.h"

/**
 ***********************************************************************************************************************
 * @def         API_VERSION
 *
 * @brief       CMSIS API version (2.1.2)
 ***********************************************************************************************************************
 */
#define API_VERSION             20010002

/**
 ***********************************************************************************************************************
 * @def         KERNEL_Id
 *
 * @brief       OneOS Kernel identification string
 ***********************************************************************************************************************
 */
#define KERNEL_Id               "OneOS"

static osKernelState_t kernel_state = osKernelInactive;

#ifdef SysTick

static volatile uint8_t          blocked;
static uint8_t                   pendSV;

__STATIC_INLINE uint8_t GetPendSV (void) 
{
    return ((uint8_t)((SCB->ICSR & (SCB_ICSR_PENDSVSET_Msk)) >> 24));
}

__STATIC_INLINE void ClrPendSV (void) 
{
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk;
}

__STATIC_INLINE void SetPendSV (void) 
{
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

static void KernelBlock (void) 
{

    OS_Tick_Disable();

    blocked = 1U;
    __DSB();

    if (GetPendSV() != 0U) 
    {
        ClrPendSV();
        pendSV = 1U;
    }
}

static void KernelUnblock (void)
{

    blocked = 0U;
    __DSB();

    if (pendSV != 0U) 
    {
        pendSV = 0U;
        SetPendSV();
    }

    OS_Tick_Enable();
}

#endif  /*SysTick*/

osStatus_t osKernelInitialize(void)
{
    kernel_state = osKernelReady;

    return osOK;
}

static uint32_t os_version_show()
{
    os_uint32_t version_size;
    os_uint16_t os_version;
    os_uint16_t os_subversion;
    os_uint16_t os_reversion;
    os_uint32_t os_kernel_version;
    
    char version_name[20];
    
    version_size = sizeof(ONEOS_VERSION);

    strncpy(version_name, ONEOS_VERSION, version_size);

    /*ASCII exchange to dec*/
    os_version    = version_name[7]  - 48;
    os_subversion = version_name[9]  - 48;
    os_reversion  = version_name[11] - 48;

    os_kernel_version =  (((os_uint32_t)os_version      * 10000000UL)     | \
                          ((os_uint32_t)os_subversion   *    10000UL)     | \
                          ((os_uint32_t)os_reversion    *        1UL));

    return (uint32_t)os_kernel_version;

}

osStatus_t osKernelGetInfo(osVersion_t *version, char *id_buf, uint32_t id_size)
{
    if ((OS_NULL == version) || (OS_NULL == id_buf) || id_size < sizeof(KERNEL_Id))
    {
        return osErrorParameter;
    }
    
    version->api    = API_VERSION;
    version->kernel = os_version_show();

    id_size = sizeof(KERNEL_Id);
    strncpy(id_buf, KERNEL_Id, id_size);

    return osOK;
}

osKernelState_t osKernelGetState(void)
{
    return kernel_state;
}

osStatus_t osKernelStart(void)
{
    osStatus_t state;

    if (osKernelReady == kernel_state)
    {
        kernel_state = osKernelRunning;

        state = osOK;
    }
    else
    {
        state = osError;
    }

    return state;
}

int32_t osKernelLock(void)
{
    int32_t lock;

    OS_DEBUG_NOT_IN_INTERRUPT;

    switch (kernel_state)
    {
    case osKernelRunning:
        os_enter_critical();
        kernel_state = osKernelLocked;
        lock = 0;
        break;
    case osKernelLocked:
        lock = 1;
        break;
    default:
        lock = (int32_t)osError;
        break;
    }
    
    return lock;
}

int32_t osKernelUnlock(void)
{
    int32_t lock;

    OS_DEBUG_NOT_IN_INTERRUPT;

    switch (kernel_state)
    {
    case osKernelRunning:
        lock = 0;
        break;
    case osKernelLocked:
        kernel_state = osKernelRunning;
        os_exit_critical();
        lock = 1;
        break;
    default:
        lock = (int32_t)osError;
        break;
    }
    return lock;
}

int32_t osKernelRestoreLock(int32_t lock)
{
    int32_t lock_new;

    OS_DEBUG_NOT_IN_INTERRUPT;

    switch (kernel_state)
    {
    case osKernelRunning:
    case osKernelLocked:
        switch (lock)
        {
        case 0:
            kernel_state = osKernelRunning;
            
            if (0 != os_critical_level())
            {
                os_exit_critical();
            }

            if (0 != os_critical_level())
            {
                lock_new = (int32_t)osError;
            }
            
            lock_new = 0;
            
            break;
        case 1:
            if (0 == os_critical_level())
            {
                os_enter_critical();
            }

            kernel_state = osKernelLocked;
            
            lock_new = 1;
            
            break;
        default:
            lock_new = (int32_t)osError;
        break;
        }
        break;
    default:
        lock_new = (int32_t)osError;
        break;
    }
    
    return lock_new;
}

uint32_t osKernelGetTickCount(void)
{
    return (uint32_t)os_tick_get();
}

uint32_t osKernelGetTickFreq(void)
{

    return OS_TICK_PER_SECOND;
}


#ifdef SysTick

uint32_t osKernelGetSysTimerCount(void)	
{
    uint32_t  irqmask ;
    uint32_t  val;
    os_tick_t ticks;

    irqmask = os_hw_interrupt_disable();

    ticks = os_tick_get();
    val   = OS_Tick_GetCount();

    if (OS_Tick_GetOverflow() != 0U) 
    {
        val = OS_Tick_GetCount();
        ticks++;
    }
    
    val += ticks * OS_Tick_GetInterval();
    os_hw_interrupt_enable(irqmask);

    return (val);
}

#endif    /*SysTick*/


uint32_t osKernelGetSysTimerFreq(void)
{
    return SystemCoreClock;
}

#ifdef SysTick

uint32_t osKernelSuspend (void)
{

    os_tick_t              min_tick;
    os_tick_t              cur_tick;
    os_tick_t              temp_tick;
    os_task_t             *task;
    os_timer_t            *timer;
    os_uint8_t             timer_index;
    struct os_list_node   *node;
    struct os_object_info *info_task;
    struct os_object_info *info_timer;

	if (kernel_state != osKernelRunning)
	{
		return 0U;
	}
    
    info_task  = os_object_get_info(OS_OBJECT_TASK);
    info_timer = os_object_get_info(OS_OBJECT_TIMER);
    
    KernelBlock();
    
    min_tick = osWaitForever;
    
    cur_tick = os_tick_get();
    
    /* check thread delay list */
    if (info_task != NULL)
    {
        for (node = info_task->object_list.next; node != &(info_task->object_list); node = node->next)
        {
            task = os_list_entry(node, struct os_task, task_list);

            if ((task->task_timer.parent.flag&OS_TIMER_FLAG_ACTIVATED) == OS_TIMER_FLAG_ACTIVATED)
            {
                temp_tick = task->task_timer.timeout_tick - cur_tick;

                if (temp_tick < min_tick)
                {
                    min_tick = temp_tick;
                }
            }
        }
    }
    
    /* check active timer list */
    if (info_timer != NULL)
    {
        for (node = info_timer->object_list.next; node != &(info_timer->object_list); node = node->next)
        {
            timer = os_list_entry(node, struct os_timer, row[timer_index++]);
            
            if ((timer->parent.flag & OS_TIMER_FLAG_ACTIVATED) == OS_TIMER_FLAG_ACTIVATED)
            {
                temp_tick = timer->timeout_tick - cur_tick;
                
                if (temp_tick < min_tick)
                {
                    min_tick = temp_tick;
                }
            }
        }
    }

    kernel_state = osKernelSuspended;

    return (min_tick);
}
#endif /* SysTick */

#ifdef SysTick

void osKernelResume (uint32_t sleep_ticks)
{
    os_tick_t delay_tick = 0;

    if (kernel_state != osKernelSuspended)
    {
        return;
    }

    delay_tick = (os_tick_t)sleep_ticks;
    
    os_enter_critical();
    
    while(delay_tick > 0)
    {
        /*Process Thread Delay list and Process Active Timer list*/
        os_tick_increase();   
        delay_tick --;
    }
    
    os_exit_critical();
    
    kernel_state = osKernelRunning;
    
    KernelUnblock();
    
    return;
}

#endif /* SysTick */

