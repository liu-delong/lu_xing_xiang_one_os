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
 * @file        drv_uart.c
 *
 * @brief       This file implements uart driver for imxrt
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_irq.h>
#include <os_memory.h>
#include <bus/bus.h>

#include "fsl_lpuart.h"
#include "fsl_lpuart_edma.h"
#include "fsl_dmamux.h"

#include "drv_uart.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

struct imxrt_uart
{
    struct os_serial_device serial;

    struct imxrt_uart_info *huart;

    os_uint8_t *rx_buff;
    os_size_t   rx_count;
    os_size_t   rx_size;

    os_list_node_t list;
};

static os_list_node_t imxrt_uart_list = OS_LIST_INIT(imxrt_uart_list);

void uart_tx_callback(struct imxrt_uart *uart)
{
    os_interrupt_enter();
    os_hw_serial_isr_txdone((struct os_serial_device *)uart);
    os_interrupt_leave();
}

void uart_rx_callback(struct imxrt_uart *uart)
{
    if (LPUART_GetStatusFlags(uart->huart->uart_base) & kLPUART_RxDataRegFullFlag)
    {
        uart->rx_buff[uart->rx_count++] = LPUART_ReadByte(uart->huart->uart_base);
        if (uart->rx_count == uart->rx_size)
        {
            DisableIRQ(uart->huart->irqn);

            os_interrupt_enter();
            os_hw_serial_isr_rxdone((struct os_serial_device *)uart, uart->rx_count);
            os_interrupt_leave();

            uart->rx_buff  = OS_NULL;
            uart->rx_count = 0;
            uart->rx_size  = 0;
        }
    }
}

void LPUART1_IRQHandler(void)
{
    struct imxrt_uart *uart;

    os_list_for_each_entry(uart, &imxrt_uart_list, struct imxrt_uart, list)
    {
        if (uart->huart->uart_base == LPUART1)
        {
            break;
        }
    }

    if (uart->huart->uart_base != LPUART1)
        return;

    /* kLPUART_RxDataRegFullFlag can only cleared or set by hardware */
    if (LPUART_GetStatusFlags(uart->huart->uart_base) & kLPUART_RxDataRegFullFlag)
    {
        uart_rx_callback(uart);
    }

    if (LPUART_GetStatusFlags(uart->huart->uart_base) & kLPUART_RxOverrunFlag)
    {
        /* Clear overrun flag, otherwise the RX does not work. */
        LPUART_ClearStatusFlags(uart->huart->uart_base, kLPUART_RxOverrunFlag);
    }
}

static uint32_t GetUartSrcFreq(void)
{
    uint32_t freq;

    /* To make it simple, we assume default PLL and divider settings, and the only variable
       from application is use PLL3 source or OSC source */
    if (CLOCK_GetMux(kCLOCK_UartMux) == 0) /* PLL3 div6 80M */
    {
        freq = (CLOCK_GetPllFreq(kCLOCK_PllUsb1) / 6U) / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    }
    else
    {
        freq = CLOCK_GetOscFreq() / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
    }

    return freq;
}

static os_err_t imxrt_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct imxrt_uart *uart;
    lpuart_config_t config;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = os_container_of(serial, struct imxrt_uart, serial);

    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = cfg->baud_rate;

    switch (cfg->data_bits)
    {
    case DATA_BITS_7:
        config.dataBitsCount = kLPUART_SevenDataBits;
        break;

    default:
        config.dataBitsCount = kLPUART_EightDataBits;
        break;
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_2:
        config.stopBitCount = kLPUART_TwoStopBit;
        break;
    default:
        config.stopBitCount = kLPUART_OneStopBit;
        break;
    }

    switch (cfg->parity)
    {
    case PARITY_ODD:
        config.parityMode = kLPUART_ParityOdd;
        break;
    case PARITY_EVEN:
        config.parityMode = kLPUART_ParityEven;
        break;
    default:
        config.parityMode = kLPUART_ParityDisabled;
        break;
    }

    config.enableTx = true;
    config.enableRx = true;

    LPUART_Init(uart->huart->uart_base, &config, GetUartSrcFreq());

    return OS_EOK;
}

static int imxrt_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    return size;
}

static int imxrt_uart_stop_send(struct os_serial_device *serial)
{
    return 0;
}

static int imxrt_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct imxrt_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct imxrt_uart, serial);    

    uart->rx_buff  = buff;
    uart->rx_count = 0;
    uart->rx_size  = size;

    LPUART_EnableInterrupts(uart->huart->uart_base, kLPUART_RxDataRegFullInterruptEnable);
    NVIC_SetPriority(uart->huart->irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0));
    EnableIRQ(uart->huart->irqn);

    return size;
}

static int imxrt_uart_stop_recv(struct os_serial_device *serial)
{
    struct imxrt_uart *uart;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct imxrt_uart, serial);

    DisableIRQ(uart->huart->irqn);

    uart->rx_buff  = OS_NULL;
    uart->rx_count = 0;
    uart->rx_size  = 0;

    return 0;
}

static int imxrt_uart_recv_state(struct os_serial_device *serial)
{
    struct imxrt_uart *uart;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct imxrt_uart, serial);

    if (uart->rx_buff != OS_NULL)
    {
        return uart->rx_count;
    }
    else
    {
        return OS_SERIAL_FLAG_RX_IDLE;
    }
}

static int imxrt_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct imxrt_uart *uart;

    int i;
    os_base_t level;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct imxrt_uart, serial);

    for (i = 0; i < size; i++)
    {
        level = os_hw_interrupt_disable();
        
        LPUART_WriteByte(uart->huart->uart_base, buff[i]);
        while (!(LPUART_GetStatusFlags(uart->huart->uart_base) & kLPUART_TxDataRegEmptyFlag));

        os_hw_interrupt_enable(level);
    }

    return size;
}

static int imxrt_uart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    return 0;
}

static const struct os_uart_ops imxrt_uart_ops = {
    .configure    = imxrt_configure,

    .start_send   = imxrt_uart_start_send,
    .stop_send    = imxrt_uart_stop_send,

    .start_recv   = imxrt_uart_start_recv,
    .stop_recv    = imxrt_uart_stop_recv,
    .recv_state   = imxrt_uart_recv_state,
    
    .poll_send    = imxrt_uart_poll_send,
    .poll_recv    = imxrt_uart_poll_recv,
};

static int imxrt_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    os_err_t    result  = 0;
    os_base_t   level;

    struct imxrt_uart *uart = os_calloc(1, sizeof(struct imxrt_uart));

    OS_ASSERT(uart);

    uart->huart = (struct imxrt_uart_info *)dev->info;

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &imxrt_uart_ops;
    serial->config = config;

    level = os_hw_interrupt_disable();
    os_list_add_tail(&imxrt_uart_list, &uart->list);
    os_hw_interrupt_enable(level);
    
    result = os_hw_serial_register(serial, dev->name, OS_DEVICE_FLAG_RDWR, NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO imxrt_uart_driver = {
    .name   = "IMX_UART",
    .probe  = imxrt_usart_probe,
};

OS_DRIVER_DEFINE(imxrt_uart_driver, "0.end.0");

