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
 * @file        rt_mem.c
 *
 * @brief       Implementation of RT-Thread adaper memory management function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <rtthread.h>
#include <string.h>

#ifdef RT_USING_MEMPOOL
rt_err_t rt_mp_init(struct rt_mempool *mp,
                    const char        *name,
                    void              *start,
                    rt_size_t          size,
                    rt_size_t          block_size)
{
    os_err_t ret;

    OS_ASSERT(mp);
    OS_ASSERT(start);

    ret = os_mp_init(&mp->oneos_mp,name,start,size,block_size);

    /* Set the flag for static creation */
    mp->is_static = OS_TRUE;    

    return (rt_err_t)ret;
}

rt_err_t rt_mp_detach(struct rt_mempool *mp)
{
    os_err_t ret;

    OS_ASSERT(mp);
    OS_ASSERT(OS_TRUE == mp->is_static);

    ret = os_mp_deinit(&mp->oneos_mp);

    return (rt_err_t)ret;
}

#ifdef RT_USING_HEAP
rt_mp_t rt_mp_create(const char *name, 
                     rt_size_t   block_count, 
                     rt_size_t   block_size)
{
    void       *block_start;
    rt_mp_t     mp;
    os_err_t    ret;
    rt_size_t   pool_size;

    OS_ASSERT(block_count);

    mp = (rt_mp_t)os_malloc(sizeof(struct rt_mempool));

    if (OS_NULL == mp)
    {
        return RT_NULL;
    }

    memset(mp, 0, sizeof(struct rt_mempool));

    /* Calculate the size of the memory pool according to block_size and block_count */
    pool_size   = OS_MEMPOOL_SIZE(block_count, block_size);

    block_start = os_malloc(pool_size);

    if (OS_NULL == block_start)
    {
        os_free(mp);
        return RT_NULL;
    }
    memset(block_start, 0, pool_size);
    
    ret = os_mp_init(&mp->oneos_mp, name, block_start, pool_size, block_size);

    if (OS_EOK != ret)
    {
        os_free(mp);
        os_free(block_start);
        return RT_NULL;
    }

    /* Set the flag for dynamic creation */
    mp->is_static = OS_FALSE; 
    
    return mp;
}

rt_err_t rt_mp_delete(rt_mp_t mp)
{  
    OS_ASSERT(mp);
    OS_ASSERT(OS_FALSE == mp->is_static);

    (void)os_mp_deinit(&mp->oneos_mp);

    os_free(mp->oneos_mp.start_address);
    os_free(mp);
    
    return RT_EOK;
}
#endif /* RT_USING_HEAP */


void *rt_mp_alloc(rt_mp_t mp, rt_int32_t time)
{
    return os_mp_alloc(&mp->oneos_mp,time);
}

void rt_mp_free(void *block)
{
    (void)os_mp_free(block);
}
#endif

#ifdef RT_USING_HEAP
void *rt_malloc(rt_size_t nbytes)
{
    return os_malloc(nbytes);
}

void rt_free(void *ptr)
{
    os_free(ptr);
}

void *rt_realloc(void *ptr, rt_size_t nbytes)
{
    return os_realloc(ptr, nbytes);
}

void *rt_calloc(rt_size_t count, rt_size_t size)
{
    return os_calloc(count,size);
}

void *rt_malloc_align(rt_size_t size, rt_size_t align)
{
    return os_malloc_align(size, align);
}

void rt_free_align(void *ptr)
{
    os_free_align(ptr);
}
#endif /* RT_USING_HEAP */

#ifdef RT_USING_MEMHEAP
rt_err_t rt_memheap_init(struct rt_memheap *memheap,
                         const char        *name,
                         void              *start_addr,
                         rt_size_t          size)
{
    OS_ASSERT(memheap);

    return (rt_err_t)os_memheap_init(&memheap->oneos_memheap, 
                                      name, 
                                      start_addr, 
                                      size);
}

rt_err_t rt_memheap_detach(struct rt_memheap *heap)
{
    OS_ASSERT(heap);
    
    return (rt_err_t)os_memheap_deinit(&heap->oneos_memheap);
}

void *rt_memheap_alloc(struct rt_memheap *heap, rt_size_t size)
{
    OS_ASSERT(heap);
    
    return os_memheap_alloc(&heap->oneos_memheap, size);
}

void *rt_memheap_realloc(struct rt_memheap *heap, 
                         void              *ptr, 
                         rt_size_t          newsize)
{
    OS_ASSERT(heap);
    
    return os_memheap_realloc(&heap->oneos_memheap, ptr, (os_size_t)newsize);
}

void rt_memheap_free(void *ptr)
{
    os_memheap_free(ptr);
}
#endif /* RT_USING_MEMHEAP */

