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
 * @file        rt_irq.c
 *
 * @brief       Implementation of RT-Thread adaper interrupt API.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_irq.h>
#include <rtdef.h>

void rt_interrupt_enter(void)
{
    os_interrupt_enter();
}

void rt_interrupt_leave(void)
{
    os_interrupt_leave();
}

rt_uint8_t rt_interrupt_get_nest(void)
{
    return (rt_uint8_t)os_interrupt_get_nest();
}

