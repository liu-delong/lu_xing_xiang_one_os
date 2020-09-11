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

#include "drv_audio.h"
#include "es8388_ll.h"
#include <drv_log.h>
#include <i2c/i2c.h>
#include <audio/sai.h>


#define DBG_ENABLE
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#define DBG_SECTION_NAME "ES8388"

#define TX_FIFO_SIZE         (2048)


struct es8388_player_device
{
    struct            os_audio_device audio;
    struct            os_audio_configure replay_config;
    os_uint8_t        *tx_fifo;
    os_uint8_t         volume;  
    os_device_sai_t    *sai;

};

static struct es8388_player_device es8388_player_dev = {0};


/* ES8388 Device Driver Interface */

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
            /* set samplerate */
            os_sai_frequency_set(aduio_dev->sai,caps->udata.config.samplerate);
               
            /* set channels */
            os_sai_channel_set(aduio_dev->sai,caps->udata.config.channels);

            /* save configs */
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

    es8388_init(BSP_ES8388_I2C_BUS, BSP_ES8388_POWER_PIN);    
    
    /* set default params */ 
    os_sai_frequency_set(aduio_dev->sai, aduio_dev->replay_config.samplerate);
    os_sai_channel_set(aduio_dev->sai, aduio_dev->replay_config.channels);

    return result;
}

static os_err_t audio_es8388_start(struct os_audio_device *audio)
{
    struct es8388_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;

    os_sai_info(aduio_dev->sai, aduio_dev->tx_fifo, &aduio_dev->audio.replay->queue, &audio->replay->cmp, &audio->replay->event);
    
    LOG_EXT_D("open sound device");
    es8388_start(ES_MODE_DAC);
    
    os_sai_transimit(aduio_dev->sai,aduio_dev->tx_fifo, TX_FIFO_SIZE / 2);
    
    return OS_EOK;
}

static os_err_t audio_es8388_stop(struct os_audio_device *audio)
{
    struct es8388_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;

    os_sai_stop(aduio_dev->sai);
    es8388_stop(ES_MODE_DAC);
    LOG_EXT_D("close sound device");
    
    return OS_EOK;
}

static void player_buffer_info(struct os_audio_device *audio, struct os_audio_buf_info *info)
{
    struct es8388_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;
    
    info->buffer      = aduio_dev->tx_fifo;  
    info->total_size  = TX_FIFO_SIZE;
    info->block_size  = TX_FIFO_SIZE / 2;
    info->block_count = 2;
}

static struct os_audio_ops es8388_player_ops =
{
    .getcaps     = OS_NULL, 
    .configure   = audio_es8388_config,
    .init        = audio_es8388_init,
    .start       = audio_es8388_start,
    .stop        = audio_es8388_stop,
    .transmit    = OS_NULL, 
    .buffer_info = player_buffer_info,
    .frame_tx_complete = &os_audio_tx_complete,
};

int os_hw_audio_player_init(void)
{
    os_uint8_t *tx_fifo;

    if (es8388_player_dev.tx_fifo)               
        return OS_EOK;

    tx_fifo = os_malloc(TX_FIFO_SIZE);
    if (tx_fifo == OS_NULL)
        return OS_ENOMEM;
    memset(tx_fifo, 0, TX_FIFO_SIZE);
    es8388_player_dev.tx_fifo = tx_fifo;

    /* init default configuration */
    {
        es8388_player_dev.replay_config.samplerate = 44100;
        es8388_player_dev.replay_config.channels   = 2;
        es8388_player_dev.replay_config.samplebits = 16;
        es8388_player_dev.volume                   = 50;
    }

    /* register sound device */
    es8388_player_dev.audio.ops = &es8388_player_ops;
    
    es8388_player_dev.sai = (os_device_sai_t *)os_device_find(BSP_SAI_BLOCK);
    if (es8388_player_dev.sai == OS_NULL)
    {
        os_kprintf("can not find the sai device!\n");
        return OS_ERROR;
    }

    os_audio_player_register(&es8388_player_dev.audio, "audio0", OS_DEVICE_FLAG_WRONLY, &es8388_player_dev);

    return OS_EOK;
}

OS_DEVICE_INIT(os_hw_audio_player_init);



