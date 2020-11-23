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
 * @file        drv_lpmgr.c
 *
 * @brief       This file implements low power manager for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <lpmgr.h>
#include <board.h>
#include <timer/clockevent.h>
#include <hc_lpm.h>
#include <drv_common.h>


os_clockevent_t *lpce;

/*for hc32 chip must enable interrupt while in sleep mode*/
static os_uint32_t os_irq_level = ~0u;

os_uint32_t lpmgr_enter_critical(os_uint8_t sleep_mode)
{
	os_uint32_t level;
	level = os_hw_interrupt_disable();
	os_irq_level = level;
	return level;
}

void lpmgr_exit_critical(os_uint32_t ctx, os_uint8_t sleep_mode)
{
	if(ctx == ~0u)
		os_hw_interrupt_enable(os_irq_level);
	else
		os_hw_interrupt_enable(ctx);
}

static void uart_console_reconfig(void)
{
	struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

	os_device_control(os_console_get_device(), OS_DEVICE_CTRL_CONFIG, &config);
}
/**
 ***********************************************************************************************************************
 * @brief           Put device into sleep mode.
 *
 * @param[in]       lpm             Low power manager structure.
 * @param[in]       mode            Low power mode.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static int sleep(struct lpmgr *lpm, uint8_t mode)
{

	switch (mode)
	{
		case SYS_SLEEP_MODE_NONE:
			break;

		case SYS_SLEEP_MODE_IDLE:
			break;

		case SYS_SLEEP_MODE_LIGHT:
			lpmgr_exit_critical(~0u,mode);
			Lpm_GotoSleep(FALSE);
			lpmgr_enter_critical(mode);
			break;

		case SYS_SLEEP_MODE_DEEP:
			lpmgr_exit_critical(~0u,mode);
			Lpm_GotoDeepSleep(FALSE);
			lpmgr_enter_critical(mode);
			break;

		case SYS_SLEEP_MODE_STANDBY:
			break;

		case SYS_SLEEP_MODE_SHUTDOWN:
			break;

		default:
			OS_ASSERT(0);
			break;
	}
	return OS_EOK;
}

static uint8_t run_speed[SYS_RUN_MODE_MAX][2] = {
	{48, 0},
	{24, 1},
	{16, 2},
	{4, 3},
};

static void run(struct lpmgr *lpm, uint8_t mode)
{
	static uint8_t last_mode = SYS_DEFAULT_RUN_MODE;
	static char *run_str[] = SYS_RUN_MODE_NAMES;

	if (mode >= SYS_RUN_MODE_MAX)
	{
		os_kprintf("invalid mode: %d\n", mode);
		return;
	}

	if (mode == last_mode)
		return;

	last_mode = mode;

	/* Set frequency according to mode */
	switch (mode)
	{
		case SYS_RUN_MODE_HIGH_SPEED:
			SystemClkInit_PLL48M_byRCH();
			break;
		case SYS_RUN_MODE_NORMAL_SPEED:
			SystemClkInit_RCH(SysctrlRchFreq24MHz);
			break;
		case SYS_RUN_MODE_MEDIUM_SPEED:
			SystemClkInit_RCH(SysctrlRchFreq16MHz);
			break;
		case SYS_RUN_MODE_LOW_SPEED:
			SystemClkInit_RCH(SysctrlRchFreq4MHz);
			break;
		default:
			break;
	}

	uart_console_reconfig();

	os_hw_systick_init();

	os_kprintf("switch to %s mode, frequency = %d MHz\n", run_str[mode], run_speed[mode][0]);
}

/**
 ***********************************************************************************************************************
 * @brief           Caculate the PM tick from OS tick.
 *
 * @param[in]       tick            OS tick.
 *
 * @return          PM tick.
 ***********************************************************************************************************************
 */
static os_tick_t lpm_tick_from_os_tick(os_tick_t tick)
{
	os_uint32_t freq = lpce->freq;

	return (freq * tick / OS_TICK_PER_SECOND);
}

/**
 ***********************************************************************************************************************
 * @brief           Caculate the OS tick from PM tick.
 *
 * @param[in]       tick            PM tick.
 *
 * @return          OS tick.
 ***********************************************************************************************************************
 */
static os_tick_t os_tick_from_lpm_tick(os_uint32_t tick)
{
	static os_uint32_t os_tick_remain = 0;
	os_uint32_t        ret, freq;

	freq = lpce->freq;
	ret  = (tick * OS_TICK_PER_SECOND + os_tick_remain) / freq;

	os_tick_remain += (tick * OS_TICK_PER_SECOND);
	os_tick_remain %= freq;

	return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Start the timer of pm.
 *
 * @param[in]       lpm             Low power manager structure.
 * @param[in]       timeout         How many OS ticks that MCU can sleep.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void lpm_timer_start(struct lpmgr *lpm, os_uint32_t timeout)
{
	OS_ASSERT(lpm != OS_NULL);
	OS_ASSERT(timeout > 0);

	if (timeout != OS_TICK_MAX)
	{
		/* Convert OS Tick to pmtimer timeout value */
		timeout = lpm_tick_from_os_tick(timeout);
		if (timeout > lpce->mask)
		{
			timeout = lpce->mask;
		}

		/* Enter LPM_TIMER_MODE */
		os_clockevent_start_oneshot(lpce, NSEC_PER_SEC * timeout / lpce->freq);
	}
}

/**
 ***********************************************************************************************************************
 * @brief           Stop the timer of pm.
 *
 * @param[in]       lpm             Low power manager structure.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void lpm_timer_stop(struct lpmgr *lpm)
{
	OS_ASSERT(lpm != OS_NULL);

	/* Reset pmtimer status */
	os_clockevent_stop(lpce);
}

/**
 ***********************************************************************************************************************
 * @brief           Calculate how many OS ticks that MCU has suspended.
 *
 * @param[in]       lpm             Low power manager structure.
 *
 * @return          OS ticks.
 ***********************************************************************************************************************
 */
static os_tick_t lpm_timer_get_tick(struct lpmgr *lpm)
{
	os_uint32_t timer_tick;

	OS_ASSERT(lpm != OS_NULL);

	timer_tick = os_clockevent_read(lpce);

	return os_tick_from_lpm_tick(timer_tick);
}

/**
 ***********************************************************************************************************************
 * @brief           Initialise low power manager.
 *
 * @param[in]		None.
 *
 * @return          0.
 ***********************************************************************************************************************
 */
int drv_lpmgr_hw_init(void)
{
	static const struct os_lpmgr_ops s_lpmgr_ops = {
		sleep,
		run,
		lpm_timer_start,
		lpm_timer_stop,
		lpm_timer_get_tick
	};

	os_uint32_t timer_mask = 0;

	lpce = (os_clockevent_t *)os_device_find("lptim1");
	OS_ASSERT(lpce != OS_NULL);
	OS_ASSERT(os_device_open((os_device_t *)lpce, 0) == OS_EOK);

	/* Initialize timer mask */
	timer_mask = 1UL << SYS_SLEEP_MODE_DEEP;

	/* Initialize system lpmgr module */
	os_lpmgr_init(&s_lpmgr_ops, timer_mask, OS_NULL);

	return 0;
}

OS_PREV_INIT(drv_lpmgr_hw_init);
