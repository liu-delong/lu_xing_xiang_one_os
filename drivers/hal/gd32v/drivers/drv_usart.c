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
 * @brief       This file implements usart driver for gd32v.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_hw.h>
#include <serial.h>
#include <drv_usart.h>
#include <os_task.h>
#include "gd32vf103.h"
#include <os_errno.h>
#include <os_irq.h>
#include <os_assert.h>
#include <os_device.h>
#include <drv_cfg.h>
#include <os_memory.h>

#ifdef OS_USING_SERIAL

#if !defined(OS_USING_USART0) && !defined(OS_USING_USART1) && !defined(OS_USING_USART2)
#error "Please define at least one BSP_USING_UARTx"
#endif

static os_list_node_t gd32_uart_list = OS_LIST_INIT(gd32_uart_list);

static void uart_isr(struct os_serial_device *serial);

#if defined(OS_USING_USART0)
void USART0_IRQHandler(void)
{
	struct gd32_uart *uart;
	
    os_list_for_each_entry(uart, &gd32_uart_list, struct gd32_uart, list)
    {
        if (uart->usart_info->uart_periph == USART0)
        {
			/* enter interrupt */
			os_interrupt_enter();
			uart_isr(&uart->serial);
			/* leave interrupt */
			os_interrupt_leave();
            break;
        }
    }

}

#endif /* OS_USING_USART0 */

#if defined(OS_USING_USART1)
void USART1_IRQHandler(void)
{
	struct gd32_uart *uart;
	
    os_list_for_each_entry(uart, &gd32_uart_list, struct gd32_uart, list)
    {
        if (uart->usart_info->uart_periph == USART1)
        {
			/* enter interrupt */
			os_interrupt_enter();
			uart_isr(&uart->serial);
			/* leave interrupt */
			os_interrupt_leave();
            break;
        }
    }
}

#endif /* OS_USING_UART1 */



/**
 ***********************************************************************************************************************
 * @brief           gd32_uart_gpio_init:UART MSP Initialization
 *
 * @details         This function configures the hardware resources used in this example:
 *                  - Peripheral's clock enable
 *                  - Peripheral's GPIO Configuration
 *                  - NVIC configuration for UART interrupt request enable
 *
 * @param[in]       uart            UART handle pointer
 *
 * @return          none
 ***********************************************************************************************************************
 */
void gd32_uart_gpio_init(struct gd32_uart *uart)
{
	/* enable USART clock */
	rcu_periph_clock_enable(uart->usart_info->tx_gpio_clk);
	rcu_periph_clock_enable(uart->usart_info->rx_gpio_clk);
	rcu_periph_clock_enable(uart->usart_info->per_clk);

	/* connect port to USARTx_Tx */
	gpio_init(uart->usart_info->tx_port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, uart->usart_info->tx_pin);
	/* connect port to USARTx_Rx */
	gpio_init(uart->usart_info->rx_port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, uart->usart_info->rx_pin);

	eclic_irq_enable(uart->usart_info->irqn, 2, 2);
}

static os_err_t gd32_uart_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
	struct gd32_uart *uart;

	OS_ASSERT(serial != OS_NULL);
	OS_ASSERT(cfg != OS_NULL);

	uart = (struct gd32_uart *)serial->parent.user_data;

	gd32_uart_gpio_init(uart);

	usart_baudrate_set(uart->usart_info->uart_periph, cfg->baud_rate);

	switch (cfg->data_bits)
	{
		case DATA_BITS_9:
			usart_word_length_set(uart->usart_info->uart_periph, USART_WL_9BIT);
			break;

		default:
			usart_word_length_set(uart->usart_info->uart_periph, USART_WL_8BIT);
			break;
	}

	switch (cfg->stop_bits)
	{
		case STOP_BITS_2:
			usart_stop_bit_set(uart->usart_info->uart_periph, USART_STB_2BIT);
			break;
		default:
			usart_stop_bit_set(uart->usart_info->uart_periph, USART_STB_1BIT);
			break;
	}

	switch (cfg->parity)
	{
		case PARITY_ODD:
			usart_parity_config(uart->usart_info->uart_periph, USART_PM_ODD);
			break;
		case PARITY_EVEN:
			usart_parity_config(uart->usart_info->uart_periph, USART_PM_EVEN);
			break;
		default:
			usart_parity_config(uart->usart_info->uart_periph, USART_PM_NONE);
			break;
	}
	usart_hardware_flow_rts_config(uart->usart_info->uart_periph, USART_RTS_DISABLE);
	usart_hardware_flow_cts_config(uart->usart_info->uart_periph, USART_CTS_DISABLE);
	usart_receive_config(uart->usart_info->uart_periph, USART_RECEIVE_ENABLE);
	usart_transmit_config(uart->usart_info->uart_periph, USART_TRANSMIT_ENABLE);
	usart_enable(uart->usart_info->uart_periph);

	return OS_EOK;
}

