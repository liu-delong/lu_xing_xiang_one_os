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
 * @file        cmsis_mq.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 messagequeue function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_memory.h>
#include <os_assert.h>
#include <os_errno.h>
#include <os_util.h>
#include <string.h>
#include <os_hw.h>

#include "cmsis_internal.h"  

#ifdef OS_USING_MESSAGEQUEUE
osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr)
{
    char                name[OS_NAME_MAX];
    void               *mq_addr;
    mq_cb_t            *mq_cb;
    os_size_t           align_msg_size;
    os_uint32_t         mq_size;
    static os_uint16_t  mq_number = 1U;

    if ((0U == msg_count) || (0U == msg_size))
    {
        return OS_NULL;
    }
    
    /* OneOS object's name can't be NULL */
    if ((OS_NULL == attr) || (OS_NULL == attr->name))
    {
        os_snprintf(name, sizeof(name), "mq%02d", mq_number++);
    }
    else
    {
        os_snprintf(name, sizeof(name), "%s", attr->name);
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->cb_mem))
    {
        mq_cb = os_malloc(sizeof(mq_cb_t));
        if (OS_NULL == mq_cb)
        {
            return OS_NULL;
        }
        memset(mq_cb, 0, sizeof(mq_cb_t));
        mq_cb->flags |= SYS_MALLOC_CTRL_BLK;
    }
    else
    {
        if (attr->cb_size >= sizeof(mq_cb_t))
        {
            mq_cb = attr->cb_mem;
            mq_cb->flags = 0;
        }
        else
            return OS_NULL;
    }
   
    align_msg_size       = OS_ALIGN_UP(msg_size, OS_ALIGN_SIZE); 
    mq_cb->init_msg_size = align_msg_size;

    if ((OS_NULL == attr) || (OS_NULL == attr->mq_mem))
    {
        mq_size = (align_msg_size + sizeof(struct os_mq_msg)) * msg_count;
        mq_addr = os_malloc(mq_size);

        if (OS_NULL == mq_addr)
        {
            if (mq_cb->flags & SYS_MALLOC_CTRL_BLK)
            {
                os_free(mq_cb);
            }

            return OS_NULL;
        }
        memset(mq_addr, 0, sizeof(mq_size));
        mq_cb->init_msg_addr = mq_addr;
        mq_cb->flags |= SYS_MALLOC_MEM;
    }
    else
    {
        mq_addr = (void *)(attr->mq_mem);
        mq_size = attr->mq_size;
    }

    os_mq_init(&mq_cb->mq, name, mq_addr, mq_size, align_msg_size, OS_IPC_FLAG_FIFO);

    return mq_cb;
}

const char *osMessageQueueGetName(osMessageQueueId_t mq_id)
{
    mq_cb_t *mq_cb;

    mq_cb = (mq_cb_t *)mq_id;

    if ((OS_NULL == mq_cb) || (os_object_get_type(&mq_cb->mq.parent.parent) != OS_OBJECT_MESSAGEQUEUE))
    {
        return OS_NULL;
    }

    return mq_cb->mq.parent.parent.name;
}
/**
 ***********************************************************************************************************************
 * @brief           The blocking function osMessageQueuePut puts the message pointed to by msg_ptr into the the message queue specified 
 *                  by parameter mq_id. The parameter msg_prio is used to sort message according their priority (higher numbers indicate 
 *                  a higher priority) on insertion.
 *
 * @attention       OneOS don't support messagequeue priority
 *
 * @param[in]       mq_id         message queue ID obtained by osMessageQueueNew.
 * @param[in]       msg_ptr       pointer to buffer with message to put into a queue.
 * @param[in]       msg_prio      message priority.
 * @param[in]       timeout       Timeout Value or 0 in case of no time-out. 
 *
 * @return          status code that indicates the execution status of the function.
 ***********************************************************************************************************************
 */
osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
    os_err_t result;
    mq_cb_t *mq_cb;

    mq_cb = (mq_cb_t *)mq_id;

    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_IPC_WAITING_FOREVER == timeout));

    if (OS_NULL == mq_cb || (OS_NULL == msg_ptr))
    {
        return osErrorParameter;
    }

    result = os_mq_send(&mq_cb->mq, (void *)msg_ptr, mq_cb->init_msg_size, timeout);

    if (OS_EOK == result)
    {
        return osOK;
    }
    else if (OS_EFULL == result)
    {
        return osErrorResource;
    }
    else if (OS_ETIMEOUT == result)
    {
        return osErrorTimeout;
    }
    else
    {
        return osError;
    }
}

