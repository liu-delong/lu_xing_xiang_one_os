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
 * @brief       This file implements some port functions of FreeRTOS queue.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-05   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include "os_kernel.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "port_tick.h"
#include "task.h"

#include "freertos_internal.h"


#define QUEUE_PORT_NAME_MAX         3
#define UINT32_TOCHAR_LEN           11
#define DECIMAL_TYPE                10

#define ADAPT_DEBUG__COUNT_SEM      0
#define ADAPT_DEBUG_QUEUE           0

struct adapt_count_sem
{
    os_sem_t*       p_os_sem_t;
    os_uint16_t     max;            
    os_list_node_t  list;      
};
typedef struct adapt_count_sem adapt_count_sem_t;

os_list_node_t g_adapt_count_sem_list = OS_LIST_INIT(g_adapt_count_sem_list);


char* oneos_itoa(int value, char* string, int radix)
{
    char zm[37] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char aa[10] = {0};

    int sum = value;
    char* cp = string;
    int i = 0;

    if (value < 0)
    {
        return string;
    } 
    else if (0 == value) 
    {
        char *res_str = "0";

        strncpy(string, res_str, strlen(res_str));
        return string;
    } 
    else 
    {
        while (sum > 0) 
        {
            aa[i++] = zm[sum%radix];
            sum/=radix;
        }
    }

    for (int j=i-1; j>=0; j--) 
    {
        *cp++ = aa[j];
    }

    *cp='\0';
    
    return string;
}

QueueHandle_t xQueueGenericCreate(const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, const uint8_t ucQueueType)
{
    QueueHandle_t rtn_handle;

    OS_ASSERT((int8_t)ucQueueType >= queueQUEUE_TYPE_BASE && ucQueueType <= queueQUEUE_TYPE_RECURSIVE_MUTEX);

#ifdef OS_USING_HEAP
    static uint32_t m = 0;           /* used for index for mutex name   */
    static uint32_t s = 0;           /* used for index for sem name     */
    static uint32_t q = 0;           /* used for index for queue name   */
    char name[OS_NAME_MAX];          /* "assume max num is MAX_UINT32"  */
    char idx[UINT32_TOCHAR_LEN];

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueGenericCreate 0x%x\n",ucQueueType));

    memset(name, 0, sizeof(name));
    memset(idx,  0, sizeof(idx));

    if (queueQUEUE_TYPE_RECURSIVE_MUTEX == ucQueueType)
    {
        oneos_itoa(m++, idx, DECIMAL_TYPE);
        strncat(name, "m_", strlen("m_"));
        strncat(name, idx, strlen(idx));        
        rtn_handle = (QueueHandle_t)os_mutex_create(name, OS_IPC_FLAG_FIFO, OS_TRUE);
    }
    else if (queueQUEUE_TYPE_MUTEX == ucQueueType)
    {
        oneos_itoa(m++, idx, DECIMAL_TYPE);
        strncat(name, "m_", strlen("m_"));
        strncat(name, idx, strlen(idx));        
        rtn_handle = (QueueHandle_t)os_mutex_create(name, OS_IPC_FLAG_FIFO, OS_FALSE);
    }
    else if (queueQUEUE_TYPE_BINARY_SEMAPHORE == ucQueueType)
    {
        oneos_itoa(s++, idx, DECIMAL_TYPE);
        strncat(name, "s_", strlen("s_"));
        strncat(name, idx, strlen(idx));
        rtn_handle = (QueueHandle_t)os_sem_create(name, 0, OS_IPC_FLAG_FIFO);
    }
    else if (queueQUEUE_TYPE_COUNTING_SEMAPHORE == ucQueueType)
    {
        oneos_itoa(s++, idx, DECIMAL_TYPE);
        strncat(name, "s_", strlen("s_"));
        strncat(name, idx, strlen(idx));
        rtn_handle = (QueueHandle_t)os_sem_create(name, uxQueueLength, OS_IPC_FLAG_FIFO);
    }
    else
    {
        oneos_itoa(q++, idx, DECIMAL_TYPE);
        strncat(name, "q_", strlen("q_"));
        strncat(name, idx, strlen(idx));
        rtn_handle = (QueueHandle_t)os_mq_create(name, uxItemSize, uxQueueLength, OS_IPC_FLAG_FIFO);
    }
#else
    #error "OS_USING_HEAP should be defined in menuconfig, otherwise xxx_create will no use"
#endif

    return rtn_handle;
}

