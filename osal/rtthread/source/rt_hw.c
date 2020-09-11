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
 * @file        rt_event.c
 *
 * @brief       Implementation of RT-Thread adaper hardware API.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <os_types.h>
#include <os_util.h>
#include <os_hw.h>
#include <rtdef.h>
#include <rthw.h>

void rt_hw_cpu_reset(void)
{
    os_hw_cpu_reset();
}

void rt_hw_cpu_shutdown(void)
{
    os_hw_cpu_shutdown();
}

rt_base_t rt_hw_interrupt_disable(void)
{
    return (rt_base_t)os_hw_interrupt_disable();
}

void rt_hw_interrupt_enable(rt_base_t level)
{
    os_hw_interrupt_enable((os_base_t)level);
}

void rt_hw_console_output(const char *str)
{
    os_hw_console_output(str);
}

void rt_hw_show_memory(rt_uint32_t addr, rt_uint32_t size)
{
    int i;
    int j;

    OS_ASSERT(addr);

    i = 0;
    j = 0;

    addr = addr & ~0xF;
    size = 4 * ((size + 3) / 4);

    while (i < size)
    {
        os_kprintf("0x%08x: ", addr );

        for(j = 0; j < 4; j++)
        {
            os_kprintf("0x%08x  ", *(rt_uint32_t *)addr);

            addr += 4;
            i++;
        }

        os_kprintf("\r\n");
    }

    return;
}

void rt_hw_us_delay(rt_uint32_t us)
{
    os_hw_us_delay((os_uint32_t)us);
    return;
}

