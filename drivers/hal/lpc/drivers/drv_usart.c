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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for lpc
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_irq.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <drv_cfg.h>
#include <drv_usart.h>
#include <os_clock.h>
#include <string.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

struct nxp_usart
{
    struct os_serial_device serial;
    struct nxp_usart_info *usart_info;
    hal_usart_transfer_t sendXfer;
    hal_usart_transfer_t receiveXfer;
    os_uint32_t clk_src;

    IRQn_Type irqn;
    usart_handle_t *usart_handle;
    usart_dma_handle_t *usart_DmaHandle;

    os_list_node_t list;
};

static os_list_node_t nxp_usart_list = OS_LIST_INIT(nxp_usart_list);

void nxp_usart_irq_callback(struct nxp_usart *nxp_usart)
{
//    if (USART_GetStatusFlags(nxp_usart->usart_info->usart_base) & kUSART_RxFifoNotEmptyFlag)
//    {
//        nxp_usart->receiveXfer.transfer.data[nxp_usart->receiveXfer.count_cur] = USART_ReadByte(nxp_usart->usart_info->usart_base);
//        nxp_usart->receiveXfer.count_cur++;
//        if ((nxp_usart->receiveXfer.count_cur == nxp_usart->receiveXfer.transfer.dataSize) && (nxp_usart->receiveXfer.transfer.data != OS_NULL))
//        {
//            USART_DisableInterrupts(nxp_usart->usart_info->usart_base, kUSART_RxLevelInterruptEnable);
//            os_interrupt_enter();
//            os_hw_serial_isr_rxdone((struct os_serial_device *)nxp_usart, nxp_usart->receiveXfer.transfer.dataSize);
//            os_interrupt_leave();
//            
//            nxp_usart->receiveXfer.transfer.data = OS_NULL;
//            nxp_usart->receiveXfer.count_cur = 0;
//            nxp_usart->receiveXfer.transfer.dataSize = 0;
//        }
//    }
//    else 
//    {
//        USART_ClearStatusFlags(nxp_usart->usart_info->usart_base, USART_FIFOSTAT_TXERR_MASK | USART_FIFOSTAT_RXERR_MASK);
//    }
}

USART_IRQHandler_DEFINE(0);
USART_IRQHandler_DEFINE(1);
USART_IRQHandler_DEFINE(2);
USART_IRQHandler_DEFINE(3);
USART_IRQHandler_DEFINE(4);
USART_IRQHandler_DEFINE(5);
USART_IRQHandler_DEFINE(6);
USART_IRQHandler_DEFINE(7);
USART_IRQHandler_DEFINE(8);

void nxp_usart_dma_callback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData)
{
    struct nxp_usart *nxp_usart = (struct nxp_usart *)userData;
    
    if (kStatus_USART_TxIdle == status)
    {
        os_interrupt_enter();
        os_hw_serial_isr_txdone((struct os_serial_device *)nxp_usart);
        os_interrupt_leave();
    }
    
    if (kStatus_USART_RxIdle == status)
    {
        os_interrupt_enter();
        os_hw_serial_isr_rxdone((struct os_serial_device *)nxp_usart, nxp_usart->receiveXfer.transfer.dataSize);
        os_interrupt_leave();
    }
}

void nxp_usart_transfer_callback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    struct nxp_usart *nxp_usart = (struct nxp_usart *)userData;
    
    if (kStatus_USART_TxIdle == status)
    {
        os_interrupt_enter();
        os_hw_serial_isr_txdone((struct os_serial_device *)nxp_usart);
        os_interrupt_leave();
    }
    
    if (kStatus_USART_RxIdle == status)
    {
        os_interrupt_enter();
        os_hw_serial_isr_rxdone((struct os_serial_device *)nxp_usart, nxp_usart->receiveXfer.transfer.dataSize);
        os_interrupt_leave();
    }
}

