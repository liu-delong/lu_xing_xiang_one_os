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
 * @file        cmsis_thread.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 thread function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <os_errno.h>
#include <os_util.h>
#include <os_task.h>
#include <string.h>
#include <os_hw.h>

#include "cmsis_internal.h"  

#define MALLOC_STACK                        0x04
#define CMSIS_DEFAULT_TICK                  5
#define CMSIS_DEFAULT_STACK_SIZE            1024

#define WAITING_THREAD_FLAGS                0x08

extern void         os_task_exit(void);
extern void         os_schedule(void);
extern os_err_t     os_task_suspend(os_task_t *task);
extern os_err_t     os_task_resume(os_task_t *task);

static void thread_cleanup(os_task_t*thread)
{
    thread_cb_t *thread_cb;
    
    thread_cb = (thread_cb_t *)(thread->user_data);
    
    thread->cleanup = OS_NULL;
    if (thread_cb->flags&osThreadJoinable)
    {
        os_sem_post(thread_cb->joinable_sem);
    }
    else
    {
        if (thread_cb->flags&MALLOC_STACK)
        {
            os_free(thread_cb->task.stack_addr);
            thread_cb->task.stack_addr = OS_NULL;
        }

        if (thread_cb->flags&SYS_MALLOC_CTRL_BLK)
        {
            os_free(thread_cb);
            thread_cb = OS_NULL;
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           The function osThreadNew starts a thread function by adding it to the list of active threads and sets it to state READY. 
 *                  Arguments for the thread function are passed using the parameter pointer *argument. When the priority of the created 
 *                  thread function is higher than the current RUNNING thread, the created thread function starts instantly and becomes 
 *                  the new RUNNING thread. Thread attributes are defined with the parameter pointer attr. Attributes include settings for 
 *                  thread priority, stack size, or memory allocation.
 *
 * @attention       CMSIS-RTOS APIs define 56 priorities and the priority increases as the number increases,but OneOS does the opposite.in 
 *                  addition to this,OneOS can config the number of priority,so this function use an algorithm to match the priority with 
 *                  OneOS.
 *
 * @param[in]       func            thread function
 * @param[in]       argument        pointer that is passed to the thread function as start argument.
 * @param[in]       attr            thread attributes; NULL: default values.
 *
 * @return          thread ID for reference by other functions or NULL in case of error.
 ***********************************************************************************************************************
 */
osThreadId_t osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr)
{
    char                name[OS_NAME_MAX];
    void               *stack;
    os_uint8_t          os_task_prio;
    os_uint32_t         stack_size;
    thread_cb_t        *thread_cb;
    static os_uint16_t  thread_number = 1U;

    if (OS_NULL == func)
    {
        return (osThreadId_t)OS_NULL;
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->cb_mem))
    {
        thread_cb = os_malloc(sizeof(thread_cb_t));
        if (OS_NULL == thread_cb)
        {
            return (osThreadId_t)OS_NULL;
        }
        memset(thread_cb, 0, sizeof(thread_cb_t));
        thread_cb->flags |= SYS_MALLOC_CTRL_BLK;
    }
    else
    {
        if (attr->cb_size >= sizeof(thread_cb_t))
        {
            thread_cb = (thread_cb_t *)attr->cb_mem;
            thread_cb->flags = 0;
        }
        else
        {
            return (osThreadId_t)OS_NULL;
        }
    }

    /* OneOS object's name can't be NULL */
    if ((OS_NULL == attr) || (OS_NULL == attr->name))
    {
        os_snprintf(name, sizeof(name), "th%02d", thread_number);
        thread_number++;
    }
    else
    {
        os_snprintf(name, sizeof(name), "%s", attr->name);
    }

    if ((OS_NULL == attr) || (osPriorityNone == attr->priority))
    {
        thread_cb->prio = osPriorityNormal;
    }
    else
    {
        if ((attr->priority < osPriorityIdle) || (attr->priority > osPriorityISR))
        {
            return (osThreadId_t)OS_NULL;
        }

        thread_cb->prio = attr->priority;
    }
    if ((OS_NULL == attr) || (0U == attr->stack_size))
    {
        stack_size = CMSIS_DEFAULT_STACK_SIZE;
    }
    else
    {
        stack_size = attr->stack_size;
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->stack_mem))
    {
        stack = os_malloc(stack_size);
        if (OS_NULL == stack)
        {
            if (thread_cb->flags & SYS_MALLOC_CTRL_BLK)
            {
                os_free(thread_cb);
            }
            return (osThreadId_t)OS_NULL;
        }
        thread_cb->flags |= MALLOC_STACK;
    }
    else
    {
        stack = (void *)(attr->stack_mem);
    }

    if ((OS_NULL != attr)&&(0 != attr->attr_bits))
    {
        thread_cb->flags |= attr->attr_bits;
    }

    /* Algorithm match the priorities CMSIS RTOS defined with OneOS */
    os_task_prio = (osPriorityISR - thread_cb->prio) * OS_TASK_PRIORITY_MAX / osPriorityISR;

    os_task_init(&thread_cb->task, 
                  name, 
                  func, 
                  argument, 
                  stack, 
                  stack_size, 
                  os_task_prio, 
                  CMSIS_DEFAULT_TICK);

    if (thread_cb->flags&osThreadJoinable)
    {
        thread_cb->joinable_sem = os_sem_create(name, 0, OS_IPC_FLAG_FIFO);
        if (OS_NULL == thread_cb->joinable_sem)
        {
            if (thread_cb->flags & SYS_MALLOC_CTRL_BLK)
            {
                os_free(thread_cb);
            }            
            if (thread_cb->flags & MALLOC_STACK)
            {
                os_free(stack);
            }
            
            /* After os_task_init,task has been inserted into task queue */
            os_task_deinit(&thread_cb->task);
            return (osThreadId_t)OS_NULL;
        }
    }
    else
    {
        thread_cb->joinable_sem = OS_NULL;
    }

    thread_cb->task.cleanup   = thread_cleanup;
    thread_cb->task.user_data = (os_uint32_t)thread_cb;

    os_task_startup(&thread_cb->task);

    return (osThreadId_t)thread_cb;
}

