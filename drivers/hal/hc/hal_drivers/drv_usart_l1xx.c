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
 * @file        drv_uart_l1xx.c
 *
 * @brief       This file implements usart driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_hw.h>
#include <os_device.h>
#include <os_irq.h>
#include <string.h>
#include "hc_gpio.h"
#include "hc_lpuart.h"
#include "hc_uart.h"
#include "drv_usart.h"
#include "board.h"

static struct os_serial_device serial0;
static struct os_serial_device serial1;
static struct os_serial_device serial2;
static struct os_serial_device serial3;

#ifdef BSP_USING_UART

/* uart driver */
struct hc_uart
{
    uint32_t    uart_device;
    IRQn_Type   irqn;
    M0P_LPUART_TypeDef* lp_idx;
    M0P_UART_TypeDef*   idx;
    en_gpio_port_t tx_port;
    en_gpio_pin_t  tx_pin;
    en_gpio_af_t   tx_af;
    en_gpio_port_t rx_port;
    en_gpio_pin_t  rx_pin;
    en_gpio_af_t   rx_af;

    struct os_serial_device *serial_dev;
    char   *device_name;

    uint8_t  *buff;
    uint32_t  rx_total;
    uint32_t  rx_cnt;
    int rx_status;
};

static struct hc_uart uarts[] = {
#ifdef BSP_USING_LPUART0
    {
        0,
        LPUART0_IRQn,
        M0P_LPUART0,
        0,
        LPUART0_TX_PORT, LPUART0_TX_PIN, LPUART0_TX_AF,
        LPUART0_RX_PORT, LPUART0_RX_PIN, LPUART0_RX_AF,
        &serial0,
        "lpuart0"
    },
#endif

#ifdef BSP_USING_LPUART1
    {
        1,
        LPUART1_IRQn,
        M0P_LPUART1,
        0,
        LPUART1_TX_PORT, LPUART1_TX_PIN, LPUART1_TX_AF,
        LPUART1_RX_PORT, LPUART1_RX_PIN, LPUART1_RX_AF,
        &serial1,
        "lpuart1"
    },
#endif

#ifdef BSP_USING_UART0
    {
        2,
        UART0_2_IRQn,
        0,
        M0P_UART0,
        UART0_TX_PORT, UART0_TX_PIN, UART0_TX_AF,
        UART0_RX_PORT, UART0_RX_PIN, UART0_RX_AF,
        &serial2,
        "uart0"
    },
#endif

#ifdef BSP_USING_UART1
    {
        3,
        UART1_3_IRQn,
        0,
        M0P_UART1,
        UART1_TX_PORT, UART1_TX_PIN, UART1_TX_AF,
        UART1_RX_PORT, UART1_RX_PIN, UART1_RX_AF,
        &serial3,
        "uart1"
    }
#endif
};

static void uart_isr(struct os_serial_device *serial)
{
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if ((uart->uart_device == 0) || (uart->uart_device == 1))
    {
        if(LPUart_GetStatus(uart->lp_idx, LPUartRC) != 0)
        {
            LPUart_ClrStatus(uart->lp_idx, LPUartRC);
            uart->buff[uart->rx_cnt++] = LPUart_ReceiveData(uart->lp_idx);

            if (uart->rx_cnt >= uart->rx_total)
            {
                LPUart_DisableIrq(uart->lp_idx, LPUartRxIrq);

                uart->rx_status = 0;

                os_hw_serial_isr_rxdone(serial, uart->rx_cnt);
            }
        }
    }
#endif

#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1)
    if ((uart->uart_device == 2) || (uart->uart_device == 3))
    {
        if(Uart_GetStatus(uart->idx, UartRC) != 0)
        {
            Uart_ClrStatus(uart->idx, UartRC);
            uart->buff[uart->rx_cnt++] = Uart_ReceiveData(uart->idx);

            if (uart->rx_cnt >= uart->rx_total)
            {
                Uart_DisableIrq(uart->idx, UartRxIrq);

                uart->rx_status = 0;

                os_hw_serial_isr_rxdone(serial, uart->rx_cnt);
            }
        }
    }
#endif
}

void LpUart0_IRQHandler(void)
{
    os_interrupt_enter();

    uart_isr(&serial0);

    os_interrupt_leave();
}

void LpUart1_IRQHandler(void)
{
    os_interrupt_enter();

    uart_isr(&serial1);

    os_interrupt_leave();
}

void Uart0_IRQHandler(void)
{
    os_interrupt_enter();

    uart_isr(&serial2);

    os_interrupt_leave();
}