static os_err_t nxp_usart_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct nxp_usart *nxp_usart = (struct nxp_usart *)serial;
    
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    LOG_EXT_D("sofeware just use to config baud_rate, other param should use MCUXpresso config tool!\n");
    
    if (USART_SetBaudRate(nxp_usart->usart_info->usart_base, cfg->baud_rate, nxp_usart->clk_src) != kStatus_Success)
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static int nxp_usart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int32_t ret = OS_EOK;
    status_t status;
    
    struct nxp_usart *nxp_usart;
    
    OS_ASSERT(serial != OS_NULL);

    nxp_usart = (struct nxp_usart *)serial;

    nxp_usart->sendXfer.transfer.data = (os_uint8_t *)buff;
    nxp_usart->sendXfer.transfer.dataSize = size;

    if (nxp_usart->usart_DmaHandle != OS_NULL)
    {
        status = USART_TransferSendDMA(nxp_usart->usart_info->usart_base, nxp_usart->usart_DmaHandle, &nxp_usart->sendXfer.transfer);
    }
    else if (nxp_usart->usart_handle != OS_NULL)
    {
        status = USART_TransferSendNonBlocking(nxp_usart->usart_info->usart_base, nxp_usart->usart_handle, &nxp_usart->sendXfer.transfer);
    }
    else if (nxp_usart->irqn >= 14)
    {
        LOG_EXT_E("lpc55 usart not support irq mode! please use transfer or dma or polling!\n");
        status = kStatus_InvalidArgument;
    }
    else
    {
        LOG_EXT_E("lpc55 usart polling mode don't use start! serial should use polling mode!\n");
        status = kStatus_InvalidArgument;
    }

    switch (status)
    {
        case kStatus_Success:
            ret = OS_EOK;
            break;
        case kStatus_InvalidArgument:
            ret = OS_EINVAL;
            break;
        case kStatus_USART_TxBusy:
            ret = OS_EBUSY;
            break;
        default:
            ret = OS_ERROR;
            break;
    }

    return (ret == OS_EOK) ? size : 0;
}

static int nxp_usart_stop_send(struct os_serial_device *serial)
{
    int32_t ret = OS_EOK;
    status_t status = kStatus_Success;
    
    struct nxp_usart *nxp_usart;
    
    OS_ASSERT(serial != OS_NULL);

    nxp_usart = (struct nxp_usart *)serial;
    
    if (nxp_usart->usart_DmaHandle != OS_NULL)
    {
        USART_TransferAbortSendDMA(nxp_usart->usart_info->usart_base, nxp_usart->usart_DmaHandle);
    }
    else if (nxp_usart->usart_handle != OS_NULL)
    {
        USART_TransferAbortSend(nxp_usart->usart_info->usart_base, nxp_usart->usart_handle);
    }
    else if (nxp_usart->irqn >= 14)
    {
        return 0;
    }
    else
    {
        return 0;
    }

    switch (status)
    {
        case kStatus_Success:
            ret = OS_EOK;
            break;
        case kStatus_InvalidArgument:
            ret = OS_EINVAL;
            break;
        case kStatus_USART_TxBusy:
            ret = OS_EBUSY;
            break;
        default:
            ret = OS_ERROR;
            break;
    }
    
    return ret;
}

static int nxp_usart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    int32_t ret = OS_EOK;
    status_t status;
    
    struct nxp_usart *nxp_usart;

    OS_ASSERT(serial != OS_NULL);

    nxp_usart = (struct nxp_usart *)serial;

    nxp_usart->receiveXfer.transfer.data = buff;
    nxp_usart->receiveXfer.transfer.dataSize = size;
    nxp_usart->receiveXfer.count_cur = 0;

    if (nxp_usart->usart_DmaHandle != OS_NULL)
    {
        status = USART_TransferReceiveDMA(nxp_usart->usart_info->usart_base, nxp_usart->usart_DmaHandle, &nxp_usart->receiveXfer.transfer);
    }
    else if (nxp_usart->usart_handle != OS_NULL)
    {
        status = USART_TransferReceiveNonBlocking(nxp_usart->usart_info->usart_base, nxp_usart->usart_handle, &nxp_usart->receiveXfer.transfer, OS_NULL);
    }
    else if (nxp_usart->irqn >= 14)
    {
        return 0;
    }
    else
    {
        return 0;
    }
    
    switch (status)
    {
        case kStatus_Success:
            ret = OS_EOK;
            break;
        case kStatus_InvalidArgument:
            ret = OS_EINVAL;
            break;
        case kStatus_USART_TxBusy:
            ret = OS_EBUSY;
            break;
        default:
            ret = OS_ERROR;
            break;

    }

    return (ret == OS_EOK) ? size : 0;
}

