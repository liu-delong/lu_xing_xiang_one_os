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
 * @file        cpuport.c
 *
 * @brief       This file provides functions related to the RISC-V architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-05-18   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <os_util.h>
#include <os_hw.h>
#include <os_task.h>
#include "cpuport.h"
#include "FreeRTOSConfig.h"

#include "portmacro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_irq.h>
#include <os_clock.h>
#include "FreeRTOS.h"
#include <task.h>

#include "os_kernel.h"
#include "rom/ets_sys.h"
#include "esp_newlib.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"



portSTACK_TYPE * volatile pxCurrentTCB[ portNUM_PROCESSORS ] = { NULL };

portSTACK_TYPE * volatile pxSaveTCB[ portNUM_PROCESSORS ] = { OS_NULL };


#define OS_USING_CPUID	0

void os_hw_context_switch_to(os_uint32_t to)
{
    pxSaveTCB[OS_USING_CPUID] = (portSTACK_TYPE *)to;
}

void os_hw_context_switch(os_uint32_t from, os_uint32_t to)
{
    pxSaveTCB[OS_USING_CPUID] = (portSTACK_TYPE *)to;
    portYIELD();
}


void vTaskSwitchContext( void )
{
    if (pxSaveTCB[OS_USING_CPUID] != NULL)
    {
        pxCurrentTCB[OS_USING_CPUID] = pxSaveTCB[OS_USING_CPUID];
        pxSaveTCB[OS_USING_CPUID] = NULL;
        return;
    }
}

os_base_t os_hw_interrupt_disable(void)
{
    return portENTER_CRITICAL_NESTED();
}

void os_hw_interrupt_enable(os_base_t level)
{
    portEXIT_CRITICAL_NESTED(level);
}


struct _reent* __getreent()
{
    return _GLOBAL_REENT;
}


static void task_exit_entry(void *parameter)
{
    extern void os_task_exit(void);

    os_task_t *task;
    void (*entry)(void* parameter);

    task = os_task_self();

    entry = task->entry;
    entry(parameter);

    /* invoke task_exit */
    os_task_exit();
}


/**
 ***********************************************************************************************************************
 * @brief           This function initializes the task stack space.
 *
 * @param[in]       task_entry      The entry of task.
 * @param[in]       parameter       The parameter of task.
 * @param[in]       stack_addr      Stack start address.
 * @param[in]       task_exit       The function will be called when task exit.
 *
 * @return          Task's current stack address.
 ***********************************************************************************************************************
 */
os_uint8_t *os_hw_stack_init(void *tentry, void *parameter, os_uint8_t *stack_addr, void *texit)
{
	os_uint8_t *ret_sp = OS_NULL;
#if( portUSING_MPU_WRAPPERS == 1 )
	ret_sp = (os_uint8_t *)pxPortInitialiseStack(stack_addr, task_exit_entry, parameter, OS_FALSE);
#else
	ret_sp = (os_uint8_t *)pxPortInitialiseStack(stack_addr, task_exit_entry, parameter);
#endif

    return ret_sp;
}


/**
 ***********************************************************************************************************************
 * @brief           This function sets the stack addr of task.
 *
 * @param[in]       from            SP of 'from' task.
 * @param[in]       to              sp of  'to' task.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hw_context_switch_interrupt(os_uint32_t from, os_uint32_t to)
{
   pxSaveTCB[OS_USING_CPUID] = (portSTACK_TYPE *)to;
   _frxt_setup_switch();
}

/**
 ***********************************************************************************************************************
 * @brief           Cpu shutdown.
 *
 * @param           No parameter.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hw_cpu_shutdown()
{
    os_uint32_t level;
    os_kprintf("shutdown...\r\n");

    level = os_hw_interrupt_disable();
    while (level)
    {
        OS_ASSERT(0);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Get current sp.
 *
 * @param           None.
 *
 * @return          The current sp.
 ***********************************************************************************************************************
 */
static inline os_size_t *get_sp()
{
    os_size_t *sp;
    asm volatile ("mov %0, sp;" : "=r" (sp));
    return sp;
}
/**
 ***********************************************************************************************************************
 * @brief           This function will return stack pointer of the current running task.
 *
 * @param           No parameter.
 *
 * @return          Return stack pointer of the current running task.
 ***********************************************************************************************************************
 */
