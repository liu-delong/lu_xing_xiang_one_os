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
 * @file        esp32_drv_common.c
 *
 * @brief       This file provides board init functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include "esp32_drv_common.h"
#include "board.h"
#ifdef OS_USING_FAL
#include <fal_cfg.h>
#endif



/**
 ***********************************************************************************************************************
 * @brief           This function will initial ESP32 board.
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
#define ESP32_HEAP_SIZE 20480
char heap_space[ESP32_HEAP_SIZE];
void os_hw_board_init()
{
    /* Pin driver initialization is open by default */
#ifdef OS_USING_PIN
    os_hw_pin_init();
#endif

    /* Heap initialization */
#if defined(OS_USING_HEAP)
    os_system_heap_init((void *)heap_space, (void *)heap_space+ESP32_HEAP_SIZE-1);
#endif

    os_board_auto_init();
}