static int nxp_usart_stop_recv(struct os_serial_device *serial)
{
    int32_t ret = OS_EOK;
    status_t status;
    struct nxp_usart *nxp_usart;

    OS_ASSERT(serial != OS_NULL);

    nxp_usart = (struct nxp_usart *)serial;

    if (nxp_usart->usart_DmaHandle != OS_NULL)
    {
        USART_TransferAbortReceiveDMA(nxp_usart->usart_info->usart_base, nxp_usart->usart_DmaHandle);
    }
    else if (nxp_usart->usart_handle != OS_NULL)
    {
        USART_TransferAbortReceive(nxp_usart->usart_info->usart_base, nxp_usart->usart_handle);
    }
    else if (nxp_usart->irqn >= 14)
    {
        
    }
    else
    {
    }
    
    nxp_usart->receiveXfer.transfer.data = OS_NULL;
    nxp_usart->receiveXfer.transfer.dataSize = 0;
    nxp_usart->receiveXfer.count_cur = 0;
    
    switch (status)
    {
        case kStatus_Success:
            ret = OS_EOK;
            break;
        case kStatus_InvalidArgument:
            ret = OS_EINVAL;
            break;
        case kStatus_USART_TxBusy:
            ret = OS_EBUSY;
            break;
        default:
            ret = OS_ERROR;
            break;
    }

    return ret;
}

static int nxp_usart_recv_state(struct os_serial_device *serial)
{
    status_t state;
    
    struct nxp_usart *nxp_usart;
    
    OS_ASSERT(serial != OS_NULL);

    nxp_usart = (struct nxp_usart *)serial;

    if (nxp_usart->usart_DmaHandle != OS_NULL)
    {
        state = USART_TransferGetReceiveCountDMA(nxp_usart->usart_info->usart_base, nxp_usart->usart_DmaHandle, (uint32_t *)&nxp_usart->receiveXfer.count_cur);
    }
    else if (nxp_usart->usart_handle != OS_NULL)
    {
        state = USART_TransferGetReceiveCount(nxp_usart->usart_info->usart_base, nxp_usart->usart_handle, (uint32_t *)&nxp_usart->receiveXfer.count_cur);
    }
    else if (nxp_usart->irqn >= 14)
    {
        return 0;
    }
    else
    {
        return 0;
    }
    
    if (serial->rx_timer_status != 0x01)
        return nxp_usart->receiveXfer.count_cur | OS_SERIAL_FLAG_RX_IDLE;
    else
        return nxp_usart->receiveXfer.count_cur;
}

static int nxp_usart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int32_t  ret;
    status_t status;
    
    struct nxp_usart *nxp_usart;
    
    int i;
    os_base_t level;
    
    OS_ASSERT(serial != OS_NULL);

    nxp_usart = os_container_of(serial, struct nxp_usart, serial);

    level = os_hw_interrupt_disable();
    status = USART_WriteBlocking(nxp_usart->usart_info->usart_base, (uint8_t *)buff, size);
    os_hw_interrupt_enable(level);

    return (status == kStatus_Success) ? size : 0;
}

static int nxp_usart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    int32_t  ret;
    status_t status;
    
    struct nxp_usart *nxp_usart;

    os_base_t level;
    
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(size == 1);

    nxp_usart = os_container_of(serial, struct nxp_usart, serial);

    level = os_hw_interrupt_disable();
    status = USART_ReadBlocking(nxp_usart->usart_info->usart_base, buff, size);
    os_hw_interrupt_enable(level);

    return (status == kStatus_Success) ? size : 0;
}

