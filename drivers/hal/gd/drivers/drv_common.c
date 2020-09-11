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
 * @file        drv_common.c
 *
 * @brief       This file provides systick time init/IRQ and board init functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_irq.h>
#include <os_clock.h>
#include <os_hw.h>
#include <os_memory.h>
#include "drv_common.h"
#include "drv_usart.h"
#include "board.h"
#include <timer/clocksource.h>
#include <timer/clocksource_cortexm.h>
#include <timer/hrtimer.h>

void os_hw_us_delay(os_uint32_t us)
{
    os_uint32_t start, now, delta, reload, us_tick;
    start = SysTick->VAL;
    reload = SysTick->LOAD;
    us_tick = SystemCoreClock / 1000000UL;
    do {
        now = SysTick->VAL;
        delta = start > now ? start - now : reload + start - now;
    } while(delta < us_tick * us);
}

#ifndef OS_USING_HRTIMER_FOR_SYSTICK
void os_tick_handler(void)
{
	os_interrupt_enter();
    os_tick_increase();
    //os_clocksource_update();
    os_interrupt_leave();
}

void cortexm_systick_init(void)
{
	/* setup systick timer for 1000Hz interrupts */
	if (SysTick_Config(SystemCoreClock / 1000U)){
		/* capture error */
		while (1){
		}
	}
	/* configure the systick handler priority */
	NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

#endif

void HAL_IncTick(void)
{
//zqw,added
#if 0
    uwTick++;
    
#ifndef OS_USING_HRTIMER_FOR_SYSTICK
    os_tick_handler();
#else
    cortexm_systick_isr();
#endif
#endif
}

void systick_config(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U)){
        /* capture error */
        while (1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

void _Error_Handler(char *s, int num)
{
    volatile int loop = 1;
    while (loop);
}

int hardware_init(void);


/**
 ***********************************************************************************************************************
 * @brief           This function will initial STM32 board.
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void os_hw_board_init()
{
#ifdef SCB_EnableICache
    /* Enable I-Cache---------------------------------------------------------*/
    SCB_EnableICache();
#endif

#ifdef SCB_EnableDCache
    /* Enable D-Cache---------------------------------------------------------*/
    SCB_EnableDCache();
#endif

    /* hardware init start, enable irq for systick */
    /* some hardware may init timeout */

    os_hw_interrupt_enable(0);

	systick_config();

    //HAL_SuspendTick();

    os_hw_interrupt_disable();

    /* hardware init end, disable irq */

    /* Pin driver initialization is open by default */
#ifdef OS_USING_PIN
    os_hw_pin_init();
#endif

#ifdef HAL_SDRAM_MODULE_ENABLED
    SDRAM_Init();
#endif

    /* Heap initialization */
#if defined(OS_USING_HEAP)
    os_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif
		/* USART driver initialization is open by default */
#ifdef OS_USING_SERIAL
		os_hw_usart_init();
#endif
#if defined(OS_USING_CLOCKSOURCE_CORTEXM) && defined(DWT)
    cortexm_dwt_init();
#endif

    //os_board_auto_init();

    //cortexm_systick_init();

    /* Set the shell console output device */
#ifdef OS_USING_CONSOLE
    os_console_set_device(OS_CONSOLE_DEVICE_NAME);
#endif
}

