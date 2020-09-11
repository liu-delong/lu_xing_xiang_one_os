/*
 * Copyright (c) 2012-2020, CMCC IOT
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */


#include "os_kernel.h"
#include "hal_feature_config.h"

#if defined(BSP_USING_I2C) && defined(OS_USING_I2C) && defined(HAL_I2C_MASTER_MODULE_ENABLED)

#include "drv_i2c.h"
#include "hal_platform.h"
#include "hal_pinmux_define.h"
#include "hal_gpio.h"
#include "hal_i2c_master.h"

#define DBG_EXT_TAG                        "drv.i2c"
#define DBG_EXT_LVL                        DBG_ERROR
#include <os_dbg_ext.h>

struct mt2625_i2c_bus
{
    struct os_i2c_bus_device i2c_dev;
    hal_i2c_port_t i2c_port;
    hal_i2c_config_t i2c_config;
    char *name;
};

static os_size_t i2c_mst_xfer(struct os_i2c_bus_device *bus,
                                struct os_i2c_msg msgs[],
                                os_uint32_t num)
{
    OS_ASSERT(bus != OS_NULL);
    uint32_t i;
    struct mt2625_i2c_bus *device = (struct mt2625_i2c_bus *)bus;

    LOG_EXT_D("i2c master transfer...");

    for (i = 0; i < num; i++)
    {
        if (msgs[i].flags == OS_I2C_RD)
        {
            if (HAL_I2C_STATUS_OK != 
                hal_i2c_master_receive_polling(device->i2c_port, msgs[i].addr, msgs[i].buf, msgs[i].len))
            {
                LOG_EXT_E("i2c recv failed.");
                return i;
            }
        }
        else if (msgs[i].flags == OS_I2C_WR)
        {
            if (HAL_I2C_STATUS_OK != 
                hal_i2c_master_send_polling(device->i2c_port, msgs[i].addr, msgs[i].buf, msgs[i].len))
            {
                LOG_EXT_E("i2c send failed.");
                return i;
            }
        }
    }
    return i;
}

#ifdef BSP_USING_I2C1

static const struct os_i2c_bus_device_ops i2c1_dev_ops =
{
    .master_xfer = i2c_mst_xfer,
    .slave_xfer = OS_NULL,
    .i2c_bus_control = OS_NULL
};

static struct mt2625_i2c_bus i2c1_dev = 
{
    .name = "i2c1",
    .i2c_port = HAL_I2C_MASTER_1,
    .i2c_config.frequency = HAL_I2C_FREQUENCY_400K
};

void mt2625_i2c1_dev_init(void)
{
    hal_i2c_status_t status;
    hal_gpio_init(HAL_GPIO_16); // SCL
    hal_gpio_init(HAL_GPIO_17); // SDA

    hal_pinmux_set_function(HAL_GPIO_16, HAL_GPIO_16_I2C1_SCL);
    hal_pinmux_set_function(HAL_GPIO_17, HAL_GPIO_17_I2C1_SDA);

#ifdef BSP_I2C1_FREQUENCY
    if (BSP_I2C1_FREQUENCY == 50)
    {
        i2c1_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_50K;
    }
    else if (BSP_I2C1_FREQUENCY == 100)
    {
        i2c1_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_100K;
    }
    else if (BSP_I2C1_FREQUENCY == 200)
    {
        i2c1_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_200K;
    }
    else if (BSP_I2C1_FREQUENCY == 300)
    {
        i2c1_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_300K;
    }
    else if (BSP_I2C1_FREQUENCY == 400)
    {
        i2c1_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_400K;
    }
    else if (BSP_I2C1_FREQUENCY == 1000)
    {
        i2c1_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_1M;
    }
#endif /* BSP_I2C1_FREQUENCY */

    if(HAL_I2C_STATUS_OK != (status = hal_i2c_master_init(HAL_I2C_MASTER_1, &i2c1_dev.i2c_config)))
    {
        LOG_EXT_E("hal_i2c_master_init failed! error: %d", status);
        return;
    }

    i2c1_dev.i2c_dev.ops = &i2c1_dev_ops;

    if (OS_EOK != os_i2c_bus_device_register(&(i2c1_dev.i2c_dev), i2c1_dev.name))
    {
        LOG_EXT_E("i2c device register failed.");
    }
}
#endif /* BSP_USING_I2C1 */

int cm_hw_i2c_init(void)
{
    LOG_EXT_D("cm_hw_i2c_init");

#ifdef BSP_USING_I2C0
    //TODO
    #ifdef NB_USING_M5311_LIB
    
    #else
    LOG_EXT_W("5330 not support i2c0");
    #endif
#endif

#ifdef BSP_USING_I2C1
    #ifdef NB_USING_M5311_LIB
    LOG_EXT_W("5311 not support i2c1");
    #else
    mt2625_i2c1_dev_init();
    #endif
#endif

    return 0;
}
OS_DEVICE_INIT(cm_hw_i2c_init);

#endif /* BSP_USING_I2C && OS_USING_I2C && HAL_I2C_MASTER_MODULE_ENABLED */
