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
 * @file        riscv-ops.h
 *
 * @brief       This file provides csr ops related to the RISC-V architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-05-18   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef RISCV_OPS_H__
#define RISCV_OPS_H__

#if defined(__GNUC__) && !defined(__ASSEMBLER__)

#define read_csr(reg) ({ unsigned long __tmp;                               \
    asm volatile ("csrr %0, " #reg : "=r"(__tmp));                          \
        __tmp; })

#define write_csr(reg, val) ({                                              \
    if (__builtin_constant_p(val) && (unsigned long)(val) < 32)             \
        asm volatile ("csrw " #reg ", %0" :: "i"(val));                     \
    else                                                                    \
        asm volatile ("csrw " #reg ", %0" :: "r"(val)); })

#define set_csr(reg, bit) ({ unsigned long __tmp;                           \
    if (__builtin_constant_p(bit) && (unsigned long)(bit) < 32)             \
        asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "i"(bit));   \
    else                                                                    \
        asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit));   \
            __tmp; })

#define clear_csr(reg, bit) ({ unsigned long __tmp;                         \
    if (__builtin_constant_p(bit) && (unsigned long)(bit) < 32)             \
        asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "i"(bit));   \
    else                                                                    \
        asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit));   \
            __tmp; })
#endif /* end of __GNUC__ */

#endif
