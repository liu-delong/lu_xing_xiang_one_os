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
 * @file        queue_port.c
 *
 * @brief       This file implements some port functions of FreeRTOS mem.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-05   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include "os_kernel.h"
#include "FreeRTOS.h"

void *pvPortMalloc(size_t xWantedSize)
{
    return os_malloc(xWantedSize);
}

void vPortFree(void *pv)
{
    os_free(pv);
}

void *pvPortRealloc(void *pv, size_t size)
{
    return os_realloc(pv, size);
}

void *pvPortCalloc(size_t nmemb, size_t size)
{
    return os_calloc(nmemb, size);
}

size_t xPortGetFreeHeapSize(void)
{
    os_uint32_t total;
    os_uint32_t used;
    os_uint32_t max_used;
    
    os_memory_info(&total, &used, &max_used);

    return (size_t)(total - used);
}

size_t xPortGetMinimumEverFreeHeapSize(void)
{
    os_uint32_t total;
    os_uint32_t used;
    os_uint32_t max_used;
    
    os_memory_info(&total, &used, &max_used);

    return (size_t)(total - max_used);
}