const char *osThreadGetName(osThreadId_t thread_id)
{
    thread_cb_t *thread_cb;

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return OS_NULL;
    }
    
    return thread_cb->task.parent.name;
}

osThreadId_t osThreadGetId(void)
{
    os_task_t *thread;

    thread = os_task_self();

    return (osThreadId_t)(thread->user_data);
}

osThreadState_t osThreadGetState(osThreadId_t thread_id)
{
    thread_cb_t    *thread_cb;
    osThreadState_t state;

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return (osThreadState_t)osThreadError;
    }
    
    switch (thread_cb->task.stat)
    {
    case OS_TASK_INIT:
        state = osThreadInactive;
        break;
    case OS_TASK_READY:
        state = osThreadReady;
        break;
    case OS_TASK_SUSPEND:
        state = osThreadBlocked;
        break;    
    case OS_TASK_CLOSE:
        state = osThreadTerminated;
        break;
    default:
        state = osThreadError;
        break;
    }

    /* State of OneOS' task has no difference between ready and running,by comparing
    the current task ID with task identified by parameter thread_id judge the state of 
    task running or not*/
    if(&thread_cb->task == os_task_self())
    {
        state =  osThreadRunning;
    }

    return state;
}

uint32_t osThreadGetStackSize(osThreadId_t thread_id)
{
    thread_cb_t *thread_cb;

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return (uint32_t)OS_NULL ;
    }
   
    return ((uint32_t)thread_cb->task.stack_size);
}

uint32_t osThreadGetStackSpace(osThreadId_t thread_id)
{
    uint32_t     stack_space;
    os_uint8_t  *ptr;
    thread_cb_t *thread_cb;

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return (uint32_t)OS_NULL;
    }

