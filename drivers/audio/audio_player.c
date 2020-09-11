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

enum
{
    REPLAY_EVT_NONE  = 0x00,
    REPLAY_EVT_START = 0x01,
    REPLAY_EVT_STOP  = 0x02,
};

static os_err_t _audio_send_replay_frame(struct os_audio_device *audio)
{
    os_err_t result = OS_EOK;
    os_uint8_t *data;
    os_size_t dst_size, src_size;
    os_uint16_t position, remain_bytes = 0, index = 0;
    struct os_audio_buf_info *buf_info;

    OS_ASSERT(audio != OS_NULL);

    buf_info = &audio->replay->buf_info;
    /* save current pos */
    position = audio->replay->pos;    
    dst_size = buf_info->block_size;

    /* check repaly queue is empty */
    if (os_data_queue_peak(&audio->replay->queue, (const void **)&data, &src_size) != OS_EOK)
    {
        /* ack stop event */
        if (audio->replay->event & REPLAY_EVT_STOP)
            os_completion_done(&audio->replay->cmp);

        /* send zero frames */
        memset(&buf_info->buffer[audio->replay->pos], 0, dst_size);

        audio->replay->pos += dst_size;
        audio->replay->pos %= buf_info->total_size;
    }
    else
    {
        memset(&buf_info->buffer[audio->replay->pos], 0, dst_size);

        /* copy data from memory pool to hardware device fifo */
        while (index < dst_size)
        {
            result = os_data_queue_peak(&audio->replay->queue, (const void **)&data, &src_size);
            if (result != OS_EOK)
            {
                LOG_EXT_D("under run %d, remain %d", audio->replay->pos, remain_bytes);
                audio->replay->pos -= remain_bytes;
                audio->replay->pos += dst_size;
                audio->replay->pos %= buf_info->total_size;
                audio->replay->read_index = 0;
                result = OS_EEMPTY;
                break;
            }

            remain_bytes = MIN((dst_size - index), (src_size - audio->replay->read_index));
            memcpy(&buf_info->buffer[audio->replay->pos],
                   &data[audio->replay->read_index], remain_bytes);

            index += remain_bytes;
            audio->replay->read_index += remain_bytes;
            audio->replay->pos += remain_bytes;
            audio->replay->pos %= buf_info->total_size;

            if (audio->replay->read_index == src_size)
            {
                /* free memory */
                audio->replay->read_index = 0;
                os_data_queue_pop(&audio->replay->queue, (const void **)&data, &src_size, OS_IPC_WAITING_NO);
                os_mp_free(data);

                /* notify transmitted complete. */
                if (audio->parent.tx_complete != OS_NULL)
                    audio->parent.tx_complete(&audio->parent, (void *)data);
            }
        }
    }

    if (audio->ops->transmit != OS_NULL)
    {
        if (audio->ops->transmit(audio, &buf_info->buffer[position], OS_NULL, dst_size) != dst_size)
            result = OS_ERROR;
    }

    return result;
}

static os_err_t _audio_flush_replay_frame(struct os_audio_device *audio)
{
    os_err_t result = OS_EOK;

    if (audio->replay->write_index)
    {
        result = os_data_queue_push(&audio->replay->queue,
                                    (const void **)audio->replay->write_data,
                                    audio->replay->write_index,
                                    OS_IPC_WAITING_FOREVER);

        audio->replay->write_index = 0;
    }

    return result;
}

static os_err_t _aduio_replay_start(struct os_audio_device *audio)
{
    os_err_t result = OS_EOK;

    if (audio->replay->activated != OS_TRUE)
    {
        /* start playback hardware device */
        if (audio->ops->start)
            result = audio->ops->start(audio);

        audio->replay->activated = OS_TRUE;
        LOG_EXT_D("start audio replay device");
    }

    return result;
}

static os_err_t _aduio_replay_stop(struct os_audio_device *audio)
{
    os_err_t result = OS_EOK;

    if (audio->replay->activated == OS_TRUE)
    {
        /* flush replay remian frames */
        _audio_flush_replay_frame(audio);

        /* notify irq(or thread) to stop the data transmission */
        audio->replay->event |= REPLAY_EVT_STOP;

        /* waiting for the remaining data transfer to complete */
        os_completion_init(&audio->replay->cmp);
        os_completion_wait(&audio->replay->cmp, OS_IPC_WAITING_FOREVER);
        audio->replay->event &= ~REPLAY_EVT_STOP;

        /* stop playback hardware device */
        if (audio->ops->stop)
            result = audio->ops->stop(audio);

        audio->replay->activated = OS_FALSE;
        LOG_EXT_D("stop audio replay device");
    }

    return result;
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

        /* init queue for audio replay */
        os_data_queue_init(&replay->queue, CFG_AUDIO_REPLAY_QUEUE_COUNT, 0, OS_NULL);

        /* init mutex lock for audio replay */
        os_mutex_init(&replay->lock, "replay", OS_IPC_FLAG_PRIO,OS_FALSE);

        replay->activated = OS_FALSE;
        audio->replay = replay;
    }
    
    /* initialize hardware configuration */
    if (audio->ops->init)
        audio->ops->init(audio);

    /* get replay buffer information */
    if (audio->ops->buffer_info)
        audio->ops->buffer_info(audio, &audio->replay->buf_info);

    return result;
}

