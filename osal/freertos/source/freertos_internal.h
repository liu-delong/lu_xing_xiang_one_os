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
 * @file        cmsis_internal.h
 *
 * @brief       internal head file for CMSIS APIs adapter
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __FREERTOS_ONEOS_H__
#define __FREERTOS_ONEOS_H__

#include <oneos_config.h>
#include "os_kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Macro to check current context */
#ifdef OS_USING_CONTEXT_CHECK
#define OS_DEBUG_IN_INTERRUPT                                                   \
do                                                                              \
{                                                                               \
    os_base_t level;                                                            \
    level = os_hw_interrupt_disable();                                          \
    if (os_interrupt_get_nest() == 0)                                           \
    {                                                                           \
        os_kprintf("Function[%s] shall be used in ISR\n", __FUNCTION__);        \
        OS_ASSERT(0);                                                           \
    }                                                                           \
    os_hw_interrupt_enable(level);                                              \
} while (0)
#else
#define OS_DEBUG_IN_INTERRUPT
#endif

#undef FREERTOS_ADAPT_LOG
#define FREERTOS_ADAPT_LOG(type, message)                     \
do                                                      \
{                                                       \
    if (type)                                           \
        os_kprintf message;                             \
}                                                       \
while (0)


extern char* oneos_itoa(int value, char* string, int radix);


#ifdef __cplusplus
}
#endif

#endif /* __FREERTOS_ONEOS_H__ */


