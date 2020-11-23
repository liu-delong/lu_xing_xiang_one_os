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
 * @file        exc.c
 *
 * @brief       This file implement stack traceback related function to the ARM M4 architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-24   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <cortexm_common.h>
#include <os_util.h>
#include <os_task.h>
#include <os_irq.h>
#include <string.h>
#include <os_assert.h>
#include <os_hw.h>

#include <cpuport.h>
#include <exc.h>

extern volatile os_uint32_t os_task_switch_interrupt_flag; 

/**
 ***********************************************************************************************************************
 * @brief           This function handles hard fault exception.
 *
 * @param[in]       info          Exception info when exception occurs.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void os_hw_hard_fault_exception(struct exception_info *info)
{
    if(OS_NULL != os_exception_hook)
    {
        os_err_t result;

        result = os_exception_hook(info);
        if (OS_EOK == result) 
        {
            return;
        }
    }

    #ifdef  STACK_TRACE_EN
        exception_stack_show(info);
    
        #ifdef OS_USING_TASK_SWITCH_MONITOR
        
        extern void task_switch_info_show(os_uint32_t context);
        
        task_switch_info_show(1);  
        
        #endif 
        
        while(1);
    #else
        struct stack_frame *context = &info->stack_frame;
        os_kprintf("psr: 0x%08x\r\n", context->exception_stack_frame.psr);

        os_kprintf("r00: 0x%08x\r\n", context->exception_stack_frame.r0);
        os_kprintf("r01: 0x%08x\r\n", context->exception_stack_frame.r1);
        os_kprintf("r02: 0x%08x\r\n", context->exception_stack_frame.r2);
        os_kprintf("r03: 0x%08x\r\n", context->exception_stack_frame.r3);
        os_kprintf("r04: 0x%08x\r\n", context->r4);
        os_kprintf("r05: 0x%08x\r\n", context->r5);
        os_kprintf("r06: 0x%08x\r\n", context->r6);
        os_kprintf("r07: 0x%08x\r\n", context->r7);
        os_kprintf("r08: 0x%08x\r\n", context->r8);
        os_kprintf("r09: 0x%08x\r\n", context->r9);
        os_kprintf("r10: 0x%08x\r\n", context->r10);
        os_kprintf("r11: 0x%08x\r\n", context->r11);
        os_kprintf("r12: 0x%08x\r\n", context->exception_stack_frame.r12);
        os_kprintf(" lr: 0x%08x\r\n", context->exception_stack_frame.lr);
        os_kprintf(" pc: 0x%08x\r\n", context->exception_stack_frame.pc);

        /* Hard fault is generated in task context. */
        if (info->exc_return & (1 << 2))
        {
            os_kprintf("hard fault in task: %s\r\n", ((os_task_t *)os_task_self())->parent.name);

#ifdef OS_USING_SHELL
            extern os_err_t sh_list_task(os_int32_t argc, char **argv);
            sh_list_task(0, OS_NULL);
#endif
        }
        /* Hard fault is generated in interrupt context. */
        else
        {
            os_kprintf("hard fault in interrupt\r\n");
        }

        if ((info->exc_return & 0x10) == 0)
        {
            os_kprintf("FPU active!\r\n");
        }

        hard_fault_track();

        while (1);
    #endif
}

#ifdef  STACK_TRACE_EN

