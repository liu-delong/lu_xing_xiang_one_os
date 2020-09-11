#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_irq.h>
#include <os_clock.h>
#include <os_hw.h>
#include <os_memory.h>
#include "drv_common.h"
#include "board.h"
#include "drv_uart.h"
#ifdef OS_USING_FAL
#include "fal_cfg.h"
#endif
#ifdef OS_USING_SHELL
#include "shell.h"
#endif

#include "drv_gpio.h"

/* SysTick configuration */
void os_hw_systick_init(void)
{
    SysTick_Config(SystemCoreClock / OS_TICK_PER_SECOND);

    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    NVIC_SetPriority(SysTick_IRQn, 0);
}

__IO uint32_t uwTick;
OS_WEAK void HAL_IncTick(void)
{
    uwTick += 1;
}

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    HAL_IncTick();
    os_tick_increase();

    /* leave interrupt */
    os_interrupt_leave();
}

uint32_t HAL_GetTick(void)
{
    return os_tick_get() * 1000 / OS_TICK_PER_SECOND;
}

void HAL_SuspendTick(void)
{
}

void HAL_ResumeTick(void)
{
}

void HAL_Delay(__IO uint32_t Delay)
{
}

/* re-implement tick interface for STM32 HAL */
uint8_t HAL_InitTick(uint32_t TickPriority)
{
    /* Return function status */
    return 0;// HAL_OK       = 0x00U, compare with stm32
}

/**
  * @brief  This functiokn is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char *s, int num)
{
    /* USER CODE BEGIN Error_Handler */
    /* User can add his own implementation to report the HAL error return state */
    while(1)
    {
    }
    /* USER CODE END Error_Handler */
}

/**
 * This function will delay for some us.
 *
 * @param us the delay time of us
 */
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

/**
 * This function will initial STM32 board.
 */

void os_hw_board_init()
{
#ifdef OS_USE_BOOTLOADER
    SCB->VTOR = OS_APP_PART_ADDR;
#endif

#ifdef SCB_EnableICache
    /* Enable I-Cache---------------------------------------------------------*/
    SCB_EnableICache();
#endif

#ifdef SCB_EnableDCache
    /* Enable D-Cache---------------------------------------------------------*/
    SCB_EnableDCache();
#endif

    /* System initialization */
    SystemInit();

    os_hw_systick_init();

    /* Pin driver initialization is open by default */
#ifdef OS_USING_PIN
    os_hw_pin_init();
#endif

#ifdef BSP_USING_SDRAM
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

#ifdef OS_USING_RTT
    os_hw_rtt_init();
#endif

    /* Set the shell console output device */
#ifdef OS_USING_CONSOLE
    os_console_set_device(OS_CONSOLE_DEVICE_NAME);
#endif

    os_board_auto_init();
}

#ifdef OS_USING_SHELL
static os_err_t reset_mcu(os_int32_t argc, char **argv)
{
    NVIC_SystemReset();

    return OS_EOK;
}
SH_CMD_EXPORT(reset, reset_mcu, "Reset the mcu");
#endif
