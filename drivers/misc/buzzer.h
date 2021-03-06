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
 * @file        buzzer.h
 *
 * @brief       this file implements buzzer related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef OS_BUZZER_H_
#define OS_BUZZER_H_

#include <os_task.h>
#include <os_device.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct buzzer
{
    int pin;
    int active_level;
} buzzer_t;

#ifdef __cplusplus
}
#endif

#endif