void vQueueDelete(QueueHandle_t xQueue)
{
    os_list_node_t    *node;
    adapt_count_sem_t *p_adapt_count_sem;
    register os_base_t temp;
    
    OS_ASSERT(NULL != xQueue);
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("vQueueDelete 0x%x\n",xQueue));

    struct os_ipc_object *parent = (struct os_ipc_object *)xQueue;

    if (OS_OBJECT_MUTEX == os_object_get_type(&parent->parent))
    {
        os_mutex_destroy((os_mutex_t*)xQueue);
    }
    else if (OS_OBJECT_SEMAPHORE == os_object_get_type(&parent->parent))
    {
        temp = os_hw_interrupt_disable();

        os_list_for_each(node, &g_adapt_count_sem_list)
        {
            p_adapt_count_sem = os_list_entry(node, adapt_count_sem_t, list);

            if (p_adapt_count_sem->p_os_sem_t == (os_sem_t*)xQueue)
            {
                os_list_del(node);

                FREERTOS_ADAPT_LOG(ADAPT_DEBUG__COUNT_SEM, ("Delete count_sem_node, left list num:%d\n",os_list_len(&g_adapt_count_sem_list)));

                /* Free semaphore object */
                os_free(p_adapt_count_sem);
                
                break;
            }
        }

        os_hw_interrupt_enable(temp);
        
        os_sem_destroy((os_sem_t*)xQueue);
    }
    else
    {
        os_mq_destroy((os_mq_t*)xQueue);
    }
}

BaseType_t xQueueGenericSend(QueueHandle_t      xQueue, 
                             const void * const pvItemToQueue, 
                             TickType_t         xTicksToWait, 
                             const BaseType_t   xCopyPosition)
{
    OS_ASSERT(NULL != xQueue);
    OS_ASSERT(queueOVERWRITE != xCopyPosition);
    
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE,("xQueueGenericSend in xQueue:0x%x\n", xQueue));

    adapt_count_sem_t *p_adapt_count_sem;
    os_list_node_t    *node = OS_NULL;
    register os_base_t temp;
      
    os_err_t ret;
    struct os_ipc_object *parent = (struct os_ipc_object *)xQueue;
    
    if (OS_OBJECT_MUTEX == os_object_get_type(&parent->parent))
    {
        /* we create MUTEX in queueQUEUE_TYPE_RECURSIVE_MUTEX */
        if (((os_mutex_t*)xQueue)->recursive)
        {
            ret = os_mutex_recursive_unlock((os_mutex_t*)xQueue);
        }
        else
        {
            ret = os_mutex_unlock((os_mutex_t*)xQueue);
        }
    }
    else if (OS_OBJECT_SEMAPHORE == os_object_get_type(&parent->parent))
    {
        temp = os_hw_interrupt_disable();
        
        os_list_for_each(node, &g_adapt_count_sem_list)
        {
            p_adapt_count_sem = os_list_entry(node, adapt_count_sem_t, list);

            if (p_adapt_count_sem->p_os_sem_t == (os_sem_t*)xQueue)
            {
                if (p_adapt_count_sem->max == ((os_sem_t*)xQueue)->count)
                {
                    os_hw_interrupt_enable(temp);
                    return pdFAIL;
                }
            }
        }

        os_hw_interrupt_enable(temp);

        ret = os_sem_post((os_sem_t*)xQueue);
    }
    else
    {    
        if (queueSEND_TO_BACK == xCopyPosition)
        {
            ret = os_mq_send((os_mq_t*)xQueue, 
                             (void *)pvItemToQueue, 
                             ((os_mq_t*)xQueue)->msg_size, 
                             xTicksToWait);
        }
        else if (queueSEND_TO_FRONT == xCopyPosition)
        {
            ret = os_mq_send_urgent((os_mq_t*)xQueue, 
                                    (void *)pvItemToQueue, 
                                    ((os_mq_t*)xQueue)->msg_size, 
                                    xTicksToWait);
        }
        else
        {
            ret = OS_ENOSYS;
        }
    }
    
    if (OS_EOK == ret)
    {
        return pdPASS;
    }
    else
    {
        return errQUEUE_FULL;
    }
}

BaseType_t xQueueGenericReceive(QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait, const BaseType_t xJustPeeking)
{
    OS_ASSERT(NULL != xQueue);
    
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueGenericReceive in:0x%x\n", xQueue));

    os_err_t ret = OS_EOK;
    os_size_t recv_size = 0;
    struct os_ipc_object *parent = (struct os_ipc_object *)xQueue;
    
    if (OS_OBJECT_MUTEX == os_object_get_type(&parent->parent))
    {
        if (((os_mutex_t*)xQueue)->recursive)
        {
            ret = os_mutex_recursive_lock((os_mutex_t*)xQueue, xTicksToWait);
        }
        else
        {
            ret = os_mutex_lock((os_mutex_t*)xQueue, xTicksToWait);
        }
    }
    else if (OS_OBJECT_SEMAPHORE == os_object_get_type(&parent->parent))
    {
        ret = os_sem_wait((os_sem_t*)xQueue, xTicksToWait);
    }
    else
    {
        OS_ASSERT(pvBuffer != NULL);
        ret = os_mq_recv((os_mq_t*)xQueue, 
                         pvBuffer, 
                         ((os_mq_t*)xQueue)->msg_size, 
                         xTicksToWait, 
                         &recv_size);
    }

    if (OS_EOK == ret)
    {
        return pdPASS;
    }
    else
    {
        return errQUEUE_EMPTY;
    }
}

