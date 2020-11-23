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
 * @file        board.h
 *
 * @brief       Board resource definition
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#ifdef _LITTLE_ENDIAN
#undef _LITTLE_ENDIAN
#endif

#ifdef SECTION
#undef SECTION
#endif

#include <os_hw.h>
#include <os_kernel.h>
#include <os_device.h>

#define AMEBA_FLASH_START_ADRESS 0x08000000
#define AMEBA_FLASH_END_ADDRESS  0x08100000
#define AMEBA_FLASH_SECTOR_SIZE  4096
#define AMEBA_FLASH_SIZE         (512 * 1024)

#endif
