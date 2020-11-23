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
 * @file        drv_sai.c
 *
 * @brief       This file implements sai driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "drv_cfg.h"
#include <os_memory.h>
#include <bus/bus.h>
#include <audio/sai.h>
#include <string.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.sai"
#include <drv_log.h>
#include <os_hw.h>
#include <clocksource.h>

#define SAI_BUFF_NUM 2

enum
{
    SAI_DMA_STATUS_STOP = 0U,
    SAI_DMA_STATUS_START = 1U,
};


typedef struct stm32_sai_transfer
{
    uint8_t *data;
    size_t datasize;
} stm32_sai_transfer_t;

struct stm32_sai
{
    os_device_sai_t sai;
    SAI_HandleTypeDef *hsai;
    os_uint8_t data_buff[OS_AUDIO_REPLAY_MP_BLOCK_SIZE * SAI_BUFF_NUM];
    stm32_sai_transfer_t sendXfer[SAI_BUFF_NUM];
    stm32_sai_transfer_t receiveXfer;
    uint8_t write_index;
    uint8_t write_status;
    struct os_device_cb_info info;
    
    os_list_node_t list;
    os_sem_t sem;
};

static os_list_node_t stm32_sai_list = OS_LIST_INIT(stm32_sai_list);

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    struct stm32_sai *sai_dev;
    
    os_list_for_each_entry(sai_dev, &stm32_sai_list, struct stm32_sai, list)
    {
        if (hsai == sai_dev->hsai)
        {
            os_interrupt_enter();
            if (sai_dev->sendXfer[1].data != OS_NULL)
            {
                sai_dev->info.data = sai_dev->sendXfer[1].data;
                sai_dev->info.size = sai_dev->sendXfer[1].datasize;
                memset(&sai_dev->data_buff[OS_AUDIO_REPLAY_MP_BLOCK_SIZE], 0, sai_dev->sendXfer[1].datasize);
                os_hw_sai_isr(&sai_dev->sai, &sai_dev->info);
                os_sem_post(&sai_dev->sem);
                sai_dev->sendXfer[1].data = OS_NULL;
            }
            else
            {
                LOG_EXT_W("audio play data is not ready yet!\n");
            }
            sai_dev->write_index = 1;
            os_interrupt_leave();
            break;
        }
    }
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    struct stm32_sai *sai_dev;
    
    os_list_for_each_entry(sai_dev, &stm32_sai_list, struct stm32_sai, list)
    {
        if (hsai == sai_dev->hsai)
        {
            os_interrupt_enter();
            if (sai_dev->sendXfer[0].data != OS_NULL)
            {
                sai_dev->info.data = sai_dev->sendXfer[0].data;
                sai_dev->info.size = sai_dev->sendXfer[0].datasize;
                memset(&sai_dev->data_buff[0], 0, sai_dev->sendXfer[0].datasize);
                os_hw_sai_isr(&sai_dev->sai, &sai_dev->info);
                os_sem_post(&sai_dev->sem);
                sai_dev->sendXfer[0].data = OS_NULL;
            }
            else
            {
                LOG_EXT_W("audio play data is not ready yet!\n");
            }
            sai_dev->write_index = 0;
            os_interrupt_leave();
            break;
        }
    }
}

