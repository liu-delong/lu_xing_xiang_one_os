/*
 * Copyright (c) 2012-2020, CMCC IOT
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */


#include "drv_common.h"
#include "board.h"

#include "mt2625.h"
#include "hal_clock_internal.h"
#include "hal_nvic.h"

#include "portmacro.h"
#include "hal_gpt.h"

#include <stdint.h>

#ifdef OS_USING_PIN
#include "drv_gpio.h"
#endif

#ifdef OS_USING_SHELL
#include <shell.h>
static void reboot(uint8_t argc, char **argv)
{
    extern void exception_reboot(void);
    exception_reboot();
}
SH_CMD_EXPORT(cmd_reboot, reboot, "Reboot System");
#endif /* OS_USING_SHELL */

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    os_tick_increase();
}

/**
 * This function will delay for some us.
 *
 * @param us the delay time of us
 */
void cm_hw_us_delay(os_uint32_t us)
{
    extern hal_gpt_status_t hal_gpt_delay_us(uint32_t us);
    hal_gpt_delay_us(us);
}

/**
 * This function will initial board.
 */
OS_WEAK void os_hw_board_init()
{
#if 0
    /* System clock initialization */
    extern void SystemClock_Config(void);
    extern void SystemCoreClockUpdate(void);
    SystemClock_Config();    /* in sysinit.c */
    SystemCoreClockUpdate(); /* in sysinit.c */

    extern void os_gpt_init(uint32_t ms);
    os_gpt_init(portTICK_PERIOD_MS); /* 1tick = 10ms */

    /* USART driver initialization is open by default */
#ifdef OS_USING_SERIAL
    cm_hw_usart_init();
#endif

    /* Set the shell console output device */
#ifdef CM_USING_CONSOLE
    os_console_set_device(OS_CONSOLE_DEVICE_NAME);
#endif

    /* Heap initialization */
#if defined(OS_USING_HEAP)
    os_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
    mt_noncached_heap_init();
#endif

    /* Pin driver initialization is open by default */
#ifdef OS_USING_PIN
    cm_hw_pin_init();
#endif

    extern int system_init(void);
    system_init();

    /* Board underlying hardware initialization */

    os_board_auto_init();

#endif
}
