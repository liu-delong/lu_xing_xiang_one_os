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
 * @file        board.c
 *
 * @brief       Initializes the CPU, System clocks, and Peripheral device
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_gpio.h>

#if defined(OS_USING_LED)
const led_t led_table[] = 
{		
		{38, PIN_LOW},/*LD0*/
    {39, PIN_LOW},/*LD1*/
    {40, PIN_LOW},/*LD2*/
    {41, PIN_LOW},/*LD3*/
};

const int led_table_size = sizeof(led_table) / sizeof(led_table[0]);
#endif

#if defined(OS_USING_PUSH_BUTTON)
const struct push_button key_table[] = 
{
    {2,  PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},/*K1*/
    //{10, PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},/*K2*/
    {21, PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},/*K3*/
    {22, PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},/*K4*/
};

const int key_table_size = ARRAY_SIZE(key_table);
#endif
