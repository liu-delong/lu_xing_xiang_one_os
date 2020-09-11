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
 * @brief       This file implements some port functions of FreeRTOS event.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-05   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>

#include "os_kernel.h"
#include "FreeRTOS.h"
#include "event_groups.h"

#include <os_event.h>

#include "freertos_internal.h"

#define UINT32_TOCHAR_LEN       11
#define DECIMAL_TYPE            10

#define ADAPT_DEBUG_IPC            0

EventGroupHandle_t xEventGroupCreate(void)
{   
    os_event_t*     event_p;
    static uint32_t i = 0;    
    char            name[OS_NAME_MAX] = "e_";
    char            idx[UINT32_TOCHAR_LEN] = {0,};

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_IPC, ("xEventGroupCreate\n"));

    i++;
    oneos_itoa(i, idx, DECIMAL_TYPE);
    strncat(name, idx, strlen(idx));

    event_p = os_event_create(name, OS_IPC_FLAG_FIFO);

    return (EventGroupHandle_t)event_p;
}

void vEventGroupDelete(EventGroupHandle_t xEventGroup)
{
    OS_ASSERT(OS_NULL != xEventGroup);
    OS_ASSERT(OS_OBJECT_EVENT == os_object_get_type(&((os_event_t*)xEventGroup)->parent.parent));
    
    os_event_destroy((os_event_t *)xEventGroup);
}

EventBits_t xEventGroupSetBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet)
{
    OS_ASSERT(OS_NULL != xEventGroup);
    OS_ASSERT(OS_OBJECT_EVENT == os_object_get_type(&((os_event_t*)xEventGroup)->parent.parent));
    
    os_event_send((os_event_t *)xEventGroup, uxBitsToSet);
    
    return ((os_event_t *)xEventGroup)->set;
}

BaseType_t xEventGroupSetBitsFromISR(EventGroupHandle_t xEventGroup,
                                     const EventBits_t  uxBitsToSet,
                                     BaseType_t        *pxHigherPriorityTaskWoken)
{
    OS_DEBUG_IN_INTERRUPT;
    
    OS_ASSERT(OS_NULL != xEventGroup);
    OS_ASSERT(OS_OBJECT_EVENT == os_object_get_type(&((os_event_t*)xEventGroup)->parent.parent));
    
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_IPC, ("xEventGroupSetBitsFromISR\n"));
    
    if (pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }

    if (os_event_send((os_event_t *)xEventGroup, uxBitsToSet) == OS_EOK)
    {
        return pdPASS;
    }
    else
    {
        return pdFAIL;
    }
}

EventBits_t xEventGroupWaitBits(EventGroupHandle_t xEventGroup, 
                                const EventBits_t  uxBitsToWaitFor, 
                                const BaseType_t   xClearOnExit, 
                                const BaseType_t   xWaitForAllBits, 
                                TickType_t         xTicksToWait)
{
    OS_ASSERT(OS_NULL != xEventGroup);
    OS_ASSERT(OS_OBJECT_EVENT == os_object_get_type(&((os_event_t*)xEventGroup)->parent.parent));
    
    os_uint8_t option = 0;
    os_uint32_t e;
    
    if (xClearOnExit)
    {
        option |= OS_EVENT_OPTION_CLEAR;
    }

    if (xWaitForAllBits)
    {
        option |= OS_EVENT_OPTION_AND;
    }
    else
    {
        option |= OS_EVENT_OPTION_OR;
    }
        
    os_event_recv((os_event_t *)xEventGroup, uxBitsToWaitFor, option, xTicksToWait, &e);

    return e;
}

EventBits_t xEventGroupClearBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear)
{
    os_ubase_t level;

    EventBits_t uxReturn;
    
    OS_ASSERT(OS_NULL != xEventGroup);
    OS_ASSERT(OS_OBJECT_EVENT == os_object_get_type(&((os_event_t*)xEventGroup)->parent.parent));

    level = os_hw_interrupt_disable();
    
    /* The value returned is the event group value prior to the bits being cleared. */
    uxReturn = ((os_event_t *)xEventGroup)->set;

    /* Clear the bits. */
    ((os_event_t*)xEventGroup)->set &= ~uxBitsToClear;

    os_hw_interrupt_enable(level);

    return uxReturn;
}

