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
 * @file        cortexm_common.h
 *
 * @brief       This file provides register width define related to the RISC-V architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-26   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __CORTEXM_COMMON_H__
#define __CORTEXM_COMMON_H__

#include <oneos_config.h>
#include <os_types.h>
         
      
#define SCB_CFSR        (*(volatile const unsigned *)0xE000ED28)   /* Configurable Fault Status Register */
#define SCB_HFSR        (*(volatile const unsigned *)0xE000ED2C)   /* HardFault Status Register */
#define SCB_MMAR        (*(volatile const unsigned *)0xE000ED34)   /* MemManage Fault Address register */
#define SCB_BFAR        (*(volatile const unsigned *)0xE000ED38)   /* Bus Fault Address Register */
#define SCB_AIRCR       (*(volatile unsigned long *)0xE000ED0C)    /* Reset control Address Register */
#define SCB_RESET_VALUE 0x05FA0004                                 /* Reset value, write to SCB_AIRCR can reset cpu */
      
#define SCB_CFSR_MFSR   (*(volatile const unsigned char*)0xE000ED28)  /* Memory-management Fault Status Register */
#define SCB_CFSR_BFSR   (*(volatile const unsigned char*)0xE000ED29)  /* Bus Fault Status Register */
#define SCB_CFSR_UFSR   (*(volatile const unsigned short*)0xE000ED2A) /* Usage Fault Status Register */


#if defined(__CC_ARM)
    #ifndef CSTACK_BLOCK_NAME
    #define CSTACK_BLOCK_NAME          STACK
    #endif
      
    #ifndef CODE_SECTION_NAME
    #define CODE_SECTION_NAME          ER_IROM1
    #endif
      
#elif defined(__ICCARM__)
    #ifndef CSTACK_BLOCK_NAME
    #define CSTACK_BLOCK_NAME          "CSTACK"
    #endif
      
    #ifndef CODE_SECTION_NAME
    #define CODE_SECTION_NAME          ".text"
    #endif
      
#elif defined(__GNUC__)
    #ifndef CSTACK_BLOCK_START
    #define CSTACK_BLOCK_START         _sstack
    #endif
       
    #ifndef CSTACK_BLOCK_END
    #define CSTACK_BLOCK_END           _estack
    #endif
      
    #ifndef CODE_SECTION_START
    #define CODE_SECTION_START         _stext
    #endif
      
    #ifndef CODE_SECTION_END
    #define CODE_SECTION_END           _etext
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
      
    extern const int CSTACK_BLOCK_START(CSTACK_BLOCK_NAME);
    extern const int CSTACK_BLOCK_END(CSTACK_BLOCK_NAME);
    extern const int CODE_SECTION_START(CODE_SECTION_NAME);
    extern const int CODE_SECTION_END(CODE_SECTION_NAME);
      
#elif defined(__ICCARM__)
    #pragma section=CSTACK_BLOCK_NAME
    #pragma section=CODE_SECTION_NAME
          
#elif defined(__GNUC__)
          extern const int CSTACK_BLOCK_START;
          extern const int CSTACK_BLOCK_END;
          extern const int CODE_SECTION_START;
          extern const int CODE_SECTION_END;
#else
    #error "not supported compiler"
#endif


#if  ((defined ( __CC_ARM ) && defined ( __TARGET_FPU_VFP ))                         \
      || (defined ( __CLANG_ARM ) && defined ( __VFP_FP__ ) && !defined(__SOFTFP__)) \
      || (defined ( __ICCARM__ ) && defined ( __ARMVFP__ ))                          \
      || (defined ( __GNUC__ ) && defined ( __VFP_FP__ ) && !defined(__SOFTFP__)) )
#define USE_FPU   1
#else
#define USE_FPU   0
#endif

extern os_size_t g_code_start_addr;
extern os_size_t g_code_end_addr;
extern os_size_t g_main_stack_start_addr;
extern os_size_t g_main_stack_end_addr;

/* exception hook */
extern os_err_t (*os_exception_hook)(void *context);

#ifdef STACK_TRACE_EN
struct call_back_trace
{
    os_uint32_t depth;
    os_size_t back_trace[CALL_BACK_TRACE_MAX_DEPTH];
};

typedef struct call_back_trace call_back_trace_t;
#endif


extern void usage_fault_track(void);

extern void bus_fault_track(void);

extern void mem_manage_fault_track(void);

extern void hard_fault_track(void);

extern os_size_t *get_current_task_sp(void);

extern os_size_t *get_msp(void);

extern os_size_t *get_psp(void);

extern os_size_t *get_sp(void);

extern void set_psp(os_size_t psp);


#ifdef STACK_TRACE_EN
extern os_bool_t disassembly_ins_is_exc_return(os_size_t ins);

extern os_bool_t disassembly_ins_is_bl_blx(os_uint32_t addr);

extern void trace_stack(os_size_t *stack_top, os_size_t *stack_bottom, call_back_trace_t *trace);

extern void dump_stack(os_uint32_t stack_start_addr, os_uint32_t stack_size, os_size_t *stack_pointer);
#endif
#endif

