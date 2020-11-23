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

#include <drv_cfg.h>


#ifdef __cplusplus
extern "C" {
#endif

#define ESP32_FLASH_SIZE        0x40000
#define ESP32_FLASH_BLOCK_SIZE  4096 
#define ESP32_FLASH_PAGE_SIZE   4


#ifdef OS_USING_LED
extern const struct push_button key_table[];
extern const int                key_table_size;
#endif

#ifdef OS_USING_PUSH_BUTTON
extern const led_t led_table[];
extern const int   led_table_size;
#endif

#ifdef __cplusplus
}
#endif

#endif
