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
 * @file        mips_context_gcc.S
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ASSEMBLY__
#define __ASSEMBLY__
#endif

#include "../common/mips.h"

.global	os_thread_switch_interrupt_flag
.global	os_interrupt_from_thread
.global	os_interrupt_to_thread

.section .text,"ax",@progbits
.set noreorder
.set noat

.globl os_hw_interrupt_disable
os_hw_interrupt_disable:
mfc0 	v0,CP0_STATUS
srl 	v1,v0,1
sll 	v1,v1,1
#	and 	v1,v0,0xfffffffe
mtc0 	v1,CP0_STATUS
jr 		ra
nop

LEAF(os_hw_interrupt_enable)
    mtc0 	a0,CP0_STATUS
    jr 		ra
    nop
END(os_hw_interrupt_enable)

    /*
     * void os_hw_context_switch_to(os_uint32 to)/*
     * a0 --> to
     */
    LEAF(os_hw_context_switch_to)
lw 	sp ,	0(a0)                 	/* switch to the new stack */
    RESTORE_CONTEXT
END(os_hw_context_switch_to)

    /*
     * void os_hw_context_switch(os_uint32 from, os_uint32 to)
     * a0 --> from
     * a1 --> to
     */
LEAF(os_hw_context_switch)
    mtc0    ra, CP0_EPC
    SAVE_CONTEXT

    sw      sp, 0(a0)       /* store sp in preempted tasks TCB */
lw      sp, 0(a1)       /* get new task stack pointer */

    RESTORE_CONTEXT
END(os_hw_context_switch)

LEAF(os_hw_context_switch_interrupt)
    la      t0, os_thread_switch_interrupt_flag
lw      t1, 0(t0)
    nop
    bnez    t1, _reswitch
    nop
    li      t1, 0x01                       /* set os_thread_switch_interrupt_flag to 1 */
sw      t1, 0(t0)
    la      t0, os_interrupt_from_thread   /* set os_interrupt_from_thread */
sw      a0, 0(t0)
    _reswitch:
    la      t0, os_interrupt_to_thread     /* set os_interrupt_to_thread */
sw      a1, 0(t0)
    jr      ra
    nop
END(os_hw_context_switch_interrupt)

