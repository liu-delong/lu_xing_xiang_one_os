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
 * @file        drv_soft_i2c.c
 *
 * @brief       This file implements soft I2C for FM33A0xx.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "drv_soft_i2c.h"
#include "drv_cfg.h"

#ifdef OS_USING_I2C

#define LOG_TAG "drv.i2c"
#include <drv_log.h>
#include <os_dbg_ext.h>

#if !defined(BSP_USING_I2C1)
#error "Please define at least one BSP_USING_I2Cx"
#endif

static const struct fm_soft_i2c_config soft_i2c_config[] =
{
#ifdef BSP_USING_I2C1
    I2C1_BUS_CONFIG,
#endif
};

static struct fm_i2c i2c_obj[sizeof(soft_i2c_config) / sizeof(soft_i2c_config[0])];

static void fm_i2c_gpio_init(struct fm_i2c *i2c)
{
    struct fm_soft_i2c_config *cfg = (struct fm_soft_i2c_config *)i2c->ops.data;

    os_pin_mode(cfg->scl, PIN_MODE_OUTPUT_OD);
    os_pin_mode(cfg->sda, PIN_MODE_OUTPUT_OD);

    os_pin_write(cfg->scl, PIN_HIGH);
    os_pin_write(cfg->sda, PIN_HIGH);
}

/**
 ***********************************************************************************************************************
 * @brief           Set the SDA pin status.
 *
 * @param[in]       data            Soft i2c config.
 * @param[in]       state           Pin status.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void fm_set_sda(void *data, os_int32_t state)
{
    struct fm_soft_i2c_config *cfg = (struct fm_soft_i2c_config *)data;
    if (state)
    {
        os_pin_write(cfg->sda, PIN_HIGH);
    }
    else
    {
        os_pin_write(cfg->sda, PIN_LOW);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Set the SCL pin status.
 *
 * @param[in]       data            Soft i2c config.
 * @param[in]       state           Pin status.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void fm_set_scl(void *data, os_int32_t state)
{
    struct fm_soft_i2c_config *cfg = (struct fm_soft_i2c_config *)data;
    if (state)
    {
        os_pin_write(cfg->scl, PIN_HIGH);
    }
    else
    {
        os_pin_write(cfg->scl, PIN_LOW);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Get the SDA pin status.
 *
 * @param[in]       data             Soft i2c config.
 *
 * @return          Pin status
 ***********************************************************************************************************************
 */
static os_int32_t fm_get_sda(void *data)
{
    struct fm_soft_i2c_config *cfg = (struct fm_soft_i2c_config *)data;
    return os_pin_read(cfg->sda);
}

/**
 ***********************************************************************************************************************
 * @brief           Get the SCL pin status.
 *
 * @param[in]       data             Soft i2c config.
 *
 * @return          Pin status
 ***********************************************************************************************************************
 */
static os_int32_t fm_get_scl(void *data)
{
    struct fm_soft_i2c_config *cfg = (struct fm_soft_i2c_config *)data;
    return os_pin_read(cfg->scl);
}

/**
 ***********************************************************************************************************************
 * @brief           Delay.
 *
 * @param[in]       us             Delay time/us.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void fm_udelay(os_uint32_t us)
{
    os_uint32_t ticks;
    os_uint32_t told, tnow, tcnt = 0;
    os_uint32_t reload = SysTick->LOAD;

    ticks = us * reload / (1000000 / OS_TICK_PER_SECOND);
    told  = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}

static const struct os_i2c_bit_ops fm_bit_ops_default =
{
    .data     = OS_NULL,
    .set_sda  = fm_set_sda,
    .set_scl  = fm_set_scl,
    .get_sda  = fm_get_sda,
    .get_scl  = fm_get_scl,
    .udelay   = fm_udelay,
    .delay_us = 1,
    .timeout  = 100
};

/**
 ***********************************************************************************************************************
 * @brief           Unlock i2c.
 *
 * @param[in]       cfg             Soft i2c config.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
static os_err_t fm_i2c_bus_unlock(const struct fm_soft_i2c_config *cfg)
{
    os_int32_t i = 0;

    if (PIN_LOW == os_pin_read(cfg->sda))
    {
        while (i++ < 9)
        {
            os_pin_write(cfg->scl, PIN_HIGH);
            fm_udelay(100);
            os_pin_write(cfg->scl, PIN_LOW);
            fm_udelay(100);
        }
    }
    if (PIN_LOW == os_pin_read(cfg->sda))
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

int os_hw_i2c_init(void)
{
    os_size_t obj_num = sizeof(i2c_obj) / sizeof(struct fm_i2c);
    os_err_t  result;

    for (int i = 0; i < obj_num; i++)
    {
        i2c_obj[i].ops           = fm_bit_ops_default;
        i2c_obj[i].ops.data      = (void *)&soft_i2c_config[i];
        i2c_obj[i].i2c2_bus.priv = &i2c_obj[i].ops;
        fm_i2c_gpio_init(&i2c_obj[i]);
        result = os_i2c_bit_add_bus(&i2c_obj[i].i2c2_bus, soft_i2c_config[i].bus_name);
        OS_ASSERT(result == OS_EOK);
        fm_i2c_bus_unlock(&soft_i2c_config[i]);

        LOG_EXT_D("software simulation %s init done, pin scl: %d, pin sda %d",
                  soft_i2c_config[i].bus_name,
                  soft_i2c_config[i].scl,
                  soft_i2c_config[i].sda);
    }

    return OS_EOK;
}
OS_BOARD_INIT(os_hw_i2c_init);

#endif /* OS_USING_SOFT_I2C */
