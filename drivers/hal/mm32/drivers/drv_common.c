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
#include <os_dma.h>
#include "drv_common.h"
#include "board.h"
#include "drv_gpio.h"
//#include "drv_sdram.h"

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#include <timer/clocksource_cortexm.h>
#endif

#include <timer/hrtimer.h>
#ifdef OS_USING_FAL
#include <fal_cfg.h>
#endif

static volatile os_bool_t hardware_init_done = OS_FALSE;



void SysTick_Handler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    os_tick_increase();

#ifdef OS_USING_CLOCKSOURCE	
    os_clocksource_update();
#endif
    /* leave interrupt */
    os_interrupt_leave();
}
void HAL_SuspendTick(void)
{
    /* Disable SysTick Interrupt */
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
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

#ifdef OS_USE_BOOTLOADER
#ifdef SOC_SERIES_STM32F0
    memcpy((void*)0x20000000, (void*)USER_APP_ENTRY, 0xBC);
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_SYSCFG_REMAPMEMORY_SRAM();
#else
    SCB->VTOR = USER_APP_ENTRY;
#endif
#endif

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

    hardware_init_done = OS_FALSE;

    os_hw_interrupt_enable(0);

    hardware_init(); 

    os_hw_interrupt_disable();

    hardware_init_done = OS_TRUE;

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

   // os_dma_mem_init();

#if defined(OS_USING_CLOCKSOURCE_CORTEXM) && defined(DWT)
    cortexm_dwt_init();
#endif

    os_board_auto_init();	
}

