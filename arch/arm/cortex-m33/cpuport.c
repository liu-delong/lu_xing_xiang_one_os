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
 * @brief       This file provides functions related to the ARM M33 architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-23   OneOS Team      First version.
 * 2020-08-28   OneOS Team      Add stack back trace when hard falut occur.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_task.h>
#include <cortexm_common.h>

#include <cpuport.h>


/* Flag in interrupt handling. */
os_uint32_t os_interrupt_from_task;
os_uint32_t os_interrupt_to_task;
os_uint32_t os_task_switch_interrupt_flag;

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

#if USE_FPU
    stack_frame->flag = 0;
#endif /* USE_FPU */

    return stk;
}

/**
 ***********************************************************************************************************************
 * @brief           Cpu shutdown.
 *
 * @param           No parameter.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void os_hw_cpu_shutdown(void)
{
    os_kprintf("shutdown...\r\n");

    OS_ASSERT(0);
}

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
    SCB_AIRCR = SCB_RESET_VALUE;
}

#ifdef OS_USING_CPU_FFS
/**
 ***********************************************************************************************************************
 * @brief           This function finds the first bit set in value and return the index of that bit.
 *
 * @attention       Bits are numbered starting at 1 (the least significant bit).A return value of zero from any of these 
 *                  functions means that the argument was zero.
 *
 * @param[in]       value          The value need to find the first bit.
 *
 * @return          The index of the first bit set. If value is 0, then this function shall return 0.
 ***********************************************************************************************************************
 */
#if defined(__CC_ARM) || defined(__CLANG_ARM)
__asm os_int32_t os_find_first_bit_set(os_int32_t value)
{
    CMP     r0, #0x00
    BEQ     exit

    RBIT    r0, r0
    CLZ     r0, r0
    ADDS    r0, r0, #0x01

exit
    BX      lr
}
#elif defined(__IAR_SYSTEMS_ICC__)
os_int32_t os_find_first_bit_set(os_int32_t value)
{
    if(value == 0)
    {
        return value;
    }
    
    asm("RBIT %0, %1" : "=r"(value) : "r"(value));
    asm("CLZ  %0, %1" : "=r"(value) : "r"(value));
    asm("ADDS %0, %1, #0x01" : "=r"(value) : "r"(value));

    return value;
}
#elif defined(__GNUC__)
os_int32_t os_find_first_bit_set(os_int32_t value)
{
    return __builtin_ffs(value);
}
#endif

#endif /* OS_USING_CPU_FFS */