static const struct os_uart_ops nxp_usart_ops = {
    .configure    = nxp_usart_configure,

    .start_send   = nxp_usart_start_send,
    .stop_send    = nxp_usart_stop_send,

    .start_recv   = nxp_usart_start_recv,
    .stop_recv    = nxp_usart_stop_recv,
    .recv_state   = nxp_usart_recv_state,
    
    .poll_send    = nxp_usart_poll_send,
    .poll_recv    = nxp_usart_poll_recv,
};

void nxp_usart_parse_configs_from_configtool(struct nxp_usart *nxp_usart)
{
    struct os_serial_device *serial = &nxp_usart->serial;
    
    serial->config.baud_rate = nxp_usart->usart_info->usart_config->baudRate_Bps;
    switch (nxp_usart->usart_info->usart_config->stopBitCount)
    {
    case kUSART_OneStopBit:
        serial->config.stop_bits = STOP_BITS_1;
        break;
    case kUSART_TwoStopBit:
        serial->config.stop_bits = STOP_BITS_2;
        break;
    }
    switch (nxp_usart->usart_info->usart_config->parityMode)
    {
    case kUSART_ParityDisabled:
        serial->config.parity   = PARITY_NONE;
        break;
    case kUSART_ParityOdd:
        serial->config.parity   = PARITY_ODD;
        break;
    case kUSART_ParityEven:
        serial->config.parity   = PARITY_EVEN;
        break;
    }

    switch (nxp_usart->usart_info->usart_config->bitCountPerChar)
    {
    case kUSART_7BitsPerChar:
        serial->config.data_bits = DATA_BITS_7;
        break;
    case kUSART_8BitsPerChar:
        serial->config.data_bits = DATA_BITS_8;
        break;
    }

}


void nxp_usart_param_cfg(struct nxp_usart *nxp_usart)
{
    switch((os_uint32_t)nxp_usart->usart_info->usart_base)
    {
    case (os_uint32_t)FLEXCOMM0:
        USART0_CFG_INIT(nxp_usart, 0);
        break;
    case (os_uint32_t)FLEXCOMM1:
        USART1_CFG_INIT(nxp_usart, 1);
        break;
    case (os_uint32_t)FLEXCOMM2:
        USART2_CFG_INIT(nxp_usart, 2);
        break;
    case (os_uint32_t)FLEXCOMM3:
        USART3_CFG_INIT(nxp_usart, 3);
        break;
    case (os_uint32_t)FLEXCOMM4:
        USART4_CFG_INIT(nxp_usart, 4);
        break;
    case (os_uint32_t)FLEXCOMM5:
        USART5_CFG_INIT(nxp_usart, 5);
        break;
    case (os_uint32_t)FLEXCOMM6:
        USART6_CFG_INIT(nxp_usart, 6);
        break;
    case (os_uint32_t)FLEXCOMM7:
        USART7_CFG_INIT(nxp_usart, 7);
        break;
    case (os_uint32_t)FLEXCOMM8:
        USART8_CFG_INIT(nxp_usart, 8);
        break;
    default:
        break;
    }
}

static int nxp_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    os_base_t   level;
    
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    struct nxp_usart_info *usart_info = (struct nxp_usart_info *)dev->info;

    struct nxp_usart *nxp_usart = os_calloc(1, sizeof(struct nxp_usart));

    OS_ASSERT(nxp_usart);
    
    nxp_usart->usart_info = usart_info;
    nxp_usart_param_cfg(nxp_usart);
    
    struct os_serial_device *serial = &nxp_usart->serial;

    serial->ops    = &nxp_usart_ops;
    serial->config = config;

    nxp_usart_parse_configs_from_configtool(nxp_usart);

    level = os_hw_interrupt_disable();
    os_list_add_tail(&nxp_usart_list, &nxp_usart->list);
    os_hw_interrupt_enable(level);
    
    result = os_hw_serial_register(serial, dev->name, OS_DEVICE_FLAG_RDWR, NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO nxp_usart_driver = {
    .name   = "USART_Type",
    .probe  = nxp_usart_probe,
};

OS_DRIVER_DEFINE(nxp_usart_driver, "0.end.0");
