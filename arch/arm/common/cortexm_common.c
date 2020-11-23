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
 * @file        cortexm_common.c
 *
 * @brief       This file provides functions related to the ARM M4 architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-23   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_types.h>
#include <os_errno.h>
#include <os_util.h>
#include <os_task.h>
#include <exc.h>

#include <cortexm_common.h>

os_size_t g_code_start_addr = 0;
os_size_t g_code_end_addr = 0;
os_size_t g_main_stack_start_addr = 0;
os_size_t g_main_stack_end_addr = 0;

/* exception hook */
os_err_t (*os_exception_hook)(void *context) = OS_NULL;


/**
 ***********************************************************************************************************************
 * @brief           This function will set a hook function, the hook will be called in os_hw_hard_fault_exception function
 *
 * @param[in]       hook             The hook function complemented by user.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hw_exception_install(os_err_t (*exception_handle)(void *context))
{
    os_exception_hook = exception_handle;
}

/**
 ***********************************************************************************************************************
 * @brief           This function analyzes the causes of usage fault. 
 *
 *
 * @param           No parameter.
 *
 * @return          No return value.
 ***********************************************************************************************************************
*/
void usage_fault_track(void)
{
    os_kprintf("usage fault:\r\n");
    os_kprintf("SCB_CFSR_UFSR:0x%02X ", SCB_CFSR_UFSR);

    if (SCB_CFSR_UFSR & (1 << 0))
    {
        os_kprintf("UNDEFINSTR ");
    }

    if (SCB_CFSR_UFSR & (1 << 1))
    {
        os_kprintf("INVSTATE ");
    }

    if (SCB_CFSR_UFSR & (1 << 2))
    {
        os_kprintf("INVPC ");
    }

    if (SCB_CFSR_UFSR & (1 << 3))
    {
        os_kprintf("NOCP ");
    }

    if (SCB_CFSR_UFSR & (1 << 8))
    {
        os_kprintf("UNALIGNED ");
    }

    if (SCB_CFSR_UFSR & (1 << 9))
    {
        os_kprintf("DIVBYZERO ");
    }

    os_kprintf("\r\n");
}