static os_err_t stm32_sai_dma_transmit(os_device_sai_t *sai, uint8_t *buff, uint32_t size)
{
    os_base_t level;
    
    HAL_StatusTypeDef status = HAL_OK;
    
    struct stm32_sai *sai_dev = (struct stm32_sai *)sai;
    
    os_sem_wait(&sai_dev->sem, OS_IPC_WAITING_FOREVER);
    
    level = os_hw_interrupt_disable();
    memcpy(&sai_dev->data_buff[sai_dev->write_index * OS_AUDIO_REPLAY_MP_BLOCK_SIZE], buff, size);
    sai_dev->sendXfer[sai_dev->write_index].data = buff;
    sai_dev->sendXfer[sai_dev->write_index].datasize = size;
    
    if ((sai_dev->sem.count > 0) && (sai_dev->sem.count < SAI_BUFF_NUM))
    {
        if (sai_dev->write_index < SAI_BUFF_NUM - 1)
            sai_dev->write_index++;
        else
            sai_dev->write_index = 0;
    } 
    os_hw_interrupt_enable(level);
    
    if (sai_dev->write_status == SAI_DMA_STATUS_STOP)
    {
        if ((sai_dev->sendXfer[0].data) && (sai_dev->sendXfer[1].data))
        {
            status = HAL_SAI_Transmit_DMA(sai_dev->hsai, sai_dev->data_buff, OS_AUDIO_REPLAY_MP_BLOCK_SIZE);
            sai_dev->write_status = SAI_DMA_STATUS_START;
        }
    }
        
    if (status != HAL_OK)
    {
        LOG_EXT_E("sai dma start transmit failed!\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t stm32_sai_dma_receive(os_device_sai_t *sai, uint8_t *buff, uint32_t size)
{
    HAL_StatusTypeDef status;
    
    struct stm32_sai *sai_dev = (struct stm32_sai *)sai;
    
    status = HAL_SAI_Receive_DMA(sai_dev->hsai, buff, size/2);
    if (status != HAL_OK)
    {
        LOG_EXT_E("sai dma receive failed!\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t stm32_sai_dma_enable(os_device_sai_t *sai, os_bool_t enable)
{
    HAL_StatusTypeDef status;

    struct stm32_sai *sai_dev = (struct stm32_sai *)sai;
    
    if (enable == OS_TRUE)
    {
        sai_dev->write_status = SAI_DMA_STATUS_STOP;
        os_sem_init(&sai_dev->sem, "sai_sem", SAI_BUFF_NUM, OS_IPC_FLAG_FIFO);
        sai_dev->sendXfer[0].data = OS_NULL;
        sai_dev->sendXfer[0].datasize = 0;
        sai_dev->sendXfer[1].data = OS_NULL;
        sai_dev->sendXfer[1].datasize = 0;
        sai_dev->write_index = 0;
        return OS_ENOSYS;
    }
    else
    {
        status = HAL_SAI_DMAStop(sai_dev->hsai);
        sai_dev->write_status = SAI_DMA_STATUS_STOP;
        os_sem_deinit(&sai_dev->sem);
    }
    
    if (status != HAL_OK)
    {
        LOG_EXT_E("sai dma stop failed!\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t stm32_sai_set_frq(os_device_sai_t *sai, uint32_t frequency)
{
    HAL_StatusTypeDef status;
    
    struct stm32_sai *sai_dev = (struct stm32_sai *)sai;
    
    __HAL_SAI_DISABLE(sai_dev->hsai);
    sai_dev->hsai->Init.AudioFrequency = frequency;
    status = HAL_SAI_Init(sai_dev->hsai);
    if (status != HAL_OK)
    {
        LOG_EXT_E("sai init failed!\n");
        return OS_ERROR;
    }
    __HAL_SAI_ENABLE(sai_dev->hsai);

    return OS_EOK;
}

static os_err_t stm32_sai_set_channel(os_device_sai_t *sai, uint8_t channels)
{    
    HAL_StatusTypeDef status;

    struct stm32_sai *sai_dev = (struct stm32_sai *)sai;
    
    if (channels == 1)
    {
        sai_dev->hsai->Init.MonoStereoMode = SAI_MONOMODE;
    }
    else
    {
        sai_dev->hsai->Init.MonoStereoMode = SAI_STEREOMODE;
    }
    
    __HAL_SAI_DISABLE(sai_dev->hsai);
    status = HAL_SAI_Init(sai_dev->hsai);
    if (status != HAL_OK)
    {
        LOG_EXT_E("sai set channel failed!\n");
        return OS_ERROR;
    }
    __HAL_SAI_ENABLE(sai_dev->hsai);

    return OS_EOK;

}

static struct os_device_sai_ops ops =
{
    .transimit      = stm32_sai_dma_transmit,
    .receive        = stm32_sai_dma_receive,
    .enable         = stm32_sai_dma_enable,
    .set_frq        = stm32_sai_set_frq,
    .set_channel    = stm32_sai_set_channel,
};

struct stm32_sai *sai_dev;
static int stm32_sai_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    
    sai_dev = os_calloc(1, sizeof(struct stm32_sai));

    OS_ASSERT(sai_dev);
   
    sai_dev->hsai = (SAI_HandleTypeDef *)dev->info;

    sai_dev->sai.ops   = &ops;
    
    level = os_hw_interrupt_disable();
    os_list_add_tail(&stm32_sai_list, &sai_dev->list);
    os_hw_interrupt_enable(level);

    os_sai_register(dev->name, &sai_dev->sai); 

    LOG_EXT_D("stm32 sai found.\r\n");
    
    return OS_EOK;
}


OS_DRIVER_INFO stm32_sai_driver = {
    .name   = "SAI_HandleTypeDef",
    .probe  = stm32_sai_probe,
};

OS_DRIVER_DEFINE(stm32_sai_driver, "1");
