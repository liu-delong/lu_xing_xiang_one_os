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
 * @brief       This file implements some port functions of FreeRTOS task.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-05   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include "os_kernel.h"

#include "FreeRTOS.h"
#include "port_tick.h"
#include "portmacro.h"
#include "task.h"

#include "freertos_internal.h"


#define ADAPT_DEBUG_TASK     0

/* convert priority from freertos to OneOs
 * in freertos, 0 is the lowest priority, so convert "0"&"1" to OS_TASK_PRIORITY_MAX - pri - 1
 * the others convert to OS_TASK_PRIORITY_MAX - pri - 219 - 1
 */
os_uint8_t free_to_oneos_pri_convert(os_uint8_t pri)
{
    /* OS_TASK_PRIORITY_MAX = configMAX_PRIORITIES */
#if (OS_TASK_PRIORITY_MAX != configMAX_PRIORITIES)
    #error "OS_TASK_PRIORITY_MAX value must equal configMAX_PRIORITIES Value"
#endif

    OS_ASSERT(pri < configMAX_PRIORITIES);

    return OS_TASK_PRIORITY_MAX - pri - 1;
}

/*convert priority from cmos to freertos*/
os_uint8_t oneos_to_free_pri_convert(os_uint8_t pri)
{
    /* OS_TASK_PRIORITY_MAX = configMAX_PRIORITIES */
#if (OS_TASK_PRIORITY_MAX != configMAX_PRIORITIES)
    #error "OS_TASK_PRIORITY_MAX value must equal configMAX_PRIORITIES Value"
#endif

    OS_ASSERT(pri < OS_TASK_PRIORITY_MAX);

    return configMAX_PRIORITIES - pri - 1;
}

TickType_t xTaskGetTickCount(void)
{
    return (TickType_t)os_tick_get();
}

TickType_t xTaskGetTickCountFromISR(void)
{
    OS_DEBUG_IN_INTERRUPT;
    
    return (TickType_t)os_tick_get();
}

void vTaskStepTick(const TickType_t xTicksToJump)
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("vTaskStepTick\n"));

    os_tick_set(os_tick_get() + xTicksToJump);
}

UBaseType_t uxTaskGetBottomOfStack(TaskHandle_t xTaskHandle)
{
    os_task_t *thread_p;
    
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("uxTaskGetBottomOfStack\n"));    
    
    thread_p = (os_task_t*)xTaskHandle;
    
    if (OS_NULL == xTaskHandle)
    {
        thread_p = os_task_self();
    }
    
    return (UBaseType_t)thread_p->stack_addr;
}

BaseType_t xTaskGenericCreate(TaskFunction_t     pxTaskCode, 
                              const char * const pcName, 
                              const uint16_t     usStackDepth, 
                              void * const       pvParameters, 
                              UBaseType_t        uxPriority, 
                              TaskHandle_t * const pxCreatedTask, 
                              StackType_t * const  puxStackBuffer, 
                              const MemoryRegion_t * const xRegions ) 
{
    OS_ASSERT(NULL != pxTaskCode);
    OS_ASSERT(NULL != pcName);
    OS_ASSERT( ( ( uxPriority & ( ~portPRIVILEGE_BIT ) ) < configMAX_PRIORITIES ) );

    os_task_t *thread_p;
    
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("xTaskGenericCreate %s,stack:0x%x\n", pcName, puxStackBuffer));
    
    if (OS_NULL == puxStackBuffer)
    {
#ifdef OS_USING_HEAP
        thread_p = os_task_create(pcName, 
                                  (TaskFunction_t)pxTaskCode, 
                                  pvParameters, 
                                  usStackDepth * sizeof(StackType_t), 
                                  free_to_oneos_pri_convert(uxPriority), 
                                  1);
#else
        #error "Please config OS_USING_HEAP!"
#endif

        if (OS_NULL == thread_p)
        {
            return pdFAIL;
        }
    }
    else
    {
        thread_p = os_malloc(sizeof(struct os_task));
        FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("malloc size:%d, ptr:%d\n",sizeof(struct os_task), thread_p));
        if (OS_NULL == thread_p)
        {
            return pdFAIL;
        }
        
        if (OS_EOK != os_task_init(thread_p, 
                                   pcName, 
                                   (TaskFunction_t)pxTaskCode, 
                                   pvParameters, puxStackBuffer,
                                   usStackDepth * sizeof(StackType_t), 
                                   free_to_oneos_pri_convert(uxPriority), 
                                   1))
        {
            return pdFAIL;
        }
    }

    os_task_startup(thread_p);
    
    if (OS_NULL != pxCreatedTask)
    {
        *pxCreatedTask = (TaskHandle_t)thread_p;
    }
    
    return pdPASS;
}
    
