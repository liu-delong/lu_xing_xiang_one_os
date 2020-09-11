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
 * @file        context_gcc.S
 *
 * @brief       This file provides context switch functions related to the RISC-V architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-05-18   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "cpuport.h"

/**
 ***********************************************************************************************************************
 * @brief           Disable interrupt.
 *
 * @param           None.
 *
 * @return          The state before disable interrupt.
 ***********************************************************************************************************************
 */
.globl os_hw_interrupt_disable
os_hw_interrupt_disable:
    csrrci a0, mstatus, 8
    ret

/**
 ***********************************************************************************************************************
 * @brief           Enable interrupt.
 *
 * @param[in]       State to restore.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
.globl os_hw_interrupt_enable
os_hw_interrupt_enable:
    csrw mstatus, a0
    ret

/**
 ***********************************************************************************************************************
 * @brief           This function is called when the scheduler starts the first task, Only used once.
 *
 * @param[in]       to              SP of 'to' task.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
.globl os_hw_context_switch_to
os_hw_context_switch_to:
    LOAD sp, (a0)
    LOAD a0,   2 * REGBYTES(sp)
    csrw mstatus, a0
    j    os_hw_context_switch_exit

 /**
 ***********************************************************************************************************************
 * @brief           Start the task swtich process.
 *
 * @param[in]       from            SP of 'from' task. from-->a0.
 * @param[in]       to              SP of 'to' task.   to  -->a1.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
.globl os_hw_context_switch
os_hw_context_switch:
    /* Saved from task context
     *     x1/ra       -> sp(0)
     *     x1/ra       -> sp(1)
     *     mstatus.mie -> sp(2)
     *     x(i)        -> sp(i-4)
     */
    addi  sp,  sp, -32 * REGBYTES
    STORE sp,  (a0)

    STORE x1,   0 * REGBYTES(sp)
    STORE x1,   1 * REGBYTES(sp)

    csrr a0, mstatus
    andi a0, a0, 8
    beqz a0, save_mpie
    li   a0, 0x80
save_mpie:
    STORE a0,   2 * REGBYTES(sp)

    STORE x4,   4 * REGBYTES(sp)
    STORE x5,   5 * REGBYTES(sp)
    STORE x6,   6 * REGBYTES(sp)
    STORE x7,   7 * REGBYTES(sp)
    STORE x8,   8 * REGBYTES(sp)
    STORE x9,   9 * REGBYTES(sp)
    STORE x10, 10 * REGBYTES(sp)
    STORE x11, 11 * REGBYTES(sp)
    STORE x12, 12 * REGBYTES(sp)
    STORE x13, 13 * REGBYTES(sp)
    STORE x14, 14 * REGBYTES(sp)
    STORE x15, 15 * REGBYTES(sp)
    STORE x16, 16 * REGBYTES(sp)
    STORE x17, 17 * REGBYTES(sp)
    STORE x18, 18 * REGBYTES(sp)
    STORE x19, 19 * REGBYTES(sp)
    STORE x20, 20 * REGBYTES(sp)
    STORE x21, 21 * REGBYTES(sp)
    STORE x22, 22 * REGBYTES(sp)
    STORE x23, 23 * REGBYTES(sp)
    STORE x24, 24 * REGBYTES(sp)
    STORE x25, 25 * REGBYTES(sp)
    STORE x26, 26 * REGBYTES(sp)
    STORE x27, 27 * REGBYTES(sp)
    STORE x28, 28 * REGBYTES(sp)
    STORE x29, 29 * REGBYTES(sp)
    STORE x30, 30 * REGBYTES(sp)
    STORE x31, 31 * REGBYTES(sp)

    /* Restore to task context
     * sp(0) -> epc;
     * sp(1) -> ra;
     * sp(i) -> x(i+2)
     */
    LOAD sp,  (a1)

    j os_hw_context_switch_exit


.global os_hw_context_switch_exit
os_hw_context_switch_exit:
    /* resw ra to mepc */
    LOAD a0,   0 * REGBYTES(sp)
    csrw mepc, a0

    LOAD x1,   1 * REGBYTES(sp)

    li    t0, 0x00001800
    csrs  mstatus, t0
    LOAD a0,   2 * REGBYTES(sp)
    csrs mstatus, a0

    LOAD x4,   4 * REGBYTES(sp)
    LOAD x5,   5 * REGBYTES(sp)
    LOAD x6,   6 * REGBYTES(sp)
    LOAD x7,   7 * REGBYTES(sp)
    LOAD x8,   8 * REGBYTES(sp)
    LOAD x9,   9 * REGBYTES(sp)
    LOAD x10, 10 * REGBYTES(sp)
    LOAD x11, 11 * REGBYTES(sp)
    LOAD x12, 12 * REGBYTES(sp)
    LOAD x13, 13 * REGBYTES(sp)
    LOAD x14, 14 * REGBYTES(sp)
    LOAD x15, 15 * REGBYTES(sp)
    LOAD x16, 16 * REGBYTES(sp)
    LOAD x17, 17 * REGBYTES(sp)
    LOAD x18, 18 * REGBYTES(sp)
    LOAD x19, 19 * REGBYTES(sp)
    LOAD x20, 20 * REGBYTES(sp)
    LOAD x21, 21 * REGBYTES(sp)
    LOAD x22, 22 * REGBYTES(sp)
    LOAD x23, 23 * REGBYTES(sp)
    LOAD x24, 24 * REGBYTES(sp)
    LOAD x25, 25 * REGBYTES(sp)
    LOAD x26, 26 * REGBYTES(sp)
    LOAD x27, 27 * REGBYTES(sp)
    LOAD x28, 28 * REGBYTES(sp)
    LOAD x29, 29 * REGBYTES(sp)
    LOAD x30, 30 * REGBYTES(sp)
    LOAD x31, 31 * REGBYTES(sp)

    addi sp,  sp, 32 * REGBYTES
    mret
