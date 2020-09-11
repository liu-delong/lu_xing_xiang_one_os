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
 * @brief       This file implements usart driver for stm32.
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
#ifdef OS_USING_SERIAL

#if !defined(OS_USING_USART0) && !defined(OS_USING_USART1) && !defined(OS_USING_USART2)
#error "Please define at least one BSP_USING_UARTx"
#endif

/* GD32 uart driver */
struct gd32_uart
{
    uint32_t        uart_periph;
    IRQn_Type       irqn;
    rcu_periph_enum per_clk;
    rcu_periph_enum tx_gpio_clk;
    rcu_periph_enum rx_gpio_clk;
    uint32_t        tx_port;
    uint16_t        tx_pin;
    uint32_t        rx_port;
    uint16_t        rx_pin;

    struct os_serial_device *serial;
    char                    *device_name;
};

static void uart_isr(struct os_serial_device *serial);

#if defined(OS_USING_USART0)
struct os_serial_device serial0;

void USART0_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();
    uart_isr(&serial0);
    /* leave interrupt */
    os_interrupt_leave();
}

#endif /* OS_USING_USART0 */

#if defined(OS_USING_USART1)
struct os_serial_device serial1;

void USART1_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();
    uart_isr(&serial1);
    /* leave interrupt */
    os_interrupt_leave();
}

#endif /* OS_USING_UART1 */

static const struct gd32_uart uarts[] = {
#ifdef OS_USING_USART0
    {
        USART0,      /* uart peripheral index */
        USART0_IRQn, /* uart iqrn */
        RCU_USART0,
        RCU_GPIOA,
        RCU_GPIOA, /* periph clock, tx gpio clock, rt gpio clock */
        GPIOA, GPIO_PIN_9, /* tx port, tx pin */
        GPIOA, GPIO_PIN_10, /* rx port, rx pin */
        &serial0,
        "uart0",
    },
#endif

#ifdef OS_USING_USART1
    {
        USART1,      /* uart peripheral index */
        USART1_IRQn, /* uart iqrn */
        RCU_USART1,
        RCU_GPIOA, RCU_GPIOA,   /* periph clock, tx gpio clock, rt gpio clock */
        GPIOA, GPIO_PIN_2,      /* tx port, tx pin */
        GPIOA, GPIO_PIN_3,      /* rx port, rx pin */
        &serial1,
        "uart1",
    },
#endif
};

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
    rcu_periph_clock_enable(uart->tx_gpio_clk);
    rcu_periph_clock_enable(uart->rx_gpio_clk);
    rcu_periph_clock_enable(uart->per_clk);

    /* connect port to USARTx_Tx */
    gpio_init(uart->tx_port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, uart->tx_pin);
    /* connect port to USARTx_Rx */
    gpio_init(uart->rx_port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, uart->rx_pin);

    eclic_irq_enable(uart->irqn, 2, 2);
}

static os_err_t gd32_uart_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct gd32_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = (struct gd32_uart *)serial->parent.user_data;

    gd32_uart_gpio_init(uart);

    usart_baudrate_set(uart->uart_periph, cfg->baud_rate);

    switch (cfg->data_bits)
    {
    case DATA_BITS_9:
        usart_word_length_set(uart->uart_periph, USART_WL_9BIT);
        break;

    default:
        usart_word_length_set(uart->uart_periph, USART_WL_8BIT);
        break;
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_2:
        usart_stop_bit_set(uart->uart_periph, USART_STB_2BIT);
        break;
    default:
        usart_stop_bit_set(uart->uart_periph, USART_STB_1BIT);
        break;
    }

    switch (cfg->parity)
    {
    case PARITY_ODD:
        usart_parity_config(uart->uart_periph, USART_PM_ODD);
        break;
    case PARITY_EVEN:
        usart_parity_config(uart->uart_periph, USART_PM_EVEN);
        break;
    default:
        usart_parity_config(uart->uart_periph, USART_PM_NONE);
        break;
    }
    usart_hardware_flow_rts_config(uart->uart_periph, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(uart->uart_periph, USART_CTS_DISABLE);
    usart_receive_config(uart->uart_periph, USART_RECEIVE_ENABLE);
    usart_transmit_config(uart->uart_periph, USART_TRANSMIT_ENABLE);
    usart_enable(uart->uart_periph);

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
        eclic_irq_disable(uart->irqn);
        /* disable interrupt */
        usart_interrupt_disable(uart->uart_periph, USART_INT_RBNE);

        break;
    case OS_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        eclic_irq_enable(uart->irqn, 2, 2);
        /* enable interrupt */
        usart_interrupt_enable(uart->uart_periph, USART_INT_RBNE);
        break;
    }

    return OS_EOK;
}

static int gd32_uart_putc(struct os_serial_device *serial, char ch)
{
    struct gd32_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct gd32_uart *)serial->parent.user_data;

    usart_data_transmit(uart->uart_periph, ch);
    while ((usart_flag_get(uart->uart_periph, USART_FLAG_TC) == RESET))
        ;

    return 1;
}

static int gd32_uart_getc(struct os_serial_device *serial)
{
    int               ch;
    struct gd32_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct gd32_uart *)serial->parent.user_data;

    ch = -1;
    if (usart_flag_get(uart->uart_periph, USART_FLAG_RBNE) != RESET)
        ch = usart_data_receive(uart->uart_periph);
    return ch;
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

    /* UART in mode Receiver */
    if ((usart_interrupt_flag_get(uart->uart_periph, USART_INT_FLAG_RBNE) != RESET) &&
        (usart_flag_get(uart->uart_periph, USART_FLAG_RBNE) != RESET))
    {
        os_hw_serial_isr(serial, OS_SERIAL_EVENT_RX_IND);
        /* Clear RXNE interrupt flag */
        usart_flag_clear(uart->uart_periph, USART_FLAG_RBNE);
    }
}

static const struct os_uart_ops gd32_uart_ops =
{
    gd32_uart_configure,
    gd32_uart_control,
    gd32_uart_putc,
    gd32_uart_getc
};

int os_hw_usart_init(void)
{
    int i;
    
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    for (i = 0; i < sizeof(uarts) / sizeof(uarts[0]); i++)
    {
        uarts[i].serial->ops    = &gd32_uart_ops;
        uarts[i].serial->config = config;

        /* register UART device */
        os_hw_serial_register(uarts[i].serial,
                              uarts[i].device_name,
                              OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_INT_RX,
                              (void *)&uarts[i]);
    }

    return 0;
}
// OS_BOARD_INIT(gd32_hw_usart_init);
#endif
