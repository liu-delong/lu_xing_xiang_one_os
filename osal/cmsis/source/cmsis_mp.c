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
 * @file        cmsis_mp.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 mempool function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <os_errno.h>
#include <os_util.h>
#include <string.h>
#include <os_hw.h>
#include <os_mq.h>

#include "cmsis_internal.h" 

#ifdef OS_USING_MEM_POOL
osMemoryPoolId_t osMemoryPoolNew(uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr)
{
    char                name[OS_NAME_MAX];
    void               *mp_addr;
    os_uint32_t         mp_size;
    mempool_cb_t       *mempool_cb;
    static os_uint16_t  memory_pool_number = 1U;

    if ((0U == block_count) || (0U == block_size))
    {
        return OS_NULL;
    }
    
    /* OneOS object's name can't be NULL */
    if ((OS_NULL == attr) || (OS_NULL == attr->name))
    {
        os_snprintf(name, sizeof(name), "mp%02d", memory_pool_number++);
    }
    else
    {
        os_snprintf(name, sizeof(name), "%s", attr->name);
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->cb_mem))
    {
        mempool_cb = os_malloc(sizeof(mempool_cb_t));
        if (OS_NULL == mempool_cb)
        {
            return OS_NULL;
        }
        memset(mempool_cb, 0, sizeof(mempool_cb_t));
        mempool_cb->flags |= SYS_MALLOC_CTRL_BLK;
    }
    else
    {
        if (attr->cb_size >= sizeof(mempool_cb_t))
        {
            mempool_cb = attr->cb_mem;
            mempool_cb->flags = 0;
        }
        else
        {
            return OS_NULL;
        }
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->mp_mem))
    {
        block_size = OS_ALIGN_UP(block_size, OS_ALIGN_SIZE);
        mp_size    = (block_size + OS_MEMPOOL_HEAD_SIZE) * block_count;
        
        mp_addr = os_malloc(mp_size);
        if (OS_NULL == mp_addr)
        {
            if (mempool_cb->flags & SYS_MALLOC_CTRL_BLK)
            {
                os_free(mempool_cb);
            }
            
            return OS_NULL;
        }
        mempool_cb->flags |= SYS_MALLOC_MEM;
    }
    else
    {
        mp_addr = (void *)(attr->mp_mem);
        mp_size = attr->mp_size;
    }

    os_mp_init(&mempool_cb->mp, name, mp_addr, mp_size, block_size);

    return (osMemoryPoolId_t)mempool_cb;
}

const char *osMemoryPoolGetName(osMemoryPoolId_t mp_id)
{
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (os_object_get_type(&mempool_cb->mp.parent) != OS_OBJECT_MEMPOOL))
    {
        return OS_NULL;
    }

    return mempool_cb->mp.parent.name;
}
void *osMemoryPoolAlloc(osMemoryPoolId_t mp_id, uint32_t timeout)
{
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_IPC_WAITING_FOREVER == timeout));

    if ((OS_NULL == mempool_cb) || (os_object_get_type(&mempool_cb->mp.parent) != OS_OBJECT_MEMPOOL))
    {
        return OS_NULL;
    }

    return os_mp_alloc(&mempool_cb->mp, timeout);
}

osStatus_t osMemoryPoolFree(osMemoryPoolId_t mp_id, void *block)
{
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    if (OS_NULL == mempool_cb)
    {
        return osErrorParameter;
    }

    os_mp_free(block);

    return osOK;
}

uint32_t osMemoryPoolGetCapacity(osMemoryPoolId_t mp_id)
{
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (os_object_get_type(&mempool_cb->mp.parent) != OS_OBJECT_MEMPOOL))
    {
        return 0U;
    }

    return mempool_cb->mp.block_total_count;
}

uint32_t osMemoryPoolGetBlockSize(osMemoryPoolId_t mp_id)
{
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (os_object_get_type(&mempool_cb->mp.parent) != OS_OBJECT_MEMPOOL))
    {
        return 0U;
    }

    return mempool_cb->mp.block_size;
}

uint32_t osMemoryPoolGetCount(osMemoryPoolId_t mp_id)
{
    os_size_t     used_blocks;
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (os_object_get_type(&mempool_cb->mp.parent) != OS_OBJECT_MEMPOOL))
    {
        return 0U;
    }

    used_blocks = mempool_cb->mp.block_total_count - mempool_cb->mp.block_free_count;

    return (uint32_t)used_blocks;
}

uint32_t osMemoryPoolGetSpace(osMemoryPoolId_t mp_id)
{
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (os_object_get_type(&mempool_cb->mp.parent) != OS_OBJECT_MEMPOOL))
    {
        return 0U;
    }

    return mempool_cb->mp.block_free_count;
}

osStatus_t osMemoryPoolDelete(osMemoryPoolId_t mp_id)
{
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (os_object_get_type(&mempool_cb->mp.parent) != OS_OBJECT_MEMPOOL))
    {
        return osErrorParameter;
    }

    os_mp_deinit(&mempool_cb->mp);

    if (mempool_cb->flags & SYS_MALLOC_MEM)
    {
        os_free(mempool_cb->mp.start_address);
    }

    if (mempool_cb->flags & SYS_MALLOC_CTRL_BLK)
    {
        os_free(mempool_cb);
    }

    return osOK;
}

#endif /* OS_USING_MEM_POOL */

