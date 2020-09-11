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
 * @file        audio_recorder.c
 *
 * @brief       This file provides functions for audio recorder device operations.
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

#define DBG_TAG              "audio_recorder"
#define DBG_LVL              DBG_INFO
#include <os_dbg_ext.h>


static os_err_t _audio_record_start(struct os_audio_device *audio)
{
    os_err_t result = OS_EOK;

    if (audio->record->activated != OS_TRUE)
    {
        /* open audio record pipe */
        os_device_open(OS_DEVICE(&audio->record->pipe), OS_DEVICE_OFLAG_RDONLY);

        /* start record hardware device */
        if (audio->ops->start)
            result = audio->ops->start(audio);

        audio->record->activated = OS_TRUE;
        LOG_EXT_D("start audio record device");
    }

    return result;
}

static os_err_t _audio_record_stop(struct os_audio_device *audio)
{
    os_err_t result = OS_EOK;

    if (audio->record->activated == OS_TRUE)
    {
        /* stop record hardware device */
        if (audio->ops->stop)
            result = audio->ops->stop(audio);

        /* close audio record pipe */
        os_device_close(OS_DEVICE(&audio->record->pipe));

        audio->record->activated = OS_FALSE;
        LOG_EXT_D("stop audio record device");
    }

    return result;
}

static os_err_t _audio_recorder_init(struct os_device *dev)
{
    os_err_t result = OS_EOK;
    struct os_audio_device *audio;

    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    /* initialize record */
    audio->record = OS_NULL;

    if (dev->flag & OS_DEVICE_FLAG_RDONLY)
    {
        struct os_audio_record *record = (struct os_audio_record *) os_malloc(sizeof(struct os_audio_record));
        os_uint8_t *buffer;

        if (record == OS_NULL)
            return OS_ENOMEM;
        memset(record, 0, sizeof(struct os_audio_record));

        /* init pipe for record*/
        buffer = os_malloc(OS_AUDIO_RECORD_PIPE_SIZE);
        if (buffer == OS_NULL)
        {
            os_free(record);
            LOG_EXT_E("malloc memory for for record pipe failed");
            return OS_ENOMEM;
        }
        os_audio_pipe_init(&record->pipe, "record",
                           (os_int32_t)(OS_PIPE_FLAG_FORCE_WR | OS_PIPE_FLAG_BLOCK_RD),
                           buffer,
                           OS_AUDIO_RECORD_PIPE_SIZE);

        record->activated = OS_FALSE;
        audio->record = record;
    }

    /* initialize hardware configuration */
    if (audio->ops->init)
        audio->ops->init(audio);

    /* get replay buffer information */
    if (audio->ops->buffer_info)
        audio->ops->buffer_info(audio, &audio->replay->buf_info);

    return result;
}

static os_err_t _audio_recorder_open(struct os_device *dev, os_uint16_t oflag)
{
    struct os_audio_device *audio;

    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    /* check device flag with the open flag */
    if ((oflag & OS_DEVICE_OFLAG_RDONLY) && !(dev->flag & OS_DEVICE_FLAG_RDONLY))
        return OS_EIO;

    /* get open flags */
    dev->open_flag = oflag & 0xff;

    /* initialize the Rx structure according to open flag */
    if (oflag & OS_DEVICE_OFLAG_RDONLY)
    {
        /* open record pipe */
        if (audio->record->activated != OS_TRUE)
        {
            LOG_EXT_D("open audio record device ,oflag = %x\n", oflag);

            _audio_record_start(audio);
            audio->record->activated = OS_TRUE;
        }
        dev->open_flag |= OS_DEVICE_OFLAG_RDONLY;
    }

    return OS_EOK;
}

static os_err_t _audio_recorder_close(struct os_device *dev)
{
    struct os_audio_device *audio;
    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    if (dev->open_flag & OS_DEVICE_OFLAG_RDONLY)
    {
        /* stop record stream */
        _audio_record_stop(audio);
        dev->open_flag &= ~OS_DEVICE_OFLAG_RDONLY;
    }

    return OS_EOK;
}

static os_size_t _audio_recorder_read(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct os_audio_device *audio;
    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    if (!(dev->open_flag & OS_DEVICE_OFLAG_RDONLY) || (audio->record == OS_NULL))
        return 0;

    return os_device_read(OS_DEVICE(&audio->record->pipe), pos, buffer, size);
}


static os_err_t _audio_recorder_control(struct os_device *dev, int cmd, void *args)
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
        result = _audio_record_start(audio);

        break;
    }

    case AUDIO_CTL_STOP:
    {
        result = _audio_record_stop(audio);

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

os_err_t os_audio_recorder_register(struct os_audio_device *audio, const char *name, os_uint32_t flag, void *data)
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
    device->init    = _audio_recorder_init;
    device->open    = _audio_recorder_open;
    device->close   = _audio_recorder_close;
    device->read    = _audio_recorder_read;
    device->write   = OS_NULL;
    device->control = _audio_recorder_control;
#endif
    device->user_data = data;

    /* register a character device */
    result = os_device_register(device, name, flag | OS_DEVICE_FLAG_REMOVABLE);

    /* initialize audio device */
    if (result == OS_EOK)
        result = os_device_init(device);

    return result;
}


void os_audio_rx_done(struct os_audio_device *audio, os_uint8_t *pbuf, os_size_t len)
{
    /* save data to record pipe */
    os_device_write(OS_DEVICE(&audio->record->pipe), 0, pbuf, len);

    /* invoke callback */
    if (audio->parent.rx_indicate != OS_NULL)
        audio->parent.rx_indicate(&audio->parent, len);
}
