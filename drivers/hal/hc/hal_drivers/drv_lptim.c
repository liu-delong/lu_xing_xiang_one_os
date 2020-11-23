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
 * @file        drv_lptim.c
 *
 * @brief       This file implements low power timer driver for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_task.h>
#include <os_irq.h>
#include <os_memory.h>
#include <timer/timer.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>

#include <drv_log.h>

#include <hc_lptim.h>

struct hc32_lptimer {

	os_clockevent_t      ce;

	os_uint32_t freq;

	const char *name;

	M0P_LPTIMER_TypeDef *base; 

	os_uint8_t prescale;
};

struct hc32_lptimer lptimers[] = {

#ifdef  BSP_USING_LPTIM0
	{
		.name = "lptim1",
		.base = M0P_LPTIMER0,
		.prescale = 5,
	},
#endif

#ifdef  BSP_USING_LPTIM1
	{
		.name = "lptim2",
		.base = M0P_LPTIMER1,
		.prescale = 5,
	},
#endif

};

#ifdef  BSP_USING_LPTIM0
void LpTim0_IRQHandler(void)
{
	if (TRUE == Lptim_GetItStatus(M0P_LPTIMER0))
	{
		Lptim_ClrItStatus(M0P_LPTIMER0);

		os_clockevent_isr((os_clockevent_t *)(&lptimers[0].ce));
	}
}
#endif

#ifdef  BSP_USING_LPTIM1
void LpTim1_IRQHandler(void)
{
	if (TRUE == Lptim_GetItStatus(M0P_LPTIMER1))
	{
		Lptim_ClrItStatus(M0P_LPTIMER1);

		os_clockevent_isr((os_clockevent_t *)(&lptimers[1].ce));
	}
}
#endif

static void hc32_lptimer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
	struct hc32_lptimer *timer;	
	stc_lptim_cfg_t    stcLptCfg;

	timer = (struct hc32_lptimer *)ce;
	DDL_ZERO_STRUCT(stcLptCfg);

	stcLptCfg.enGate   = LptimGateLow;
	stcLptCfg.enGatep  = LptimGatePLow;
	stcLptCfg.enTcksel = LptimRcl;
	stcLptCfg.enTogen  = LptimTogEnLow;
	stcLptCfg.enCt	   = LptimTimerFun;
	stcLptCfg.enMd	   = LptimMode1;
	stcLptCfg.u16Arr   = timer->ce.count_mask + 1 - (count & timer->ce.count_mask);

	Lptim_Init(timer->base, &stcLptCfg);	

	//prescale
	timer->base->CR_f.PRS = timer->prescale;

	Lptim_ClrItStatus(timer->base); 
	Lptim_ConfIt(timer->base, TRUE);

	Lptim_Cmd(timer->base, TRUE);	

}

static void hc32_lptimer_stop(os_clockevent_t *ce)
{
	struct hc32_lptimer *timer;

	timer = (struct hc32_lptimer *)ce;

	Lptim_Cmd(timer->base, FALSE);
}

os_uint64_t hc32_lptimer_read(void *clock)
{
	struct hc32_lptimer *timer=OS_NULL;
	os_uint32_t cnt=0,arr=0;


	timer = (struct hc32_lptimer *)clock;

	cnt = (os_uint32_t)(timer->base->CNT_f.CNT);
	arr = (os_uint32_t)(timer->base->ARR_f.ARR);

	if(cnt < arr)
		return (os_uint64_t)(timer->ce.mask-arr);
	else
		return (os_uint64_t)(cnt-arr);
}

static const struct os_clockevent_ops hc32_lptim_ops =
{
	.start = hc32_lptimer_start,
	.stop  = hc32_lptimer_stop,
	.read  = hc32_lptimer_read,
};


static void __os_hw_lptim_init(void)
{

	Sysctrl_SetPeripheralGate(SysctrlPeripheralLpTim0, TRUE);
	Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);	
	EnableNvic(LPTIM_0_1_IRQn, IrqLevel3, TRUE);
}

/**
 ***********************************************************************************************************************
 * @brief           os_hw_lptim_init:init lptimer device.
 *
 * @param[in]       none
 *
 * @return          Return timer probe status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int os_hw_lptim_init(void)
{

	os_uint32_t idx = 0;

	__os_hw_lptim_init();

	for(idx=0;idx<(sizeof(lptimers)/sizeof(lptimers[0]));idx++)
	{

		lptimers[idx].freq = 32768/(1<<lptimers[idx].prescale);

		lptimers[idx].ce.rating  = 50;
		lptimers[idx].ce.freq    = lptimers[idx].freq;
		lptimers[idx].ce.mask    = 0xfffful;

		lptimers[idx].ce.prescaler_mask = 0;
		lptimers[idx].ce.prescaler_bits = 0;

		lptimers[idx].ce.count_mask = 0xfffffffful;
		lptimers[idx].ce.count_bits = 32;

		lptimers[idx].ce.feature  = OS_CLOCKEVENT_FEATURE_PERIOD;

		lptimers[idx].ce.min_nsec = NSEC_PER_SEC / lptimers[idx].ce.freq;

		lptimers[idx].ce.ops     = &hc32_lptim_ops;

		os_clockevent_register(lptimers[idx].name, &lptimers[idx].ce);

	}

	return OS_EOK;
}


OS_BOARD_INIT(os_hw_lptim_init);

