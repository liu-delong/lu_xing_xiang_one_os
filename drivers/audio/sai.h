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
 * @file        sai.h
 *
 * @brief       SAI function declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _SAI_H_
#define _SAI_H_

#include <board.h>
#include <drv_cfg.h>
#include <os_completion.h>

enum
{
    SAI_REPLAY_EVT_NONE  = 0x00,
    SAI_REPLAY_EVT_START = 0x01,
    SAI_REPLAY_EVT_STOP  = 0x02,
};

typedef struct os_device_sai os_device_sai_t;

struct os_sai_buf_info
{
    os_uint8_t *buffer;
    os_uint16_t block_size;
    os_uint16_t block_count;
    os_uint32_t total_size;
};

struct os_device_sai_ops
{
    void (*transimit)(os_device_sai_t *sai, uint8_t *pData, uint16_t Size);
    void (*stop)(os_device_sai_t *sai);
    void (*frequency_set)(os_device_sai_t *sai, uint32_t frequency);
    void (*channel_set)(os_device_sai_t *sai, uint8_t channels);
    void (*sai_info)(os_device_sai_t *sai, uint8_t *tx_fifo, struct os_data_queue *queue, struct os_completion *cmp, os_uint8_t *event);
};

struct os_sai_replay
{
    struct os_data_queue *queue;
    struct os_sai_buf_info buf_info;
    struct os_completion *cmp;
    
    os_uint8_t *write_data;
    os_uint16_t write_index;
    os_uint16_t read_index;
    os_uint32_t pos;

    os_uint8_t *event;
};

struct os_device_sai {
    os_device_t parent;
    SAI_HandleTypeDef *hsai;
    struct  os_sai_replay *replay;  
    struct os_device_sai_ops *ops;
};

#define os_sai_transimit(sai, pData, Size)      sai->ops->transimit(sai, pData, Size)
#define os_sai_stop(sai)                        sai->ops->stop(sai)
#define os_sai_frequency_set(sai, frequency)    sai->ops->frequency_set(sai, frequency)
#define os_sai_channel_set(sai, channel)        sai->ops->channel_set(sai, channel)
#define os_sai_info(sai, tx_fifo, queue, cmp, event)  sai->ops->sai_info(sai, tx_fifo, queue, cmp, event)

void os_sai_register(const char *name, os_device_sai_t *graphic);

#endif /* _sai_H_ */
