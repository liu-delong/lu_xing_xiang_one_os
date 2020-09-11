/**
***********************************************************************************************************************
* Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
* COPYRIGHT (C) 2016 - 2019,RT-Thread Development Team
* Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
* an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
* specific language governing permissions and limitations under the License.
*
* @file        cmdump.h
*
* @brief       Header file for cortex-M backtrace interface.
*
* @revision
* Date         Author          Notes
* 2016-12-15   Armink          the first version
* 2020-08-18   OneOS Team      adapt the code to OneOS and delelte other OS source
***********************************************************************************************************************
*/

#ifndef _CMPDUMP_H_
#define _CMPDUMP_H_

#include <oneos_config.h>
#include <os_kernel.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __STDC_VERSION__ < 199901L
	#error "must be C99 or higher. try to add '-std=c99' to compile parameters"
#endif

#if defined(__CC_ARM)
	#pragma O1
#elif defined(__ICCARM__)
	#pragma optimize=none
#elif defined(__GNUC__)
	#pragma GCC optimize ("O0")
#endif

#define CPU_ARM_CORTEX_M0          0
#define CPU_ARM_CORTEX_M3          1
#define CPU_ARM_CORTEX_M4          2
#define CPU_ARM_CORTEX_M7          3

#define CMB_CALL_STACK_MAX_DEPTH       16

/* include or export for supported function */
#if defined(__CC_ARM)
    static __inline __asm os_uint32_t cmb_get_msp(void) 
    {
        mrs r0, msp
        bx lr
    }
    static __inline __asm os_uint32_t cmb_get_psp(void) 
    {
        mrs r0, psp
        bx lr
    }
    static __inline __asm os_uint32_t cmb_get_sp(void) 
    {
        mov r0, sp
        bx lr
    }
    static __inline __asm void cmb_set_psp(os_uint32_t psp) 
    {
        msr psp, r0
        bx lr
    }
#elif defined(__ICCARM__)
#pragma diag_suppress=Pe940    
    static os_uint32_t cmb_get_msp(void)
    {
        __asm("mrs r0, msp");
        __asm("bx lr");        
    }
    static os_uint32_t cmb_get_psp(void)
    {
        __asm("mrs r0, psp");
        __asm("bx lr");        
    }
    static os_uint32_t cmb_get_sp(void)
    {
        __asm("mov r0, sp");
        __asm("bx lr");       
    }
    static void cmb_set_psp(os_uint32_t psp)
    {
        __asm("msr psp, r0");
        __asm("bx lr");
    }
#pragma diag_default=Pe940  
#elif defined(__GNUC__)
    __attribute__( ( always_inline ) ) static inline os_uint32_t cmb_get_msp(void) 
    {
        register os_uint32_t result;
        __asm volatile ("MRS %0, msp\n" : "=r" (result) );
        return(result);
    }
    __attribute__( ( always_inline ) ) static inline os_uint32_t cmb_get_psp(void) 
    {
        register os_uint32_t result;
        __asm volatile ("MRS %0, psp\n" : "=r" (result) );
        return(result);
    }
    __attribute__( ( always_inline ) ) static inline os_uint32_t cmb_get_sp(void) 
    {
        register os_uint32_t result;
        __asm volatile ("MOV %0, sp\n" : "=r" (result) );
        return(result);
    }
    __attribute__( ( always_inline ) ) static inline void cmb_set_psp(os_uint32_t psp) 
    {
        __asm volatile ("MSR psp, %0\n\t" :: "r" (psp) );
    }
#endif

#if defined(__CC_ARM)
    #ifndef CMB_CSTACK_BLOCK_NAME
    #define CMB_CSTACK_BLOCK_NAME          STACK
    #endif

    #ifndef CMB_CODE_SECTION_NAME
    #define CMB_CODE_SECTION_NAME          ER_IROM1
    #endif

#elif defined(__ICCARM__)
    #ifndef CMB_CSTACK_BLOCK_NAME
    #define CMB_CSTACK_BLOCK_NAME          "CSTACK"
    #endif

    #ifndef CMB_CODE_SECTION_NAME
    #define CMB_CODE_SECTION_NAME          ".text"
    #endif

