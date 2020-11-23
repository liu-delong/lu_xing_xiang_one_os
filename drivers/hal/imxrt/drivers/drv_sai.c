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
#include <os_memory.h>
#include <bus/bus.h>
#include <audio/sai.h>
#include <string.h>
#include <drv_log.h>
#include "drv_cfg.h"
#include "peripherals.h"
#include "drv_sai.h"

typedef struct
{
    os_device_sai_t sai;
    struct nxp_sai_info *info;
}imxrt_sai;

/* SAI transfer Tx callback function for the SAI1 component (init. function BOARD_InitPeripherals)*/
void SAI1_SAI_Tx_eDMA_CallBack(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
    os_kprintf("SAI1_SAI_Tx_eDMA_CallBack\n");

    return;
}

/* SAI transfer Rx callback function for the SAI1 component (init. function BOARD_InitPeripherals)*/
void SAI1_SAI_Rx_eDMA_CallBack(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
    os_kprintf("SAI1_SAI_Rx_eDMA_CallBack\n");

    return;
}

void imxrt_sai_dma_transmit(os_device_sai_t *sai, uint8_t *pData, uint16_t Size)
{
    sai_transfer_t xfer;
    imxrt_sai *imxrtSai = (imxrt_sai *)sai;

    os_kprintf("sai dma transmit\n");
    OS_ASSERT(sai);    

    xfer.data     = pData;
    xfer.dataSize = Size;
    if (kStatus_Success != SAI_TransferSendEDMA(imxrtSai->info->sai_base, &SAI1_SAI_Tx_eDMA_Handle, &xfer))
    {
        os_kprintf("sai transfer fail\n");
    }

    return;
}

void imxrt_sai_dma_stop(os_device_sai_t *sai)
{
    os_kprintf("sai dma stop\n");
    OS_ASSERT(sai);
    
    //HAL_SAI_DMAStop(sai->hsai);

    return;
}

void imxrt_sai_frequency_set(os_device_sai_t *sai, uint32_t frequency)
{ 
    os_kprintf("sai frequency set: %d\n", frequency);
    OS_ASSERT(sai);

    return;
}

void imxrt_sai_channel_set(os_device_sai_t *sai, uint8_t channels)
{    
    os_kprintf("sai channel set: %d\n", channels);
    OS_ASSERT(sai);

    //set monomode or steremode

    return;
}

void imxrt_sai_info(os_device_sai_t *sai, uint8_t *tx_fifo, struct os_data_queue *queue, struct os_completion *cmp, os_uint8_t *event)
{
    os_kprintf("sai info\n");

    return;
}

static struct os_device_sai_ops imxrt_sai_ops =
{
    .transimit      = OS_NULL,
    .receive        = OS_NULL,
    .enable         = OS_NULL,
    .set_frq        = OS_NULL,
    .set_channel    = OS_NULL,
};

static int imxrt_sai_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{ 
    os_err_t    result  = OS_ERROR;
    imxrt_sai *sai_dev = NULL;

    sai_dev = (imxrt_sai *)os_calloc(1, sizeof(imxrt_sai));
    OS_ASSERT(sai_dev);
    
    sai_dev->sai.ops = &imxrt_sai_ops;
    sai_dev->info = (struct nxp_sai_info *)dev->info;
        
    result = os_sai_register(dev->name, &sai_dev->sai); 
    OS_ASSERT(result == OS_EOK);
    
    return result;
}

OS_DRIVER_INFO imxrt_sai_driver = {
    .name   = "SAI_Type",
    .probe  = imxrt_sai_probe,
};

OS_DRIVER_DEFINE(imxrt_sai_driver, "1");

