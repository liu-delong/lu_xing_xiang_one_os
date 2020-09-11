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
 * @file        drv_pwm.c
 *
 * @brief       This file implements PWM driver for mt2625.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
//#include <drv_hwtimer.h>
#include <string.h>
#include <os_memory.h>
#include "drv_pwm.h"
#include "hal_pwm.h"
#include "hal_platform.h"
#include "hal_pwm_internal.h"
#include "hal_clock.h"

#define DRV_EXT_LVL    DBG_EXT_INFO
#define DRV_EXT_TAG    "drv.pwm"
#include <drv_log.h>

static os_list_node_t mt_pwm_list = OS_LIST_INIT(mt_pwm_list);

 static os_err_t  pwm_gpio_enable(struct mt_pwm *mt_pwm)
 {
    PWM_HandleTypeDef *mt2625_pwm;

    OS_ASSERT(mt_pwm);
    mt2625_pwm = mt_pwm->mt_pwm;

    if (mt2625_pwm->pwmx == HAL_PWM_0)
    {
        #ifdef  GPIO_0_PWM0
        hal_gpio_init(HAL_GPIO_0);
        hal_pinmux_set_function(HAL_GPIO_0, HAL_GPIO_0_PWM0);
        #endif
        
        #ifdef  GPIO_1_PWM0
        hal_gpio_init(HAL_GPIO_1);
        hal_pinmux_set_function(HAL_GPIO_1, HAL_GPIO_1_PWM0);
        #endif

        #ifdef  GPIO_4_PWM0
        hal_gpio_init(HAL_GPIO_4);
        hal_pinmux_set_function(HAL_GPIO_4, HAL_GPIO_4_PWM0);
        #endif

    }
    else if (mt2625_pwm->pwmx == HAL_PWM_1)
    {
        #ifdef  GPIO_7_PWM1
        hal_gpio_init(HAL_GPIO_7);
        hal_pinmux_set_function(HAL_GPIO_7, HAL_GPIO_7_PWM1);
        #endif

        #ifdef  GPIO_8_PWM1
        hal_gpio_init(HAL_GPIO_8);
        hal_pinmux_set_function(HAL_GPIO_8, HAL_GPIO_8_PWM1);
        #endif

    }
    else if (mt2625_pwm->pwmx == HAL_PWM_2)
    {
        #ifdef  GPIO_13_PWM2
        hal_gpio_init(HAL_GPIO_13);
        hal_pinmux_set_function(HAL_GPIO_13, HAL_GPIO_13_PWM2);
        #endif

        #ifdef  GPIO_15_PWM2
        hal_gpio_init(HAL_GPIO_15);
        hal_pinmux_set_function(HAL_GPIO_15, HAL_GPIO_15_PWM2);
        #endif

    }
    else if (mt2625_pwm->pwmx == HAL_PWM_3)
    {
        #ifdef  GPIO_20_PWM3
        hal_gpio_init(HAL_GPIO_20);
        hal_pinmux_set_function(HAL_GPIO_20, HAL_GPIO_20_PWM3);
        #endif

        #ifdef  GPIO_26_PWM3
        hal_gpio_init(HAL_GPIO_26);
        hal_pinmux_set_function(HAL_GPIO_26, HAL_GPIO_26_PWM3);
        #endif
        
        #ifdef  GPIO_32_PWM3
        hal_gpio_init(HAL_GPIO_32);
        hal_pinmux_set_function(HAL_GPIO_32, HAL_GPIO_32_PWM3);
        #endif
    }
    else
    {
        LOG_EXT_E("pwm para error, pwmx[%d]", mt2625_pwm->pwmx);
        return OS_ENOSYS;
    }
    
    return OS_EOK;
 }

static os_err_t mt_pwm_enabled(struct os_pwm_device *dev, os_uint32_t channel, os_bool_t enable)
{
    hal_pwm_status_t ret;
    struct mt_pwm *mt_pwm;

    mt_pwm = os_container_of(dev, struct mt_pwm, pwm);
    
    if (enable)
    {
         pwm_gpio_enable(mt_pwm);
         ret = hal_pwm_start(mt_pwm->mt_pwm->pwmx);
         if(HAL_PWM_STATUS_OK != ret) {
            LOG_EXT_E("hal_pwm_start failed, ret[%d]", ret);
            return OS_ENOSYS;
        }
    }
    else
    {
        ret = hal_pwm_stop(mt_pwm->mt_pwm->pwmx);
        if(HAL_PWM_STATUS_OK != ret) {
            LOG_EXT_E("hal_pwm_stop failed, ret[%d]", ret);
            return OS_ENOSYS;
        }
    }

    return OS_EOK;
}