#ifdef ARCH_CPU_STACK_GROWS_UPWARD
    ptr = (os_uint8_t *)(thread_cb->task.stack_addr + thread_cb->task.stack_size);
    while (*ptr == '#')
    {
        ptr --;
    }   
    stack_space = (uint32_t)(ptr - (thread_cb->task.stack_addr + thread_cb->task.stack_size));
#else   
    ptr = (os_uint8_t *)thread_cb->task.stack_addr;
    while (*ptr == '#')
    {
        ptr ++;
    }

    stack_space = (uint32_t)(ptr - (os_uint8_t *)thread_cb->task.stack_addr);
#endif
    return stack_space;
}

osStatus_t osThreadSetPriority(osThreadId_t thread_id, osPriority_t priority)
{
    os_uint8_t   oneos_priority;
    thread_cb_t *thread_cb;

    thread_cb = (thread_cb_t *)thread_id;

    if((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return osErrorParameter;
    }
  
    if((priority < osPriorityNone) || (priority > osPriorityISR))
    {
        return osErrorParameter;
    }
    
    thread_cb->prio = priority;
    oneos_priority = (osPriorityISR - thread_cb->prio) * OS_TASK_PRIORITY_MAX / osPriorityISR;

    os_task_control(&(thread_cb->task), OS_TASK_CTRL_CHANGE_PRIORITY, &oneos_priority);

    return osOK;
}

osPriority_t osThreadGetPriority(osThreadId_t thread_id)
{
    thread_cb_t *thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return osPriorityError;
    }

    return (osPriority_t)thread_cb->prio;
}

osStatus_t osThreadYield(void)
{
    os_task_yield();

    return osOK;
}

osStatus_t osThreadSuspend(osThreadId_t thread_id)
{
    os_err_t     result;
    thread_cb_t *thread_cb;

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return osErrorParameter;
    }

    result = os_task_suspend(&thread_cb->task);

    if(&thread_cb->task == os_task_self())
    {
        os_schedule();
    }

    if (OS_EOK == result)
    {
        return osOK;
    }
    else
    {
        return osErrorResource;
    }
}

osStatus_t osThreadResume(osThreadId_t thread_id)
{
    os_err_t     result;
    thread_cb_t *thread_cb;

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return osErrorParameter;
    }

    result = os_task_resume(&thread_cb->task);

    os_schedule();

    if (OS_EOK == result)
    {
        return osOK;
    }
    else
    {
        return osErrorResource;
    }
}

osStatus_t osThreadDetach(osThreadId_t thread_id)
{
    thread_cb_t       *thread_cb;
    register os_base_t temp;

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return osErrorParameter;
    }

    if ((thread_cb->flags&osThreadJoinable) == osThreadJoinable)
    {
        return osErrorResource;
    }

    if ((thread_cb->task.stat&OS_TASK_STAT_MASK) == OS_TASK_CLOSE)
    {
        temp = os_hw_interrupt_disable();

        /* delete to defunct task list */
        os_list_del(&thread_cb->task.task_list);

        /* detach the object from system object container */
        os_object_deinit(&thread_cb->task.parent);

        os_hw_interrupt_enable(temp);

        if (thread_cb->flags&MALLOC_STACK)
        {
            os_free(thread_cb->task.stack_addr);
        }
        
        if (thread_cb->flags&SYS_MALLOC_CTRL_BLK)
        {
            os_free(thread_cb);
        }
    }
    else
    {
        os_enter_critical();

        /* Set task attribute to osThreadDetached */
        thread_cb->flags &= ~osThreadJoinable;
        thread_cb->joinable_sem = OS_NULL;
        
        os_exit_critical();
    }

    return osOK;
}

