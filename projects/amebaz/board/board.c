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

#include <rtl8710b.h>
#include <stdint.h>
#include "board.h"
#include "drv_uart.h"

void SysTick_Handler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    os_tick_increase();

    /* leave interrupt */
    os_interrupt_leave();
}

uint32_t SysTick_Config(uint32_t ticks)
{
    /* Reload value impossible */
    if ((ticks - 1) > SysTick_LOAD_RELOAD_Msk)
        return 1;

    SysTick->LOAD = ticks - 1;                                   /* set reload register */
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1); /* set Priority for Systick Interrupt */
    SysTick->VAL  = 0;                                           /* Load the SysTick Counter Value */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk; /* Enable SysTick IRQ and SysTick Timer */
    return (0);                              /* Function successful */
}

#include "rtl8710b_boot.h"
extern BOOT_EXPORT_SYMB_TABLE boot_export_symbol;
#define HEAP_SIZE 91260
static uint8_t heap_space[HEAP_SIZE];

void os_hw_board_init(void)
{
    extern uint32_t SystemCoreClock;
    SysTick_Config(SystemCoreClock / OS_TICK_PER_SECOND);
    /* Heap initialization */
#if defined(OS_USING_HEAP)
    // os_system_heap_init((void *)boot_export_symbol.boot_ram_end, (void *)0x10005000);
    os_system_heap_init((void *)heap_space, (void *)(heap_space + HEAP_SIZE - 1));
#endif
#ifdef OS_USING_SERIAL
    os_hw_uart_init();
#endif
    os_console_set_device(OS_CONSOLE_DEVICE_NAME);
}
