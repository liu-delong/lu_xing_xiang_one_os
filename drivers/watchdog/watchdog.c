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
 * @file        watchdog.c
 *
 * @brief       This file provides functions for registering watchdog.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <watchdog/watchdog.h>

/**
 ***********************************************************************************************************************
 * @brief           This function initializes watchdog
 *
 * @param[in]       dev             os_device
 *
 * @return          Return init status.
 * @retval          OS_EOK          init success.
 * @retval          OS_ENOSYS       No system.
 ***********************************************************************************************************************
 */
static os_err_t os_watchdog_init(struct os_device *dev)
{
    os_watchdog_t *wtd;

    OS_ASSERT(dev != OS_NULL);
    wtd = (os_watchdog_t *)dev;
    if (wtd->ops->init)
    {
        return (wtd->ops->init(wtd));
    }

    return (OS_ENOSYS);
}

static os_err_t os_watchdog_open(struct os_device *dev, os_uint16_t oflag)
{
    return (OS_EOK);
}

static os_err_t os_watchdog_close(struct os_device *dev)
{
    os_watchdog_t *wtd;

    OS_ASSERT(dev != OS_NULL);
    wtd = (os_watchdog_t *)dev;

    if (wtd->ops->control(wtd, OS_DEVICE_CTRL_WDT_STOP, OS_NULL) != OS_EOK)
    {
        os_kprintf(" This watchdog can not be stoped\n");

        return (OS_ERROR);
    }

    return (OS_EOK);
}

static os_err_t os_watchdog_control(struct os_device *dev, int cmd, void *args)
{
    os_watchdog_t *wtd;

    OS_ASSERT(dev != OS_NULL);
    wtd = (os_watchdog_t *)dev;

    return (wtd->ops->control(wtd, cmd, args));
}

#ifdef OS_USING_DEVICE_OPS
const static struct os_device_ops wdt_ops = {
    os_watchdog_init,
    os_watchdog_open,
    os_watchdog_close,
    OS_NULL,
    OS_NULL,
    os_watchdog_control,
};
#endif

/**
 ***********************************************************************************************************************
 * @brief           This function register a watchdog device.
 *
 * @param[in]       wtd             pointer to struct os_watchdog_device.
 * @param[in]       name            watchdog device's name.
 * @param[in]       flag            watchdog device flag.
 * @param[in]       data            can device private data.
 *
 * @return          Return watchdog register status.
 * @retval          OS_EOK       register success.
 * @retval          Others       register failed.
 ***********************************************************************************************************************
 */
os_err_t os_hw_watchdog_register(struct os_watchdog_device *wtd, const char *name, os_uint32_t flag, void *data)
{
    struct os_device *device;
    OS_ASSERT(wtd != OS_NULL);

    device = &(wtd->parent);

    device->type        = OS_DEVICE_TYPE_MISCELLANEOUS;
    device->rx_indicate = OS_NULL;
    device->tx_complete = OS_NULL;

#ifdef OS_USING_DEVICE_OPS
    device->ops = &wdt_ops;
#else
    device->init    = os_watchdog_init;
    device->open    = os_watchdog_open;
    device->close   = os_watchdog_close;
    device->read    = OS_NULL;
    device->write   = OS_NULL;
    device->control = os_watchdog_control;
#endif
    device->user_data = data;

    /* register a character device */
    return os_device_register(device, name, flag);
}
