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
 * @file        exc.h
 *
 * @brief       This file implement stack traceback related function declaration to the ARM M33 architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-26   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __EXC_H__
#define __EXC_H__

#include <oneos_config.h>
#include <os_types.h>
#include <cpuport.h>
#include <os_task.h>

struct exception_info
{
    os_uint32_t        exc_return;
    struct stack_frame stack_frame;
};

#ifdef  STACK_TRACE_EN

extern void task_stack_show_exc(char *name);

extern void task_stack_show(char *name);

extern void exception_stack_show(struct exception_info *info);

#endif  /* STACK_TRACE_EN */

#endif  /* __EXC_H__ */

