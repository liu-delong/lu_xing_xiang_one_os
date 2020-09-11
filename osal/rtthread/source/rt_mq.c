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
 * @file        rt_mq.c
 *
 * @brief       Implementation of RT-Thread adaper messagequeue function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <rtthread.h>
#include <string.h>
#include <os_assert.h>

#ifdef RT_USING_MESSAGEQUEUE
rt_err_t rt_mq_init(rt_mq_t     mq,
                    const char *name,
                    void       *msgpool,
                    rt_size_t   msg_size,
                    rt_size_t   pool_size,
                    rt_uint8_t  flag)
{
    os_err_t ret;

    OS_ASSERT(mq);
    OS_ASSERT(msgpool);
    OS_ASSERT(msg_size >= OS_ALIGN_SIZE);
    OS_ASSERT(pool_size >= (OS_ALIGN_SIZE + sizeof(os_mq_msg_t)));

    ret = os_mq_init(&mq->oneos_mq, name, msgpool, pool_size, msg_size, (os_ipc_flag_t)flag);

    /* Set the flag for static creation */
    mq->is_static  = OS_TRUE;
    mq->alloc_pool = OS_NULL;

    return (rt_err_t)ret;
}

rt_err_t rt_mq_detach(rt_mq_t mq)
{
    OS_ASSERT(mq);
    OS_ASSERT(OS_TRUE == mq->is_static);

    return (rt_err_t)os_mq_deinit(&mq->oneos_mq);
}

rt_mq_t rt_mq_create(const char *name,
                     rt_size_t   msg_size,
                     rt_size_t   max_msgs,
                     rt_uint8_t  flag)
{
    rt_mq_t    mq;
    os_size_t  pool_size;
    os_size_t  align_msg_size;
    os_err_t   ret;
    void      *msgpool;

    OS_ASSERT(msg_size >= OS_ALIGN_SIZE);
    OS_ASSERT(max_msgs > 0);
        
    mq = (rt_mq_t)os_malloc(sizeof(struct rt_messagequeue));
    if(OS_NULL == mq)
    {
        return RT_NULL;
    }
    memset(mq, 0, sizeof(struct rt_messagequeue));
    
    align_msg_size  = OS_ALIGN_UP(msg_size, OS_ALIGN_SIZE);
    pool_size       = max_msgs * (align_msg_size + sizeof(os_mq_msg_t));
    
    msgpool = os_malloc(pool_size);   
    if(OS_NULL == msgpool)
    {
        os_free(mq);
        return RT_NULL;
    }
    memset(msgpool, 0, pool_size);
    
    ret = os_mq_init(&mq->oneos_mq, name, msgpool, pool_size, msg_size, (os_ipc_flag_t)flag);
    if(OS_EOK != ret)
    {
        os_free(msgpool);
        os_free(mq);

        return RT_NULL;
    }

    mq->is_static  = OS_FALSE;
    mq->alloc_pool = msgpool;

    return mq;
}

rt_err_t rt_mq_delete(rt_mq_t mq)
{
    OS_ASSERT(mq);
    OS_ASSERT(OS_FALSE == mq->is_static);
    OS_ASSERT(mq->alloc_pool);

    os_mq_deinit(&mq->oneos_mq);
    
    os_free(mq->alloc_pool);
    os_free(mq);

    return RT_EOK;
}

rt_err_t rt_mq_send(rt_mq_t mq, void *buffer, rt_size_t size)
{
    OS_ASSERT(mq);
    OS_ASSERT(buffer);
    OS_ASSERT(size);
    
    return (rt_err_t)os_mq_send(&mq->oneos_mq, buffer, size, OS_IPC_WAITING_NO);
}

rt_err_t rt_mq_urgent(rt_mq_t mq, void *buffer, rt_size_t size)
{
    OS_ASSERT(mq);
    OS_ASSERT(buffer);
    OS_ASSERT(size);

    return (rt_err_t)os_mq_send_urgent(&mq->oneos_mq, buffer, size, OS_IPC_WAITING_NO);
}

rt_err_t rt_mq_recv(rt_mq_t    mq,
                    void      *buffer,
                    rt_size_t  size,
                    rt_int32_t timeout)
{
    os_size_t recv_size;
    os_tick_t timeout_tmp;
    os_err_t  ret;

    OS_ASSERT(mq);
    OS_ASSERT(buffer);
    OS_ASSERT(size);

    /*For OneOS,only support -1 for timeout,so set timeout is -1 when timeout is less than zero*/
    if (timeout < 0)
    {
        timeout_tmp = OS_IPC_WAITING_FOREVER;
    }
    else if (0 == timeout)
    {
        timeout_tmp = OS_IPC_WAITING_NO;
    }
    else
    {
        timeout_tmp = (os_tick_t)timeout;
    }

    ret = os_mq_recv(&mq->oneos_mq, buffer, size, timeout_tmp, &recv_size);
    if (OS_EEMPTY == ret)
    {
        ret = OS_ETIMEOUT;
    }

    return (rt_err_t)ret;
}

rt_err_t rt_mq_control(rt_mq_t mq, int cmd, void *arg)
{
    OS_ASSERT(mq);

    return (rt_err_t)os_mq_control(&mq->oneos_mq, (os_ipc_cmd_t)cmd, arg);  
}
#endif /* RT_USING_MESSAGEQUEUE */

