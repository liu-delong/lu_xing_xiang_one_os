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
 * @file        cmdump.c
 *
 * @brief       This file implements backtrack for cortex-M when the task is on exception.
 *
 * @revision
 * Date         Author          Notes
 * 2016-12-15   Armink          the first version
 * 2020-08-18   OneOS Team      adapt the code to OneOS and delelte other OS source
 ***********************************************************************************************************************
 */


#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <os_kernel.h>
#include <os_hw.h>
#include <cmdump.h>

static os_uint32_t gs_main_stack_start_addr = 0;
static os_size_t gs_main_stack_size = 0;
static os_uint32_t gs_code_start_addr = 0;
static os_size_t gs_code_size = 0;
static bool gs_cmdump_init_ok = OS_FALSE;
static bool gs_on_fault = OS_FALSE;
static bool gs_stack_is_overflow = OS_FALSE;
static struct cmb_hard_fault_regs gs_fault_regs;

#if (CPU_PLATFORM_TYPE == CPU_ARM_CORTEX_M4) || (CPU_PLATFORM_TYPE == CPU_ARM_CORTEX_M7)
static bool statck_has_fpu_regs = OS_FALSE;
#endif /* CPU_PLATFORM_TYPE */

static bool gs_on_task_before_fault = OS_FALSE;

/**
 ***********************************************************************************************************************
 * @brief           This function initialize the global variables of this module.
 *
 * @param           None           
 * 
 * @return          None
 ***********************************************************************************************************************
 */
void debug_backtrace_init(void)
{
#if defined(__CC_ARM)
    gs_main_stack_start_addr = (os_uint32_t)&CSTACK_BLOCK_START(CMB_CSTACK_BLOCK_NAME);
    gs_main_stack_size = (os_uint32_t)&CSTACK_BLOCK_END(CMB_CSTACK_BLOCK_NAME) - gs_main_stack_start_addr;
    gs_code_start_addr = (os_uint32_t)&CODE_SECTION_START(CMB_CODE_SECTION_NAME);
    gs_code_size = (os_uint32_t)&CODE_SECTION_END(CMB_CODE_SECTION_NAME) - gs_code_start_addr;
#elif defined(__ICCARM__)
    gs_main_stack_start_addr = (os_uint32_t)__section_begin(CMB_CSTACK_BLOCK_NAME);
    gs_main_stack_size = (os_uint32_t)__section_end(CMB_CSTACK_BLOCK_NAME) - gs_main_stack_start_addr;
    gs_code_start_addr = (os_uint32_t)__section_begin(CMB_CODE_SECTION_NAME);
    gs_code_size = (os_uint32_t)__section_end(CMB_CODE_SECTION_NAME) - gs_code_start_addr;
#elif defined(__GNUC__)
    gs_main_stack_start_addr = (os_uint32_t)(&CMB_CSTACK_BLOCK_START);
    gs_main_stack_size = (os_uint32_t)(&CMB_CSTACK_BLOCK_END) - gs_main_stack_start_addr;
    gs_code_start_addr = (os_uint32_t)(&CMB_CODE_SECTION_START);
    gs_code_size = (os_uint32_t)(&CMB_CODE_SECTION_END) - gs_code_start_addr;
#endif

    if (gs_main_stack_size == 0)
    {
        os_kprintf("ERROR: Unable to get the main stack information!\r\n");
        return;
    }

    gs_cmdump_init_ok = OS_TRUE;
}

/**
 ***********************************************************************************************************************
 * @brief           This function dump all information about the task stack.
 *
 * @param[in]       stack_start_addr    start of stack
 * @param[in]       stack_size          Stack size in bytes    
 * @param[in]       stack_pointer       Current stack pointer   
 *
 * @return          None
 ***********************************************************************************************************************
 */