os_size_t *get_current_task_sp(void)
{
    return get_sp();
}


#ifdef OS_USING_OVERFLOW_CHECK

/**
 ***********************************************************************************************************************
 * @brief           This function is used to check the stack of "from" task when switching task.
 *
 * @param[in]       task            The descriptor of task control block
 * 
 * @return          None.
 ***********************************************************************************************************************
 */
void schedule_from_task_stack_check(os_task_t *task)
{
    os_size_t sp;
    OS_ASSERT(OS_NULL != task);

    sp = (os_size_t)task->sp;

#if defined(ARCH_CPU_STACK_GROWS_UPWARD)
    if ((*((os_uint8_t *)((os_ubase_t)task->stack_addr + task->stack_size - 1)) != '#') ||
#else
    if ((*((os_uint8_t *)task->stack_addr) != '#') ||
#endif
        ((os_uint32_t)sp < (os_uint32_t)task->stack_addr) ||
        ((os_uint32_t)sp >= (os_uint32_t)task->stack_addr + (os_uint32_t)task->stack_size))
    {
        os_kprintf("schedule to task:%s stack overflow,the sp is 0x%x.\r\n", task->parent.name, sp);
#ifdef OS_USING_SHELL
        {
            extern os_err_t sh_list_task(os_int32_t argc, char **argv);
            sh_list_task(0, OS_NULL);
        }
#endif
        (void)os_hw_interrupt_disable();
        OS_ASSERT(0);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function is used to check the stack of "to" task when switching task.
 *
 * @param[in]       task            The descriptor of task control block
 * 
 * @return          None.
 ***********************************************************************************************************************
 */
void schedule_to_task_stack_check(os_task_t *task)
{
    os_size_t sp;
    OS_ASSERT(OS_NULL != task);

    sp = (os_size_t)task->sp;

#if defined(ARCH_CPU_STACK_GROWS_UPWARD)
    if ((*((os_uint8_t *)((os_ubase_t)task->stack_addr + task->stack_size - 1)) != '#') ||
#else
    if ((*((os_uint8_t *)task->stack_addr) != '#') ||
#endif
        ((os_uint32_t)sp < (os_uint32_t)task->stack_addr) ||
        ((os_uint32_t)sp >= (os_uint32_t)task->stack_addr + (os_uint32_t)task->stack_size))
    {
        os_kprintf("schedule to task:%s stack overflow,the sp is 0x%x.\r\n", task->parent.name, sp);
#ifdef OS_USING_SHELL
        {
            extern os_err_t sh_list_task(os_int32_t argc, char **argv);
            sh_list_task(0, OS_NULL);
        }
#endif
        (void)os_hw_interrupt_disable();
        OS_ASSERT(0);
    }
}


#endif /* OS_USING_OVERFLOW_CHECK */

BaseType_t xTaskIncrementTick( void )
{
    os_interrupt_enter();
    os_tick_increase();
    os_interrupt_leave();
    return 0;
}



BaseType_t xTaskCreatePinnedToCore(TaskFunction_t pxTaskCode, const char * const pcName, const uint32_t usStackDepth, void * const pvParameters, UBaseType_t uxPriority, TaskHandle_t * const pxCreatedTask, const BaseType_t xCoreID )
{

    return 0;
}

void vTaskDelete(xTaskHandle xTaskToDelete)
{

}

TaskHandle_t xTaskGetCurrentTaskHandleForCPU( BaseType_t cpuid )
{
    return NULL;
}

BaseType_t xTaskGetSchedulerState( void )
{

    return 0;
}

void vTaskEnterCritical( portMUX_TYPE *mux )
{

}

void vTaskExitCritical( portMUX_TYPE *mux )
{

}

void vTaskSuspendAll( void )
{

}

BaseType_t xTaskResumeAll( void )
{
    return 1;
}


void *pvTaskGetThreadLocalStoragePointer( TaskHandle_t xTaskToQuery, BaseType_t xIndex )
{

    return NULL;
}

void vTaskSetThreadLocalStoragePointerAndDelCallback( TaskHandle_t xTaskToSet, BaseType_t xIndex, void *pvValue , TlsDeleteCallbackFunction_t xDelCallback)
{

}

TickType_t xTaskGetTickCount( void )
{
    return 0;
}

void vTaskStartScheduler(void)
{

}

void vTaskDelay( const TickType_t xTicksToDelay ) { }
TaskHandle_t xTaskGetCurrentTaskHandle( void ) { return 0; }
BaseType_t xTaskGetAffinity( TaskHandle_t xTask ) { return OS_USING_CPUID; }
char *pcTaskGetTaskName( TaskHandle_t xTaskToQuery ) { return 0; }


QueueHandle_t xQueueGenericCreate( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, const uint8_t ucQueueType )
{

    return 0;
}

void vQueueDelete( QueueHandle_t xQueue )
{

}

BaseType_t xQueueGenericSend( QueueHandle_t xQueue, const void * const pvItemToQueue, TickType_t xTicksToWait, const BaseType_t xCopyPosition )
{
    return 0;
}

BaseType_t xQueueGenericSendFromISR( QueueHandle_t xQueue,
    const void * const pvItemToQueue,
    BaseType_t * const pxHigherPriorityTaskWoken,
    const BaseType_t xCopyPosition )
{

    return 0;
}

BaseType_t xQueueGenericReceive( QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait, const BaseType_t xJustPeeking )
{

    return 0;
}

BaseType_t xQueueReceiveFromISR( QueueHandle_t xQueue, void * const pvBuffer, BaseType_t * const pxHigherPriorityTaskWoken )
{

    return 0;
}

UBaseType_t uxQueueMessagesWaiting( const QueueHandle_t xQueue )
{

    return 0;
}

UBaseType_t uxQueueMessagesWaitingFromISR( const QueueHandle_t xQueue ) { return uxQueueMessagesWaiting(xQueue); }
QueueHandle_t xQueueCreateMutex( const uint8_t ucQueueType ) { return xQueueGenericCreate(1,0,ucQueueType); }
BaseType_t xQueueTakeMutexRecursive( QueueHandle_t xMutex, TickType_t xTicksToWait ) { return xQueueGenericReceive(xMutex,0,xTicksToWait,pdFALSE); }
BaseType_t xQueueGiveMutexRecursive( QueueHandle_t xMutex ) { return xQueueGenericSend(xMutex,0,0,queueSEND_TO_BACK); }

BaseType_t xQueueGiveFromISR( QueueHandle_t xQueue, BaseType_t * const pxHigherPriorityTaskWoken )
{
    return xQueueGenericSendFromISR(xQueue,0,pxHigherPriorityTaskWoken,queueSEND_TO_BACK);
}

void* xQueueGetMutexHolder( QueueHandle_t xSemaphore )
{
    return NULL;
}

BaseType_t xQueueGenericReset( QueueHandle_t xQueue, BaseType_t xNewQueue )
{
    return 0;
}

BaseType_t xQueueIsQueueFullFromISR( const QueueHandle_t xQueue )
{

    return 0;
}

UBaseType_t uxQueueSpacesAvailable( const QueueHandle_t xQueue )
{
    return 0;
}

TimerHandle_t xTimerCreate( const char * const pcTimerName, const TickType_t xTimerPeriodInTicks, const UBaseType_t uxAutoReload, void * const pvTimerID, TimerCallbackFunction_t pxCallbackFunction )
{
    return 0;
}

BaseType_t xTimerGenericCommand( TimerHandle_t xTimer, const BaseType_t xCommandID, const TickType_t xOptionalValue, BaseType_t * const pxHigherPriorityTaskWoken, const TickType_t xTicksToWait )
{

    return 0;
}

EventGroupHandle_t xEventGroupCreate( void )
{

    return 0;
}
void vEventGroupDelete( EventGroupHandle_t xEventGroup )
{

}
EventBits_t xEventGroupSetBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet )
{

    return 0;
}
EventBits_t xEventGroupClearBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear )
{

    return 0;
}
EventBits_t xEventGroupWaitBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToWaitFor, const BaseType_t xClearOnExit, const BaseType_t xWaitForAllBits, TickType_t xTicksToWait )
{

    return 0;
}


