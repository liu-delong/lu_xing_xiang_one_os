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
 * @brief       This file provides functions related to the ARM MO architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-23   OneOS Team      First version.
 * 2020-07-31   OneOS Team      Add exception hook.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_dbg.h>
#include <os_task.h>

/* Flag in interrupt handling. */
os_uint32_t os_interrupt_from_task;
os_uint32_t os_interrupt_to_task;
os_uint32_t os_task_switch_interrupt_flag;

/* exception hook */
static os_err_t (*os_exception_hook)(void *context) = OS_NULL;

struct exception_stack_frame
{
    os_uint32_t r0;
    os_uint32_t r1;
    os_uint32_t r2;
    os_uint32_t r3;
    os_uint32_t r12;
    os_uint32_t lr;
    os_uint32_t pc;
    os_uint32_t psr;
};

struct stack_frame
{
    /* r4 ~ r11 register */
    os_uint32_t r4;
    os_uint32_t r5;
    os_uint32_t r6;
    os_uint32_t r7;
    os_uint32_t r8;
    os_uint32_t r9;
    os_uint32_t r10;
    os_uint32_t r11;

    struct exception_stack_frame exception_stack_frame;
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
os_uint8_t *os_hw_stack_init(void *task_entry, void *parameter, os_uint8_t *stack_addr, void *task_exit)
{
    os_ubase_t          i;
    os_uint8_t         *stk;
    struct stack_frame *stack_frame;

    stk  = stack_addr + sizeof(os_uint32_t);
    stk  = (os_uint8_t *)OS_ALIGN_DOWN((os_uint32_t)stk, 8);
    stk -= sizeof(struct stack_frame);

    stack_frame = (struct stack_frame *)stk;

    /* init all register */
    for (i = 0; i < sizeof(struct stack_frame) / sizeof(os_uint32_t); i++)
    {
        ((os_uint32_t *)stack_frame)[i] = 0xdeadbeef;
    }

    stack_frame->exception_stack_frame.r0  = (unsigned long)parameter; /* r0 : argument */
    stack_frame->exception_stack_frame.r1  = 0;                        /* r1 */
    stack_frame->exception_stack_frame.r2  = 0;                        /* r2 */
    stack_frame->exception_stack_frame.r3  = 0;                        /* r3 */
    stack_frame->exception_stack_frame.r12 = 0;                        /* r12 */
    stack_frame->exception_stack_frame.lr  = (unsigned long)task_exit; /* lr */
    stack_frame->exception_stack_frame.pc  = (unsigned long)task_entry;/* entry point, pc */
    stack_frame->exception_stack_frame.psr = 0x01000000L;              /* PSR */

    /* return task's current stack address */
    return stk;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will set a hook function, the hook will be called in os_hw_hard_fault_exception function
 *
 * @param[in]       hook             The hook function complemented by user.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void os_hw_exception_install(os_err_t (*exception_handle)(void *context))
{
    os_exception_hook = exception_handle;
}

/**
 ***********************************************************************************************************************
 * @brief           This function handles hard fault exception.
 *
 * @param[in]       contex          Context when exception occurs.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void os_hw_hard_fault_exception(struct exception_stack_frame *contex)
{
    if (OS_NULL != os_exception_hook)
    {
        os_err_t result;

        result = os_exception_hook(contex);
        if (OS_EOK == result) 
        {
            return;
        }
    }
    
    os_kprintf("psr: 0x%08x\r\n", contex->psr);
    os_kprintf(" pc: 0x%08x\r\n", contex->pc);
    os_kprintf(" lr: 0x%08x\r\n", contex->lr);
    os_kprintf("r12: 0x%08x\r\n", contex->r12);
    os_kprintf("r03: 0x%08x\r\n", contex->r3);
    os_kprintf("r02: 0x%08x\r\n", contex->r2);
    os_kprintf("r01: 0x%08x\r\n", contex->r1);
    os_kprintf("r00: 0x%08x\r\n", contex->r0);

    os_kprintf("hard fault on task: %s\r\n", ((os_task_t *)os_task_self())->parent.name);

#ifdef OS_USING_SHELL
    extern os_err_t sh_list_task(os_int32_t argc, char **argv);
    sh_list_task(0, OS_NULL);
#endif

    while (1);
}

#define SCB_CFSR        (*(volatile const unsigned *)0xE000ED28)   /* Configurable Fault Status Register */
#define SCB_HFSR        (*(volatile const unsigned *)0xE000ED2C)   /* HardFault Status Register */
#define SCB_MMAR        (*(volatile const unsigned *)0xE000ED34)   /* MemManage Fault Address register */
#define SCB_BFAR        (*(volatile const unsigned *)0xE000ED38)   /* Bus Fault Address Register */
#define SCB_AIRCR       (*(volatile unsigned long *)0xE000ED00)    /* Reset control Address Register */
#define SCB_RESET_VALUE 0x05FA0004                                 /* Reset value, write to SCB_AIRCR can reset cpu */

#define SCB_CFSR_MFSR   (*(volatile const unsigned char*)0xE000ED28)  /* Memory-management Fault Status Register */
#define SCB_CFSR_BFSR   (*(volatile const unsigned char*)0xE000ED29)  /* Bus Fault Status Register */
#define SCB_CFSR_UFSR   (*(volatile const unsigned short*)0xE000ED2A) /* Usage Fault Status Register */

/**
 ***********************************************************************************************************************
 * @brief           Cpu reset.
 *
 * @attention       This function is weak, and could be implemented in Bsp.
 *
 * @param           No parameter.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_WEAK void os_hw_cpu_reset(void)
{
    SCB_AIRCR  = SCB_RESET_VALUE;
}
