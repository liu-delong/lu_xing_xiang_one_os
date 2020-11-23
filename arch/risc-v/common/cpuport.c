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

/* Flag in interrupt handling. */
volatile os_ubase_t  os_interrupt_from_task = 0;
volatile os_ubase_t  os_interrupt_to_task   = 0;
volatile os_uint32_t os_task_switch_interrupt_flag = 0;

struct os_hw_stack_frame
{
    os_ubase_t epc;        /* epc - epc    - program counter                     */
    os_ubase_t ra;         /* x1  - ra     - return address for jumps            */
    os_ubase_t mstatus;    /*              - machine status register             */
    os_ubase_t gp;         /* x3  - gp     - global pointer                      */
    os_ubase_t tp;         /* x4  - tp     - task pointer                        */
    os_ubase_t t0;         /* x5  - t0     - temporary register 0                */
    os_ubase_t t1;         /* x6  - t1     - temporary register 1                */
    os_ubase_t t2;         /* x7  - t2     - temporary register 2                */
    os_ubase_t s0_fp;      /* x8  - s0/fp  - saved register 0 or frame pointer   */
    os_ubase_t s1;         /* x9  - s1     - saved register 1                    */
    os_ubase_t a0;         /* x10 - a0     - return value or function argument 0 */
    os_ubase_t a1;         /* x11 - a1     - return value or function argument 1 */
    os_ubase_t a2;         /* x12 - a2     - function argument 2                 */
    os_ubase_t a3;         /* x13 - a3     - function argument 3                 */
    os_ubase_t a4;         /* x14 - a4     - function argument 4                 */
    os_ubase_t a5;         /* x15 - a5     - function argument 5                 */
    os_ubase_t a6;         /* x16 - a6     - function argument 6                 */
    os_ubase_t a7;         /* x17 - s7     - function argument 7                 */
    os_ubase_t s2;         /* x18 - s2     - saved register 2                    */
    os_ubase_t s3;         /* x19 - s3     - saved register 3                    */
    os_ubase_t s4;         /* x20 - s4     - saved register 4                    */
    os_ubase_t s5;         /* x21 - s5     - saved register 5                    */
    os_ubase_t s6;         /* x22 - s6     - saved register 6                    */
    os_ubase_t s7;         /* x23 - s7     - saved register 7                    */
    os_ubase_t s8;         /* x24 - s8     - saved register 8                    */
    os_ubase_t s9;         /* x25 - s9     - saved register 9                    */
    os_ubase_t s10;        /* x26 - s10    - saved register 10                   */
    os_ubase_t s11;        /* x27 - s11    - saved register 11                   */
    os_ubase_t t3;         /* x28 - t3     - temporary register 3                */
    os_ubase_t t4;         /* x29 - t4     - temporary register 4                */
    os_ubase_t t5;         /* x30 - t5     - temporary register 5                */
    os_ubase_t t6;         /* x31 - t6     - temporary register 6                */
};

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
    os_int32_t                i;
    os_uint8_t               *stk;
    struct os_hw_stack_frame *frame;

    stk  = stack_addr + sizeof(os_ubase_t);
    stk  = (os_uint8_t *)OS_ALIGN_DOWN((os_ubase_t)stk, REGBYTES);
    stk -= sizeof(struct os_hw_stack_frame);

    frame = (struct os_hw_stack_frame *)stk;

    for (i = 0; i < sizeof(struct os_hw_stack_frame) / sizeof(os_ubase_t); i++)
    {
        ((os_ubase_t *)frame)[i] = 0xdeadbeef;
    }

    frame->ra      = (os_ubase_t)texit;
    frame->a0      = (os_ubase_t)parameter;
    frame->epc     = (os_ubase_t)tentry;

    /* force to machine mode(MPP=11) and set MPIE to 1 */
    frame->mstatus = 0x00007880;

    return stk;
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
    if (os_task_switch_interrupt_flag == 0)
    {
        os_interrupt_from_task = from;
    }

    os_interrupt_to_task = to;
    os_task_switch_interrupt_flag = 1;
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
 * @brief           This function will return stack pointer of the current running task.
 *
 * @param           No parameter.
 *
 * @return          Return stack pointer of the current running task.
 ***********************************************************************************************************************
 */
os_size_t *get_current_task_sp(void)
{
	extern os_size_t *get_sp(void);
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

