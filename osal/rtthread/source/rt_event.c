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
 * @file        rt_event.c
 *
 * @brief       Implementation of RT-Thread adaper hardware API.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <rtthread.h>
#include <string.h>

#ifdef RT_USING_EVENT
rt_err_t rt_event_init(rt_event_t event, const char *name, rt_uint8_t flag)
{
    os_err_t ret;

    OS_ASSERT(event);

    ret = os_event_init(&event->oneos_event, name, (os_ipc_flag_t)flag);

    /* Set the flag for static creation */
    event->is_static = OS_TRUE;

    return (rt_err_t)ret;
}

rt_err_t rt_event_detach(rt_event_t event)
{
    os_err_t ret;

    OS_ASSERT(event);
    OS_ASSERT(OS_TRUE == event->is_static);

    ret = os_event_deinit(&event->oneos_event);

    return (rt_err_t)ret;
}

#ifdef RT_USING_HEAP
rt_event_t rt_event_create(const char *name, rt_uint8_t flag)
{
    rt_event_t event;
    os_err_t   ret;

    event = (rt_event_t)os_malloc(sizeof(struct rt_event));
    if (OS_NULL == event)
    {
        return RT_NULL;
    }
    memset(event, 0, sizeof(struct rt_event));

    ret = os_event_init(&event->oneos_event, name, (os_ipc_flag_t)flag);
    if (OS_EOK != ret)
    {
        os_free(event);
        return RT_NULL;
    }

    /* Set the flag for dynamic creation */
    event->is_static = OS_FALSE;

    return event;
}

rt_err_t rt_event_delete(rt_event_t event)
{
    OS_ASSERT(event);
    OS_ASSERT(OS_FALSE == event->is_static);

    (void)os_event_deinit(&event->oneos_event);
    os_free(event);

    return RT_EOK;
}
#endif  /* RT_USING_HEAP */

rt_err_t rt_event_send(rt_event_t event, rt_uint32_t set)
{
    os_err_t ret;
    
    ret = os_event_send(&(event->oneos_event), set);

    return (rt_err_t)ret;
}

rt_err_t rt_event_recv(rt_event_t       event,
                           rt_uint32_t  set,
                           rt_uint8_t   opt,
                           rt_int32_t   timeout,
                           rt_uint32_t *recved)
{
    os_err_t  ret;
    os_tick_t timeout_tmp;

    OS_ASSERT(event);

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
 
    ret = os_event_recv(&event->oneos_event,
                        (os_uint32_t)set,
                        (os_uint8_t)opt,
                        timeout_tmp,
                        (os_uint32_t *)recved);

    return (rt_err_t)ret;
}

rt_err_t rt_event_control(rt_event_t event, int cmd, void *arg)
{
    os_err_t ret;
    
    ret = os_event_control(&event->oneos_event, (os_ipc_cmd_t)cmd, arg);

    return (rt_err_t)ret;
}
#endif /* RT_USING_EVENT */