osStatus_t osThreadJoin(osThreadId_t thread_id)
{
    os_err_t     result;
    thread_cb_t *thread_cb;

    thread_cb = (thread_cb_t *)thread_id;

    if (OS_NULL == thread_cb)
    {
        return osErrorParameter;
    }
    if (((&thread_cb->task) == os_task_self()) ||       
         (0 == (thread_cb->flags & osThreadJoinable)))
    {
        /* Join self or join a detached thread*/
        return osErrorResource;
    }

    result = os_sem_wait(thread_cb->joinable_sem, OS_IPC_WAITING_FOREVER);
    if (OS_EOK == result)
    {
        /* Release system resource */
        if (thread_cb->flags&osThreadJoinable)
        {
            os_sem_destroy(thread_cb->joinable_sem);
        }

        if (thread_cb->flags&MALLOC_STACK)
        {
            os_free(thread_cb->task.stack_addr);
        }
        
        if (thread_cb->flags&SYS_MALLOC_CTRL_BLK)
        {
            os_free(thread_cb);
        }
    }
    else
    {
        return osError;
    }

    return osOK;
}

__NO_RETURN void osThreadExit(void)
{
    os_task_exit();

    /* Monitor whether the mission is successfully closed or not,if not OS_ASSERT() will detect it*/
    OS_ASSERT(0);

    /*Because OS_ASSERT() can be turned off by configrable options ,use while(1) to monitor*/
    while(1);
}

osStatus_t osThreadTerminate(osThreadId_t thread_id)
{
    thread_cb_t *thread_cb ; 

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return osErrorParameter;
    }

    os_task_deinit(&thread_cb->task);
    if(&thread_cb->task == os_task_self())
    {
        os_schedule();
    }
    
    return osOK;
    
}

/**
 ***********************************************************************************************************************
 * @brief           Number of active threads.
 *
 * @attention       This function only return task osThreadNew create number
 *
 * @param[in]       None
 *
 * @return          The number of active threads or 0 in case of an error.
 ***********************************************************************************************************************
 */
uint32_t osThreadGetCount(void)
{
    os_task_t             *thread;
    os_uint32_t            thread_count = 0U;
    struct os_list_node   *node;
    struct os_object_info *info;

    info =os_object_get_info(OS_OBJECT_TASK);

    os_enter_critical();
    for (node = info->object_list.next; node != &(info->object_list); node = node->next)
    {
        thread = os_list_entry(node, struct os_task, parent.list);

        /* Judging CMSIS API or OneOS API create the task*/
        if(OS_NULL != (osThreadId_t)thread->user_data)
        {
            thread_count++;
        }
    }
    os_exit_critical();

    return thread_count;
}

/**
 ***********************************************************************************************************************
 * @brief           Enumerate active threads.
 *
 * @attention       This function only return task ID osThreadNew create 
 *
 * @param[out]      thread_array       Pointer to array for retrieving thread IDs
 * @param[in]       array_items        Maximum number of items in array for retrieving thread IDs. 
 *
 * @return          Number of enumerated threads.
 ***********************************************************************************************************************
 */
uint32_t osThreadEnumerate(osThreadId_t *thread_array, uint32_t array_items)
{
    os_task_t             *thread;
    os_uint32_t            thread_count = 0U;
    struct os_list_node   *node;
    struct os_object_info *info;

    if ((OS_NULL == thread_array) || (0U == array_items))
    {
        return 0U;
    }

    info = os_object_get_info(OS_OBJECT_TASK);

    os_enter_critical();
    for (node = info->object_list.next; node != &(info->object_list); node = node->next)
    {
        thread = os_list_entry(node, struct os_task, parent.list);

        /* Judging CMSIS API or OneOS API create the task*/
        if(OS_NULL != (osThreadId_t)thread->user_data)
        { 
            thread_array[thread_count] = (osThreadId_t)thread->user_data;
            thread_count++;
        }

        if (thread_count >= array_items)
        {
            break;
        }
    }
    os_exit_critical();

    return thread_count;
}

uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{

    os_bool_t           need_schedule = OS_FALSE;
    thread_cb_t        *thread_cb;
    uint32_t            return_value;
    register os_base_t  status;
    register os_ubase_t level;

    thread_cb = (thread_cb_t *)(thread_id);

    if ((OS_NULL == thread_cb) || (os_object_get_type(&(thread_cb->task.parent)) != OS_OBJECT_TASK))
    {
        return osFlagsErrorParameter;
    }

    /* Check flag value to avoid highest bits set */
    if((flags&(~(OS_UINT32_MAX >> 1U))) != 0)
    {
        return osFlagsErrorParameter;
    }

    level = os_hw_interrupt_disable();

    thread_cb->flag_set |= flags;

    /* Check if Thread is waiting for Thread Flags */
    if (thread_cb->task.event_info&WAITING_THREAD_FLAGS)
    {
        status = OS_ERROR;
        if (thread_cb->task.event_info & osFlagsWaitAll)
        {
            if ((thread_cb->task.event_set&thread_cb->flag_set) == thread_cb->task.event_set)
            {
                status = OS_EOK;
            }
        }
        else
        {
            if (thread_cb->task.event_set&thread_cb->flag_set)
            {
                thread_cb->task.event_set &= thread_cb->flag_set;
                status = OS_EOK;
            }
        }

        /* Condition is satisfied, resume thread */
        if (OS_EOK == status)
        {
            thread_cb->task.event_info &= ~WAITING_THREAD_FLAGS;
            
            if (!(thread_cb->task.event_info&osFlagsNoClear))
            {
                thread_cb->flag_set &= ~thread_cb->task.event_set;
            }

            os_task_resume(&thread_cb->task);
            need_schedule = OS_TRUE;
        }
    }

    return_value = thread_cb->flag_set;

    os_hw_interrupt_enable(level);

    if (need_schedule == OS_TRUE)
    {
        os_schedule();
    }

    return return_value;
}

uint32_t osThreadFlagsClear(uint32_t flags)
{
    os_task_t   *thread;
    thread_cb_t *thread_cb;
    os_uint32_t  flag;

    thread = os_task_self();

    if (OS_NULL == thread)
    {
        return osFlagsErrorParameter;
    }

    /* Check flag value to avoid highest bits set */
    if((flags&(~(OS_UINT32_MAX >> 1U))) != 0)
    {
        return osFlagsErrorParameter;
    }

    thread_cb = (thread_cb_t *)(thread->user_data);

    os_enter_critical();

    flag = thread_cb->flag_set;
    thread_cb->flag_set &= ~flags;

    os_exit_critical();

    return flag;
}

uint32_t osThreadFlagsGet(void)
{
    os_task_t   *thread;
    thread_cb_t *thread_cb;

    thread = os_task_self();

    if (OS_NULL == thread)
    {
        return osFlagsErrorParameter;
    }

    thread_cb = (thread_cb_t *)(thread->user_data);

    return thread_cb->flag_set;
}

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{
    os_task_t          *thread;
    os_uint32_t         return_value;
    thread_cb_t        *thread_cb;
    register os_ubase_t level;
    register os_base_t  status = OS_ERROR;

    thread = os_task_self();
    
    thread->error = OS_EOK;
    thread_cb = (thread_cb_t *)(thread->user_data);

    level = os_hw_interrupt_disable();

    if (options&osFlagsWaitAll)
    {
        if ((thread_cb->flag_set & flags) == flags)
        {
            status = OS_EOK;
        }
    }
    else
    {
        if (thread_cb->flag_set & flags)
        {
            status = OS_EOK;
        }
    }

    if (OS_EOK == status)
    {
        return_value = thread_cb->flag_set&flags;
        if (!(options & osFlagsNoClear))
        {
            thread_cb->flag_set &= ~flags;
        }
    }
    else if (0U == timeout)
    {
        os_hw_interrupt_enable(level);
        return osFlagsErrorResource;
    }
    else
    {
        thread->event_set  = flags;
        thread->event_info = options | WAITING_THREAD_FLAGS;

        os_task_suspend(thread);

        if ((timeout > 0U) && (timeout != osWaitForever))
        {
            os_timer_control(&thread->task_timer,
                             OS_TIMER_CTRL_SET_TIME,
                             &timeout);
                             
            os_timer_start(&thread->task_timer);
        }

        os_hw_interrupt_enable(level);
        os_schedule();

        if (thread->error != OS_EOK)
        {
            return thread->error;
        }

        level = os_hw_interrupt_disable();
        return_value = thread->event_set;
    }

    os_hw_interrupt_enable(level);

    return return_value;
}


