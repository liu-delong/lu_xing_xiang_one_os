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
 * @file        cmiot_hal_flash.h
 *
 * @brief       The hal flash header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_HAL_FLASH_H__
#define __CMIOT_HAL_FLASH_H__

#include "cmiot_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

extern cmiot_int cmiot_erase_sector(cmiot_uint32 addr, cmiot_uint size);
extern cmiot_int cmiot_flash_write(cmiot_uint32 addr, cmiot_uint8 *buf, cmiot_uint size);
extern cmiot_int cmiot_flash_read(cmiot_uint32 addr, cmiot_uint8 *buf, cmiot_uint size);

#ifdef __cplusplus
}
#endif

#endif
