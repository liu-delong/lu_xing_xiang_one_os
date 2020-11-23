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
 * @file        sensor.c
 *
 * @brief       This file provides functions for sensor.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <os_mutex.h>
#include "sensor.h"

#define DRV_EXT_TAG "sensor"
#define DRV_EXT_LVL DBG_EXT_INFO
#include <drv_log.h>

#include <string.h>

static char *const sensor_name_str[] =
{
    "none",
    "acce_", /* Accelerometer     */
    "gyro_", /* Gyroscope         */
    "mag_",  /* Magnetometer      */
    "temp_", /* Temperature       */
    "humi_", /* Relative Humidity */
    "baro_", /* Barometer         */
    "li_",   /* Ambient light     */
    "pr_",   /* Proximity         */
    "hr_",   /* Heart Rate        */
    "tvoc_", /* TVOC Level        */
    "noi_",  /* Noise Loudness    */
    "step_", /* Step sensor       */
    "forc_", /* Force sensor      */
    "alti_", /* Altitude sensor   */
};

void os_sensor_cb(os_sensor_t sen)
{
    struct os_device_cb_info *info = &sen->parent.cb_table[OS_DEVICE_CB_TYPE_RX];

    if (info->cb == OS_NULL)
    {
        return;
    }

    if (sen->irq_handle != OS_NULL)
    {
        sen->irq_handle(sen);
    }

    /* The buffer is not empty. Read the data in the buffer first */
    if (sen->data_len > 0)
    {
        info->size = sen->data_len / sizeof(struct os_sensor_data);
        info->cb(&sen->parent, info);
    }
    else if (sen->config.mode == OS_SENSOR_MODE_INT)
    {
        /* The interrupt mode only produces one data at a time */
        info->size = 1;
        info->cb(&sen->parent, info);
    }
    else if (sen->config.mode == OS_SENSOR_MODE_FIFO)
    {
        info->size = sen->info.fifo_max;
        info->cb(&sen->parent, info);
    }
}

static void irq_callback(void *args)
{
    os_sensor_t sensor = (os_sensor_t)args;
    os_uint8_t  i;

    if (sensor->module)
    {
        /* Invoke a callback for all sensors in the module */
        for (i = 0; i < sensor->module->sen_num; i++)
        {
            os_sensor_cb(sensor->module->sen[i]);
        }
    }
    else
    {
        os_sensor_cb(sensor);
    }
}

static os_err_t os_sensor_irq_init(os_sensor_t sensor)
{
    if (sensor->config.irq_pin.pin == OS_PIN_NONE)
    {
        return OS_EINVAL;
    }

    os_pin_mode(sensor->config.irq_pin.pin, sensor->config.irq_pin.mode);

    if (sensor->config.irq_pin.mode == PIN_MODE_INPUT_PULLDOWN)
    {
        os_pin_attach_irq(sensor->config.irq_pin.pin, PIN_IRQ_MODE_RISING, irq_callback, (void *)sensor);
    }
    else if (sensor->config.irq_pin.mode == PIN_MODE_INPUT_PULLUP)
    {
        os_pin_attach_irq(sensor->config.irq_pin.pin, PIN_IRQ_MODE_FALLING, irq_callback, (void *)sensor);
    }
    else if (sensor->config.irq_pin.mode == PIN_MODE_INPUT)
    {
        os_pin_attach_irq(sensor->config.irq_pin.pin, PIN_IRQ_MODE_RISING_FALLING, irq_callback, (void *)sensor);
    }

    os_pin_irq_enable(sensor->config.irq_pin.pin, OS_TRUE);

    LOG_EXT_I("interrupt init success");

    return 0;
}

