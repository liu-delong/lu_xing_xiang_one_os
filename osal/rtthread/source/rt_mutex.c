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
 * @file        rt_mutex.c
 *
 * @brief       Implementation of RT-Thread adaper mutex function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <rtthread.h>
#include <string.h>

#ifdef RT_USING_MUTEX
rt_err_t rt_mutex_init(rt_mutex_t mutex, const char *name, rt_uint8_t flag)
{
    os_err_t ret;

    OS_ASSERT(mutex);

    /*Default use recursion interface*/
    ret = os_mutex_init(&mutex->oneos_mutex, name, (os_ipc_flag_t)flag, OS_TRUE);

    /* Set the flag for static creation */
    mutex->is_static = OS_TRUE;

    return (rt_err_t)ret;
}

rt_err_t rt_mutex_detach(rt_mutex_t mutex)
{
    os_err_t ret;

    OS_ASSERT(mutex);
    OS_ASSERT(OS_TRUE == mutex->is_static);

    ret = os_mutex_deinit(&mutex->oneos_mutex);

    return (rt_err_t)ret;
}

#ifdef RT_USING_HEAP
rt_mutex_t rt_mutex_create(const char *name, rt_uint8_t flag)
{
    rt_mutex_t mutex;
    os_err_t   ret;

    mutex = (rt_mutex_t)os_malloc(sizeof(struct rt_mutex));
    if (OS_NULL == mutex)
    {
        return RT_NULL;
    }
    memset(mutex, 0, sizeof(struct rt_mutex));

    ret = os_mutex_init(&mutex->oneos_mutex, name, (os_ipc_flag_t)flag, OS_TRUE);
    if (OS_EOK != ret)
    {
        os_free(mutex);
        return RT_NULL;
    }

    /* Set the flag for dynamic creation */
    mutex->is_static = OS_FALSE;

    return mutex;
}

rt_err_t rt_mutex_delete(rt_mutex_t mutex)
{
    OS_ASSERT(mutex);
    OS_ASSERT(OS_FALSE == mutex->is_static);

    (void)os_mutex_deinit(&mutex->oneos_mutex);
    os_free(mutex);
    
    return RT_EOK;
}
#endif /* RT_USING_HEAP */

rt_err_t rt_mutex_take(rt_mutex_t mutex, rt_int32_t time)
{
    os_tick_t timeout;
    os_err_t  ret;

    OS_ASSERT(mutex);

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

    ret = os_mutex_recursive_lock(&mutex->oneos_mutex, timeout);

    /* OneOS'return value is different from RT-Thread */
    if (OS_EBUSY == ret)
    {
        ret = OS_ETIMEOUT;
    }
    
    return (rt_err_t)ret;
}

rt_err_t rt_mutex_release(rt_mutex_t mutex)
{
    os_err_t ret;

    OS_ASSERT(mutex);
    
    ret = os_mutex_recursive_unlock(&mutex->oneos_mutex);

    return (rt_err_t)ret;
}

rt_err_t rt_mutex_control(rt_mutex_t mutex, int cmd, void *arg)
{
    os_err_t ret;

    OS_ASSERT(mutex);

    ret = os_mutex_control(&(mutex->oneos_mutex), (os_ipc_cmd_t)cmd, arg);

    return (rt_err_t)ret;
}
#endif /* RT_USING_MUTEX */

