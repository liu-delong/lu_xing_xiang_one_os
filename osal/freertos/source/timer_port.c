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
 * @brief       This file implements some port functions of FreeRTOS timer.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-05   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>

#include "os_kernel.h"
#include "FreeRTOS.h"
#include "timers.h"

#include "freertos_internal.h"

#define ADAPT_DEBUG_TIMER 0

struct adapt_timer
{
    os_timer_t      *timer_t;
    void 			*pvTimerID;	           
    os_list_node_t  list;      
};
typedef struct adapt_timer adapt_timer_t;

os_list_node_t g_adapt_timer_list = OS_LIST_INIT(g_adapt_timer_list);

TimerHandle_t xTimerCreate(const char * const pcTimerName, 
                           const TickType_t   xTimerPeriodInTicks, 
                           const UBaseType_t  uxAutoReload, 
                           void * const       pvTimerID, 
                           TimerCallbackFunction_t pxCallbackFunction)
{
    OS_ASSERT(xTimerPeriodInTicks > 0);

    os_timer_t*     timer_t;
    static uint32_t t                 = 0;        /* used for index for timer name  */
    char            name[OS_NAME_MAX] = "t_";     /* "assume max num is MAX_UINT32" */
    char            idx[11]           = {0,};
    register os_base_t temp;
    adapt_timer_t 	*p_adapt_timer;

    if (pcTimerName)
    {
        memset(name, 0, sizeof(name));
        strncpy(name, pcTimerName, strlen(pcTimerName) < OS_NAME_MAX ? strlen(pcTimerName) : (OS_NAME_MAX-1));
    }
    else
    {
        oneos_itoa(t++, idx, 10);
        strncat(name, idx, strlen(idx));
    }

    timer_t = os_timer_create(name, 
                              pxCallbackFunction, 
                              OS_NULL, 
                              xTimerPeriodInTicks, 
                              (uxAutoReload == 1 ? OS_TIMER_FLAG_PERIODIC : OS_TIMER_FLAG_ONE_SHOT) | OS_TIMER_FLAG_SOFT_TIMER);

    if(timer_t != OS_NULL)
    {
        p_adapt_timer = (adapt_timer_t *)os_malloc(sizeof(adapt_timer_t));

        if (OS_NULL == p_adapt_timer)
        {
            return OS_NULL;
        }

        p_adapt_timer->pvTimerID = pvTimerID;
        
        p_adapt_timer->timer_t = timer_t;
    
        temp = os_hw_interrupt_disable();

        os_list_add_tail(&g_adapt_timer_list, &p_adapt_timer->list);

        os_hw_interrupt_enable(temp);
    }
    
    return (TimerHandle_t)timer_t;
}

void *pvTimerGetTimerID(TimerHandle_t xTimer)
{
    os_list_node_t  *node;
    adapt_timer_t   *p_adapt_timer;

    OS_ASSERT(xTimer != NULL);
    
    /* Traverse task object list. */
    os_list_for_each(node, &g_adapt_timer_list)
    {
        p_adapt_timer = os_list_entry(node, adapt_timer_t, list);

        if(p_adapt_timer->timer_t == (os_timer_t *)xTimer)
        {
            return p_adapt_timer->pvTimerID;
        }
    }

    return NULL;
}

BaseType_t xTimerGenericCommand(TimerHandle_t      xTimer, 
                                const BaseType_t   xCommandID, 
                                const TickType_t   xOptionalValue, 
                                BaseType_t * const pxHigherPriorityTaskWoken, 
                                const TickType_t   xTicksToWait)
{
    OS_ASSERT(xTimer != NULL);
    os_err_t        ret;
    os_list_node_t  *node;
    adapt_timer_t   *p_adapt_timer;

    OS_ASSERT(xTimer != NULL);

    switch (xCommandID)
    {
        case tmrCOMMAND_RESET:
        case tmrCOMMAND_START:
        case tmrCOMMAND_RESET_FROM_ISR:
        case tmrCOMMAND_START_FROM_ISR:
            ret = os_timer_start((os_timer_t*)xTimer);
            break;
        case tmrCOMMAND_STOP:
        case tmrCOMMAND_STOP_FROM_ISR:
            ret = os_timer_stop((os_timer_t*)xTimer);
            break;
        case tmrCOMMAND_DELETE:
            ret = os_timer_destroy((os_timer_t*)xTimer);
            if(ret == OS_EOK)
            {
                /* Traverse task object list. */
                os_list_for_each(node, &g_adapt_timer_list)
                {
                    p_adapt_timer = os_list_entry(node, adapt_timer_t, list);
                
                    if(p_adapt_timer->timer_t == (os_timer_t *)xTimer)
                    {
                        os_list_del(node);

                        FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TIMER, ("Delete timerID:%d, left list num:%d\n", 
                                                     p_adapt_timer->pvTimerID, 
                                                     os_list_len(&g_adapt_timer_list)));
                        
                        os_free(p_adapt_timer);
                        
                        break;
                    }
                }
            }
            break;
        case tmrCOMMAND_CHANGE_PERIOD:
        case tmrCOMMAND_CHANGE_PERIOD_FROM_ISR:
            ret = os_timer_control((os_timer_t*)xTimer, OS_TIMER_CTRL_SET_TIME, (void *)&xOptionalValue);
            break;
        default:
            FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TIMER, ("timer cmd(%d) not realize yet\r\n", xCommandID));
            ret = OS_ENOSYS;
            break;
    }

    if (OS_NULL != pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }
    
    return ret == OS_EOK ? pdPASS : pdFAIL;
}

BaseType_t xTimerIsTimerActive(TimerHandle_t xTimer)
{
    OS_ASSERT(xTimer != NULL);

    return ((os_timer_t*)xTimer)->parent.flag & OS_TIMER_FLAG_ACTIVATED;    
}

