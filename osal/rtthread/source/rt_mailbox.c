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
 * @file        rt_mailbox.c
 *
 * @brief       Implementation of RT-Thread adaper mailbox function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <rtthread.h>
#include <string.h>

#ifdef RT_USING_MAILBOX
rt_err_t rt_mb_init(rt_mailbox_t mb,
                       const char  *name,
                       void        *msgpool,
                       rt_size_t    size,
                       rt_uint8_t   flag)
{
    os_err_t ret;

    OS_ASSERT(mb);
    OS_ASSERT(msgpool);
    OS_ASSERT(size >= sizeof(os_uint32_t));

    ret = os_mb_init(&mb->oneos_mailbox, name, msgpool, size, (os_ipc_flag_t)flag);

    /* Set the flag for static creation */
    mb->is_static  = OS_TRUE;
    mb->alloc_pool = OS_NULL;

    return (rt_err_t)ret;
}

rt_err_t rt_mb_detach(rt_mailbox_t mb)
{
    OS_ASSERT(mb);
    OS_ASSERT(OS_TRUE == mb->is_static);

    return (rt_err_t)os_mb_deinit(&mb->oneos_mailbox);
}

#ifdef RT_USING_HEAP
rt_mailbox_t rt_mb_create(const char *name, rt_size_t size, rt_uint8_t flag)
{
    void         *msg_pool;
    os_err_t      ret;
    os_size_t     pool_size;
    rt_mailbox_t  mailbox;

    mailbox = (rt_mailbox_t)os_malloc(sizeof(struct rt_mailbox));
    if (OS_NULL == mailbox)
    {
        return RT_NULL;
    }
    memset(mailbox, 0, sizeof(struct rt_mailbox));

    /* Calculates the size of the requested mailbox */
    pool_size = size * sizeof(os_uint32_t);
    msg_pool = os_malloc(pool_size);
    if (OS_NULL == msg_pool)
    {
        os_free(mailbox);
        return RT_NULL;
    }
    memset(msg_pool, 0, pool_size);

    ret = os_mb_init(&mailbox->oneos_mailbox, name, msg_pool, pool_size, (os_ipc_flag_t)flag);
    if (OS_EOK != ret)
    {
        os_free(mailbox);
        os_free(msg_pool);
        
        return RT_NULL;
    }

    /* Set the flag for dynamic creation */
    mailbox->is_static  = OS_FALSE;
    mailbox->alloc_pool = msg_pool;

    return mailbox;   
}

rt_err_t rt_mb_delete(rt_mailbox_t mb)
{
    OS_ASSERT(mb);
    OS_ASSERT(OS_FALSE == mb->is_static);
    OS_ASSERT(mb->alloc_pool);

    (void)os_mb_deinit(&mb->oneos_mailbox);

    os_free(mb->alloc_pool);
    os_free(mb);
    
    return RT_EOK;
}
#endif /* RT_USING_HEAP */

rt_err_t rt_mb_send(rt_mailbox_t mb, rt_uint32_t value)
{
    OS_ASSERT(mb);

    return (rt_err_t)os_mb_send(&(mb->oneos_mailbox), value, OS_IPC_WAITING_NO);
}

rt_err_t rt_mb_send_wait(rt_mailbox_t mb,
                                rt_uint32_t  value,
                                rt_int32_t   timeout)
{
    os_err_t  ret;
    os_tick_t timeout_tmp;

    OS_ASSERT(mb);

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
    
    ret = os_mb_send(&mb->oneos_mailbox, value, timeout_tmp);

    return (rt_err_t)ret;
}

rt_err_t rt_mb_recv(rt_mailbox_t mb, rt_uint32_t *value, rt_int32_t timeout)
{
    os_err_t  ret;
    os_tick_t timeout_tmp;

    OS_ASSERT(mb);
    OS_ASSERT(value);

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

    ret = os_mb_recv(&mb->oneos_mailbox, (os_uint32_t *)value, timeout_tmp);
    if (OS_EEMPTY == ret)
    {
        ret = OS_ETIMEOUT;
    }

    return (rt_err_t)ret;
}

rt_err_t rt_mb_control(rt_mailbox_t mb, int cmd, void *arg)
{
    OS_ASSERT(mb);
    
    return (rt_err_t)os_mb_control(&mb->oneos_mailbox, (os_ipc_cmd_t)cmd, arg);
}
#endif /* RT_USING_MAILBOX */

