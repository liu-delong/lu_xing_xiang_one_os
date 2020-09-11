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
 * @brief       This file implements uart driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <os_hw.h>
#include <os_device.h>
#include <os_irq.h>

#include <string.h>

#include "am_mcu_apollo.h"

/* AM uart driver */
struct am_uart
{
    os_uint32_t uart_device;
    os_uint32_t uart_interrupt;
    os_uint8_t *buff;
};

static os_err_t am_uart_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct am_uart *     uart;
    am_hal_uart_config_t uart_cfg;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = (struct am_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    memset(&uart_cfg, 0, sizeof(uart_cfg));

    /* Get the configure */
    uart_cfg.ui32BaudRate = cfg->baud_rate;

    if (cfg->data_bits == DATA_BITS_5)
        uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_5;
    else if (cfg->data_bits == DATA_BITS_6)
        uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_6;
    else if (cfg->data_bits == DATA_BITS_7)
        uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_7;
    else if (cfg->data_bits == DATA_BITS_8)
        uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_8;

    if (cfg->stop_bits == STOP_BITS_1)
        uart_cfg.bTwoStopBits = false;
    else if (cfg->stop_bits == STOP_BITS_2)
        uart_cfg.bTwoStopBits = true;

    if (cfg->parity == PARITY_NONE)
        uart_cfg.ui32Parity = AM_HAL_UART_PARITY_NONE;
    else if (cfg->parity == PARITY_ODD)
        uart_cfg.ui32Parity = AM_HAL_UART_PARITY_ODD;
    else if (cfg->parity == PARITY_EVEN)
        uart_cfg.ui32Parity = AM_HAL_UART_PARITY_EVEN;

    uart_cfg.ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE;

    /* UART Config */
    am_hal_uart_config(uart->uart_device, &uart_cfg);

    /* Enable the UART FIFO */
    //am_hal_uart_fifo_config(uart->uart_device, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);

    /* Enable the uart interrupt in the NVIC */
    am_hal_interrupt_enable(uart->uart_interrupt);
    am_hal_interrupt_priority_set(uart->uart_interrupt,10);
    /* Enable the UART */
    am_hal_uart_enable(uart->uart_device);
    return OS_EOK;
}
static void uart_isr(struct os_serial_device *serial)
{
    uint32_t        status;
    struct am_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct am_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    /* Read the interrupt status */
    status = am_hal_uart_int_status_get(uart->uart_device, true);

    /* Clear the UART interrupt */
    am_hal_uart_int_clear(uart->uart_device, status);

    if (status & (AM_HAL_UART_INT_RX_TMOUT))
    {
        //os_hw_serial_isr_rxdone(serial, 1);
    }

    if (status & AM_HAL_UART_INT_RX)
    {
        am_hal_uart_char_receive_polled(uart->uart_device, (char *)uart->buff);
        os_hw_serial_isr_rxdone(serial, 1);
    }

    if (status & AM_HAL_UART_INT_TX)
    {
        os_hw_serial_isr_txdone(serial);
    }
    
}

static int am_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct am_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct am_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);
    
    if(!(am_hal_uart_int_enable_get(uart->uart_device) & AM_HAL_UART_INT_TX))
    {
        am_hal_uart_int_enable(uart->uart_device,AM_HAL_UART_INT_TX);
    }
    am_hal_uart_char_transmit_polled(uart->uart_device,(char)*buff);
    /*for (i = 0; i < size; i++)
    {
        if(i == (size-1))
        {
            am_hal_uart_int_enable(uart->uart_device,AM_HAL_UART_INT_TX);
        }
        am_hal_uart_char_transmit_polled(uart->uart_device, buff[i]);
    }
    return size;
    */

    return 1;
}

static int am_uart_stop_send(struct os_serial_device *serial)
{
    struct am_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct am_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

     am_hal_uart_int_disable(uart->uart_device, AM_HAL_UART_INT_TX);

    return 0;
}

static int am_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct am_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct am_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);
/*   if(am_hal_uart_int_status_get(uart->uart_device, true) & AM_HAL_UART_INT_RX)
    {
        am_hal_uart_char_receive_polled(uart->uart_device, (char *)buff);
        return 1;
    }*/
    uart->buff = buff;
    if(!(am_hal_uart_int_enable_get(uart->uart_device) & AM_HAL_UART_INT_RX))
    {
        am_hal_uart_int_enable(uart->uart_device,AM_HAL_UART_INT_RX);
        return 0;
    }
    return 0;
}

