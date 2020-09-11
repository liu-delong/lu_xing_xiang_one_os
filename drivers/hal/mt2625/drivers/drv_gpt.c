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
 * @file        drv_hwtimer.c
 *
 * @brief       This file implements timer driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_irq.h>
#include <os_memory.h>
#include <timer/timer.h>

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#endif

#ifdef OS_USING_CLOCKEVENT
#include <timer/clockevent.h>
#endif

#include <drv_gpt.h>
#include "hal_gpt.h"

#ifdef OS_USING_PWM
#include "drv_pwm.h"
#endif

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.hwtimer"
#include <drv_log.h>


#define TIMER_MODE_TIM 0x00

static os_list_node_t stm32_timer_list = OS_LIST_INIT(stm32_timer_list);

void HAL_TIM_PeriodElapsedCallback(void *tim)
{
    struct stm32_timer *timer;
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)tim;

    os_list_for_each_entry(timer, &stm32_timer_list, struct stm32_timer, list)
    {
        if (timer->handle == htim)
        {
            os_clockevent_isr((os_clockevent_t *)timer);
            return;
        }
    }
}

static os_uint64_t stm32_clocksourcetimer_read(void *clock)
{
    uint32_t count;
    hal_gpt_status_t ret;

    ret = hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count);
    if(HAL_GPT_STATUS_OK != ret) {
        LOG_EXT_E("get run error, ret[%d]", ret);
        return 0;
    }

    return count;
}

#ifdef OS_USING_CLOCKEVENT
static void stm32_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct stm32_timer *timer;
    hal_gpt_status_t ret;
    TIM_HandleTypeDef *tim;

    timer = (struct stm32_timer *)ce;
    tim = timer->handle;

    hal_gpt_running_status_t running_status;
    
    /* Get the running status to check if this port is in use or not. */
    ret = hal_gpt_get_running_status(tim->gpt_port,&running_status);
    if ((HAL_GPT_STATUS_OK != ret) || (running_status != HAL_GPT_STOPPED)) {
        LOG_EXT_E("gpt_port[%d], ret[%d]", tim->gpt_port, ret);
        return;
    }

    /* Set the GPT base environment.*/
    ret = hal_gpt_init(tim->gpt_port);
    if(HAL_GPT_STATUS_OK != ret) {
        LOG_EXT_E("gpt_port[%d], ret[%d]", tim->gpt_port, ret);
        return;
    }

    ret = hal_gpt_register_callback(tim->gpt_port, HAL_TIM_PeriodElapsedCallback, (void *)tim);
    ret = hal_gpt_start_timer_us(tim->gpt_port, count, HAL_GPT_TIMER_TYPE_REPEAT);
    if(HAL_GPT_STATUS_OK != ret) {
        LOG_EXT_E("gpt_port[%d], ret[%d]", tim->gpt_port, ret);
        return;
    }
    
}

static void stm32_timer_stop(os_clockevent_t *ce)
{
    struct stm32_timer *timer;
    hal_gpt_status_t ret;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct stm32_timer *)ce;

    ret = hal_gpt_stop_timer(timer->handle->gpt_port);
    ret = hal_gpt_deinit(timer->handle->gpt_port);
    if (HAL_GPT_STATUS_OK != ret) {
        LOG_EXT_E("gpt_port[%d], ret[%d]", timer->handle->gpt_port, ret);
        return;
    }

}

static os_uint64_t stm32_timer_read(void *clock)
{
    
    return 0;
}


static const struct os_clockevent_ops stm32_tim_ops =
{
    .start = stm32_timer_start,
    .stop  = stm32_timer_stop,
    .read  = stm32_timer_read,
};
#endif

/**
 ***********************************************************************************************************************
 * @brief           stm32_tim_probe:probe timer device.
 *
 * @param[in]       none
 *
 * @return          Return timer probe status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int stm32_tim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct stm32_timer *timer;
    TIM_HandleTypeDef  *tim;
    
    timer = os_calloc(1, sizeof(struct stm32_timer));
    OS_ASSERT(timer);

    tim = (TIM_HandleTypeDef *)dev->info;
    timer->handle = tim;

#ifdef OS_USING_CLOCKSOURCE
    if (os_clocksource_best() == OS_NULL)
    {
    
        timer->clock.cs.rating  = 320;
        timer->clock.cs.freq    = 1000*1000;
        timer->clock.cs.mask    = 0xffffffffull;
        timer->clock.cs.read    = stm32_clocksourcetimer_read;
        os_clocksource_register(dev->name, &timer->clock.cs);
    }
    else
#endif
    {
#ifdef OS_USING_CLOCKEVENT
        timer->clock.ce.rating  = 240;
        timer->clock.ce.freq    = 1000*1000;
        timer->clock.ce.mask    = 0xffffffffull;
        
        timer->clock.ce.prescaler_mask = 0;
        timer->clock.ce.prescaler_bits = 0;
        
        timer->clock.ce.count_mask = 0xfffffffful;
        timer->clock.ce.count_bits = 32;

        timer->clock.ce.feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

        timer->clock.ce.min_nsec = NSEC_PER_SEC / timer->clock.ce.freq;

        timer->clock.ce.ops     = &stm32_tim_ops;
        os_clockevent_register(dev->name, &timer->clock.ce);
#endif
    }

    os_list_add(&stm32_timer_list, &timer->list);

    return OS_EOK;
}

OS_DRIVER_INFO stm32_tim_driver = {
    .name   = "TIM_HandleTypeDef",
    .probe  = stm32_tim_probe,
};

OS_DRIVER_DEFINE(stm32_tim_driver, "1");