BaseType_t xQueueReceiveFromISR(QueueHandle_t xQueue, void * const pvBuffer, BaseType_t * const pxHigherPriorityTaskWoken)
{
    OS_DEBUG_IN_INTERRUPT;
    
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueReceiveFromISR\n"));

    if (pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }

    return xQueueGenericReceive(xQueue, pvBuffer, 0, 0);
}

BaseType_t xQueueGenericSendFromISR(QueueHandle_t      xQueue, 
                                    const void * const pvItemToQueue, 
                                    BaseType_t * const pxHigherPriorityTaskWoken, 
                                    const BaseType_t   xCopyPosition )
{
    OS_DEBUG_IN_INTERRUPT;
    
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueGenericSendFromISR\n"));

    if (pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }

    return xQueueGenericSend(xQueue, pvItemToQueue, 0, xCopyPosition);
}

BaseType_t xQueueGiveFromISR(QueueHandle_t xQueue, BaseType_t * const pxHigherPriorityTaskWoken)
{
    OS_DEBUG_IN_INTERRUPT;
    
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueGiveFromISR\n"));

    if (pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }
    
    return xQueueGenericSend(xQueue, NULL, 0, 0);  
}

UBaseType_t uxQueueSpacesAvailable(const QueueHandle_t xQueue)
{
    OS_ASSERT(NULL != xQueue);
    
    struct os_ipc_object *parent = (struct os_ipc_object *)xQueue;
    UBaseType_t spaces = 0;

    if (OS_OBJECT_MESSAGEQUEUE == os_object_get_type(&parent->parent))
    {
        os_mq_t* mq_p = (os_mq_t*)xQueue;
        
        os_enter_critical();
        spaces = mq_p->max_msgs - mq_p->entry;
        os_exit_critical();
    }
    else
    {
        FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("uxQueueSpacesAvailable Not support orther queue type\n"));
        OS_ASSERT(0);
    }
    
    return spaces;
}

UBaseType_t uxQueueMessagesWaiting(const QueueHandle_t xQueue)
{
    OS_ASSERT(NULL != xQueue);

    UBaseType_t spaces = 0;
    struct os_ipc_object *parent = (struct os_ipc_object *)xQueue;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("uxQueueMessagesWaiting\n"));

    os_enter_critical();

    if (OS_OBJECT_SEMAPHORE == os_object_get_type(&parent->parent))
    {
        spaces = ((os_sem_t *)xQueue)->count;
    }
    else if (OS_OBJECT_MESSAGEQUEUE == os_object_get_type(&parent->parent))
    {
        spaces = ((os_mq_t*)xQueue)->entry;
    }
    else
    {
        FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("uxQueueMessagesWaiting Not support orther queue type\n"));
        OS_ASSERT(0);
    }

    os_exit_critical();

    return spaces;
} 


#if (configUSE_MUTEXES == 1)
QueueHandle_t xQueueCreateMutex(const uint8_t ucQueueType)
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueCreateMutex %d\n",ucQueueType));
    
    return xQueueGenericCreate(1, 0, ucQueueType);
}

#if (configUSE_RECURSIVE_MUTEXES == 1)
BaseType_t xQueueTakeMutexRecursive(QueueHandle_t xMutex, TickType_t xTicksToWait)
{    
    return xQueueGenericReceive(xMutex, 0, xTicksToWait, 0);
}
BaseType_t xQueueGiveMutexRecursive(QueueHandle_t xMutex)
{
    return xQueueGenericSend(xMutex, 0, 0, 0);
}
#endif /* configUSE_RECURSIVE_MUTEXES == 1 */

#endif /* configUSE_MUTEXES == 1 */


#if (configUSE_COUNTING_SEMAPHORES == 1)
QueueHandle_t xQueueCreateCountingSemaphore(const UBaseType_t uxMaxCount, const UBaseType_t uxInitialCount)
{
    os_sem_t *sem_p;
    register os_base_t temp;
    adapt_count_sem_t *p_adapt_count_sem;
    
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueCreateCountingSemaphore %d-%d\n", uxMaxCount, uxInitialCount));

    sem_p = xQueueGenericCreate(uxInitialCount, 0, queueQUEUE_TYPE_COUNTING_SEMAPHORE);

    if (OS_NULL == sem_p)
    {
        return OS_NULL;
    }
    
    p_adapt_count_sem = (adapt_count_sem_t *)os_malloc(sizeof(adapt_count_sem_t));

    if (OS_NULL == p_adapt_count_sem)
    {
        return OS_NULL;
    }

    p_adapt_count_sem ->max = uxMaxCount;
    p_adapt_count_sem ->p_os_sem_t = sem_p;

    temp = os_hw_interrupt_disable();

    os_list_add_tail(&g_adapt_count_sem_list, &p_adapt_count_sem->list);

    os_hw_interrupt_enable(temp);

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG__COUNT_SEM, ("Add count_sem_node, total list num:%d\n",os_list_len(&g_adapt_count_sem_list)));

    return sem_p;
}
#endif