void Uart1_IRQHandler(void)
{
    os_interrupt_enter();

    uart_isr(&serial3);

    os_interrupt_leave();
}

void App_LpUartClkCfg(void)
{
    Sysctrl_SetRCLTrim(SysctrlRclFreq38400);
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
}

static os_err_t hc_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct hc_uart *uart;

    stc_lpuart_cfg_t  stcCfg;
    DDL_ZERO_STRUCT(stcCfg);

    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);

    stcGpioCfg.enDir =  GpioDirOut;
    Gpio_Init(uart->tx_port, uart->tx_pin, &stcGpioCfg);       /* TX */
    stcGpioCfg.enDir =  GpioDirIn;
    Gpio_Init(uart->rx_port, uart->rx_pin, &stcGpioCfg);       /* RX */
    Gpio_SetAfMode(uart->tx_port, uart->tx_pin, uart->tx_af);
    Gpio_SetAfMode(uart->rx_port, uart->rx_pin, uart->rx_af);

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if ((uart->uart_device == 0) || (uart->uart_device == 1))
    {
        App_LpUartClkCfg();  /* LPUART clock configuration, transfer clock select RCL */

        if (uart->uart_device == 0)
        {
            Sysctrl_SetPeripheralGate(SysctrlPeripheralLpUart0, TRUE);
        }
        if (uart->uart_device == 1)
        {
            Sysctrl_SetPeripheralGate(SysctrlPeripheralLpUart1, TRUE);
        }

        switch (cfg->stop_bits)
        {
        case STOP_BITS_1:
            stcCfg.enStopBit = LPUart1bit;
            break;
        case STOP_BITS_2:
            stcCfg.enStopBit = LPUart2bit;
            break;
        default:
            return OS_EINVAL;
        }

        switch (cfg->parity)
        {
        case PARITY_NONE:
            stcCfg.enRunMode = LPUartMskMode1;
            break;
        case PARITY_ODD:
            stcCfg.enMmdorCk = LPUartOdd;
            stcCfg.enRunMode = LPUartMskMode3;
            break;
        case PARITY_EVEN:
            stcCfg.enMmdorCk = LPUartEven;
            stcCfg.enRunMode = LPUartMskMode3;
            break;
        default:
            return OS_EINVAL;
        }

        stcCfg.stcBaud.enSclkSel = LPUartMskRcl;  /* Transfer clock source RCL */
        stcCfg.stcBaud.u32Sclk = 38400;  /* RCL clock frequency 38400Hz */
        stcCfg.stcBaud.enSclkDiv = LPUartMsk4Or8Div;
        stcCfg.stcBaud.u32Baud = cfg->baud_rate;

        LPUart_Init(uart->lp_idx, &stcCfg);

        LPUart_ClrStatus(uart->lp_idx, LPUartRC);
        LPUart_ClrStatus(uart->lp_idx, LPUartTC);
        EnableNvic(uart->irqn, IrqLevel3, TRUE);
    }
#endif

#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1)
    if ((uart->uart_device == 2) || (uart->uart_device == 3))
    {
        stc_uart_cfg_t  stcCfg;
        DDL_ZERO_STRUCT(stcCfg);

        if (uart->uart_device == 2)
        {
            Sysctrl_SetPeripheralGate(SysctrlPeripheralUart0, TRUE);
        }

        if (uart->uart_device == 3)
        {
            Sysctrl_SetPeripheralGate(SysctrlPeripheralUart1, TRUE);
        }

        switch (cfg->stop_bits)
        {
        case STOP_BITS_1:
            stcCfg.enStopBit = UartMsk1bit;
            break;
        case STOP_BITS_2:
            stcCfg.enStopBit = UartMsk2bit;
            break;
        default:
            return OS_EINVAL;
        }

        switch (cfg->parity)
        {
        case PARITY_NONE:
            stcCfg.enRunMode = UartMskMode1;
            break;
        case PARITY_ODD:
            stcCfg.enMmdorCk = UartMskOdd;
            stcCfg.enRunMode = UartMskMode3;
            break;
        case PARITY_EVEN:
            stcCfg.enMmdorCk = UartMskEven;
            stcCfg.enRunMode = UartMskMode3;
            break;
        default:
            return OS_EINVAL;
        }

        stcCfg.stcBaud.u32Pclk = Sysctrl_GetPClkFreq();
        stcCfg.stcBaud.enClkDiv = UartMsk8Or16Div;
        stcCfg.stcBaud.u32Baud = cfg->baud_rate;

        Uart_Init(uart->idx, &stcCfg);

        Uart_ClrStatus(uart->idx, UartRC);
        Uart_ClrStatus(uart->idx, UartTC);
        EnableNvic(uart->irqn, IrqLevel3, TRUE);
    }