static os_err_t os_sensor_open(os_device_t *dev, os_uint16_t oflag)
{
    os_sensor_t sensor = (os_sensor_t)dev;
    OS_ASSERT(dev != OS_NULL);
    os_err_t res = OS_EOK;

    if (sensor->module)
    {
        /* Take the module mutex */
        os_mutex_lock(sensor->module->lock, OS_IPC_WAITING_FOREVER);
    }

    if (sensor->module != OS_NULL && sensor->info.fifo_max > 0 && sensor->data_buf == OS_NULL)
    {
        /* Allocate memory for the sensor buffer */
        sensor->data_buf = os_malloc(sizeof(struct os_sensor_data) * sensor->info.fifo_max);
        if (sensor->data_buf == OS_NULL)
        {
            res = OS_ENOMEM;
            goto __exit;
        }
    }

    if (oflag & OS_DEVICE_FLAG_RDONLY && dev->flag & OS_DEVICE_FLAG_RDONLY)
    {
        if (sensor->ops->control != OS_NULL)
        {
            /* If polling mode is supported, configure it to polling mode */
            sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_MODE, (void *)OS_SENSOR_MODE_POLLING);
        }
        sensor->config.mode = OS_SENSOR_MODE_POLLING;
    }
    else if (oflag & OS_DEVICE_FLAG_INT_RX && dev->flag & OS_DEVICE_FLAG_INT_RX)
    {
        if (sensor->ops->control != OS_NULL)
        {
            /* If interrupt mode is supported, configure it to interrupt mode */
            sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_MODE, (void *)OS_SENSOR_MODE_INT);
        }
        /* Initialization sensor interrupt */
        os_sensor_irq_init(sensor);
        sensor->config.mode = OS_SENSOR_MODE_INT;
    }
    else if (oflag & OS_DEVICE_FLAG_FIFO_RX && dev->flag & OS_DEVICE_FLAG_FIFO_RX)
    {
        if (sensor->ops->control != OS_NULL)
        {
            /* If fifo mode is supported, configure it to fifo mode */
            sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_MODE, (void *)OS_SENSOR_MODE_FIFO);
        }
        /* Initialization sensor interrupt */
        os_sensor_irq_init(sensor);
        sensor->config.mode = OS_SENSOR_MODE_FIFO;
    }
    else
    {
        res = OS_EINVAL;
        goto __exit;
    }

    /* Configure power mode to normal mode */
    if (sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_POWER, (void *)OS_SENSOR_POWER_NORMAL) == OS_EOK)
    {
        sensor->config.power = OS_SENSOR_POWER_NORMAL;
    }

__exit:
    if (sensor->module)
    {
        /* Release the module mutex */
        os_mutex_unlock(sensor->module->lock);
    }

    return res;
}

static os_err_t os_sensor_close(os_device_t *dev)
{
    os_sensor_t sensor = (os_sensor_t)dev;
    int         i;

    OS_ASSERT(dev != OS_NULL);

    if (sensor->module)
    {
        os_mutex_lock(sensor->module->lock, OS_IPC_WAITING_FOREVER);
    }

    /* Configure power mode to power down mode */
    if (sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_POWER, (void *)OS_SENSOR_POWER_DOWN) == OS_EOK)
    {
        sensor->config.power = OS_SENSOR_POWER_DOWN;
    }

    /* Sensor disable interrupt */
    if (sensor->config.irq_pin.pin != OS_PIN_NONE)
    {
        os_pin_irq_enable(sensor->config.irq_pin.pin, OS_FALSE);
    }

    if (sensor->module != OS_NULL && sensor->info.fifo_max > 0 && sensor->data_buf != OS_NULL)
    {
        for (i = 0; i < sensor->module->sen_num; i++)
        {
            if (sensor->module->sen[i]->parent.ref_count > 0)
                goto __exit;
        }

        /* Free memory for the sensor buffer */
        for (i = 0; i < sensor->module->sen_num; i++)
        {
            if (sensor->module->sen[i]->data_buf != OS_NULL)
            {
                os_free(sensor->module->sen[i]->data_buf);
                sensor->module->sen[i]->data_buf = OS_NULL;
            }
        }
    }

__exit:
    if (sensor->module)
    {
        os_mutex_unlock(sensor->module->lock);
    }

    return OS_EOK;
}

static os_size_t os_sensor_read(os_device_t *dev, os_off_t pos, void *buf, os_size_t len)
{
    os_sensor_t sensor = (os_sensor_t)dev;
    os_size_t   result = 0;
    OS_ASSERT(dev != OS_NULL);

    if (buf == NULL || len == 0)
    {
        return 0;
    }

    if (sensor->module)
    {
        os_mutex_lock(sensor->module->lock, OS_IPC_WAITING_FOREVER);
    }

    /* The buffer is not empty. Read the data in the buffer first */
    if (sensor->data_len > 0)
    {
        if (len > sensor->data_len / sizeof(struct os_sensor_data))
        {
            len = sensor->data_len / sizeof(struct os_sensor_data);
        }

        memcpy(buf, sensor->data_buf, len * sizeof(struct os_sensor_data));

        /* Clear the buffer */
        sensor->data_len = 0;
        result           = len;
    }
    else
    {
        /* If the buffer is empty read the data */
        result = sensor->ops->fetch_data(sensor, buf, len);
    }

    if (sensor->module)
    {
        os_mutex_unlock(sensor->module->lock);
    }

    return result;
}

