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
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_irq.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <drv_cfg.h>
#include <drv_usart.h>
#include "peripherals.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

#ifndef HAL_UART_ISR_PRIORITY
#define HAL_UART_ISR_PRIORITY (25U)
#endif

#if 0
#define USART_MODE_POLL
#define USART_MODE_INTERRUPT
#define USART_MODE_NOPOLL
#define USART_MODE_DMA
#endif
#define USART_MODE_DMA

static uint8_t usart0_rxRingBuffer[100];
uint8_t usart0_txRingBuffer[]   = "Usart polling example! \r\n";

static USART_Type *const s_UsartAdapterBase[] = USART_BASE_PTRS;
struct lpc_uart *uart;

struct lpc_uart
{
    struct os_serial_device serial;
    USART_Type *uart_base;
    const usart_config_t *config;
    char *device_name;
    usart_transfer_t sendXfer;
    usart_transfer_t receiveXfer;

#if defined(USART_MODE_INTERRUPT)
    IRQn_Type irqn;
    os_int32_t priority;
//    clock_name_t clock_src;
#elif defined(USART_MODE_DMA)       
    usart_dma_handle_t *uartDmaHandle;
//    dma_handle_t *uartTxDmaHandle;
//    dma_handle_t *uartRxDmaHandle;
#endif
};

#if defined(USART_MODE_INTERRUPT)
void FLEXCOMM0_IRQHandler(void)
{
    uint8_t data;
    
    /* If new data arrived. */
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(s_UsartAdapterBase[0]))
    {
        uart->serial.rx_fifo->line_buff[0] = USART_ReadByte(s_UsartAdapterBase[0]);
        os_hw_serial_isr_rxdone(&uart->serial, 1);
    }
    SDK_ISR_EXIT_BARRIER;
}
#elif defined(USART_MODE_DMA)
void lpc_usart_callback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData)
{   
    struct lpc_uart *uart;
    
    uart = os_container_of(base, struct lpc_uart, uart_base);

    if (kStatus_USART_TxIdle == status)
    {
        os_interrupt_enter();
        os_hw_serial_isr_txdone((struct os_serial_device *)uart);
        os_interrupt_leave();
    }
    if (kStatus_USART_RxIdle == status)
    {
        os_interrupt_enter();
        os_hw_serial_isr_rxdone((struct os_serial_device *)uart, uart->receiveXfer.dataSize);
        os_interrupt_leave();
    }

    /* User data is actually CMSIS driver callback. */
    if (userData)
    {
//        ((ARM_USART_SignalEvent_t)userData)(event);
    }
}
#elif defined(USART_MODE_NOPULL)
void lpc_usart_callback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{   
    struct lpc_uart *uart;
    
    uart = os_container_of(base, struct lpc_uart, uart_base);

    if (kStatus_USART_TxIdle == status)
    {
        os_interrupt_enter();
        os_hw_serial_isr_txdone((struct os_serial_device *)uart);
        os_interrupt_leave();
    }
    if (kStatus_USART_RxIdle == status)
    {
        os_interrupt_enter();
        os_hw_serial_isr_rxdone((struct os_serial_device *)uart, uart->receiveXfer.dataSize);
        os_interrupt_leave();
    }

    /* User data is actually CMSIS driver callback. */
    if (userData)
    {
//        ((ARM_USART_SignalEvent_t)userData)(event);
    }
}

#endif

static os_err_t lpc_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    return OS_EOK;
}

static int lpc_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int32_t ret;
    status_t status;
    struct lpc_uart *uart;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct lpc_uart, serial);

    uart->sendXfer.data = (os_uint8_t *)buff;
    uart->sendXfer.dataSize = size;

#if defined(USART_MODE_DMA)
    status = USART_TransferSendDMA(uart->uart_base, uart->uartDmaHandle, &uart->sendXfer);
#elif defined(USART_MODE_NOPULL)
    status = USART_TransferSendNonBlocking(uart->uart_base, &uart->uart_handle, &uart->sendXfer);
#endif
    
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

static int lpc_uart_stop_send(struct os_serial_device *serial)
{
    struct lpc_uart *uart;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct lpc_uart, serial);
    
#if defined(USART_MODE_DMA)
    USART_TransferAbortSendDMA(uart->uart_base, uart->uartDmaHandle);
#elif defined(USART_MODE_NOPULL)
    USART_TransferAbortSend(uart->uart_base, &uart->uart_handle);
#endif
    return 0;
}

static int lpc_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    int32_t ret;
    status_t status;
    struct lpc_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct lpc_uart, serial); 

    uart->receiveXfer.data = buff;
    uart->receiveXfer.dataSize = size;
    