#ifdef CM_USING_DUMP_STACK_INFO
/* dump current stack information*/
void dump_stack(os_uint32_t stack_start_addr, os_size_t stack_size, os_uint32_t *stack_pointer) 
{
    if (gs_stack_is_overflow) 
    {
        if (gs_on_task_before_fault)
        {
            os_kprintf("Error: Task stack(%08x) was overflow.\r\n", stack_pointer);
        }
        else
        {
            os_kprintf("Error: Main stack(%08x) was overflow.\r\n", stack_pointer);
        }
        
        if ((os_uint32_t) stack_pointer < stack_start_addr)
        {
            stack_pointer = (os_uint32_t *) stack_start_addr;
        }
        else if ((os_uint32_t) stack_pointer > stack_start_addr + stack_size)
        {
            stack_pointer = (os_uint32_t *) (stack_start_addr + stack_size);
        }
    }
        
    os_kprintf("==================   Task stack info  =====================\r\n");
    for (; (os_uint32_t) stack_pointer < stack_start_addr + stack_size; stack_pointer++) 
    {
        os_kprintf("        addr: 0x%08x      data: %08x\r\n", stack_pointer, *stack_pointer);
    }
    
    os_kprintf("-------------------------------------------------------------\r\n");    
}
#endif /* CM_USING_DUMP_STACK_INFO */


#define BL_INS_MASK         0xF800
#define BL_INS_HIGH         0xF800
#define BL_INS_LOW          0xF000
#define BLX_INX_MASK        0xFF00
#define BLX_INX             0x4700

/**
 ***********************************************************************************************************************
 * @brief           This function check the disassembly instruction is 'BL' or 'BLX'.
 *
 * @param[in]       addr    Address of instruction
 *
 * @return          On success, return OS_TRUE; on error, return OS_FALSE.
 ***********************************************************************************************************************
 */
static bool disassembly_ins_is_bl_blx(os_uint32_t addr) 
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
 * @brief           This function does a stack traceback.
 *
 * @param[in]       buffer      The buffer to storage stack traceback data
 * @param[in]       size        The buffer depths.    
 * @param[in]       sp          Stack pointer of a task  
 *
 * @return          The depths of stack traceback.
 ***********************************************************************************************************************
 */
os_size_t debug_backtrace_call_stack(os_uint32_t *buffer, os_size_t size, os_uint32_t sp) 
{
    os_uint32_t     pc;
    os_size_t       depth; 
    os_size_t       stack_size;
    os_uint32_t     stack_start_addr;
    bool            regs_saved_lr_is_valid;

    pc                      = 0;
    depth                   = 0;
    stack_size              = gs_main_stack_size;
    stack_start_addr        = gs_main_stack_start_addr;
    regs_saved_lr_is_valid  = OS_FALSE;
    if (gs_on_fault) 
    {
        if (!gs_stack_is_overflow)
        {
            /* first depth is PC */
            buffer[depth++] = gs_fault_regs.saved.pc;
            /* fix the LR address in thumb mode */
            pc = gs_fault_regs.saved.lr - 1;
            if ((pc >= gs_code_start_addr) 
                && (pc <= gs_code_start_addr + gs_code_size) 
                && (depth < CMB_CALL_STACK_MAX_DEPTH) 
                && (depth < size))
            {
                buffer[depth++] = pc;
                regs_saved_lr_is_valid = OS_TRUE;
            }
        }

        /* program is running on task before fault */
        if (gs_on_task_before_fault)
        {
            stack_start_addr = (os_uint32_t)os_task_self()->stack_addr;
            stack_size = os_task_self()->stack_size;
        }      
    }   
    else if (cmb_get_sp() == cmb_get_psp())
    {
        stack_start_addr = (os_uint32_t)os_task_self()->stack_addr;
        stack_size = os_task_self()->stack_size;
    }

    if (gs_stack_is_overflow) 
    {
        if (sp < stack_start_addr)
        {
            sp = stack_start_addr;
        
        } else if (sp > stack_start_addr + stack_size) 
        {
            sp = stack_start_addr + stack_size;
        }
    }

    for (; sp < stack_start_addr + stack_size; sp += sizeof(os_size_t)) 
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
        if ((pc >= gs_code_start_addr) 
            && (pc <= gs_code_start_addr + gs_code_size)
            && (depth < CMB_CALL_STACK_MAX_DEPTH) 
            && disassembly_ins_is_bl_blx(pc - sizeof(os_size_t)) && (depth < size))
        {
            /* ignore repeat */
            if ((depth == 2) && regs_saved_lr_is_valid && (pc == buffer[1]))
            {
                continue;
            }
            
            buffer[depth++] = pc;
        }
    }
    return depth;
}

