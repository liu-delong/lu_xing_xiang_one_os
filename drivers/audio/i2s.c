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
 * @file        i2s.c
 *
 * @brief       This file implements i2s driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_cfg.h>
#include <string.h>
#include <os_sem.h>
#include <os_memory.h>
#include <os_irq.h>
#include <audio/i2s.h>
#include <drv_log.h>

void os_hw_i2s_isr(struct os_i2s_device *i2s, struct os_device_cb_info *info)
{
    os_device_t  *dev = (os_device_t  *)&i2s->parent;

    if (dev->cb_table[OS_DEVICE_CB_TYPE_TX].cb != OS_NULL)
    {
        dev->cb_table[OS_DEVICE_CB_TYPE_TX].cb(dev, info);
    }
}

os_err_t  i2s_init(os_device_t *dev)
{
    os_err_t status = OS_EOK;
    
    struct os_i2s_device *i2s = (struct os_i2s_device *)dev;
    if (i2s == OS_NULL)
    {
        LOG_EXT_E("i2s device not exist!\n");
        return OS_EEMPTY;
    }

    return i2s->ops->init(i2s);
}

os_size_t i2s_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_err_t status = OS_EOK;

    struct os_i2s_device *i2s = (struct os_i2s_device *)dev;
    if (i2s == OS_NULL)
    {
        LOG_EXT_E("i2s device not exist!\n");
        return OS_EEMPTY;
    }

    status = i2s->ops->receive(i2s, (os_uint8_t *)buffer, size);
    if (status != OS_EOK)
    {
        return 0;
    }
    
    return size;
}

os_size_t i2s_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_err_t status = OS_EOK;

    struct os_i2s_device *i2s = (struct os_i2s_device *)dev;
    if (i2s == OS_NULL)
    {
        LOG_EXT_E("i2s device not exist!\n");
        return OS_EEMPTY;
    }

    status = i2s->ops->transimit(i2s, (os_uint8_t *)buffer, size);
    if (status != OS_EOK)
    {
        return 0;
    }
    
    return size;
}

os_err_t i2s_control(os_device_t *dev, os_int32_t cmd, void *args)
{
    os_err_t status = OS_EOK;
    
    struct os_i2s_device *i2s = (struct os_i2s_device *)dev;
    if (i2s == OS_NULL)
    {
        LOG_EXT_E("i2s device not exist!\n");
        return OS_EEMPTY;
    }

    switch (cmd)
    {
    case OS_AUDIO_CMD_ENABLE:
        status = i2s->ops->enable(i2s, OS_TRUE);
        break;
    case OS_AUDIO_CMD_DISABLE:
        status = i2s->ops->enable(i2s, OS_FALSE);
        break;
    case OS_AUDIO_CMD_SET_FRQ:
        status = i2s->ops->set_frq(i2s, *(uint32_t *)args);
    case OS_AUDIO_CMD_SET_CHANNEL:
        status = i2s->ops->set_channel(i2s, *(uint8_t *)args);
        break;
    case OS_AUDIO_CMD_SET_INFO:
//       status = i2s->ops->i2s_info(i2s, (struct os_i2s_replay *)args);
        break;
    default:
        status = OS_ENOSYS;
        break;
    }

    return status;
}

const static struct os_device_ops i2s_ops = {
    i2s_init, 
    OS_NULL,
    OS_NULL,
    i2s_read,
    i2s_write,
    i2s_control
};

os_err_t os_i2s_register(struct os_i2s_device *i2s, const char *name, os_uint32_t flag, void *data)
{
    OS_ASSERT(i2s != OS_NULL);
    OS_ASSERT(i2s->ops != OS_NULL);
    
    i2s->parent.type  = OS_DEVICE_TYPE_SOUND;
    i2s->parent.ops  = &i2s_ops;
    
    return os_device_register(&i2s->parent, name, OS_DEVICE_FLAG_STANDALONE);
}