static os_err_t mt_freq_check(PWM_HandleTypeDef *mt_pwm, uint32_t frequency)
{
    PWM_HandleTypeDef *mt2625_pwm = mt_pwm;
    uint32_t smallest_frequency;
    uint32_t clock;

    if (((hal_pwm_source_clock_t)mt2625_pwm->pwm_clock) == HAL_PWM_CLOCK_13MHZ)
    {
        clock = PWM_CLOCK_SEL2;
    }
    else if (((hal_pwm_source_clock_t)mt2625_pwm->pwm_clock) == HAL_PWM_CLOCK_32KHZ)
    {
        clock = PWM_CLOCK_SEL1;
    }
    else
    {
        LOG_EXT_E("mt_freq_check clock error, pwm_clock[%d]", mt2625_pwm->pwm_clock);
        return OS_ERROR;
    }
    
    smallest_frequency = clock / PWM_MAX_COUNT;

    if ((frequency < smallest_frequency) || (frequency > PWM_MAX_COUNT))
    {
        LOG_EXT_E("mt_freq_check frequency error, frequency[%d], clock[%d]", frequency, clock);
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t mt_pwm_set_period(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t nsec)
{
    uint32_t frequency = 0;
    PWM_HandleTypeDef *mt_pwm;
    struct mt_pwm *st_pwm;
    hal_pwm_status_t ret;
    os_err_t ret2;

    st_pwm = os_container_of(dev, struct mt_pwm, pwm);
    mt_pwm = st_pwm->mt_pwm;
    mt_pwm->pwm_period = nsec;
    
    frequency = (os_uint64_t)1000*1000*1000/nsec;

    ret2 = mt_freq_check(mt_pwm, frequency);
    if (ret2 != OS_EOK)
    {
        LOG_EXT_E("pwm_set_frequency failed, ret2[%d]", ret2);
        return OS_ERROR;
    }
    ret = hal_pwm_set_frequency(mt_pwm->pwmx, frequency, (uint32_t *)&mt_pwm->total_count);
    if(HAL_PWM_STATUS_OK != ret) {
        LOG_EXT_E("pwm_set_frequency failed, ret[%d]", ret);
        return OS_ENOSYS;
    }

    return OS_EOK;
}

static os_err_t mt_pwm_set_pulse(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t buffer)
{
    os_uint32_t duty_cycle = 0;
    os_uint32_t duty_ratio = 0;
    PWM_HandleTypeDef *mt_pwm;
    hal_pwm_status_t ret;

    struct mt_pwm *st_pwm;

    st_pwm = os_container_of(dev, struct mt_pwm, pwm);
    mt_pwm = st_pwm->mt_pwm;

    duty_ratio = (100*buffer)/mt_pwm->pwm_period;
    /* duty_cycle is calcauted as a product of application's duty_ratio and hardware's total_count. */
    duty_cycle = (mt_pwm->total_count * duty_ratio)/100; 

    ret = hal_pwm_set_duty_cycle(mt_pwm->pwmx, duty_cycle);
    if(HAL_PWM_STATUS_OK != ret) {
        LOG_EXT_E("pwm_set_duty_cycler failed, ret[%d]", ret);
        return OS_ENOSYS;
    }

    return OS_EOK;
}

static const struct os_pwm_ops mt_pwm_ops =
{
    .enabled = mt_pwm_enabled,
    .set_period = mt_pwm_set_period,
    .set_pulse = mt_pwm_set_pulse,
    .control  = OS_NULL,
};


static os_err_t  mt_pwm_clock_init(PWM_HandleTypeDef *mt_pwm)
{
    PWM_HandleTypeDef *mt2625_pwm = mt_pwm;
    hal_pwm_status_t ret;

    ret = hal_pwm_init(mt2625_pwm->pwmx, (hal_pwm_source_clock_t)mt2625_pwm->pwm_clock);
    if(HAL_PWM_STATUS_OK != ret) {
        LOG_EXT_E("pwm_set_duty_cycler failed, ret[%d]", ret);
        return OS_ENOSYS;
    }
    
    return OS_EOK;
}

static int mt_pwm_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    os_err_t    result  = 0;
    os_base_t   level;

    struct mt_pwm *mt_pwm = os_calloc(1, sizeof(struct mt_pwm));

    OS_ASSERT(mt_pwm);

    mt_pwm->mt_pwm = (PWM_HandleTypeDef *)dev->info;
    
    result = mt_pwm_clock_init(mt_pwm->mt_pwm);
    if (result != OS_EOK)
    {
        LOG_EXT_E("pwm_set_duty_cycler failed, result[%d]", result);
        return OS_ENOSYS;
    }

    struct os_pwm_device *pwm = &mt_pwm->pwm;

    pwm->ops   = &mt_pwm_ops;

    level = os_hw_interrupt_disable();
    os_list_add_tail(&mt_pwm_list, &mt_pwm->list);
    os_hw_interrupt_enable(level);
    
    result = os_device_pwm_register(pwm, dev->name);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO mt_pwm_driver = {
    .name   = "PWM_HandleTypeDef",
    .probe  = mt_pwm_probe,
};

OS_DRIVER_DEFINE(mt_pwm_driver, "1");


