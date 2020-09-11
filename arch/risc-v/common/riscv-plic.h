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
 * @file        riscv-plic.h
 *
 * @brief       This file provides some registers ops related to the RISC-V architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-05-18   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef RISCV_PLIC_H__
#define RISCV_PLIC_H__

#include "riscv-ops.h"

#ifndef PLIC_BASE_ADDR
#define PLIC_BASE_ADDR 0x0
#endif

/* Priority Register - 32 bits per source */
#define PLIC_PRIORITY_OFFSET (0x00000000UL)
#define PLIC_PRIORITY_SHIFT_PER_SOURCE 2

/* Pending Register - 1 bit per soirce */
#define PLIC_PENDING_OFFSET (0x00001000UL)
#define PLIC_PENDING_SHIFT_PER_SOURCE 0

/* Enable Register - 0x80 per target */
#define PLIC_ENABLE_OFFSET (0x00002000UL)
#define PLIC_ENABLE_SHIFT_PER_TARGET 7

/* Priority Threshold Register - 0x1000 per target */
#define PLIC_THRESHOLD_OFFSET (0x00200000UL)
#define PLIC_THRESHOLD_SHIFT_PER_TARGET 12

/* Claim Register - 0x1000 per target */
#define PLIC_CLAIM_OFFSET (0x00200004UL)
#define PLIC_CLAIM_SHIFT_PER_TARGET 12

#if defined(__GNUC__) && !defined(__ASSEMBLER__)
__attribute__((always_inline)) static inline void __plic_set_feature(unsigned int feature)
{
    volatile unsigned int *feature_ptr = (volatile unsigned int *)PLIC_BASE_ADDR;
    *feature_ptr = feature;
}

__attribute__((always_inline)) static inline void __plic_set_threshold(unsigned int threshold)
{
    unsigned int hart_id = read_csr(mhartid);
    volatile unsigned int *threshold_ptr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                     PLIC_THRESHOLD_OFFSET +
                                                                     (hart_id << PLIC_THRESHOLD_SHIFT_PER_TARGET));
    *threshold_ptr = threshold;
}

__attribute__((always_inline)) static inline void __plic_set_priority(unsigned int source, unsigned int priority)
{
    volatile unsigned int *priority_ptr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                    PLIC_PRIORITY_OFFSET +
                                                                    (source << PLIC_PRIORITY_SHIFT_PER_SOURCE));
    *priority_ptr = priority;
}

__attribute__((always_inline)) static inline void __plic_set_pending(unsigned int source)
{
    volatile unsigned int *current_ptr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                   PLIC_PENDING_OFFSET +
                                                                   ((source >> 5) << 2));
    *current_ptr = (1 << (source & 0x1F));
}

__attribute__((always_inline)) static inline void __plic_irq_enable(unsigned int source)
{
    unsigned int hart_id = read_csr(mhartid);
    volatile unsigned int *current_ptr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                   PLIC_ENABLE_OFFSET +
                                                                   (hart_id << PLIC_ENABLE_SHIFT_PER_TARGET) +
                                                                   ((source >> 5) << 2));
    unsigned int current = *current_ptr;
    current = current | (1 << (source & 0x1F));
    *current_ptr = current;
}

__attribute__((always_inline)) static inline void __plic_irq_disable(unsigned int source)
{
    unsigned int hart_id = read_csr(mhartid);
    volatile unsigned int *current_ptr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                   PLIC_ENABLE_OFFSET +
                                                                   (hart_id << PLIC_ENABLE_SHIFT_PER_TARGET) +
                                                                   ((source >> 5) << 2));
    unsigned int current = *current_ptr;
    current = current & ~((1 << (source & 0x1F)));
    *current_ptr = current;
}

__attribute__((always_inline)) static inline unsigned int __plic_irq_claim(void)
{
    unsigned int hart_id = read_csr(mhartid);
    volatile unsigned int *claim_addr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                  PLIC_CLAIM_OFFSET +
                                                                  (hart_id << PLIC_CLAIM_SHIFT_PER_TARGET));
    return *claim_addr;
}

__attribute__((always_inline)) static inline void __plic_irq_complete(unsigned int source)
{
    unsigned int hart_id = read_csr(mhartid);
    volatile unsigned int *claim_addr = (volatile unsigned int *)(PLIC_BASE_ADDR +
                                                                  PLIC_CLAIM_OFFSET +
                                                                  (hart_id << PLIC_CLAIM_SHIFT_PER_TARGET));
    *claim_addr = source;
}
#endif /* end of __GNUC__ */

#endif
