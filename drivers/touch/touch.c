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
 * @file        touch.c
 *
 * @brief       touch
 *
 * @details     touch
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <touch/touch.h>
#include <string.h>
#include <os_errno.h>
#include <pin/pin.h>
#include <os_assert.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "touch"
#include <drv_log.h>

/* ISR for touch interrupt */
static void irq_callback(void *args)
{
    os_touch_t *touch;

    touch = (os_touch_t *)args;

    struct os_device_cb_info *info = &touch->parent.cb_table[OS_DEVICE_CB_TYPE_RX];

    if (info->cb == OS_NULL)
    {
        return;
    }

    if (touch->irq_handle != OS_NULL)
    {
        touch->irq_handle(touch);
    }


    info->size = 1;
    info->cb(&touch->parent, info);
}

/* touch interrupt initialization function */
static os_err_t os_touch_irq_init(os_touch_t *touch)
{
    if (touch->config.irq_pin->pin == OS_PIN_NONE)
    {
        return OS_EINVAL;
    }

    os_pin_mode(touch->config.irq_pin->pin, touch->config.irq_pin->mode);

    if (touch->config.irq_pin->mode == PIN_MODE_INPUT_PULLDOWN)
    {
        os_pin_attach_irq(touch->config.irq_pin->pin, PIN_IRQ_MODE_RISING, irq_callback, (void *)touch);
    }
    else if (touch->config.irq_pin->mode == PIN_MODE_INPUT_PULLUP)
    {
        os_pin_attach_irq(touch->config.irq_pin->pin, PIN_IRQ_MODE_FALLING, irq_callback, (void *)touch);
    }
    else if (touch->config.irq_pin->mode == PIN_MODE_INPUT)
    {
        os_pin_attach_irq(touch->config.irq_pin->pin, PIN_IRQ_MODE_RISING_FALLING, irq_callback, (void *)touch);
    }

    os_pin_irq_enable(touch->config.irq_pin->pin, PIN_IRQ_ENABLE);

    return OS_EOK;
}

/* touch interrupt enable */
static void os_touch_irq_enable(os_touch_t *touch)
{
    if (touch->config.irq_pin->pin != OS_PIN_NONE)
    {
        os_pin_irq_enable(touch->config.irq_pin->pin, OS_TRUE);
    }
}

/* touch interrupt disable */
static void os_touch_irq_disable(os_touch_t *touch)
{
    if (touch->config.irq_pin->pin != OS_PIN_NONE)
    {
        os_pin_irq_enable(touch->config.irq_pin->pin, OS_FALSE);
    }
}

static os_err_t os_touch_open(os_device_t *dev, os_uint16_t oflag)
{
    os_touch_t *touch;
    OS_ASSERT(dev != OS_NULL);
    touch = (os_touch_t *)dev;

    if (oflag & OS_DEVICE_FLAG_INT_RX && dev->flag & OS_DEVICE_FLAG_INT_RX)
    {
        /* Initialization touch interrupt */
        os_touch_irq_init(touch);
    }

    return OS_EOK;
}

static os_err_t os_touch_close(os_device_t *dev)
{
    os_touch_t *touch;
    OS_ASSERT(dev != OS_NULL);
    touch = (os_touch_t *)dev;

    /* touch disable interrupt */
    os_touch_irq_disable(touch);

    return OS_EOK;
}

static os_size_t os_touch_read(os_device_t *dev, os_off_t pos, void *buf, os_size_t len)
{
    os_touch_t *touch;
    os_size_t   result = 0;
    OS_ASSERT(dev != OS_NULL);
    touch = (os_touch_t *)dev;

    if (buf == NULL || len == 0)
    {
        return 0;
    }

    result = touch->ops->touch_readpoint(touch, buf, len);

    return result;
}

static os_err_t os_touch_control(os_device_t *dev, int cmd, void *args)
{
    os_touch_t *touch;
    os_err_t    result = OS_EOK;
    OS_ASSERT(dev != OS_NULL);
    touch = (os_touch_t *)dev;

    switch (cmd)
    {
    case OS_TOUCH_CTRL_GET_ID:
        if (args)
        {
            result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_GET_ID, args);
        }
        else
        {
            result = OS_ERROR;
        }

        break;
    case OS_TOUCH_CTRL_GET_INFO:
        if (args)
        {
            result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_GET_INFO, args);
        }
        else
        {
            result = OS_ERROR;
        }

        break;
    case OS_TOUCH_CTRL_SET_MODE:
        result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_SET_MODE, args);

        if (result == OS_EOK)
        {
            os_uint16_t mode;
            mode = *(os_uint16_t *)args;
            if (mode == OS_DEVICE_FLAG_INT_RX)
            {
                os_touch_irq_enable(touch); /* enable interrupt */
            }
        }

        break;
    case OS_TOUCH_CTRL_SET_X_RANGE:
        result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_SET_X_RANGE, args);

        if (result == OS_EOK)
        {
            touch->info.range_x = *(os_int32_t *)args;
            LOG_EXT_D("set x coordinate range :%d\n", touch->info.range_x);
        }

        break;
    case OS_TOUCH_CTRL_SET_Y_RANGE:
        result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_SET_Y_RANGE, args);

        if (result == OS_EOK)
        {
            touch->info.range_y = *(os_uint32_t *)args;
            LOG_EXT_D("set y coordinate range :%d \n", touch->info.range_x);
        }

        break;
    case OS_TOUCH_CTRL_DISABLE_INT:
        os_touch_irq_disable(touch);
        break;
    case OS_TOUCH_CTRL_ENABLE_INT:
        os_touch_irq_enable(touch);
        break;
    default:
        return OS_ERROR;
    }

    return result;
}

const static struct os_device_ops os_touch_ops =
{
    OS_NULL,
    os_touch_open,
    os_touch_close,
    os_touch_read,
    OS_NULL,
    os_touch_control
};

/**
 ***********************************************************************************************************************
 * @brief          register a touch device
 *
 * \@param[in]      touch           touch device
 * \@param[in]      name            touch device name
 * \@param[in]      flag            touch device flag
 * \@param[in]      data            user data
 *
 * \@return         error code
 * \@retval         OS_EOK          register success
 * \@retval         else            register fail
 ***********************************************************************************************************************
 */
int os_hw_touch_register(os_touch_t *touch, const char *name, os_uint32_t flag, void *data)
{
    os_int8_t    result;
    os_device_t *device;
    OS_ASSERT(touch != OS_NULL);

    device = &touch->parent;

    device->ops = &os_touch_ops;
    device->type        = OS_DEVICE_TYPE_TOUCH;
    device->cb_table[OS_DEVICE_CB_TYPE_RX].cb = OS_NULL;
    device->cb_table[OS_DEVICE_CB_TYPE_TX].cb = OS_NULL;
    device->user_data   = data;

    result = os_device_register(device, name, flag | OS_DEVICE_FLAG_STANDALONE);

    if (result != OS_EOK)
    {
        LOG_EXT_E("os_touch register err code: %d", result);
        return result;
    }

    LOG_EXT_I("os_touch init success");

    return OS_EOK;
}
