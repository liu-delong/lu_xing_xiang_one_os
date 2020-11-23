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
 * @file        drv_dac.c
 *
 * @brief       This file implements adc driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <string.h>
#include <os_memory.h>
#include <misc/dac.h>
#include "driver/dac.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.dac"
#include <drv_log.h>

struct esp32_dac {
    os_dac_device_t dac;
};

static os_uint32_t esp32_dac_channel(os_uint32_t channel)
{
    os_uint32_t ch;

    switch (channel)
    {
    case 1:
        ch = DAC_CHANNEL_1;
        break;
    case 2:
        ch = DAC_CHANNEL_2;
        break;
    default:
        ch = 0xFFFF;
        break;
    }

    return ch;
}

static os_err_t esp32_dac_enabled(os_dac_device_t *dac, os_uint32_t channel, os_bool_t enabled)
{
    dac_channel_t ch;

    OS_ASSERT(dac != OS_NULL);

    ch = esp32_dac_channel(channel);
    if (ch == 0xFFFF)
    {
        LOG_EXT_E("dac channel %d cannot find!\n", channel);
        return OS_ERROR;
    }

    if (enabled)
    {
        dac_output_enable(ch);
    }
    else
    {
        dac_output_disable(ch);
    }

    return OS_EOK;
}

static os_err_t esp32_dac_write(os_dac_device_t *dac, os_uint32_t channel, os_uint32_t value)
{
    dac_channel_t ch;

    OS_ASSERT(dac != OS_NULL);

    ch = esp32_dac_channel(channel);
    if (ch == 0xFFFF)
    {
        LOG_EXT_E("dac channel %d cannot find!\n", channel);
        return OS_ERROR;
    }

    dac_output_voltage(ch, value);

    return OS_EOK;
}

static const struct os_dac_ops esp32_dac_ops = {
    .enabled = esp32_dac_enabled,
    .write   = esp32_dac_write,
};

static int esp32_dac_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct esp32_dac *dac;

    dac = os_calloc(1, sizeof(struct esp32_dac));

    OS_ASSERT(dac);
    
    dac->dac.max_value = (1UL << 8) - 1;    /* 8bit */
    dac->dac.ref_low   = 0;                 /* ref 0 - 3.3v */
    dac->dac.ref_hight = 3300;
    dac->dac.ops       = &esp32_dac_ops;

    if (os_dac_register(&dac->dac, dev->name, OS_NULL) != OS_EOK)
    {
        LOG_EXT_E("dac device register failed.");
        return OS_ERROR;
    }
    LOG_EXT_D("dac device register success.");
    return OS_EOK;
}

OS_DRIVER_INFO esp32_dac_driver = {
    .name   = "ESP32_DAC_DRIVER",
    .probe  = esp32_dac_probe,
};

OS_DRIVER_DEFINE(esp32_dac_driver, "1");

