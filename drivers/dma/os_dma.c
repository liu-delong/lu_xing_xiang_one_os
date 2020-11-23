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
 * @file        os_dma.c
 *
 * @brief       this file implements dam
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-10-29    OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <board.h>
#include <os_memory.h>
#include <os_drivers.h>

#ifdef DMA_HEAP_SIZE

#ifndef OS_USING_MEM_HEAP
#error:please define OS_USING_MEM_HEAP for dma heap
#endif

static struct os_memheap g_dma_heap;

void os_dma_mem_init(void)
{
    os_memheap_init(&g_dma_heap, "dma", (void *)DMA_HEAP_BEGIN, (os_size_t)DMA_HEAP_SIZE);
}

void *os_dma_malloc_align(os_size_t size, os_size_t align)
{
    void *align_ptr;
    void *ptr;
    os_size_t align_size;

    /* Align the alignment size to 4 byte */
    align = ((align + 0x03) & ~0x03);

    /* Get total aligned size */
    align_size = ((size + 0x03) & ~0x03) + align;

    /* Allocate memory block from heap */
    ptr = os_memheap_alloc(&g_dma_heap, align_size);
    if (ptr != OS_NULL)
    {
        /* The allocated memory block is aligned */
        if (((os_uint32_t)ptr & (align - 1)) == 0)
        {
            align_ptr = (void *)((os_uint32_t)ptr + align);
        }
        /* The allocated memory block is not aligned */
        else
        {
            align_ptr = (void *)(((os_uint32_t)ptr + (align - 1)) & ~(align - 1));
        }

        /* Set the pointer before alignment pointer to the real pointer */
        *((os_uint32_t *)((os_uint32_t)align_ptr - sizeof(void *))) = (os_uint32_t)ptr;

        ptr = align_ptr;
    }

    return ptr;
}

void os_dma_free_align(void *ptr)
{
    void *real_ptr;

    OS_ASSERT(ptr);

    real_ptr = (void *) * (os_uint32_t *)((os_uint32_t)ptr - sizeof(void *));

    os_memheap_free(real_ptr);
}

#else

void os_dma_mem_init(void)
{
    
}

void *os_dma_malloc_align(os_size_t size, os_size_t align)
{
    return os_malloc_align(size, align);
}

void os_dma_free_align(void *ptr)
{
    os_free_align(ptr);
}

#endif