/* print current call stack information*/
/**
 ***********************************************************************************************************************
 * @brief           This function print the information of stack traceback.
 *
 * @param[in]       sp          Stack pointer of a task  
 *
 * @return          None
 ***********************************************************************************************************************
 */
static void print_call_stack(os_uint32_t sp) 
{
    os_size_t i;
    os_size_t cur_depth;
    
    os_uint32_t call_stack_buf[CMB_CALL_STACK_MAX_DEPTH] = {0};

    cur_depth = debug_backtrace_call_stack(call_stack_buf, CMB_CALL_STACK_MAX_DEPTH, sp);

    os_kprintf("======================  Call stack  =========================\r\n");
    os_kprintf("===================  Call stack Begin =======================\r\n");
    os_kprintf("you can user below command for backtrace:\r\n");
    os_kprintf("addr2line -e *.axf -a -f ");
    for (i = 0; i < cur_depth; i++)
    {
        os_kprintf("0x%08x ", call_stack_buf[i]);
    }
    os_kprintf("\r\n");
    os_kprintf("===================  Call stack End  ========================\r\n");
    os_kprintf("-------------------------------------------------------------\r\n");
}

/**
 ***********************************************************************************************************************
 * @brief           This function does a stack traceback when a task assert.
 *
 * @param[in]       sp          Stack pointer of a task
 *
 * @return          None
 ***********************************************************************************************************************
 */
void debug_backtrace_assert(os_uint32_t sp)
{
    OS_ASSERT(gs_cmdump_init_ok);

    os_uint32_t cur_stack_pointer;

    cur_stack_pointer = cmb_get_sp();

    if (cur_stack_pointer == cmb_get_msp()) 
    {
        os_kprintf("Assert on interrupt environment\r\n");

#ifdef CM_USING_DUMP_STACK_INFO
        dump_stack(gs_main_stack_start_addr, gs_main_stack_size, (os_uint32_t *) sp);
#endif /* CM_USING_DUMP_STACK_INFO */ 
    } 
    else if(cur_stack_pointer == cmb_get_psp()) 
    {
        os_kprintf("Assert on task %s.\r\n", os_task_self()->parent.name);

#ifdef CM_USING_DUMP_STACK_INFO
        os_uint32_t stack_start_addr;
        os_size_t stack_size;
        stack_start_addr = (os_uint32_t)os_task_self()->stack_addr;
        stack_size = os_task_self()->stack_size;
        dump_stack(stack_start_addr, stack_size, (os_uint32_t *) sp);
#endif /* CM_USING_DUMP_STACK_INFO */
    }

    print_call_stack(sp);
}

#if (CPU_PLATFORM_TYPE == CPU_ARM_CORTEX_M4) || (CPU_PLATFORM_TYPE == CPU_ARM_CORTEX_M7)
static os_uint32_t statck_del_fpu_regs(os_uint32_t fault_handler_lr, os_uint32_t sp) 
{
    statck_has_fpu_regs = (fault_handler_lr & (1UL << 4)) == 0 ? OS_TRUE : OS_FALSE;
    return statck_has_fpu_regs == OS_TRUE ? sp + sizeof(os_size_t) * 18 : sp;
}
#endif /* CPU_PLATFORM_TYPE */

/**
 ***********************************************************************************************************************
 * @brief           This function does a stack traceback when a task exception.
 *
 * @param[in]       fault_handler_lr          The lr of fault handler
 * @param[in]       fault_handler_sp          TheStack pointer of fault handler
 *
 * @return          None
 ***********************************************************************************************************************
 */