osStatus_t osMessageQueueGet(osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout)
{
    mq_cb_t  *mq_cb;
    os_err_t  result;
    os_size_t receive_len;

    mq_cb = (mq_cb_t *)mq_id;

    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_IPC_WAITING_FOREVER == timeout));

    if (OS_NULL == mq_cb || (OS_NULL == msg_ptr))
    {
        return osErrorParameter;
    }
    result = os_mq_recv(&mq_cb->mq, msg_ptr, mq_cb->init_msg_size, timeout, &receive_len);

    if (OS_EOK == result)
    {
        return osOK;
    }
    else if (OS_ETIMEOUT == result)
    {        
        return osErrorTimeout;
    }
    else if(OS_EEMPTY == result)
    {
        return osErrorResource;
    }
    else
    {
        return osError;
    }
}

uint32_t osMessageQueueGetCapacity(osMessageQueueId_t mq_id)
{
    mq_cb_t *mq_cb;

    mq_cb = (mq_cb_t *)mq_id;

    if ((OS_NULL == mq_cb) || (os_object_get_type(&mq_cb->mq.parent.parent) != OS_OBJECT_MESSAGEQUEUE))
    {
        return 0U;
    }

    return (uint32_t)mq_cb->mq.max_msgs;
}

uint32_t osMessageQueueGetMsgSize(osMessageQueueId_t mq_id)
{
    mq_cb_t *mq_cb;

    mq_cb = (mq_cb_t *)mq_id;

    if ((OS_NULL == mq_cb) || (os_object_get_type(&mq_cb->mq.parent.parent) != OS_OBJECT_MESSAGEQUEUE))
    {
        return 0U;
    }

    return (uint32_t)mq_cb->mq.msg_size;
}

uint32_t osMessageQueueGetCount(osMessageQueueId_t mq_id)
{
    mq_cb_t *mq_cb;

    mq_cb = (mq_cb_t *)mq_id;

    if ((OS_NULL == mq_cb) || (os_object_get_type(&mq_cb->mq.parent.parent) != OS_OBJECT_MESSAGEQUEUE))
    {
        return 0U;
    }

    return (uint32_t)mq_cb->mq.entry;
}

uint32_t osMessageQueueGetSpace(osMessageQueueId_t mq_id)
{
    mq_cb_t *mq_cb;

    mq_cb = (mq_cb_t *)mq_id;

    if ((OS_NULL == mq_cb) || (os_object_get_type(&mq_cb->mq.parent.parent) != OS_OBJECT_MESSAGEQUEUE))
    {
        return 0U;
    }

    return (mq_cb->mq.max_msgs - mq_cb->mq.entry);
}

osStatus_t osMessageQueueReset(osMessageQueueId_t mq_id)
{
    os_err_t result;
    mq_cb_t *mq_cb;

    mq_cb = (mq_cb_t *)mq_id;

    if (OS_NULL == mq_cb || (os_object_get_type(&mq_cb->mq.parent.parent) != OS_OBJECT_MESSAGEQUEUE))
    {
        return osErrorParameter;
    }

    result = os_mq_control(&mq_cb->mq, OS_IPC_CMD_RESET, OS_NULL);

    if (OS_EOK == result)
    {
        return osOK;
    }
    else
    {
        return osError;
    }
}

osStatus_t osMessageQueueDelete(osMessageQueueId_t mq_id)
{
    mq_cb_t *mq_cb;

    mq_cb = (mq_cb_t *)mq_id;
    
    if (OS_NULL == mq_cb || (os_object_get_type(&mq_cb->mq.parent.parent) != OS_OBJECT_MESSAGEQUEUE))
    {
        return osErrorParameter;
    }

    os_mq_deinit(&(mq_cb->mq));

    if (mq_cb->flags & SYS_MALLOC_MEM)
    {
        os_free(mq_cb->init_msg_addr);
    }

    if (mq_cb->flags & SYS_MALLOC_CTRL_BLK)
    {
        os_free(mq_cb);
    }

    return osOK;
}

#endif  /* OS_USING_MESSAGEQUEUE */