/**
 ***********************************************************************************************************************
 * @brief           This function displays a list of the nested routine calls that the specified task. 
 *
 * @attention       This function is be used in exception context. 
 *
 * @param[in]       name            Pointer to task name string.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void task_stack_show_exc(char *name)
{
    os_size_t          *task_sp;
    os_size_t          *task_bottom;
    os_task_t          *task;
    os_uint32_t         i;
    call_back_trace_t   record;

    struct stack_frame              *whole_stack_frame;
    struct exception_stack_frame    *cpu_stack_frame;

    if(OS_NULL == name)
    {   
        return;
    }
    
    task = os_task_find(name);

    if(OS_NULL == task)
    {
        os_kprintf("The system does not have a task with this name %s\r\n", name);
        return;
    }

    os_kprintf("=================Task %s stack trace======================\r\n", name);
    
    if(task != os_task_self())
    {
        /* After the task switch occurs, task->sp records the stack pointer of the task. */
        task_sp = task->sp;
        whole_stack_frame = (struct stack_frame *)task_sp;
        os_kprintf(" sp: 0x%08x\r\n", task_sp);
        os_kprintf("psr: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.psr);

        os_kprintf("r00: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r0);
        os_kprintf("r01: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r1);
        os_kprintf("r02: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r2);
        os_kprintf("r03: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r3);
        os_kprintf("r04: 0x%08x\r\n", whole_stack_frame->r4);
        os_kprintf("r05: 0x%08x\r\n", whole_stack_frame->r5);
        os_kprintf("r06: 0x%08x\r\n", whole_stack_frame->r6);
        os_kprintf("r07: 0x%08x\r\n", whole_stack_frame->r7);
        os_kprintf("r08: 0x%08x\r\n", whole_stack_frame->r8);
        os_kprintf("r09: 0x%08x\r\n", whole_stack_frame->r9);
        os_kprintf("r10: 0x%08x\r\n", whole_stack_frame->r10);
        os_kprintf("r11: 0x%08x\r\n", whole_stack_frame->r11);
        os_kprintf("r12: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r12);
        os_kprintf(" lr: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.lr);
        os_kprintf(" pc: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.pc);

        record.depth = 0;
        record.back_trace[record.depth++] = whole_stack_frame->exception_stack_frame.pc;
        record.back_trace[record.depth++] = whole_stack_frame->exception_stack_frame.lr - 1;
                
        /*Task overflow.*/
        if((os_size_t)task_sp > ((os_size_t)task->stack_addr + task->stack_size) 
            || (task_sp < (os_size_t*)(task->stack_addr)))
        {
            task_bottom = task_sp 
                          + (sizeof(struct stack_frame) + TASK_STACK_OVERFLOW_STACK_SIZE ) / sizeof(os_size_t);
            os_kprintf("The stack of task %s is overflow!\r\n", name);
            trace_stack((task_sp + sizeof(struct stack_frame) / sizeof(os_size_t)), task_bottom, &record);
        }
        else
        {
            trace_stack((task_sp + sizeof(struct stack_frame) / sizeof(os_size_t)),
                        (os_size_t*)((os_uint32_t)task->stack_addr + task->stack_size),
                        &record);
        }
    }
    else
    {
        /* Current context is interrupt context. */
        if (os_interrupt_get_nest() > 0)
        {
            /* The running task always uses psp, task->sp is not used. */
            task_sp = get_psp();
            cpu_stack_frame = (struct exception_stack_frame *)task_sp;
            os_kprintf(" sp: 0x%08x\r\n", task_sp);
            os_kprintf("psr: 0x%08x\r\n", cpu_stack_frame->psr);

            os_kprintf("r00: 0x%08x\r\n", cpu_stack_frame->r0);
            os_kprintf("r01: 0x%08x\r\n", cpu_stack_frame->r1);
            os_kprintf("r02: 0x%08x\r\n", cpu_stack_frame->r2);
            os_kprintf("r03: 0x%08x\r\n", cpu_stack_frame->r3);
            os_kprintf("r12: 0x%08x\r\n", cpu_stack_frame->r12);
            os_kprintf(" lr: 0x%08x\r\n", cpu_stack_frame->lr);
            os_kprintf(" pc: 0x%08x\r\n", cpu_stack_frame->pc);
            
            record.depth = 0;
            record.back_trace[record.depth++] = cpu_stack_frame->pc;
            record.back_trace[record.depth++] = cpu_stack_frame->lr - 1;

            /*Task overflow.*/
            if((os_size_t)task_sp > ((os_size_t)task->stack_addr + task->stack_size) 
                || (task_sp < (os_size_t*)(task->stack_addr)))
            {
                task_bottom = task_sp 
                              + (sizeof(struct exception_stack_frame) + TASK_STACK_OVERFLOW_STACK_SIZE )
                              / sizeof(os_size_t);
                
                os_kprintf("The stack of task %s is overflow!\r\n", name);
                trace_stack((task_sp + sizeof(struct exception_stack_frame) / sizeof(os_size_t)), task_bottom, &record);
            }
            else
            {
                trace_stack((task_sp + sizeof(struct exception_stack_frame) / sizeof(os_size_t)),
                            (os_size_t*)((os_uint32_t)task->stack_addr + task->stack_size),
                            &record);
            }
        }
        else
        {
            os_kprintf("Task %s stack trace has been processed by hard fault exception\r\n", name);
            return;
        }
    }
    
    #ifdef EXC_DUMP_STACK
    dump_stack((os_uint32_t)task->stack_addr,task->stack_size, (os_size_t*)task_sp);
    #endif
    
    os_kprintf("you can use below command for backtrace:\r\n");
    os_kprintf("addr2line -e *.axf/*.elf -a -f ");
    for (i = 0; i < record.depth; i++)
    {
        os_kprintf("0x%08x ", record.back_trace[i]);
    }
    os_kprintf("\r\n");
}

/**
 ***********************************************************************************************************************
 * @brief           This function displays a list of the nested routine calls that the specified task. 
 *
 * @attention       This function is used in task context and interrupt context, and cannot be used in exception 
 *                  context. The processing method of the currently running task and other tasks is different.
 *
 * @param[in]       name            Pointer to task name string
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void task_stack_show(char *name)
{
    os_size_t          *task_sp;
    os_size_t          *task_bottom;
    os_task_t          *task;
    os_uint32_t         i;
    call_back_trace_t   record;

    struct stack_frame              *whole_stack_frame;
    struct exception_stack_frame    *cpu_stack_frame;

    if(OS_NULL == name)
    {   
        return;
    }
    
    task = os_task_find(name);

    if(OS_NULL == task)
    {
        os_kprintf("The system does not have a task with this name %s\r\n", name);
        return;
    }

    os_kprintf("=================Task %s stack trace======================\r\n", name);

    /* Current context is interrupt context. */
    if (os_interrupt_get_nest() > 0)
    {
        if(task != os_task_self())
        {
            /* After the task switch occurs, task->sp records the stack pointer of the task. */
            task_sp = task->sp;
            whole_stack_frame = (struct stack_frame *)task_sp;
            os_kprintf(" sp: 0x%08x\r\n", task_sp);
            os_kprintf("psr: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.psr);

            os_kprintf("r00: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r0);
            os_kprintf("r01: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r1);
            os_kprintf("r02: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r2);
            os_kprintf("r03: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r3);
            os_kprintf("r04: 0x%08x\r\n", whole_stack_frame->r4);
            os_kprintf("r05: 0x%08x\r\n", whole_stack_frame->r5);
            os_kprintf("r06: 0x%08x\r\n", whole_stack_frame->r6);
            os_kprintf("r07: 0x%08x\r\n", whole_stack_frame->r7);
            os_kprintf("r08: 0x%08x\r\n", whole_stack_frame->r8);
            os_kprintf("r09: 0x%08x\r\n", whole_stack_frame->r9);
            os_kprintf("r10: 0x%08x\r\n", whole_stack_frame->r10);
            os_kprintf("r11: 0x%08x\r\n", whole_stack_frame->r11);
            os_kprintf("r12: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r12);
            os_kprintf(" lr: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.lr);
            os_kprintf(" pc: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.pc);
            
            record.depth = 0;
            record.back_trace[record.depth++] = whole_stack_frame->exception_stack_frame.pc;
            record.back_trace[record.depth++] = whole_stack_frame->exception_stack_frame.lr - 1;
            
            /*Task overflow.*/
            if((os_size_t)task_sp > ((os_size_t)task->stack_addr + task->stack_size) 
                || (task_sp < (os_size_t*)(task->stack_addr)))
            {
                task_bottom = task_sp 
                              + (sizeof(struct stack_frame) + TASK_STACK_OVERFLOW_STACK_SIZE ) / sizeof(os_size_t);
                os_kprintf("The stack of task %s is overflow!\r\n", name);
                trace_stack((task_sp + sizeof(struct stack_frame) / sizeof(os_size_t)), task_bottom, &record);
            }
            else
            {
                trace_stack((task_sp + sizeof(struct stack_frame) / sizeof(os_size_t)),
                            (os_size_t*)((os_uint32_t)task->stack_addr + task->stack_size),
                            &record);
            }
        }
        else
        {
            /* The running task always uses psp, task->sp is not used. */
            task_sp = get_psp();
            cpu_stack_frame = (struct exception_stack_frame *)task_sp;
            os_kprintf(" sp: 0x%08x\r\n", task_sp);
            os_kprintf("psr: 0x%08x\r\n", cpu_stack_frame->psr);

            os_kprintf("r00: 0x%08x\r\n", cpu_stack_frame->r0);
            os_kprintf("r01: 0x%08x\r\n", cpu_stack_frame->r1);
            os_kprintf("r02: 0x%08x\r\n", cpu_stack_frame->r2);
            os_kprintf("r03: 0x%08x\r\n", cpu_stack_frame->r3);
            os_kprintf("r12: 0x%08x\r\n", cpu_stack_frame->r12);
            os_kprintf(" lr: 0x%08x\r\n", cpu_stack_frame->lr);
            os_kprintf(" pc: 0x%08x\r\n", cpu_stack_frame->pc);
            
            record.depth = 0;
            record.back_trace[record.depth++] = cpu_stack_frame->pc;
            record.back_trace[record.depth++] = cpu_stack_frame->lr - 1;

            /*Task overflow.*/
            if((os_size_t)task_sp > ((os_size_t)task->stack_addr + task->stack_size) 
                || (task_sp < (os_size_t*)(task->stack_addr)))
            {
                task_bottom = task_sp 
                              + (sizeof(struct exception_stack_frame) + TASK_STACK_OVERFLOW_STACK_SIZE )
                              / sizeof(os_size_t);
                
                os_kprintf("The stack of task %s is overflow!\r\n", name);
                trace_stack((task_sp + sizeof(struct exception_stack_frame) / sizeof(os_size_t)), task_bottom, &record);
            }
            else
            {
                trace_stack((task_sp + sizeof(struct exception_stack_frame) / sizeof(os_size_t)),
                            (os_size_t*)((os_uint32_t)task->stack_addr + task->stack_size),
                            &record);
            }
        }
    }
    /* Current context is task context. */
    else
    {
        if(task != os_task_self())
        {
            /* After the task switch occurs, task->sp records the stack pointer of the task. */
            task_sp = task->sp;
            whole_stack_frame = (struct stack_frame *)task_sp;
            os_kprintf(" sp: 0x%08x\r\n", task_sp);
            os_kprintf("psr: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.psr);

            os_kprintf("r00: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r0);
            os_kprintf("r01: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r1);
            os_kprintf("r02: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r2);
            os_kprintf("r03: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r3);
            os_kprintf("r04: 0x%08x\r\n", whole_stack_frame->r4);
            os_kprintf("r05: 0x%08x\r\n", whole_stack_frame->r5);
            os_kprintf("r06: 0x%08x\r\n", whole_stack_frame->r6);
            os_kprintf("r07: 0x%08x\r\n", whole_stack_frame->r7);
            os_kprintf("r08: 0x%08x\r\n", whole_stack_frame->r8);
            os_kprintf("r09: 0x%08x\r\n", whole_stack_frame->r9);
            os_kprintf("r10: 0x%08x\r\n", whole_stack_frame->r10);
            os_kprintf("r11: 0x%08x\r\n", whole_stack_frame->r11);
            os_kprintf("r12: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.r12);
            os_kprintf(" lr: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.lr);
            os_kprintf(" pc: 0x%08x\r\n", whole_stack_frame->exception_stack_frame.pc);
            
            record.depth = 0;
            record.back_trace[record.depth++] = whole_stack_frame->exception_stack_frame.pc;
            record.back_trace[record.depth++] = whole_stack_frame->exception_stack_frame.lr - 1;
            
            /*Task overflow.*/
            if((os_size_t)task_sp > ((os_size_t)task->stack_addr + task->stack_size) 
                || (task_sp < (os_size_t*)(task->stack_addr)))
            {
                task_bottom = task_sp 
                              + (sizeof(struct stack_frame) + TASK_STACK_OVERFLOW_STACK_SIZE ) / sizeof(os_size_t);
                
                os_kprintf("The stack of task %s is overflow!\r\n", name);
                trace_stack((task_sp + sizeof(struct stack_frame) / sizeof(os_size_t)), task_bottom, &record);
            }
            else
            {
                trace_stack((task_sp + sizeof(struct stack_frame) / sizeof(os_size_t)),
                            (os_size_t*)((os_uint32_t)task->stack_addr + task->stack_size),
                            &record);
            }
        }
        else
        {   
            /* The running task always uses psp, task->sp is not used. */
            task_sp = get_psp();
            os_kprintf(" sp: 0x%08x\r\n", task_sp);
            record.depth = 0;
            
            /*Task overflow.*/
            if((os_size_t)task_sp > ((os_size_t)task->stack_addr + task->stack_size) 
                || (task_sp < (os_size_t*)(task->stack_addr)))
            {
                task_bottom = task_sp + TASK_STACK_OVERFLOW_STACK_SIZE / sizeof(os_size_t);
                os_kprintf("The stack of task %s is overflow!\r\n", name);
                trace_stack(task_sp, task_bottom, &record);
            }
            else
            {
                trace_stack(task_sp , 
                            (os_size_t*)((os_uint32_t)task->stack_addr + task->stack_size), 
                            &record);
            }
        }  
    }

    #ifdef EXC_DUMP_STACK
    dump_stack((os_uint32_t)task->stack_addr,task->stack_size, (os_size_t*)task_sp);
    #endif
    
    os_kprintf("you can use below command for backtrace:\r\n");
    os_kprintf("addr2line -e *.axf/*.elf -a -f ");
    for (i = 0; i < record.depth; i++)
    {
        os_kprintf("0x%08x ", record.back_trace[i]);
    }
    os_kprintf("\r\n");
}

/**
***********************************************************************************************************************
* @brief           A shell command to display task stack information.
*
* @param[in]       argc             argment count
* @param[in]       argv             argment list
*
* @return          On success, return OS_EOK; on error, OS_ERROR is returned.
* @retval          OS_EOK           Success.
* @retval          OS_ERROR         Shell command has no task name.
***********************************************************************************************************************
*/
static os_err_t sh_task_stack_show(os_int32_t argc, char **argv)
{
    char name[OS_NAME_MAX + 1];
    os_uint32_t len;

    if(argc < 2)
    {
        return OS_ERROR;
    }
    
    len = strlen(argv[1]);
    memset(name, 0, OS_NAME_MAX + 1);
    strncpy(name, argv[1], len > OS_NAME_MAX ? len : OS_NAME_MAX);

    task_stack_show(name);
    
    return OS_EOK;
}

#ifdef OS_USING_SHELL

#include <shell.h>
SH_CMD_EXPORT(task_stack_show, sh_task_stack_show, "show stack call back trace of a task, para is the name of task");

#endif /* OS_USING_SHELL */

/**
 ***********************************************************************************************************************
 * @brief           This function handles hard fault exception.
 *
 * @param[in]       info          Exception info when exception occurs.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void exception_stack_show(struct exception_info *info)
{
    struct stack_frame *context;
    os_size_t *sp;
    os_size_t *sp_bottom;
    call_back_trace_t record;
    os_uint16_t i;

    context = &info->stack_frame;
    
    sp = (os_size_t*)info;
    sp +=  (sizeof(struct exception_info) / sizeof(os_size_t));

    /* Hard fault is generated in interrupt context. */
    if (os_interrupt_get_nest() > 0)
    {
        os_kprintf("hard fault in interrupt\r\n");
        sp_bottom = sp;
        while((os_size_t)sp_bottom < g_main_stack_end_addr)
        {
            if(OS_TRUE == disassembly_ins_is_exc_return(*sp_bottom++))
            {
                break;
            }
        }
    }
    /* Hard fault is generated in task context. */
    else
    {
        os_kprintf("hard fault in task: %s\r\n", (os_task_self())->parent.name);

        /*Task overflow.*/
        if((os_size_t)info > ((os_size_t)os_task_self()->stack_addr + os_task_self()->stack_size) 
            || ((os_size_t)info < (os_size_t)(os_task_self()->stack_addr)))
        {
            sp_bottom = sp + TASK_STACK_OVERFLOW_STACK_SIZE / sizeof(os_size_t);
            os_kprintf("The stack of task %s is overflow!\r\n", os_task_self()->parent.name);
        }
        else
        {
            sp_bottom = (os_size_t *)((os_uint32_t)os_task_self()->stack_addr + os_task_self()->stack_size);
        }
    }
    
    os_kprintf("psr: 0x%08x\r\n", context->exception_stack_frame.psr);

    os_kprintf("r00: 0x%08x\r\n", context->exception_stack_frame.r0);
    os_kprintf("r01: 0x%08x\r\n", context->exception_stack_frame.r1);
    os_kprintf("r02: 0x%08x\r\n", context->exception_stack_frame.r2);
    os_kprintf("r03: 0x%08x\r\n", context->exception_stack_frame.r3);
    os_kprintf("r04: 0x%08x\r\n", context->r4);
    os_kprintf("r05: 0x%08x\r\n", context->r5);
    os_kprintf("r06: 0x%08x\r\n", context->r6);
    os_kprintf("r07: 0x%08x\r\n", context->r7);
    os_kprintf("r08: 0x%08x\r\n", context->r8);
    os_kprintf("r09: 0x%08x\r\n", context->r9);
    os_kprintf("r10: 0x%08x\r\n", context->r10);
    os_kprintf("r11: 0x%08x\r\n", context->r11);
    os_kprintf("r12: 0x%08x\r\n", context->exception_stack_frame.r12);
    os_kprintf(" lr: 0x%08x\r\n", context->exception_stack_frame.lr);
    os_kprintf(" pc: 0x%08x\r\n", context->exception_stack_frame.pc);

    if ((info->exc_return & 0x10) == 0)
    {
        os_kprintf("FPU active!\r\n");
    }

    hard_fault_track();

    record.depth = 0;
    record.back_trace[record.depth++] = context->exception_stack_frame.pc;
    record.back_trace[record.depth++] = context->exception_stack_frame.lr - 1;
    
    trace_stack(sp, sp_bottom, &record);

    os_kprintf("you can user below command for backtrace:\r\n");
    os_kprintf("addr2line -e *.axf/*.elf -a -f ");
    for (i = 0; i < record.depth; i++)
    {
        os_kprintf("0x%08x ", record.back_trace[i]);
    }
    os_kprintf("\r\n");

    #ifdef EXC_DUMP_STACK
    dump_stack((os_uint32_t)info, (os_uint32_t)sp_bottom - (os_uint32_t)info, (os_size_t*)info);
    #endif

    #ifdef OS_USING_SHELL
    extern os_err_t sh_list_task(os_int32_t argc, char **argv);
    extern os_err_t sh_list_mem(os_int32_t argc, char **argv);

    sh_list_task(0, OS_NULL);
    sh_list_mem(0, OS_NULL);
    #endif
}

#endif /* STACK_TRACE_EN */

#ifdef OS_USING_OVERFLOW_CHECK

/**
 ***********************************************************************************************************************
 * @brief           This function is used to check the stack of "from" task when switching task.
 *
 * @param[in]       task            The descriptor of task control block
 * 
 * @return          None.
 ***********************************************************************************************************************
 */
void schedule_from_task_stack_check(os_task_t *task)
{
    os_size_t sp;
    OS_ASSERT(OS_NULL != task);

    if ((os_interrupt_get_nest() > 0) && (1 == os_task_switch_interrupt_flag))
    {
        return;
    }
    
    sp = (os_size_t)get_current_task_sp();

#if defined(ARCH_CPU_STACK_GROWS_UPWARD)
    if ((*((os_uint8_t *)((os_ubase_t)task->stack_addr + task->stack_size - 1)) != '#') ||
       ((os_uint32_t)sp >= ((os_uint32_t)task->stack_addr + task->stack_size - sizeof(struct stack_frame))) ||
#else
    if ((*((os_uint8_t *)task->stack_addr) != '#') ||
       ((os_uint32_t)sp < ((os_uint32_t)task->stack_addr + sizeof(struct stack_frame))) ||
#endif
        ((os_uint32_t)sp < (os_uint32_t)task->stack_addr) ||
        ((os_uint32_t)sp >= (os_uint32_t)task->stack_addr + (os_uint32_t)task->stack_size))
    {

        os_kprintf("schedule from task:%s stack overflow,the sp is 0x%x.\r\n", task->parent.name, sp);
#ifdef OS_USING_SHELL
        {
            extern os_err_t sh_list_task(os_int32_t argc, char **argv);
            sh_list_task(0, OS_NULL);
        }
#endif
        (void)os_hw_interrupt_disable();
        OS_ASSERT(0);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function is used to check the stack of "to" task when switching task.
 *
 * @param[in]       task            The descriptor of task control block
 * 
 * @return          None.
 ***********************************************************************************************************************
 */
void schedule_to_task_stack_check(os_task_t *task)
{
    os_size_t sp;
    OS_ASSERT(OS_NULL != task);

    sp = (os_size_t)task->sp;

#if defined(ARCH_CPU_STACK_GROWS_UPWARD)
    if ((*((os_uint8_t *)((os_ubase_t)task->stack_addr + task->stack_size - 1)) != '#') ||
#else
    if ((*((os_uint8_t *)task->stack_addr) != '#') ||
#endif
        ((os_uint32_t)sp < (os_uint32_t)task->stack_addr) ||
        ((os_uint32_t)sp >= (os_uint32_t)task->stack_addr + (os_uint32_t)task->stack_size))
    {
        os_kprintf("schedule to task:%s stack overflow,the sp is 0x%x.\r\n", task->parent.name, sp);
#ifdef OS_USING_SHELL
        {
            extern os_err_t sh_list_task(os_int32_t argc, char **argv);
            sh_list_task(0, OS_NULL);
        }
#endif
        (void)os_hw_interrupt_disable();
        OS_ASSERT(0);
    }
}

#endif /* OS_USING_OVERFLOW_CHECK */

