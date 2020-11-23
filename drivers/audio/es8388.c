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
 * @file        es8388.c
 *
 * @brief       This file implements audio driver for es8388.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_device.h>
#include <os_hw.h>

#include <stdint.h>
#include <string.h>
#include "drv_cfg.h"

#include "es8388_ll.h"
#include <drv_log.h>
#include <i2c/i2c.h>

#ifdef OS_USING_SAI
#include <audio/sai.h>
#endif
#ifdef OS_USING_I2S
#include <audio/i2s.h>
#endif

#define DBG_ENABLE
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#define DBG_SECTION_NAME "ES8388"

struct es8388_player_device
{
    struct            os_audio_device audio;
    struct            os_audio_configure replay_config;
    os_uint8_t         volume; 
    os_device_t     *cfg_bus;
    os_device_t     *data_bus;
};

static os_err_t audio_es8388_config(struct os_audio_device *audio, struct os_audio_caps *caps)
{
    os_err_t result = OS_EOK;
    struct es8388_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;

    switch (caps->config_type)
    {
        case AUDIO_VOLUME_CMD:
        {
            os_uint8_t volume = caps->udata.value;

            es8388_volume_set(volume);
            aduio_dev->volume = volume;
            LOG_EXT_D("set volume %d", volume);
            break;
        }

        case AUDIO_PARAM_CMD:
        {
            os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &caps->udata.config.samplerate);
            os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &caps->udata.config.channels);
            
            aduio_dev->replay_config.samplerate = caps->udata.config.samplerate;
            aduio_dev->replay_config.channels   = caps->udata.config.channels;
            aduio_dev->replay_config.samplebits = caps->udata.config.samplebits;
            LOG_EXT_D("set samplerate %d", aduio_dev->replay_config.samplerate);
            break;
        }

        default:
            result = OS_ERROR;
            break;
        }

    return result;
}


static os_err_t audio_es8388_init(struct os_audio_device *audio)  
{  
    os_err_t result = OS_EOK;
    
    struct es8388_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;
    
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &aduio_dev->replay_config.samplerate);
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &aduio_dev->replay_config.channels);

    return result;
}

static os_err_t audio_es8388_start(struct os_audio_device *audio)
{
    struct es8388_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;
    
    LOG_EXT_D("open sound device");
    es8388_start(ES_MODE_DAC);
    
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_ENABLE, OS_NULL);
    
    return OS_EOK;
}

static os_err_t audio_es8388_stop(struct os_audio_device *audio)
{
    struct es8388_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;

    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_DISABLE, OS_NULL);
    
    es8388_stop(ES_MODE_DAC);
    
    return OS_EOK;
}

os_size_t audio_es8388_transmit(struct os_audio_device *audio, const void *writeBuf, os_size_t size)
{
    struct es8388_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;

    return os_device_write(aduio_dev->data_bus, 0, (os_uint8_t *)writeBuf, size);
}

os_size_t es8388_audio_receive(struct os_audio_device *audio, void *readBuf, os_size_t size)
{
    struct es8388_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;
    
    return os_device_read(aduio_dev->data_bus, 0, (os_uint8_t *)readBuf, size);
}

os_err_t audio_es8388_data_tx_done(os_device_t *dev, struct os_device_cb_info *info)
{
    if (dev->user_data != OS_NULL)
    {
        struct es8388_player_device *es8388_player_dev = dev->user_data;
        return es8388_player_dev->audio.parent.cb_table[OS_DEVICE_CB_TYPE_TX].cb((os_device_t *)es8388_player_dev, info);
    }
    return OS_ENOSYS;
}

static struct os_audio_ops es8388_player_ops =
{
    .getcaps            = OS_NULL,
    .configure          = audio_es8388_config,
    .init               = audio_es8388_init,
    .start              = audio_es8388_start,
    .stop               = audio_es8388_stop,
    .transmit           = audio_es8388_transmit,
    .receive            = OS_NULL,
};

int os_hw_audio_player_init(void)
{
    struct es8388_player_device *es8388_player_dev = os_calloc(1, sizeof(struct es8388_player_device));

    es8388_player_dev->replay_config.samplerate = 44100;
    es8388_player_dev->replay_config.channels   = 2;
    es8388_player_dev->replay_config.samplebits = 16;
    es8388_player_dev->volume                   = 50;

    es8388_player_dev->audio.ops = &es8388_player_ops;
    
    es8388_player_dev->cfg_bus = os_device_find(BSP_ES8388_I2C_BUS);
    if (es8388_player_dev->cfg_bus == OS_NULL)
    {
        LOG_EXT_E("can not find the config device!\n");
        return OS_ERROR;
    }
    es8388_init(es8388_player_dev->cfg_bus, BSP_ES8388_POWER_PIN);
    
    es8388_player_dev->data_bus = os_device_find(BSP_ES8388_DATA_BUS);
    if (es8388_player_dev->data_bus == OS_NULL)
    {
        LOG_EXT_E("can not find the data device!\n");
        return OS_ERROR;
    }
    es8388_player_dev->data_bus->user_data = es8388_player_dev;
    os_device_open(es8388_player_dev->data_bus, OS_DEVICE_OFLAG_RDWR);
    
    struct os_device_cb_info *info = os_calloc(1, sizeof(struct os_device_cb_info));
    info->type = OS_DEVICE_CB_TYPE_TX;
    info->cb = audio_es8388_data_tx_done;
    os_device_control(es8388_player_dev->data_bus, IOC_SET_CB, info);
    
    os_audio_player_register(&es8388_player_dev->audio, "audio0", OS_DEVICE_FLAG_WRONLY, es8388_player_dev);

    return OS_EOK;
}

OS_DEVICE_INIT(os_hw_audio_player_init);

