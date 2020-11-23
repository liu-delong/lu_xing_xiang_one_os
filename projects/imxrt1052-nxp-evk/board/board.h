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

#include "fsl_common.h"
#include "clock_config.h"

#include <os_hw.h>
#include <os_device.h>
#include <os_assert.h>
#include <drv_cfg.h>

#ifdef __CC_ARM
extern int Image$$OS_HEAP$$ZI$$Base;
extern int Image$$OS_HEAP$$ZI$$Limit;
#define HEAP_BEGIN          (&Image$$OS_HEAP$$ZI$$Base)
#define HEAP_END            (&Image$$OS_HEAP$$ZI$$Limit)

#elif __ICCARM__
#pragma section="HEAP"
#define HEAP_BEGIN          (__segment_end("HEAP"))
extern void __OS_HEAP_END;
#define HEAP_END            (&__OS_HEAP_END)

#else
extern int heap_start;
extern int heap_end;
#define HEAP_BEGIN          (&heap_start)
#define HEAP_END            (&heap_end)
#endif

#define HEAP_SIZE           ((uint32_t)HEAP_END - (uint32_t)HEAP_BEGIN)

#ifdef BSP_USING_ETH
void imxrt_enet_pins_init(void);
void imxrt_enet_phy_reset_by_gpio(void);
#define PHY_ADDRESS     0x02u
#endif

extern const struct push_button key_table[];
extern const int key_table_size;

extern const led_t led_table[];
extern const int   led_table_size;

#endif