static os_err_t _audio_player_open(struct os_device *dev, os_uint16_t oflag)
{
    struct os_audio_device *audio;

    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;
    
    /* check device flag with the open flag */
    if ((oflag & OS_DEVICE_OFLAG_WRONLY) && !(dev->flag & OS_DEVICE_FLAG_WRONLY))
        return OS_EIO;
    
    /* get open flags */
    dev->open_flag = oflag & 0xff;
    
    /* initialize the Tx structure according to open flag */
    if (oflag & OS_DEVICE_OFLAG_WRONLY)
    {
        if (audio->replay->activated != OS_TRUE)
        {
            LOG_EXT_D("open audio replay device, oflag = %x\n", oflag);
            audio->replay->write_index = 0;
            audio->replay->read_index = 0;
            audio->replay->pos = 0;
            audio->replay->event = REPLAY_EVT_NONE;
        }
    }
    return OS_EOK;
}

static os_err_t _audio_player_close(struct os_device *dev)
{
    struct os_audio_device *audio;
    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    if (dev->open_flag & OS_DEVICE_OFLAG_WRONLY)
    {
        /* stop replay stream */
        _aduio_replay_stop(audio);   
    }

    return OS_EOK;
}


static os_size_t _audio_player_write(struct os_device *dev, os_off_t pos, const void *buffer, os_size_t size)
{

    struct os_audio_device *audio;
    os_uint8_t *ptr;
    os_uint16_t block_size, remain_bytes, index = 0;

    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    /* push a new frame to replay data queue */
    ptr = (os_uint8_t *)buffer;
    block_size = OS_AUDIO_REPLAY_MP_BLOCK_SIZE; /* BLOCK_SIZE = 4k */

    os_mutex_lock(&audio->replay->lock, OS_IPC_WAITING_FOREVER);
    while (index < size)
    {
        /* request buffer from replay memory pool */
        if (audio->replay->write_index % block_size == 0)
        {
            audio->replay->write_data = os_mp_alloc(audio->replay->mp, OS_IPC_WAITING_FOREVER);   
            memset(audio->replay->write_data, 0, block_size);
        }

        /* copy data to replay memory pool */
        remain_bytes = MIN((block_size - audio->replay->write_index), (size - index));           
        memcpy(&audio->replay->write_data[audio->replay->write_index], &ptr[index], remain_bytes); 

        index += remain_bytes;
        audio->replay->write_index += remain_bytes;
        audio->replay->write_index %= block_size;   

        if (audio->replay->write_index == 0)         
        {
            os_data_queue_push(&audio->replay->queue,
                               audio->replay->write_data,
                               block_size,
                               OS_IPC_WAITING_FOREVER);
        }
    }
    os_mutex_unlock(&audio->replay->lock);

    /* check replay state */
    if (audio->replay->activated != OS_TRUE)
    {
        _aduio_replay_start(audio);
        audio->replay->activated = OS_TRUE;
    }

    return index;
}

static os_err_t _audio_player_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t result = OS_EOK;
    struct os_audio_device *audio;
    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    /* dev stat...*/
    switch (cmd)
    {
        
    case AUDIO_CTL_CONFIGURE:
    {
        struct os_audio_caps *caps = (struct os_audio_caps *) args;

        if (audio->ops->configure != OS_NULL)
        {
            result = audio->ops->configure(audio, caps);
        }

        break;
    }

    case AUDIO_CTL_START:
    {
        result = _aduio_replay_start(audio);

        break;
    }

    case AUDIO_CTL_STOP:
    {
        
        result = _aduio_replay_stop(audio);
        
        break;
    }

    default:
        break;
    }

    return result;
}

#ifdef OS_USING_DEVICE_OPS
const static struct os_device_ops audio_ops =
{
    _audio_dev_init,
    _audio_dev_open,
    _audio_dev_close,
    _audio_dev_read,
    _audio_dev_write,
    _audio_dev_control
};
#endif

os_err_t os_audio_player_register(struct os_audio_device *audio, const char *name, os_uint32_t flag, void *data)
{
    os_err_t result = OS_EOK;
    struct os_device *device;

    OS_ASSERT(audio != OS_NULL);
    device = &(audio->parent);

    device->type = OS_DEVICE_TYPE_SOUND;
    device->rx_indicate = OS_NULL;
    device->tx_complete = OS_NULL;

#ifdef OS_USING_DEVICE_OPS
    device->ops  = &audio_ops;
#else
    device->init    = _audio_player_init;
    device->open    = _audio_player_open;
    device->close   = _audio_player_close;
    device->read    = OS_NULL;
    device->write   = _audio_player_write;
    device->control = _audio_player_control;
#endif
    device->user_data = data;

    /* register a character device */
    result = os_device_register(device, name, flag | OS_DEVICE_FLAG_REMOVABLE);

    /* initialize audio device */
    if (result == OS_EOK)
        result = os_device_init(device);

    return result;
}

void os_audio_tx_complete(struct os_audio_device *audio)
{
    /* try to send next frame */
    _audio_send_replay_frame(audio);
}


