/**

*******************************************************************************
****************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        drv_common.h
 *
 * @brief       This file provides _Error_Handler() declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version

*******************************************************************************
****************************************
 */

#ifndef __DRV_COMMON_H__
#define __DRV_COMMON_H__

#include <os_hw.h>
#include <os_device.h>
#include "define_all.h"

#ifdef __cplusplus
extern "C" {
#endif

void _Error_Handler(char *s, int num);

void AltFunIO( GPIOx_Type* GPIOx, os_uint32_t PinNum, os_uint8_t Type  );
void InputtIO( GPIOx_Type* GPIOx, os_uint32_t PinNum, os_uint8_t Type );
void OutputIO( GPIOx_Type* GPIOx, os_uint32_t PinNum, os_uint8_t Type );
void AnalogIO( GPIOx_Type* GPIOx, os_uint32_t PinNum );

#ifndef Error_Handler
#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __DRV_COMMON_H__ */