UBaseType_t uxTaskGetNumberOfTasks( void )
{
    struct os_object_info *information;

    information = os_object_get_info(OS_OBJECT_TASK);
    
    OS_ASSERT(information != OS_NULL);

    return (UBaseType_t)os_list_len(&information->object_list);
}

void vTaskSuspendAll( void )
{
    os_enter_critical();
}

BaseType_t xTaskResumeAll( void )
{
    os_exit_critical();
    
    return pdFALSE;
}

#if (INCLUDE_vTaskPrioritySet == 1)
void vTaskPrioritySet(TaskHandle_t xTask, UBaseType_t uxNewPriority)
{
    OS_ASSERT( ( ( uxNewPriority & ( ~portPRIVILEGE_BIT ) ) < configMAX_PRIORITIES ) );

    os_task_t *thread_p;
    os_uint8_t pri;

    pri = free_to_oneos_pri_convert(uxNewPriority);
    
    thread_p = (os_task_t*)xTask;
    if (OS_NULL == thread_p)
    {
        thread_p = os_task_self();
    }
    
    os_task_control(thread_p, OS_TASK_CTRL_CHANGE_PRIORITY, &pri);
}
#endif

#if (INCLUDE_vTaskSuspend == 1)
extern os_err_t os_task_suspend(os_task_t *task);
extern void     os_schedule(void);
extern os_err_t os_task_resume(os_task_t *task);

void vTaskSuspend(TaskHandle_t xTaskToSuspend)
{
    os_task_t *thread_p;

    thread_p = (os_task_t*)xTaskToSuspend;
    
    if (OS_NULL == thread_p)
    {
        thread_p = os_task_self();
    }
    os_task_suspend(thread_p);

	os_schedule();
}

void vTaskResume(TaskHandle_t xTaskToResume)
{
    os_task_t *thread_p;

    thread_p = (os_task_t *)xTaskToResume;
    
    if (OS_NULL == thread_p)
    {
        thread_p = os_task_self();
    }
    
    os_task_resume(thread_p);
    
    os_schedule();
}

BaseType_t xTaskResumeFromISR(TaskHandle_t xTaskToResume)
{
    OS_DEBUG_IN_INTERRUPT;
    
    vTaskResume(xTaskToResume);

    return pdFALSE;
}
#endif

#if (INCLUDE_vTaskDelete == 1)
extern void os_task_exit(void);
void vTaskDelete(TaskHandle_t xTaskToDelete)
{
    os_task_t *thread_p;
    
    thread_p = (os_task_t *)xTaskToDelete;
    
    if (OS_NULL == thread_p || thread_p == os_task_self())
    {
        os_task_exit();
    }
    
    os_task_destroy(thread_p);
}
#endif


#if (( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ))
BaseType_t xTaskGetSchedulerState( void )
{    
    if (0 == os_critical_level())
    {
        return taskSCHEDULER_RUNNING;
    }
    else
    {
        return taskSCHEDULER_SUSPENDED;
    }
}
#endif

#if (INCLUDE_vTaskDelay == 1)
void vTaskDelay(const TickType_t xTicksToDelay)
{
    os_task_delay(xTicksToDelay);
}
#endif

#if (INCLUDE_uxTaskPriorityGet == 1)
UBaseType_t uxTaskPriorityGet(TaskHandle_t pthread)
{
    os_uint8_t thread_pri;
    os_task_t  *thread_p;

    thread_p = (os_task_t *)pthread;

    os_enter_critical();
    
    if (OS_NULL == thread_p)
    {
        thread_p = os_task_self();
    }
    
    thread_pri = oneos_to_free_pri_convert(thread_p->current_priority);
    
    os_exit_critical();

    return thread_pri;
}
#endif

#if (INCLUDE_eTaskGetState == 1)
eTaskState eTaskGetState(TaskHandle_t xTask)
{
    os_task_t *thread_p;

    thread_p = (os_task_t *)xTask;
    if (OS_NULL == thread_p)
    {
        thread_p = os_task_self();
    }
    
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("eTaskGetState %d\n" ,thread_p->stat & OS_TASK_STAT_MASK));

    switch(((os_task_t*)thread_p)->stat & OS_TASK_STAT_MASK)
    {
        case OS_TASK_RUNNING:
            return eRunning;
        case OS_TASK_READY:
            return eReady;
        case OS_TASK_SUSPEND:
            return eSuspended;
        case OS_TASK_CLOSE:
            /*use default value*/
        case OS_TASK_INIT:
            /*use default value*/
        default:
            return eDeleted;
    }
}
#endif

#if (( INCLUDE_xTaskGetCurrentTaskHandle == 1 ) || ( configUSE_MUTEXES == 1 ))
TaskHandle_t xTaskGetCurrentTaskHandle(void)
{
    return (TaskHandle_t)os_task_self();
}
#endif 