void debug_backtrace_fault(os_uint32_t fault_handler_lr, os_uint32_t fault_handler_sp) 
{
    os_uint32_t stack_pointer = fault_handler_sp, saved_regs_addr = stack_pointer;
    const char *regs_name[] = { "R0 ", "R1 ", "R2 ", "R3 ", "R12", "LR ", "PC ", "PSR" };

#ifdef CM_USING_DUMP_STACK_INFO
    os_uint32_t stack_start_addr = gs_main_stack_start_addr;
    os_size_t stack_size = gs_main_stack_size;
#endif /* CM_USING_DUMP_STACK_INFO */

    OS_ASSERT(gs_cmdump_init_ok);
    OS_ASSERT(!gs_on_fault);

    gs_on_fault = OS_TRUE;
    gs_on_task_before_fault = fault_handler_lr & (1UL << 2);

    if (gs_on_task_before_fault) 
    {
        saved_regs_addr = stack_pointer = cmb_get_psp();

#ifdef CM_USING_DUMP_STACK_INFO
        stack_start_addr = (os_uint32_t)os_task_self()->stack_addr;
        stack_size = os_task_self()->stack_size;
#endif /* CM_USING_DUMP_STACK_INFO */
    } 
    else 
    {
        os_kprintf("Fault on interrupt or no OS environment.\r\n");
    }

    os_kprintf("=======================  stack_pointer 0x%x =========================\r\n", stack_pointer);

#ifdef CM_USING_DUMP_STACK_INFO
    if (stack_pointer < stack_start_addr || stack_pointer > stack_start_addr + stack_size) 
    {
        gs_stack_is_overflow = OS_TRUE;
    }        
    dump_stack(stack_start_addr, stack_size, (os_uint32_t *) stack_pointer);
#endif /* CM_USING_DUMP_STACK_INFO */ 

    /* delete saved R0~R3, R12, LR,PC,xPSR registers space */
    stack_pointer += sizeof(os_size_t) * 8;

#if (CPU_PLATFORM_TYPE == CPU_ARM_CORTEX_M4) || (CPU_PLATFORM_TYPE == CPU_ARM_CORTEX_M7)
    stack_pointer = statck_del_fpu_regs(fault_handler_lr, stack_pointer);
#endif /* CPU_PLATFORM_TYPE */ 

    /* 4 printf reg info */
    if (!gs_stack_is_overflow)
    {
        os_kprintf("=======================  Regs info  =========================\r\n");
        gs_fault_regs.saved.r0        = ((os_uint32_t *)saved_regs_addr)[0];  // R0
        gs_fault_regs.saved.r1        = ((os_uint32_t *)saved_regs_addr)[1];  // R1
        gs_fault_regs.saved.r2        = ((os_uint32_t *)saved_regs_addr)[2];  // R2
        gs_fault_regs.saved.r3        = ((os_uint32_t *)saved_regs_addr)[3];  // R3
        gs_fault_regs.saved.r12       = ((os_uint32_t *)saved_regs_addr)[4];  // R12
        gs_fault_regs.saved.lr        = ((os_uint32_t *)saved_regs_addr)[5];  // LR
        gs_fault_regs.saved.pc        = ((os_uint32_t *)saved_regs_addr)[6];  // PC
        gs_fault_regs.saved.psr.value = ((os_uint32_t *)saved_regs_addr)[7];  // PSR
        os_kprintf("        %s: 0x%08x        %s: 0x%08x\r\n", regs_name[0], gs_fault_regs.saved.r0, regs_name[1], gs_fault_regs.saved.r1);
        os_kprintf("        %s: 0x%08x        %s: 0x%08x\r\n", regs_name[2], gs_fault_regs.saved.r2, regs_name[3], gs_fault_regs.saved.r3);
        os_kprintf("        %s: 0x%08x        %s: 0x%08x\r\n", regs_name[4], gs_fault_regs.saved.r12, regs_name[5], gs_fault_regs.saved.lr);
        os_kprintf("        %s: 0x%08x        %s: 0x%08x\r\n", regs_name[6], gs_fault_regs.saved.pc, regs_name[7], gs_fault_regs.saved.psr.value);
        os_kprintf("-------------------------------------------------------------\r\n");
    }

    /* 5 printf call stack info */
    print_call_stack(stack_pointer);

    /* 6 shwo result  */
    os_kprintf("=========================   Result  =========================\r\n");
    if (gs_on_task_before_fault)
    {
        os_kprintf("Fault on task %s.\r\n", os_task_self()->parent.name);
    }
    else
    {
        os_kprintf("Fault on interrupt.\r\n");
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function implement a exception hook function.
 *
 * @param[in]       context          The context of a task
 *
 * @return          None
 ***********************************************************************************************************************
 */
OS_WEAK os_err_t exception_hook(void *context) 
{
    volatile os_uint8_t _continue = 1;
    os_uint32_t lr_offset = 0;
    os_uint32_t lr;
        
    os_enter_critical();
    os_kprintf("\r\n!!!!!!!!!!!!!!!!!!!!!!!!   Exception  !!!!!!!!!!!!!!!!!!!!!!!\r\n");

#ifdef OS_USING_SHELL
    os_kprintf("=======================   Heap Info   =======================\r\n");
    sh_list_mem(0, OS_NULL);
    os_kprintf("-------------------------------------------------------------\r\n");
    
    os_kprintf("=======================   Task Info   =======================\r\n");
    sh_list_task(0, OS_NULL);
    os_kprintf("-------------------------------------------------------------\r\n");
#endif /* OS_USING_SHELL */ 

#if (CPU_PLATFORM_TYPE != CPU_ARM_CORTEX_M0)
    /* the PSP is changed by CMCC HardFault_Handler, so restore it to HardFault context */
#if (defined (__VFP_FP__) && !defined(__SOFTFP__)) || (defined (__ARMVFP__)) || (defined(__ARM_PCS_VFP) || defined(__TARGET_FPU_VFP))
    cmb_set_psp(cmb_get_psp() + 4 * 10);
#else
    cmb_set_psp(cmb_get_psp() + 4 * 9);  
#endif
#endif

    /* Find exception return.*/
    for (lr_offset = cmb_get_sp(); 
         lr_offset < gs_main_stack_start_addr + gs_main_stack_size;
         lr_offset += sizeof(os_uint32_t))
    {
        lr = *(os_uint32_t *)lr_offset;

        if ((lr == 0xFFFFFFF1) || (lr == 0xFFFFFFF9) || (lr == 0xFFFFFFFD)
#if (CPU_PLATFORM_TYPE == CPU_ARM_CORTEX_M4) || (CPU_PLATFORM_TYPE == CPU_ARM_CORTEX_M7)
        || (lr == 0xFFFFFFE1) || (lr == 0xFFFFFFE9) || (lr == 0xFFFFFFED)
#endif /* CPU_PLATFORM_TYPE */ 
        )
        {
            break;
        }
    }
    
    if(lr_offset == gs_main_stack_start_addr + gs_main_stack_size)
    {
        os_kprintf("Find Exception Return error\r\n");
        while(1);
    }
    

    debug_backtrace_fault(lr, lr + sizeof(os_uint32_t));

    os_kprintf("\r\n!!!!!!!!!!!!!!!!!!!!!!!!   Dump End   !!!!!!!!!!!!!!!!!!!!!!!\r\n");

    while (_continue == 1);

    return OS_EOK;
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
    volatile os_uint8_t _continue;

    os_enter_critical();

    os_kprintf("\r\n!!!!!!!!!!!!!!!!!!!!!!!!    Assert   !!!!!!!!!!!!!!!!!!!!!!!\r\n");
    os_kprintf("(%s) assertion failed at function: %s, line number: %d \r\n", ex, func, line);

#ifdef OS_USING_SHELL
    os_kprintf("=======================   Heap Info   =======================\r\n");
    sh_list_mem(0, OS_NULL);
    os_kprintf("-------------------------------------------------------------\r\n");

    os_kprintf("========================   Task Info  =======================\r\n");
    sh_list_task(0, OS_NULL);
    os_kprintf("-------------------------------------------------------------\r\n");
#endif /* CM_USING_FINSH */ 

    debug_backtrace_assert(cmb_get_sp());

    os_kprintf("\r\n!!!!!!!!!!!!!!!!!!!!!!   Assert End   !!!!!!!!!!!!!!!!!!!!!!!\r\n");

    while (_continue == 1);

}

/**
 ***********************************************************************************************************************
 * @brief           This function initialize this module.
 *
 * @param           None           
 * 
 * @return          None
 ***********************************************************************************************************************
 */
int os_cm_backtrace_init(void)
{
    debug_backtrace_init();

    os_hw_exception_install(exception_hook);

    os_assert_set_hook(assert_hook);
    
    return 0;
}
OS_DEVICE_INIT(os_cm_backtrace_init);

