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
 * @file        cpuport.h
 *
 * @brief       This file provides data struct related to the ARM M3 architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-26   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __CPUPORT_H__
#define __CPUPORT_H__

#include <oneos_config.h>
#include <os_types.h>

struct exception_stack_frame
{
    os_uint32_t r0;
    os_uint32_t r1;
    os_uint32_t r2;
    os_uint32_t r3;
    os_uint32_t r12;
    os_uint32_t lr;
    os_uint32_t pc;
    os_uint32_t psr;
};

struct stack_frame
{
    /* r4 ~ r11 register */
    os_uint32_t r4;
    os_uint32_t r5;
    os_uint32_t r6;
    os_uint32_t r7;
    os_uint32_t r8;
    os_uint32_t r9;
    os_uint32_t r10;
    os_uint32_t r11;

    struct exception_stack_frame exception_stack_frame;
};

#endif

