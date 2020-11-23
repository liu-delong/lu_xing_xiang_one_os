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
 * @file        audio_player.c
 *
 * @brief       This file provides functions for audio player device operations.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <string.h>

#include "drv_cfg.h"

#include "audio.h"
#include "audio_pipe.h"

#define DBG_TAG              "audio_player"
#define DBG_LVL              DBG_INFO
#include <os_dbg_ext.h>

os_err_t _audio_player_callback(os_device_t *dev, struct os_device_cb_info *info)
{
    OS_ASSERT(dev != OS_NULL);
    OS_ASSERT(info != OS_NULL);
    
    os_mp_free(info->data);
    return OS_EOK;
}

static os_err_t _audio_player_init(struct os_device *dev)
{
    os_err_t result = OS_EOK;
    struct os_audio_device *audio;

    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    /* initialize replay & record */
    audio->replay = OS_NULL;

    /* initialize replay */
    if (dev->flag & OS_DEVICE_FLAG_WRONLY)
    {
        struct os_audio_replay *replay = (struct os_audio_replay *) os_malloc(sizeof(struct os_audio_replay));

        if (replay == OS_NULL)
            return OS_ENOMEM;
        memset(replay, 0, sizeof(struct os_audio_replay));

        /* init memory pool for replay */
        replay->mp = os_mp_create("adu_mp", OS_AUDIO_REPLAY_MP_BLOCK_COUNT, OS_AUDIO_REPLAY_MP_BLOCK_SIZE);
        if (replay->mp == OS_NULL)
        {
            os_free(replay);
            LOG_EXT_E("create memory pool for repaly failed");
            return OS_ENOMEM;
        }

        /* init mutex lock for audio replay */
        os_mutex_init(&replay->lock, "replay", OS_IPC_FLAG_PRIO,OS_FALSE);

        replay->activated = OS_FALSE;
        audio->replay = replay;
    }
    
    if (audio->ops->init)
        audio->ops->init(audio);

    return result;
}

static os_err_t _audio_player_open(struct os_device *dev, os_uint16_t oflag)
{
    if ((oflag & OS_DEVICE_OFLAG_WRONLY) && !(dev->flag & OS_DEVICE_FLAG_WRONLY))
        return OS_EIO;
    
    /* get open flags */
    dev->open_flag = oflag & 0xff;
    
    return OS_EOK;
}

static os_err_t _audio_player_close(struct os_device *dev)
{
    struct os_audio_device *audio;
    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    if (dev->open_flag & OS_DEVICE_OFLAG_WRONLY)
    {
        if (audio->replay->activated == OS_TRUE)
        {
            /*wait for all data has been played!*/
            while (audio->replay->mp->block_free_count != audio->replay->mp->block_total_count);
            if (audio->ops->stop)
                audio->ops->stop(audio);

            audio->replay->activated = OS_FALSE;
            LOG_EXT_D("stop audio replay device");
        }   
    }

    return OS_EOK;
}

static os_size_t _audio_player_write(struct os_device *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    struct os_audio_device *audio;

    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    os_mutex_lock(&audio->replay->lock, OS_IPC_WAITING_FOREVER);
    audio->replay->write_data = os_mp_alloc(audio->replay->mp, OS_IPC_WAITING_FOREVER);
    memcpy(audio->replay->write_data, buffer, size);
    os_mutex_unlock(&audio->replay->lock);

    if (audio->replay->activated != OS_TRUE)
    {
        if (audio->ops->start)
            audio->ops->start(audio);

        audio->replay->activated = OS_TRUE;
        LOG_EXT_D("start audio replay device\n");
    }

    if (audio->ops->transmit)
    {
        audio->ops->transmit(audio, audio->replay->write_data, size);
    }

    if (size < OS_AUDIO_REPLAY_MP_BLOCK_SIZE)
    {
        _audio_player_close(dev);
    }
    
    return size;
}

static os_err_t _audio_player_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t result = OS_EOK;
    struct os_audio_device *audio;
    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    switch (cmd)
    {
    case AUDIO_CTL_CONFIGURE:
    {
        struct os_audio_caps *caps = (struct os_audio_caps *) args;

        if (audio->ops->configure)
        {
            result = audio->ops->configure(audio, caps);
        }
        break;
    }
    case AUDIO_CTL_START:
    {
        if (audio->ops->start)
            result = audio->ops->start(audio);
        break;
    }
    case AUDIO_CTL_STOP:
    {
        if (audio->ops->stop)
            result = audio->ops->stop(audio);
        break;
    }

    default:
        break;
    }

    return result;
}

const static struct os_device_ops audio_ops =
{
    _audio_player_init,
    _audio_player_open,
    _audio_player_close,
    OS_NULL,
    _audio_player_write,
    _audio_player_control
};

os_err_t os_audio_player_register(struct os_audio_device *audio, const char *name, os_uint32_t flag, void *data)
{
    os_err_t result = OS_EOK;
    struct os_device *device;

    OS_ASSERT(audio != OS_NULL);
    device = &(audio->parent);

    device->type = OS_DEVICE_TYPE_SOUND;
    device->cb_table[OS_DEVICE_CB_TYPE_RX].cb = OS_NULL;
    device->cb_table[OS_DEVICE_CB_TYPE_TX].cb = _audio_player_callback;
    device->ops  = &audio_ops;
    device->user_data = data;

    /* register a character device */
    result = os_device_register(device, name, flag | OS_DEVICE_FLAG_REMOVABLE);

    /* initialize audio device */
    if (result == OS_EOK)
        result = os_device_init(device);

    return result;
}