#endif

    return OS_EOK;
}

static int hc_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    return 1;
}

static int hc_uart_stop_send(struct os_serial_device *serial)
{
    return 0;
}

static int hc_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    uart->buff = buff;
    uart->rx_cnt = 0;
    uart->rx_total = size;
    uart->rx_status = 1;

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if ((uart->uart_device == 0) || (uart->uart_device == 1))
    {
        LPUart_ClrStatus(uart->lp_idx, LPUartRC);
        LPUart_EnableIrq(uart->lp_idx, LPUartRxIrq);
    }
#endif

#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1)
    if ((uart->uart_device == 2) || (uart->uart_device == 3))
    {
        Uart_ClrStatus(uart->idx, UartRC);
        Uart_EnableIrq(uart->idx, UartRxIrq);
    }
#endif

    return 0;
}

static int hc_uart_stop_recv(struct os_serial_device *serial)
{
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    uart->buff = OS_NULL;
    uart->rx_cnt = 0;
    uart->rx_total = 0;
    uart->rx_status = 0;

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if ((uart->uart_device == 0) || (uart->uart_device == 1))
    {
        LPUart_DisableIrq(uart->lp_idx, LPUartRxIrq);
    }
#endif

#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1)
    if ((uart->uart_device == 2) || (uart->uart_device == 3))
    {
        Uart_DisableIrq(uart->idx, UartRxIrq);
    }
#endif

    return 0;
}

static int hc_uart_recv_state(struct os_serial_device *serial)
{
    int state;
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    state = uart->rx_cnt;

    if (uart->rx_status == 0)
    {
        state |= OS_SERIAL_FLAG_RX_IDLE;
    }


    return state;
}

static int hc_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    os_base_t level;
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if ((uart->uart_device == 0) || (uart->uart_device == 1))
    {
        for (i = 0; i < size; i++)
        {
            level = os_hw_interrupt_disable();

            LPUart_SendData(uart->lp_idx, buff[i]);

            os_hw_interrupt_enable(level);
        }
    }
#endif

#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1)
    if ((uart->uart_device == 2) || (uart->uart_device == 3))
    {
        for (i = 0; i < size; i++)
        {
            level = os_hw_interrupt_disable();

            Uart_SendDataPoll(uart->idx, buff[i]);

            os_hw_interrupt_enable(level);
        }
    }
#endif

    return size;
}

static int hc_uart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    int i;
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    OS_ASSERT(size == 1);

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if ((uart->uart_device == 0) || (uart->uart_device == 1))
    {
        for (i = 0; i < size; i++)
        {
            if (LPUart_GetStatus(uart->lp_idx, LPUartRC) != 0)
            {
                buff[i] = LPUart_ReceiveData(uart->lp_idx);
                LPUart_ClrStatus(uart->lp_idx, LPUartRC);
            }
        }
    }
#endif

#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1)
    if ((uart->uart_device == 2) || (uart->uart_device == 3))
    {
        for (i = 0; i < size; i++)
        {
            if (Uart_GetStatus(uart->idx, UartRC) != 0)
            {
                buff[i] = Uart_ReceiveData(uart->idx);
                Uart_ClrStatus(uart->idx, UartRC);
            }
        }
    }
#endif

    return size;
}

static const struct os_uart_ops hc_uart_ops =
{
    .configure    = hc_configure,

    .start_send   = hc_uart_start_send,
    .stop_send    = hc_uart_stop_send,
    .start_recv   = hc_uart_start_recv,
    .stop_recv    = hc_uart_stop_recv,
    .recv_state   = hc_uart_recv_state,

    .poll_send    = hc_uart_poll_send,
    .poll_recv    = hc_uart_poll_recv,
};

/**
 * @brief Initialize the UART
 *
 * This function initialize the UART
 *
 * @return None.
 */
int os_hw_usart_init(void)
{
    int i;
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    for (i = 0; i < sizeof(uarts) / sizeof(uarts[0]); i++)
    {
        uarts[i].serial_dev->ops    = &hc_uart_ops;
        uarts[i].serial_dev->config = config;

        /* register uart device */
        os_hw_serial_register(uarts[i].serial_dev, uarts[i].device_name,
                              OS_DEVICE_FLAG_RDWR, (void *)&uarts[i]);
    }

    return 0;
}
#endif
