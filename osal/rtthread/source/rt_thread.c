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
 * @file        rt_thread.c
 *
 * @brief       Implementation of RT-Thread adaper thread function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <rtthread.h>
#include <os_idle.h>
#include <string.h>

extern os_err_t   os_task_suspend(os_task_t *task);
extern os_err_t   os_task_resume(os_task_t *task);
extern void       os_schedule(void);

static void rt_thread_cleanup(os_task_t*thread)
{
    rt_thread_t rt_thread;

    rt_thread = (rt_thread_t)(thread->user_data);
    
    thread->cleanup = OS_NULL;
    if(rt_thread->is_static == OS_TRUE)
    {
        return ;
    }
    
    os_free(rt_thread->oneos_task.stack_addr);
    rt_thread->oneos_task.stack_addr = OS_NULL;
    
    os_free(rt_thread);   
    rt_thread = OS_NULL;
}

rt_err_t rt_thread_init(struct rt_thread *thread,
                        const char       *name,
                        void            (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        rt_uint32_t       stack_size,
                        rt_uint8_t        priority,
                        rt_uint32_t       tick)
{
    os_err_t ret;

    ret = os_task_init(&(thread->oneos_task), 
                       name, 
                       entry, 
                       parameter,
                       stack_start, 
                       stack_size, 
                       priority, 
                       tick);

    /* Set the flag for static creation */
    thread->is_static = OS_TRUE;

    return (rt_err_t)ret;
}

rt_err_t rt_thread_detach(rt_thread_t thread)
{
    os_err_t ret;

    OS_ASSERT(thread);
    OS_ASSERT(OS_TRUE == thread->is_static);

    ret = os_task_deinit(&(thread->oneos_task));

    return (rt_err_t)ret;
}

#ifdef RT_USING_HEAP
rt_thread_t rt_thread_create(const char *name,
                             void      (*entry)(void *parameter),
                             void       *parameter,
                             rt_uint32_t stack_size,
                             rt_uint8_t  priority,
                             rt_uint32_t tick)
{
    void       *stack_start;
    os_err_t    ret;
    rt_thread_t thread;

    thread = (rt_thread_t)os_malloc(sizeof(struct rt_thread));
    if (OS_NULL == thread)
    {
        return RT_NULL;
    }
    memset(thread, 0, sizeof(struct rt_thread));

    stack_size  = OS_ALIGN_UP(stack_size, OS_ALIGN_SIZE);
    stack_start = os_malloc(stack_size);
    if (OS_NULL == stack_start)
    {
        os_free(thread);
        return RT_NULL;
    }
    memset(stack_start, 0, stack_size);
    
    ret = os_task_init(&thread->oneos_task, name, entry, parameter, stack_start, stack_size, priority, tick);
    if (OS_EOK != ret)
    {
        os_free(stack_start);
        os_free(thread);
        
        return RT_NULL;
    }

    /* To achieve dynamic memory release */
    thread->oneos_task.cleanup   = rt_thread_cleanup;
    thread->oneos_task.user_data = (os_uint32_t)thread;

    /* Set the flag for dynamic creation */
    thread->is_static = OS_FALSE;
    
    return thread;
}

rt_err_t rt_thread_delete(rt_thread_t thread)
{
    OS_ASSERT(thread);
    OS_ASSERT(OS_FALSE == thread->is_static);

    (void)os_task_deinit(&thread->oneos_task);

    return RT_EOK;
  
}
#endif  /* RT_USING_HEAP */

rt_thread_t rt_thread_self(void)
{
    rt_thread_t  thread;
    os_task_t   *task_ptr;

    task_ptr = os_task_self();
    thread   = os_container_of(task_ptr, struct rt_thread, oneos_task);

    return thread;
}

rt_thread_t rt_thread_find(char *name)
{
    rt_thread_t  thread;
    os_task_t   *task_ptr;

    OS_ASSERT(name);

    task_ptr = os_task_find(name);
    thread   = os_container_of(task_ptr, struct rt_thread, oneos_task);

    return thread;
}

rt_err_t rt_thread_startup(rt_thread_t thread)
{
    os_err_t ret;

    ret = os_task_startup(&thread->oneos_task);

    return (rt_err_t)ret;
}

rt_err_t rt_thread_yield(void)
{
    os_err_t ret;
    
    ret = os_task_yield();

    return (rt_err_t)ret;
}

rt_err_t rt_thread_delay(rt_tick_t tick)
{
    os_err_t ret;
    
    ret = os_task_delay((os_tick_t)tick);

    return (rt_err_t)ret;
}

rt_err_t rt_thread_mdelay(rt_int32_t ms)
{
    OS_ASSERT(ms >= 0);

    return (rt_err_t)os_task_mdelay((os_uint32_t)ms);
}

rt_err_t rt_thread_control(rt_thread_t thread, int cmd, void *arg)
{
    os_err_t ret;
    
    ret = os_task_control(&thread->oneos_task, (enum os_task_ctrl_cmd)cmd, arg);

    return (rt_err_t)ret;
}

rt_err_t rt_thread_suspend(rt_thread_t thread)
{
    os_err_t ret;
    
    ret = os_task_suspend(&thread->oneos_task);

    return (rt_err_t)ret;
}

rt_err_t rt_thread_resume(rt_thread_t thread)
{
    os_err_t ret;

    ret = os_task_resume(&thread->oneos_task);

    return (rt_err_t)ret;
}

void rt_schedule(void)
{
    os_schedule();
}

void rt_enter_critical(void)
{
    os_enter_critical();
}

void rt_exit_critical(void)
{
    os_exit_critical();
}

rt_uint16_t rt_critical_level(void)
{
    return (rt_uint16_t)os_critical_level();
}

#if defined(RT_USING_HOOK) || defined(RT_USING_IDLE_HOOK)
rt_err_t rt_thread_idle_sethook(void (*hook)(void))
{
    os_err_t ret;

    OS_ASSERT(hook);
    
    ret = os_idle_task_set_hook(hook);

    return (rt_err_t)ret;
}

rt_err_t rt_thread_idle_delhook(void (*hook)(void))
{
    os_err_t ret;

    OS_ASSERT(hook);
    
    ret = os_idle_task_del_hook(hook);

    return (rt_err_t)ret;

}
#endif  /* defined(RT_USING_HOOK) || defined(RT_USING_IDLE_HOOK) */