#elif defined(__GNUC__)
    #ifndef CMB_CSTACK_BLOCK_START
    #define CMB_CSTACK_BLOCK_START         _sstack
    #endif
 
    #ifndef CMB_CSTACK_BLOCK_END
    #define CMB_CSTACK_BLOCK_END           _estack
    #endif

    #ifndef CMB_CODE_SECTION_START
    #define CMB_CODE_SECTION_START         _stext
    #endif

    #ifndef CMB_CODE_SECTION_END
    #define CMB_CODE_SECTION_END           _etext
    #endif
#else
    #error "not supported compiler"
#endif

#if defined(__CC_ARM)
    #define SECTION_START(_name_)                _name_##$$Base
    #define SECTION_END(_name_)                  _name_##$$Limit
    #define IMAGE_SECTION_START(_name_)          Image$$##_name_##$$Base
    #define IMAGE_SECTION_END(_name_)            Image$$##_name_##$$Limit
    #define CSTACK_BLOCK_START(_name_)           SECTION_START(_name_)
    #define CSTACK_BLOCK_END(_name_)             SECTION_END(_name_)
    #define CODE_SECTION_START(_name_)           IMAGE_SECTION_START(_name_)
    #define CODE_SECTION_END(_name_)             IMAGE_SECTION_END(_name_)

    extern const int CSTACK_BLOCK_START(CMB_CSTACK_BLOCK_NAME);
    extern const int CSTACK_BLOCK_END(CMB_CSTACK_BLOCK_NAME);
    extern const int CODE_SECTION_START(CMB_CODE_SECTION_NAME);
    extern const int CODE_SECTION_END(CMB_CODE_SECTION_NAME);

#elif defined(__ICCARM__)
    #pragma section=CMB_CSTACK_BLOCK_NAME
    #pragma section=CMB_CODE_SECTION_NAME
	
#elif defined(__GNUC__)
    extern const int CMB_CSTACK_BLOCK_START;
    extern const int CMB_CSTACK_BLOCK_END;
    extern const int CMB_CODE_SECTION_START;
    extern const int CMB_CODE_SECTION_END;
#else
    #error "not supported compiler"
#endif

/* Cortex-M fault registers */
struct cmb_hard_fault_regs{
  struct {
    unsigned int r0;                     // R0
    unsigned int r1;                     // R1
    unsigned int r2;                     // R2
    unsigned int r3;                     // R3
    unsigned int r12;                    // R12
    unsigned int lr;                     // Link register
    unsigned int pc;                     // Program counter
    union {
      unsigned int value;
      struct {
        unsigned int IPSR : 8;           // Interrupt Program Status register
        unsigned int EPSR : 19;          // Execution Program Status register
        unsigned int APSR : 5;           // Application Program Status register
      } bits;
    } psr;                               // Program status register.
  } saved;
};

/* cpu platform type, must config by user */
#if defined(ARCH_ARM_CORTEX_M0)
    #define CPU_PLATFORM_TYPE      CPU_ARM_CORTEX_M0
#elif defined(ARCH_ARM_CORTEX_M3)
    #define CPU_PLATFORM_TYPE      CPU_ARM_CORTEX_M3
#elif defined(ARCH_ARM_CORTEX_M4)
    #define CPU_PLATFORM_TYPE      CPU_ARM_CORTEX_M4
#elif defined(ARCH_ARM_CORTEX_M7)
    #define CPU_PLATFORM_TYPE      CPU_ARM_CORTEX_M7
#endif 


#if defined(CM_DEBUG_BACKTRACE_DUMP_STACK)
    #define CM_USING_DUMP_STACK_INFO
#endif

extern void debug_backtrace_init(void);
extern os_size_t debug_backtrace_call_stack(os_uint32_t *buffer, os_size_t size, os_uint32_t sp);
extern void debug_backtrace_assert(os_uint32_t sp);
extern void debug_backtrace_fault(os_uint32_t fault_handler_lr, os_uint32_t fault_handler_sp);
extern OS_WEAK os_err_t exception_hook(void *context);
extern OS_WEAK void assert_hook(const char* ex, const char* func, os_int32_t line);

#ifdef __cplusplus
}
#endif

#endif /* _CMPDUMP_H_ */