/**
 ***********************************************************************************************************************
 * @brief           This function analyzes the causes of bus fault. 
 *
 *
 * @param           No parameter.
 *
 * @return          No return value.
 ***********************************************************************************************************************
*/
void bus_fault_track(void)
{
    os_kprintf("bus fault:\r\n");
    os_kprintf("SCB_CFSR_BFSR:0x%02X ", SCB_CFSR_BFSR);

    if (SCB_CFSR_BFSR & (1 << 0))
    {
        os_kprintf("IBUSERR ");
    }

    if (SCB_CFSR_BFSR & (1 << 1))
    {
        os_kprintf("PRECISERR ");
    }

    if (SCB_CFSR_BFSR & (1 << 2))
    {
        os_kprintf("IMPRECISERR ");
    }

    if (SCB_CFSR_BFSR & (1 << 3))
    {
        os_kprintf("UNSTKERR ");
    }

    if (SCB_CFSR_BFSR & (1 << 4))
    {
        os_kprintf("STKERR ");
    }

    if (SCB_CFSR_BFSR & (1 << 7))
    {
        os_kprintf("SCB->BFAR:%08X\r\n", SCB_BFAR);
    }
    else
    {
        os_kprintf("\r\n");
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function analyzes the causes of memory manage fault. 
 *
 *
 * @param           No parameter.
 *
 * @return          No return value.
 ***********************************************************************************************************************
*/
void mem_manage_fault_track(void)
{
    os_kprintf("mem manage fault:\r\n");
    os_kprintf("SCB_CFSR_MFSR:0x%02X ", SCB_CFSR_MFSR);

    if (SCB_CFSR_MFSR & (1 << 0))
    {
        os_kprintf("IACCVIOL ");
    }

    if (SCB_CFSR_MFSR & (1 << 1))
    {
        os_kprintf("DACCVIOL ");
    }

    if (SCB_CFSR_MFSR & (1 << 3))
    {
        os_kprintf("MUNSTKERR ");
    }

    if (SCB_CFSR_MFSR & (1 << 4))
    {
        os_kprintf("MSTKERR ");
    }

    if (SCB_CFSR_MFSR & (1 << 7))
    {
        os_kprintf("SCB->MMAR:%08X\r\n", SCB_MMAR);
    }
    else
    {
        os_kprintf("\r\n");
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function analyzes the causes of hard fault.
 *
 * @param           No parameter.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void hard_fault_track(void)
{
    if (SCB_HFSR & (1UL << 1))
    {
        os_kprintf("failed vector fetch\r\n");
    }

    if (SCB_HFSR & (1UL << 30))
    {
        if (SCB_CFSR_BFSR)
        {
            bus_fault_track();
        }

        if (SCB_CFSR_MFSR)
        { 
            mem_manage_fault_track();
        }

        if (SCB_CFSR_UFSR)
        {
            usage_fault_track();
        }
    }

    if(SCB_HFSR & (1UL << 31))
    {
        os_kprintf("debug event\r\n");
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
    return get_psp();
}

#ifdef STACK_TRACE_EN

#define BL_INS_MASK         0xF800
#define BL_INS_HIGH         0xF800
#define BL_INS_LOW          0xF000
#define BLX_INX_MASK        0xFF00
#define BLX_INX             0x4700

/**
 ***********************************************************************************************************************
 * @brief           This function check the disassembly instruction is exception return.
 *
 * @param[in]       ins    Instruction.
 *
 * @return          On success, return OS_TRUE; on error, return OS_FALSE.
 ***********************************************************************************************************************
 */
os_bool_t disassembly_ins_is_exc_return(os_size_t ins) 
{
     if ((ins == 0xFFFFFFF1) 
         || (ins == 0xFFFFFFF9)
         || (ins == 0xFFFFFFFD)
#if (USE_FPU == 1) 
         || (ins == 0xFFFFFFE1)
         || (ins == 0xFFFFFFE9)
         || (ins == 0xFFFFFFED)
#endif
     )
        {
            return OS_TRUE;
        }

    return OS_FALSE;
}

/**
 ***********************************************************************************************************************
 * @brief           This function check the disassembly instruction is 'BL' or 'BLX'.
 *
 * @param[in]       addr    Address of instruction
 *
 * @return          On success, return OS_TRUE; on error, return OS_FALSE.
 ***********************************************************************************************************************
 */
os_bool_t disassembly_ins_is_bl_blx(os_uint32_t addr) 
{
    os_uint16_t ins1 = *((os_uint16_t *)addr);
    os_uint16_t ins2 = *((os_uint16_t *)(addr + 2));

    /* instruction is 'BL' */
    if ((ins2 & BL_INS_MASK) == BL_INS_HIGH && (ins1 & BL_INS_MASK) == BL_INS_LOW) 
    {
        return OS_TRUE;
    }
    /* instruction is 'BLX' */
    else if ((ins2 & BLX_INX_MASK) == BLX_INX) 
    {
        return OS_TRUE;
    } 
    else 
    {
        return OS_FALSE;
    }
}


/**
 ***********************************************************************************************************************
 * @brief           The function will find and record all function calls in the stack.
 *
 * @attention       
 *
 * @param[in]       stack_top       The top of stack.
 * @param[in]       stack_bottom    The bottom of stack.
 * @param[out]      trace           a memory area is used to record function calls
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void trace_stack(os_size_t *stack_top, os_size_t *stack_bottom, call_back_trace_t *trace)
{
    os_uint32_t pc;
    os_size_t *sp;
    
    pc = 0;
    for (sp = stack_top; sp < stack_bottom; sp ++) 
    {
        /* the *sp value may be LR, so need decrease a word to PC */
        pc = *((os_uint32_t *) sp) - sizeof(os_size_t);
        
        /* the Cortex-M using thumb instruction, so the pc must be an odd number */
        if (pc % 2 == 0) 
        {
            continue;
        }

        /* fix the PC address in thumb mode */
        pc = *((os_uint32_t *) sp) - 1;
        if (trace->depth < CALL_BACK_TRACE_MAX_DEPTH)
        {
            if ((pc >= g_code_start_addr)
                && (pc < g_code_end_addr)
                && disassembly_ins_is_bl_blx(pc - sizeof(os_size_t)))
            {

                /* ignore repeat */
                if ((2 == trace->depth)
                    && (pc == trace->back_trace[1]))
                {
                    continue;
                }
            
                trace->back_trace[trace->depth++] = pc;
            }
        }
        else
        {
            break;
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           The function will dump all information in task stack.
 *
 * @attention       
 *
 * @param[in]       stack_start_addr    Address of start of stack
 * @param[in]       stack_size          Stack size in bytes
 * @param[in]       stack_point         The stack pointer.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void dump_stack(os_uint32_t stack_start_addr, os_uint32_t stack_size, os_size_t *stack_pointer) 
{    
    os_size_t *sp;
    
    os_kprintf("=======================   Stack info  =====================\r\n");
    for (sp = stack_pointer; (os_uint32_t) sp < stack_start_addr + stack_size; sp++) 
    {
        os_kprintf("        addr: 0x%08x      data: %08x\r\n", sp, *sp);
    }
    
    os_kprintf("-------------------------------------------------------------\r\n");    
}


/**
 ***********************************************************************************************************************
 * @brief           This function implement a assert hook function.
 *
 * @param[in]       ex_string       The assertion condition string.
 * @param[in]       func            The function name when assertion.
 * @param[in]       line            The file line number when assertion.
 *
 * @return          None
 ***********************************************************************************************************************
 */

OS_WEAK void assert_hook(const char* ex, const char* func, os_int32_t line) 
{
    os_enter_critical();

    os_kprintf("\r\n!!!!!!!!!!!!!!!!!!!!!!!!    Assert   !!!!!!!!!!!!!!!!!!!!!!!\r\n");
    os_kprintf("(%s) assertion failed at function: %s, line number: %d \r\n", ex, func, line);

#ifdef OS_USING_SHELL
    os_kprintf("=======================   Heap Info   =======================\r\n");
    extern os_err_t sh_list_mem(os_int32_t argc, char **argv);
    sh_list_mem(0, OS_NULL);
    os_kprintf("-------------------------------------------------------------\r\n");

    os_kprintf("========================   Task Info  =======================\r\n");
    extern os_err_t sh_list_task(os_int32_t argc, char **argv);
    sh_list_task(0, OS_NULL);
    os_kprintf("-------------------------------------------------------------\r\n");
#endif /* OS_USING_SHELL */ 

    task_stack_show(os_task_self()->parent.name);

    os_kprintf("\r\n!!!!!!!!!!!!!!!!!!!!!!   Assert End   !!!!!!!!!!!!!!!!!!!!!!!\r\n");

    while (1);

}

/**
***********************************************************************************************************************
* @brief           Initialize address range of code segment and main stack.
*
* @param           None
*
* @return          Will only return OS_EOK.
***********************************************************************************************************************
*/
os_err_t call_back_trace_init(void)
{

#if defined(__CC_ARM)

    g_main_stack_start_addr = (os_uint32_t)&CSTACK_BLOCK_START(CSTACK_BLOCK_NAME);
    g_main_stack_end_addr   = (os_uint32_t)&CSTACK_BLOCK_END(CSTACK_BLOCK_NAME);
    g_code_start_addr       = (os_uint32_t)&CODE_SECTION_START(CODE_SECTION_NAME);
    g_code_end_addr         = (os_uint32_t)&CODE_SECTION_END(CODE_SECTION_NAME);
    
#elif defined(__ICCARM__)

    g_main_stack_start_addr = (os_uint32_t)__section_begin(CSTACK_BLOCK_NAME);
    g_main_stack_end_addr   = (os_uint32_t)__section_end(CSTACK_BLOCK_NAME);
    g_code_start_addr       = (os_uint32_t)__section_begin(CODE_SECTION_NAME);
    g_code_end_addr         = (os_uint32_t)__section_end(CODE_SECTION_NAME);
    
#elif defined(__GNUC__)

    g_main_stack_start_addr = (os_uint32_t)(&CSTACK_BLOCK_START);
    g_main_stack_end_addr   = (os_uint32_t)(&CSTACK_BLOCK_END);
    g_code_start_addr       = (os_uint32_t)(&CODE_SECTION_START);
    g_code_end_addr         = (os_uint32_t)(&CODE_SECTION_END);
    
#endif

    if (g_main_stack_start_addr >= g_main_stack_end_addr)
    {
        os_kprintf("ERROR: Unable to get the main stack information!\r\n");
        return OS_ERROR;
    }

    os_assert_set_hook(assert_hook);

    return OS_EOK;
}
OS_APP_INIT(call_back_trace_init);
#endif

/* include or export for supported function */
#if defined(__CC_ARM)

__asm os_size_t *get_msp(void) 
{
    mrs r0, msp
    bx lr
}

__asm os_size_t *get_psp(void) 
{
    mrs r0, psp
    bx lr
}

__asm os_size_t *get_sp(void) 
{
    mov r0, sp
    bx lr
}

__asm void set_psp(os_size_t psp) 
{
    msr psp, r0
    bx lr
}

#elif defined(__CLANG_ARM)
os_size_t *get_msp(void) 
{
    register os_size_t *result;
    __asm volatile ("MRS %0, msp\n" : "=r" (result) );
    return(result);
}

os_size_t *get_psp(void) 
{
    register os_size_t *result;
    __asm volatile ("MRS %0, psp\n" : "=r" (result) );
    return(result);
}

os_size_t *get_sp(void) 
{
    register os_size_t *result;
    __asm volatile ("MOV %0, sp\n" : "=r" (result) );
    return(result);
}

void set_psp(os_size_t psp) 
{
	__asm volatile ("MSR psp, %0\n\t" :: "r" (psp) );
}

#elif defined(__IAR_SYSTEMS_ICC__)
#pragma diag_suppress=Pe940    
os_size_t *get_msp(void)
{
    __asm("mrs r0, msp");
    __asm("bx lr");        
}
os_size_t *get_psp(void)
{
    __asm("mrs r0, psp");
    __asm("bx lr");        
}
os_size_t *get_sp(void)
{
    __asm("mov r0, sp");
    __asm("bx lr");       
}
void set_psp(os_size_t psp)
{
    __asm("msr psp, r0");
    __asm("bx lr");
}
#pragma diag_default=Pe940  
#elif defined(__GNUC__)
os_size_t *get_msp(void) 
{
    register os_size_t *result;
    __asm volatile ("MRS %0, msp\n" : "=r" (result) );
    return(result);
}
os_size_t *get_psp(void) 
{
    register os_size_t *result;
    __asm volatile ("MRS %0, psp\n" : "=r" (result) );
    return(result);
}
os_size_t *get_sp(void) 
{
    register os_size_t *result;
    __asm volatile ("MOV %0, sp\n" : "=r" (result) );
    return(result);
}
void set_psp(os_size_t psp) 
{
    __asm volatile ("MSR psp, %0\n\t" :: "r" (psp) );
}
#endif