#if defined(USART_MODE_DMA)
    status = USART_TransferReceiveDMA(uart->uart_base, uart->uartDmaHandle, &uart->receiveXfer);
#elif defined(USART_MODE_NOPULL)
    status = USART_TransferReceiveNonBlocking(uart->uart_base, &uart->uart_handle, &uart->receiveXfer, OS_NULL);
#endif
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

static int lpc_uart_stop_recv(struct os_serial_device *serial)
{
    int32_t ret;
    status_t status;
    struct lpc_uart *uart;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct lpc_uart, serial);

#if defined(USART_MODE_DMA)
    USART_TransferAbortReceiveDMA(uart->uart_base, uart->uartDmaHandle);
#elif defined(USART_MODE_NOPULL)
    USART_TransferAbortReceive(uart->uart_base, &uart->uart_handle);
#endif
    
    return 0;
}

static int lpc_uart_recv_state(struct os_serial_device *serial)
{
    os_uint32_t count;
    status_t state;
    
    struct lpc_uart *uart;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct lpc_uart, serial);

#if defined(USART_MODE_DMA)
    state = USART_TransferGetReceiveCountDMA(uart->uart_base, uart->uartDmaHandle, &count);
#elif defined(USART_MODE_NOPULL)
    state = USART_TransferGetReceiveCount(uart->uart_base, &uart->uart_handle, &count);
#endif
    
    count |= OS_SERIAL_FLAG_RX_IDLE;

    return count;
}

static int lpc_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int32_t  ret;
    status_t status;
    
    struct lpc_uart *uart;
    
    int i;
    os_base_t level;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct lpc_uart, serial);

    for (i = 0; i < size; i++)
    {
        level = os_hw_interrupt_disable();
        status = USART_WriteBlocking(uart->uart_base, (uint8_t *)buff + i, 1);
        os_hw_interrupt_enable(level);
    }

    return (status == kStatus_Success) ? size : 0;
}

static int lpc_uart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    int32_t  ret;
    status_t status;
    
    struct lpc_uart *uart;

    os_base_t level;
    
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(size == 1);

    uart = os_container_of(serial, struct lpc_uart, serial);

    level = os_hw_interrupt_disable();
    status = USART_ReadBlocking(uart->uart_base, buff, size);
    os_hw_interrupt_enable(level);

    return (status == kStatus_Success) ? size : 0;
}

static const struct os_uart_ops lpc_uart_ops = {
    .configure    = lpc_configure,

    .start_send   = lpc_uart_start_send,
    .stop_send    = lpc_uart_stop_send,

    .start_recv   = lpc_uart_start_recv,
    .stop_recv    = lpc_uart_stop_recv,
    .recv_state   = lpc_uart_recv_state,
    
    .poll_send    = lpc_uart_poll_send,
    .poll_recv    = lpc_uart_poll_recv,
};

void lpc_usart_parse_configs_from_configtool(struct lpc_uart *uart)
{
    struct os_serial_device *serial = &uart->serial;
    
    serial->config.baud_rate = uart->config->baudRate_Bps;

    switch (uart->config->stopBitCount)
    {
    case kUSART_OneStopBit:
        serial->config.stop_bits = STOP_BITS_1;
        break;
    case kUSART_TwoStopBit:
        serial->config.stop_bits = STOP_BITS_2;
        break;
    }
    switch (uart->config->parityMode)
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

    switch (uart->config->bitCountPerChar)
    {
    case kUSART_7BitsPerChar:
        serial->config.data_bits = DATA_BITS_8;
        break;
    case kUSART_8BitsPerChar:
        serial->config.data_bits = DATA_BITS_9;
        break;
    }

}

static int lpc_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    os_err_t    result  = 0;
    os_base_t   level;
    
    hal_usart_handle_t *uart_handle = (hal_usart_handle_t *)dev->info;

    struct lpc_uart *uart = os_calloc(1, sizeof(struct lpc_uart));

    OS_ASSERT(uart);
    
    uart->uart_base = uart_handle->uart_base;
    uart->config = uart_handle->uart_config;
    uart->uartDmaHandle = uart_handle->uartDmaHandle;

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &lpc_uart_ops;
    serial->config = config;

    lpc_usart_parse_configs_from_configtool(uart);

//    level = os_hw_interrupt_disable();
//    os_list_add_tail(&lpc_uart_list, &uart->list);
//    os_hw_interrupt_enable(level);
    
    result = os_hw_serial_register(serial, dev->name, OS_DEVICE_FLAG_RDWR, NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO lpc_usart_driver = {
    .name   = "USART_Type",
    .probe  = lpc_usart_probe,
};

OS_DRIVER_DEFINE(lpc_usart_driver, "0.end.0");
