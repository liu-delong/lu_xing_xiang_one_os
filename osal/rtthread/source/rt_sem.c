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
 * @file        rt_sem.c
 *
 * @brief       Implementation of RT-Thread adaper semaphore function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <rtthread.h>
#include <string.h>

#ifdef RT_USING_SEMAPHORE
rt_err_t rt_sem_init(rt_sem_t    sem,
                     const char *name,
                     rt_uint32_t value,
                     rt_uint8_t  flag)
{
    os_err_t ret;

    OS_ASSERT(sem);
    OS_ASSERT(value < OS_SEM_MAX_COUNT);

    ret = os_sem_init(&sem->oneos_sem, name, (os_uint16_t)value, (os_ipc_flag_t)flag);

    /* Set the flag for static creation */
    sem->is_static = OS_TRUE;

    return (rt_err_t)ret;
}

rt_err_t rt_sem_detach(rt_sem_t sem)
{
    os_err_t ret;

    OS_ASSERT(sem);
    OS_ASSERT(OS_TRUE == sem->is_static);

    ret = os_sem_deinit(&sem->oneos_sem);
    
    return (rt_err_t)ret;
}

#ifdef RT_USING_HEAP
rt_sem_t rt_sem_create(const char *name, rt_uint32_t value, rt_uint8_t flag)
{
    rt_sem_t sem;
    os_err_t ret;

    OS_ASSERT(value < OS_SEM_MAX_COUNT);

    sem = (rt_sem_t)os_malloc(sizeof(struct rt_semaphore));
    if(OS_NULL == sem)
    {
       return RT_NULL; 
    }
    memset(sem, 0, sizeof(struct rt_semaphore));

    ret = os_sem_init(&sem->oneos_sem, name, (os_uint16_t)value, (os_ipc_flag_t)flag);
    if (OS_EOK != ret)
    {
        os_free(sem);
        return RT_NULL;
    }

    /* Set the flag for dynamic creation */
    sem->is_static = OS_FALSE;

    return sem;
}

rt_err_t rt_sem_delete(rt_sem_t sem)
{
    OS_ASSERT(sem);
    OS_ASSERT(OS_FALSE == sem->is_static);

    (void)os_sem_deinit(&sem->oneos_sem);
    os_free(sem);
    
    return RT_EOK;
}
#endif  /* RT_USING_HEAP */

rt_err_t rt_sem_take(rt_sem_t sem, rt_int32_t time)
{
    os_tick_t timeout;
    os_err_t  ret;

    OS_ASSERT(sem);

    /*For OneOS,only support -1 for timeout,so set timeout is -1 when timeout is less than zero*/
    if (time < 0)
    {
        timeout = OS_IPC_WAITING_FOREVER;
    }
    else if (0 == time)
    {
        timeout = OS_IPC_WAITING_NO;
    }
    else
    {
        timeout = (os_tick_t)time;
    }

    ret = os_sem_wait(&sem->oneos_sem, timeout);
    if (OS_EBUSY == ret)
    {
        ret = OS_ETIMEOUT;
    }
    
    return (rt_err_t)ret;
}

rt_err_t rt_sem_trytake(rt_sem_t sem)
{
    os_err_t ret;

    OS_ASSERT(sem);

    ret = os_sem_wait(&sem->oneos_sem, OS_IPC_WAITING_NO);

    /* OneOS'return value is different from RT-Thread */
    if (OS_EBUSY == ret)
    {
        ret = OS_ETIMEOUT;
    }
    
    return (rt_err_t)ret;
}

rt_err_t rt_sem_release(rt_sem_t sem)
{
    os_err_t ret;

    OS_ASSERT(sem);
    
    ret = os_sem_post(&sem->oneos_sem);

    return (rt_err_t)ret;
}

rt_err_t rt_sem_control(rt_sem_t sem, int cmd, void *arg)
{
    os_uint16_t reset_count;
    os_err_t    ret;

    OS_ASSERT(sem);

    if (RT_IPC_CMD_RESET == cmd)
    {
        /* OneOS usage parameter arg is different from RT-Thread */
        reset_count = (os_uint16_t)(rt_uint32_t)arg;
        ret = os_sem_control(&sem->oneos_sem, (os_ipc_cmd_t)cmd, &reset_count);   
    }
    else
    {
        ret = os_sem_control(&sem->oneos_sem, (os_ipc_cmd_t)cmd, arg);
    }
    
    return (rt_err_t)ret;
}
#endif /* RT_USING_SEMAPHORE */