static int am_uart_stop_recv(struct os_serial_device *serial)
{
    struct am_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct am_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    am_hal_uart_int_disable(uart->uart_device,AM_HAL_UART_INT_RX);

    return 0;
}

static int am_uart_recv_state(struct os_serial_device *serial)
{
    int state;
    struct am_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct am_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    state = OS_SERIAL_FLAG_RX_IDLE;

    return state;
}

static int am_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;    
    struct am_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct am_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        am_hal_uart_char_transmit_polled(uart->uart_device, buff[i]);
    }

    return size;
}

static int am_uart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct am_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct am_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    OS_ASSERT(size == 1);

    if (am_hal_uart_flags_get(uart->uart_device) & AM_REG_UART_FR_RXFE_RCVFIFO_EMPTY)
        return 0;

    am_hal_uart_line_receive_polled(uart->uart_device,size,(char *)buff);
    return size;
}

static const struct os_uart_ops am_uart_ops =
{
    .configure    = am_uart_configure,

    .start_send   = am_uart_start_send,
    .stop_send    = am_uart_stop_send,

    .start_recv   = am_uart_start_recv,
    .stop_recv    = am_uart_stop_recv,
    .recv_state   = am_uart_recv_state,
    
    .poll_send    = am_uart_poll_send,
    .poll_recv    = am_uart_poll_recv,
};

#if defined(BSP_USING_UART0)
/* UART0 device driver structure */
struct am_uart uart0 =
{
    AM_UART0_INST,
    AM_HAL_INTERRUPT_UART
};
static struct os_serial_device serial0;

void am_uart0_isr(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    uart_isr(&serial0);

    /* leave interrupt */
    os_interrupt_leave();
}
#endif /* BSP_USING_UART0 */

#if defined(BSP_USING_UART1)
/* UART1 device driver structure */
struct am_uart uart1 =
{
    AM_UART1_INST,
    AM_HAL_INTERRUPT_UART1
};
static struct os_serial_device serial1;

void am_uart1_isr(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    uart_isr(&serial1);

    /* leave interrupt */
    os_interrupt_leave();
}
#endif /* BSP_USING_UART1 */

void am_uart_isr(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    uart_isr(&serial0);

    /* leave interrupt */
    os_interrupt_leave();
}

static void GPIO_Configuration(void)
{
#if defined(BSP_USING_UART0)
    /* Make sure the UART RX and TX pins are enabled */
    am_hal_gpio_pin_config(UART0_GPIO_TX, UART0_GPIO_CFG_TX | AM_HAL_GPIO_PULL24K);
    am_hal_gpio_pin_config(UART0_GPIO_RX, UART0_GPIO_CFG_RX | AM_HAL_GPIO_PULL24K);
#endif /* BSP_USING_UART0 */

#if defined(BSP_USING_UART1)
    /* Make sure the UART RX and TX pins are enabled */
    am_hal_gpio_pin_config(UART1_GPIO_TX, UART1_GPIO_CFG_TX | AM_HAL_GPIO_PULL24K);
    am_hal_gpio_pin_config(UART1_GPIO_RX, UART1_GPIO_CFG_RX | AM_HAL_GPIO_PULL24K);
#endif /* BSP_USING_UART1 */
}

static void RCC_Configuration(struct am_uart *uart)
{
    /* Power on the selected UART */
    am_hal_uart_pwrctrl_enable(uart->uart_device);

    /* Start the UART interface, apply the desired configuration settings */
    am_hal_uart_clock_enable(uart->uart_device);

    /* Disable the UART before configuring it */
    am_hal_uart_disable(uart->uart_device);
}

int os_hw_usart_init(void)
{
    struct am_uart *        uart;
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    GPIO_Configuration();

#if defined(BSP_USING_UART0)
    uart             = &uart0;
    config.baud_rate = BAUD_RATE_115200;

    RCC_Configuration(uart);

    serial0.ops    = &am_uart_ops;
    serial0.config = config;

    /* register UART0 device */
    os_hw_serial_register(&serial0, "uart0", OS_DEVICE_FLAG_RDWR , uart);
#endif /* BSP_USING_UART0 */

#if defined(BSP_USING_UART1)
    uart             = &uart1;
    config.baud_rate = BAUD_RATE_115200;

    RCC_Configuration(uart);

    serial1.ops    = &am_uart_ops;
    serial1.config = config;

    /* register UART1 device */
    os_hw_serial_register(&serial1, "uart1", OS_DEVICE_FLAG_RDWR , uart);
#endif /* BSP_USING_UART1 */

    return 0;
}
