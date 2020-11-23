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
 * @file        rtt.c
 *
 * @brief       rtt
 *
 * @details     rtt
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_device.h>
#include <os_hw.h>
#include <os_irq.h>
#include <os_assert.h>

#include <os_device.h>
#include <drv_cfg.h>

#include <stdio.h>
#include <string.h>
#include "SEGGER_RTT.h"

#define RTT_BUFF_INDEX_FOR_SERIAL   (0)

struct rtt_uart
{
    struct os_serial_device serial;
    
    os_bool_t   rx_isr_enabled;

    os_uint8_t *buff;
    os_size_t   size;
    os_size_t   count;
};

static int rtt_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct rtt_uart *uart = os_container_of(serial, struct rtt_uart, serial);

    os_base_t level;

    level = os_hw_interrupt_disable();

    uart->buff  = buff;
    uart->size  = size;
    uart->count = 0;

    uart->rx_isr_enabled = OS_TRUE;

    os_hw_interrupt_enable(level);
    
    return 0;
}

static int rtt_uart_stop_recv(struct os_serial_device *serial)
{
    struct rtt_uart *uart = os_container_of(serial, struct rtt_uart, serial);
    
    uart->rx_isr_enabled = OS_FALSE;
    
    return 0;
}

static void rtt_rx_process(struct rtt_uart *uart)
{
    int count;

    if (SEGGER_RTT_HasData(RTT_BUFF_INDEX_FOR_SERIAL))
    {
        count = SEGGER_RTT_Read(RTT_BUFF_INDEX_FOR_SERIAL, uart->buff + uart->count, uart->size - uart->count);                
        uart->count += count;

        OS_ASSERT(uart->count <= uart->size);
        
        if (uart->count == uart->size)
        {
            uart->rx_isr_enabled = OS_FALSE;
            os_interrupt_enter();
            os_hw_serial_isr_rxdone((struct os_serial_device *)uart, uart->size);
            os_interrupt_leave();
        }
    }
}

static int rtt_uart_recv_state(struct os_serial_device *serial)
{    
    struct rtt_uart *uart = os_container_of(serial, struct rtt_uart, serial);

    if (uart->rx_isr_enabled == OS_TRUE)
    {
        rtt_rx_process(uart);
        return uart->count;
    }
    else
    {
        return OS_SERIAL_FLAG_RX_IDLE;
    }
}

static int rtt_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    SEGGER_RTT_Write(RTT_BUFF_INDEX_FOR_SERIAL, buff, size);

    return size;
}

static const struct os_uart_ops rtt_uart_ops = {
    .start_recv   = rtt_uart_start_recv,
    .stop_recv    = rtt_uart_stop_recv,
    .recv_state   = rtt_uart_recv_state,
    
    .poll_send    = rtt_uart_poll_send,
};

static struct rtt_uart uart;

static int os_hw_rtt_init(void)
{
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    memset(&uart, 0, sizeof(uart));

    uart.serial.ops    = &rtt_uart_ops;
    uart.serial.config = config;

    os_hw_serial_register(&uart.serial, OS_RTT_DEVICE_NAME, OS_DEVICE_FLAG_RDWR, OS_NULL);

    return 0;
}

OS_INIT_EXPORT(os_hw_rtt_init, "0.end.0");

