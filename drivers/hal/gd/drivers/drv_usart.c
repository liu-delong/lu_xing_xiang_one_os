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
 * @brief       This file implements usart driver for gd32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "drv_usart.h"

//extern struct gd32_uart guart[1];
#if 1
static struct gd32_uart guart[] = {
	{
		.buffer = OS_NULL,
		.device_name = "uart0",
		.uart_base = EVAL_COM0,
		.count = 0,
		.size = 0,
		.state = SET
	}
};
#endif
static rcu_periph_enum COM_CLK[COMn] = {EVAL_COM0_CLK, EVAL_COM1_CLK};
static uint32_t COM_TX_PIN[COMn] = {EVAL_COM0_TX_PIN, EVAL_COM1_TX_PIN};
static uint32_t COM_RX_PIN[COMn] = {EVAL_COM0_RX_PIN, EVAL_COM1_RX_PIN};
static uint32_t COM_GPIO_PORT[COMn] = {EVAL_COM0_GPIO_PORT, EVAL_COM1_GPIO_PORT};
static rcu_periph_enum COM_GPIO_CLK[COMn] = {EVAL_COM0_GPIO_CLK, EVAL_COM1_GPIO_CLK};
static uint32_t COM_IRQn[COMn] = {USART0_IRQn, USART1_IRQn};
static uint32_t COM_BASE[COMn] = {EVAL_COM0, EVAL_COM1};


static os_err_t gd32_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{

	return OS_EOK;
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
	struct gd32_uart *uart;

	OS_ASSERT(serial != OS_NULL);

	uart = os_container_of(serial, struct gd32_uart, serial_dev);    

	OS_ASSERT(uart != OS_NULL);

	uart->count = 0;
	uart->buffer = buff;
	uart->size = size;
	uart->state = SET;

	usart_interrupt_enable(uart->uart_base, USART_INT_RBNE);

	return OS_EOK;
}



static int gd32_uart_stop_recv(struct os_serial_device *serial)
{
	struct gd32_uart *uart;

	OS_ASSERT(serial != OS_NULL);

	uart = os_container_of(serial, struct gd32_uart, serial_dev);

	OS_ASSERT(uart != OS_NULL);

	uart->count = 0;
	uart->buffer = OS_NULL;
	uart->state = RESET;

	usart_interrupt_disable(uart->uart_base, USART_INT_RBNE);


	return OS_EOK;
}

static int gd32_uart_recv_state(struct os_serial_device *serial)
{
	int state = 0;
	struct gd32_uart *uart;

	OS_ASSERT(serial != OS_NULL);

	uart = os_container_of(serial, struct gd32_uart, serial_dev);

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

	uart = os_container_of(serial, struct gd32_uart, serial_dev);

	OS_ASSERT(uart != OS_NULL);

	for (i = 0; i < size; i++)
	{
		level = os_hw_interrupt_disable();
		usart_data_transmit(uart->uart_base, buff[i]);
		while (RESET == usart_flag_get(uart->uart_base, USART_FLAG_TBE));
		os_hw_interrupt_enable(level);
	}

	return size;
}

static int gd32_uart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
	return OS_EOK;
}

static const struct os_uart_ops gd32_uart_ops = {
	.configure    = gd32_configure,

	.start_send   = gd32_uart_start_send,
	.stop_send    = gd32_uart_stop_send,

	.start_recv   = gd32_uart_start_recv,
	.stop_recv    = gd32_uart_stop_recv,
	.recv_state   = gd32_uart_recv_state,

	.poll_send    = gd32_uart_poll_send,
	.poll_recv    = gd32_uart_poll_recv,
};

/*!
    \brief      this function handles USART RBNE interrupt request and TBE interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/

void USART0_IRQHandler(void)
{

	if(RESET != usart_interrupt_flag_get(guart[0].uart_base, USART_INT_FLAG_RBNE))
	{

		/* receive data */
		*(guart[0].buffer+guart[0].count) = usart_data_receive(guart[0].uart_base);
		guart[0].count++;
		if(guart[0].count == guart[0].size)
		{
			usart_interrupt_disable(guart[0].uart_base, USART_INT_TBE);
			guart[0].state = RESET;
		}

	}
}

void gd_eval_com_init(uint32_t com)
{
    uint32_t com_id = 0U;
    if(EVAL_COM0 == com){
        com_id = 0U;
    }else if(EVAL_COM1 == com){
        com_id = 1U;
    }
    
    /* enable GPIO clock */
    rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);

    /* enable USART clock */
    rcu_periph_clock_enable(COM_CLK[com_id]);

    /* connect port to USARTx_Tx */
    gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);

    /* connect port to USARTx_Rx */
    gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);

    /* USART configure */
    usart_deinit(com);
    usart_baudrate_set(com, 115200U);
    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);
}

static void __os_hw_usart_init(os_uint32_t com)
{

	nvic_irq_enable(COM_IRQn[com], 0, 0);
	gd_eval_com_init(COM_BASE[com]);

	return;
}

int os_hw_usart_init(void)
{	
	os_uint32_t i;	
	struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

	os_err_t result  = 0;

	for (i = 0; i < sizeof(guart) / sizeof(guart[0]); i++)
	{
		if(i >= COMn){
			os_kprintf("usart number error!\n");
			break;
		}
		
		__os_hw_usart_init(i);
	
		guart[i].state = RESET;
		guart[i].serial_dev.ops = &gd32_uart_ops;
		guart[i].serial_dev.config = config;

		result = os_hw_serial_register(&(guart[i].serial_dev), guart[i].device_name, OS_DEVICE_FLAG_RDWR, OS_NULL);

		OS_ASSERT(result == OS_EOK);
	}    

	return OS_EOK;
}

//OS_DRIVER_DEFINE(gd32_usart_driver, "0.end.0");

