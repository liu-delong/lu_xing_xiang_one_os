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
 * @brief       This file implements usart driver for ingenic
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_usart.h>
#include <os_errno.h>
#include <drv_common.h>
#include <ingenic_ost.h>
#include <interrupt.h>
#include <os_memory.h>

static os_err_t ingenic_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{

    return OS_EOK;
}

static int ingenic_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    return OS_EOK;
}

static int ingenic_uart_stop_send(struct os_serial_device *serial)
{
    return OS_EOK;
}

static int ingenic_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct ingenic_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);


    uart->count = 0;
    uart->buffer = buff;
    uart->size = size;
    uart->state = SET;

    uart_enableirq(uart->usart_info->irqno,uart->usart_info->hw_base);

    return OS_EOK;
}

static int ingenic_uart_stop_recv(struct os_serial_device *serial)
{
    struct ingenic_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    uart->count = 0;
    uart->buffer = OS_NULL;
    uart->state = RESET;

    uart_disableirq(uart->usart_info->irqno,uart->usart_info->hw_base);

    return OS_EOK;
}

static int ingenic_uart_recv_state(struct os_serial_device *serial)
{
    int state = 0;
    struct ingenic_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    state = uart->count;

    if (uart->state == RESET)
        state |= OS_SERIAL_FLAG_RX_IDLE;
    return state;
}

static int ingenic_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    os_base_t level;
    os_uint32_t i=0,retry=0;
    struct ingenic_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_hw_interrupt_disable();
        while(uart_sendstate(uart->usart_info->hw_base))
        {
            retry ++;
            if (retry > 0xfffff)
            {
                retry = 0;
            }
        }
        uart_senddatapoll(uart->usart_info->hw_base,*(os_uint8_t *)(buff+i));
        os_hw_interrupt_enable(level);
    }
    return size;
}

static int ingenic_uart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    return OS_EOK;
}

static const struct os_uart_ops ingenic_uart_ops = {
    .configure    = ingenic_configure,

    .start_send   = ingenic_uart_start_send,
    .stop_send    = ingenic_uart_stop_send,

    .start_recv   = ingenic_uart_start_recv,
    .stop_recv    = ingenic_uart_stop_recv,
    .recv_state   = ingenic_uart_recv_state,

    .poll_send    = ingenic_uart_poll_send,
    .poll_recv    = ingenic_uart_poll_recv,
};

static os_err_t uart_configure (struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct ingenic_uart *uart;
    struct uart_hw_config hw_cfg;

    OS_ASSERT(serial != OS_NULL);
    serial->config = *cfg;

    uart = serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    hw_cfg.baud_rate = cfg->baud_rate;
    hw_cfg.bit_order = cfg->bit_order;
    hw_cfg.data_bits = cfg->data_bits;
    hw_cfg.parity = cfg->parity;
    hw_cfg.stop_bits = cfg->stop_bits;

    return uart_init(uart->usart_info->hw_base,&hw_cfg);

}

static void uart_isr(struct os_serial_device *serial)
{

    struct ingenic_uart* uart = serial->parent.user_data;

    while(1){


        if(!uart_receivestate(uart->usart_info->hw_base))
        {
            *(uart->buffer+uart->count) = uart_receivedata(uart->usart_info->hw_base);
            uart->count++;
            if(uart->count == uart->size)
            {
                uart->state = RESET;
                uart_disableirq(uart->usart_info->irqno,uart->usart_info->hw_base);
            }
        }
        else
            break;
    }


}

static void uart_irq_handler(int irqno, void *param)
{
    os_ubase_t isr;
    struct os_serial_device *serial = (struct os_serial_device*)param;
    struct ingenic_uart *uart = serial->parent.user_data;

    /* read interrupt status and clear it */
    isr = uart_get_interrupt(uart->usart_info->hw_base);
    if (isr & UARTISR_IID_RDI)      /* Receive Data Available */
    {
        uart_isr(serial);
    }

    return;
}

static void __os_hw_usart_init(struct ingenic_uart *uart,const os_device_info_t *dev)
{

    clk_enable(clk_get(dev->name));
    os_hw_interrupt_install(uart->usart_info->irqno, uart_irq_handler,&(uart->serial_dev),dev->name);

    return;
}

static int ingenic_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_uint32_t i;	
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

    os_err_t result  = 0;

    struct ingenic_uart *uart = os_calloc(1, sizeof(struct ingenic_uart));

    OS_ASSERT(uart);

    uart->usart_info = (struct ingenic_usart_info *)dev->info;
    uart->state = RESET;

    struct os_serial_device *serial = &(uart->serial_dev);

    __os_hw_usart_init(uart,dev);

    serial->ops = &ingenic_uart_ops;
    serial->config = config;

    result = os_hw_serial_register(serial, dev->name, OS_DEVICE_FLAG_RDWR,uart);

    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO ingenic_usart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = ingenic_usart_probe,
};

OS_DRIVER_DEFINE(ingenic_usart_driver, "0.end.0");


