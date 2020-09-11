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
 * @brief       This file provides register width define related to the RISC-V architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-05-18   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef CPUPORT_H__
#define CPUPORT_H__

#include <oneos_config.h>

/* bytes of register width  */
#ifdef ARCH_CPU_64BIT
#define STORE                   sd
#define LOAD                    ld
#define REGBYTES                8
#else
#define STORE                   sw
#define LOAD                    lw
#define REGBYTES                4
#endif

#endif