static os_err_t os_sensor_control(os_device_t *dev, int cmd, void *args)
{
    os_sensor_t sensor = (os_sensor_t)dev;
    os_err_t    result = OS_EOK;
    OS_ASSERT(dev != OS_NULL);

    if (sensor->module)
    {
        os_mutex_lock(sensor->module->lock, OS_IPC_WAITING_FOREVER);
    }

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        if (args)
        {
            sensor->ops->control(sensor, OS_SENSOR_CTRL_GET_ID, args);
        }
        break;
    case OS_SENSOR_CTRL_GET_INFO:
        if (args)
        {
            memcpy(args, &sensor->info, sizeof(struct os_sensor_info));
        }
        break;
    case OS_SENSOR_CTRL_SET_RANGE:

        /* Configuration measurement range */
        result = sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_RANGE, args);
        if (result == OS_EOK)
        {
            sensor->config.range = (os_int32_t)args;
            LOG_EXT_D("set range %d", sensor->config.range);
        }
        break;
    case OS_SENSOR_CTRL_SET_ODR:

        /* Configuration data output rate */
        result = sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_ODR, args);
        if (result == OS_EOK)
        {
            sensor->config.odr = (os_uint32_t)args & 0xFFFF;
            LOG_EXT_D("set odr %d", sensor->config.odr);
        }
        break;
    case OS_SENSOR_CTRL_SET_POWER:

        /* Configuration sensor power mode */
        result = sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_POWER, args);
        if (result == OS_EOK)
        {
            sensor->config.power = (os_uint32_t)args & 0xFF;
            LOG_EXT_D("set power mode code:", sensor->config.power);
        }
        break;
    case OS_SENSOR_CTRL_SELF_TEST:

        /* Device self-test */
        result = sensor->ops->control(sensor, OS_SENSOR_CTRL_SELF_TEST, args);
        break;
    default:
        return OS_ERROR;
    }

    if (sensor->module)
    {
        os_mutex_unlock(sensor->module->lock);
    }

    return result;
}

const static struct os_device_ops os_sensor_ops =
{
    OS_NULL,
    os_sensor_open,
    os_sensor_close,
    os_sensor_read,
    OS_NULL,
    os_sensor_control
};

int os_hw_sensor_register(os_sensor_t sensor, const char *name, os_uint32_t flag, void *data)
{
    os_int8_t    result;
    os_device_t *device;
    OS_ASSERT(sensor != OS_NULL);

    char *sensor_name = OS_NULL, *device_name = OS_NULL;

    /* Add a type name for the sensor device */
    sensor_name = sensor_name_str[sensor->info.type];
    device_name = (char *)os_calloc(1, strlen(sensor_name) + 1 + strlen(name));
    if (device_name == OS_NULL)
    {
        LOG_EXT_E("device_name calloc failed!");
        return OS_ERROR;
    }

    memcpy(device_name, sensor_name, strlen(sensor_name) + 1);
    strcat(device_name, name);

    if (sensor->module != OS_NULL /* && sensor->module->lock == OS_NULL*/)
    {
        /* Create a mutex lock for the module */
        sensor->module->lock = os_mutex_create(name, OS_IPC_FLAG_FIFO, OS_FALSE);
        if (sensor->module->lock == OS_NULL)
        {
            os_free(device_name);
            return OS_ERROR;
        }
    }

    device = &sensor->parent;

    device->ops = &os_sensor_ops;
    
    device->type        = OS_DEVICE_TYPE_SENSOR;
    device->cb_table[OS_DEVICE_CB_TYPE_RX].cb = OS_NULL;
    device->cb_table[OS_DEVICE_CB_TYPE_TX].cb = OS_NULL;
    device->user_data   = data;

    result = os_device_register(device, device_name, flag | OS_DEVICE_FLAG_STANDALONE);
    if (result != OS_EOK)
    {
        os_free(device_name);
        LOG_EXT_E("os_sensor register err code: %d", result);
        return result;
    }

    os_free(device_name);
    LOG_EXT_I("os_sensor init success");
    return OS_EOK;
}

