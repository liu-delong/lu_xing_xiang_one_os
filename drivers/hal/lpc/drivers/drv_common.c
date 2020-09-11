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
#include "board.h"
#include <drv_cfg.h>
#include <drv_gpio.h>
#include <drv_usart.h>
#include "peripherals.h"
//#include "drv_sdram.h"
//#include <timer/clocksource.h>
//#include <timer/clocksource_cortexm.h>
//#include <timer/hrtimer.h>

uint32_t uwTick;

uint32_t HAL_GetTick(void)
{
//    if (hardware_init_done)
//    {
//        return os_clocksource_gettime() / NSEC_PER_MSEC;
//    }
//    else
//    {
//        return uwTick;
//    }
	return uwTick;
}

void HAL_IncTick(void)
{
    uwTick++;
//    cortexm_systick_isr();
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
    BOARD_InitBootPins();
    BOARD_InitBootPeripherals();
    CLOCK_EnableClock(kCLOCK_InputMux);
    
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);
    
    GPIO_PortInit(GPIO, 0);
    GPIO_PortInit(GPIO, 1);
	
    /* NVIC Configuration */
#define NVIC_VTOR_MASK              0x3FFFFF80
#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x10000000 */
    SCB->VTOR  = (0x10000000 & NVIC_VTOR_MASK);
#else  /* VECT_TAB_FLASH  */

#ifdef PKG_USING_TFM
    /* Set the Vector Table base location at 0x00020000 when RTT with TF-M*/
    SCB->VTOR  = (0x00020000 & NVIC_VTOR_MASK);
#else
    /* Set the Vector Table base location at 0x00000000 */
    SCB->VTOR  = (0x00000000 & NVIC_VTOR_MASK);
#endif
#endif

//#ifndef PKG_USING_TFM
//    /* This init has finished in secure side of TF-M  */
//    BOARD_BootClockPLL150M();
//#endif

    /* hardware init start, enable irq for systick */
    /* some hardware may init timeout */

//    hardware_init_done = OS_FALSE;

    os_hw_interrupt_enable(0);
    
    BOARD_InitBootClocks();
    /* init systick  1 systick = 1/(100M / 100) 100¸ösystick = 1s*/
    SysTick_Config(SystemCoreClock / OS_TICK_PER_SECOND);
    /* set pend exception priority */
    NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
    
    os_hw_interrupt_disable();
    
//#ifdef OS_USING_CONSOLE
//    os_console_set_device(OS_CONSOLE_DEVICE_NAME);
//#endif

//    hardware_init_done = OS_TRUE;

    /* hardware init end, disable irq */

#ifdef OS_USING_PIN
    os_hw_pin_init();
#endif

#ifdef HAL_SDRAM_MODULE_ENABLED
    SDRAM_Init();
#endif

    /* Heap initialization */
#if defined(OS_USING_HEAP)
    os_kprintf("sram heap, begin: 0x%p, end: 0x%p\n", HEAP_BEGIN, HEAP_END);
    os_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif

    os_board_auto_init();

    /* Set the shell console output device */

}