#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
UBaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t thread)
{
    UBaseType_t uxReturn;
    os_uint8_t *ptr;
    os_task_t  *thread_p;

    thread_p = (os_task_t*)thread;
    if (OS_NULL == thread_p)
    {
        thread_p = os_task_self();
    }

#if defined(ARCH_CPU_STACK_GROWS_UPWARD)
    ptr = (os_uint8_t *)thread_p->stack_addr + thread_p->stack_size;
    while(*ptr == '#')
    {
        ptr--;
    }
    
    uxReturn = ((os_uint32_t)ptr - (os_uint32_t)thread_p->stack_addr);
#else
    ptr = (os_uint8_t *)thread_p->stack_addr;
    while(*ptr == '#')
    {
        ptr++;
    }

    uxReturn = (thread_p->stack_size + (os_uint32_t)thread_p->stack_addr - (os_uint32_t) ptr);
#endif

    uxReturn = thread_p->stack_size -  uxReturn;

    uxReturn /= ( uint32_t ) sizeof( StackType_t );

    return uxReturn;
}
#endif

#if (INCLUDE_pcTaskGetTaskName == 1)
char *pcTaskGetTaskName(TaskHandle_t xTaskToQuery) 
{
    os_task_t *thread_p;

    thread_p = (os_task_t *)xTaskToQuery;
    
    if (OS_NULL == thread_p)
    {
        thread_p = os_task_self();
    }

    return thread_p->parent.name;
}
#endif

#if (( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ))
static char *prvWriteNameToBuffer(char *pcBuffer, const char *pcTaskName)
{
    BaseType_t x;

    /* Start by copying the entire string. */
    strcpy( pcBuffer, pcTaskName );

    /* Pad the end of the string with spaces to ensure columns line up whenprinted out. */
    for (x = strlen( pcBuffer ); x < ( configMAX_TASK_NAME_LEN - 1 ); x++)
    {
        pcBuffer[ x ] = ' ';
    }

    /* Terminate. */
    pcBuffer[ x ] = 0x00;

    /* Return the new end of string. */
    return &( pcBuffer[ x ] );
}
#endif

#if (( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ))
void vTaskList(char * pcWriteBuffer)
{
    struct os_object_info *info;
    struct os_task        *thread;
    struct os_list_node   *node;
    struct os_list_node   *list;
    os_object_t *obj;
    os_uint8_t  *ptr;
    os_uint32_t  water_mark;
    char         cStatus;
    os_uint8_t   stat;

    info = os_object_get_info(OS_OBJECT_TASK);
    list = &info->object_list;
    
    /* Make sure the write buffer does not contain a string. */
    *pcWriteBuffer = 0x00;

    for (node = list->next; node != list; node = node->next)
    {
        obj = os_list_entry(node, struct os_object, list);
        thread = (struct os_task *)obj;

        stat = (thread->stat & OS_TASK_STAT_MASK);
        if (stat == OS_TASK_READY)
        {
            cStatus = 'R';
        }
        else if (stat == OS_TASK_SUSPEND)
        {
            cStatus = 'S';
        }
        else if (stat == OS_TASK_INIT)
        {
            cStatus = 'I';
        }
        else if (stat == OS_TASK_CLOSE)
        {
            cStatus = 'D';
        }

        pcWriteBuffer = prvWriteNameToBuffer( pcWriteBuffer, thread->parent.name );
        
#if defined(ARCH_CPU_STACK_GROWS_UPWARD)
        water_mark = 0;
        ptr = (os_uint8_t *)thread->stack_addr + thread->stack_size;
        while (*ptr-- == '#')
        {
            water_mark++;
        }
#else
        water_mark = 0;
        ptr = (os_uint8_t *)thread->stack_addr;
        while (*ptr++ == '#')
        {
            water_mark++;
        }
#endif
        /* Write the rest of the string. */
        sprintf(pcWriteBuffer, "\t%c\t%u\t%u\t%u\r\n", 
                cStatus, 
                oneos_to_free_pri_convert(thread->current_priority),
                water_mark / sizeof( StackType_t ), 
                0);

        pcWriteBuffer += strlen( pcWriteBuffer );
    }
}
#endif

void vTaskStartScheduler(void)
{    
    /* os_startup has been startup before main by $Sub$$main */
    while (1)
    {
        vTaskDelay(1000);
    }
}

void vPortEnterCritical(void)
{
    os_enter_critical();
}

void vPortExitCritical(void)
{
    os_exit_critical();
}

void vPortRaiseBASEPRI(void)
{
    os_hw_interrupt_disable();
}

uint32_t ulPortRaiseBASEPRI(void)
{
    return (uint32_t)os_hw_interrupt_disable();
}

void vPortSetBASEPRI(uint32_t ulNewMaskValue)
{
    os_hw_interrupt_enable(ulNewMaskValue);
}