static os_err_t gd32_uart_control(struct os_serial_device *serial, int cmd, void *arg)
{
	struct gd32_uart *uart;

	OS_ASSERT(serial != OS_NULL);
	uart = (struct gd32_uart *)serial->parent.user_data;

	switch (cmd)
	{
		case OS_DEVICE_CTRL_CLR_INT:
			/* disable rx irq */
			eclic_irq_disable(uart->usart_info->irqn);
			/* disable interrupt */
			usart_interrupt_disable(uart->usart_info->uart_periph, USART_INT_RBNE);

			break;
		case OS_DEVICE_CTRL_SET_INT:
			/* enable rx irq */
			eclic_irq_enable(uart->usart_info->irqn, 2, 2);
			/* enable interrupt */
			usart_interrupt_enable(uart->usart_info->uart_periph, USART_INT_RBNE);
			break;
	}

	return OS_EOK;
}


/**
 ***********************************************************************************************************************
 * @brief           uart_isr:Uart common interrupt process. This need add to uart ISR.
 *
 * @param[in]       serial          serial device.
 *
 * @return          none
 ***********************************************************************************************************************
 */
static void uart_isr(struct os_serial_device *serial)
{

	struct gd32_uart *uart = (struct gd32_uart *)serial->parent.user_data;

	OS_ASSERT(uart != OS_NULL);

	if(RESET != usart_interrupt_flag_get(uart->usart_info->uart_periph, USART_INT_FLAG_RBNE) &&
			(usart_flag_get(uart->usart_info->uart_periph, USART_FLAG_RBNE) != RESET))
	{

		/* receive data */
		*(unsigned char *)(uart->buffer+uart->count) = (unsigned char)usart_data_receive(uart->usart_info->uart_periph);

		while ((usart_flag_get(uart->usart_info->uart_periph, USART_FLAG_TC) == RESET));
		uart->count++;
		if(uart->count == uart->size)
		{
			usart_interrupt_disable(uart->usart_info->uart_periph, USART_INT_TBE);
			uart->state = RESET;
		}

	}
}

static int gd32_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
	return OS_EOK;
}

static int gd32_uart_stop_send(struct os_serial_device *serial)
{
	return OS_EOK;
}

static int gd32_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
	struct gd32_uart *uart = (struct gd32_uart *)serial->parent.user_data;

	OS_ASSERT(uart != OS_NULL);

	uart->count = 0;
	uart->buffer = buff;
	uart->size = size;
	uart->state = SET;

	usart_interrupt_enable(uart->usart_info->uart_periph, USART_INT_RBNE);
	return OS_EOK;
}



static int gd32_uart_stop_recv(struct os_serial_device *serial)
{
	struct gd32_uart *uart = (struct gd32_uart *)serial->parent.user_data;

	OS_ASSERT(uart != OS_NULL);

	usart_interrupt_disable(uart->usart_info->uart_periph, USART_INT_RBNE);

	uart->count = 0;
	uart->buffer = OS_NULL;
	uart->state = RESET;

	return OS_EOK;
}

static int gd32_uart_recv_state(struct os_serial_device *serial)
{
	int state = 0;
	struct gd32_uart *uart = (struct gd32_uart *)serial->parent.user_data;

	OS_ASSERT(uart != OS_NULL);

	state = uart->count;

	if (uart->state == RESET)
		state |= OS_SERIAL_FLAG_RX_IDLE;
	return state;
}

static int gd32_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
	struct gd32_uart *uart;
	os_size_t i;
	os_base_t level;

	OS_ASSERT(serial != OS_NULL);

	uart = (struct gd32_uart *)serial->parent.user_data;

	OS_ASSERT(uart != OS_NULL);

	for (i = 0; i < size; i++)
	{
		level = os_hw_interrupt_disable();
		usart_data_transmit(uart->usart_info->uart_periph,buff[i]);
		while ((usart_flag_get(uart->usart_info->uart_periph, USART_FLAG_TC) == RESET));
		os_hw_interrupt_enable(level);
	}    

	return size;

}


static int gd32_uart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{

	return OS_EOK;
}


static const struct os_uart_ops gd32_uart_ops =
{


	.configure    = gd32_uart_configure,

	.start_send   = gd32_uart_start_send,
	.stop_send    = gd32_uart_stop_send,

	.start_recv   = gd32_uart_start_recv,
	.stop_recv    = gd32_uart_stop_recv,
	.recv_state   = gd32_uart_recv_state,

	.poll_send    = gd32_uart_poll_send,
	.poll_recv    = gd32_uart_poll_recv,
};
	
static int gd32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    os_err_t    result  = 0;
    os_base_t   level;

    struct gd32_uart *uart = os_calloc(1, sizeof(struct gd32_uart));

    OS_ASSERT(uart);

    uart->usart_info = (struct gd_usart_info *)dev->info;
	uart->state = RESET;

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &gd32_uart_ops;
    serial->config = config;

    level = os_hw_interrupt_disable();
    os_list_add_tail(&gd32_uart_list, &uart->list);
    os_hw_interrupt_enable(level);
	
    result = os_hw_serial_register(serial, dev->name, OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_INT_RX, uart);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}


OS_DRIVER_INFO gd32_usart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = gd32_usart_probe,
};

OS_DRIVER_DEFINE(gd32_usart_driver, "0.end.0");
#endif
