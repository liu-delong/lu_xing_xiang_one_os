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
 * @file        drv_uart_l13x.c
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
#include "drv_usart.h"
#include "board.h"

static struct os_serial_device serial0;
static struct os_serial_device serial1;

#ifdef BSP_USING_UART

/* uart driver */
struct hc_uart
{
    uint32_t    uart_device;
    IRQn_Type   irqn;
    uint8_t lp_idx;
    M0P_LPUART_TypeDef* lp_idx_inst;
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
        LPUART0,
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
        LPUART1,
        M0P_LPUART1,
        0,
        LPUART1_TX_PORT, LPUART1_TX_PIN, LPUART1_TX_AF,
        LPUART1_RX_PORT, LPUART1_RX_PIN, LPUART1_RX_AF,
        &serial1,
        "lpuart1"
    }
#endif
};

/* callback */
void TxIntCallback(void)
{
}

void RxIntCallback(void)
{
}

void ErrIntCallback(void)
{
}

void PErrIntCallBack(void)
{
}

void CtsIntCallBack(void)
{
}

static void uart_isr(struct os_serial_device *serial)
{
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if ((uart->uart_device == 0) || (uart->uart_device == 1))
    {
        if (LPUart_GetStatus(uart->lp_idx, LPUartRC) != 0)
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
}

void LPUART0_IRQHandler(void)
{
    os_interrupt_enter();

    uart_isr(&serial0);

    os_interrupt_leave();
}

void LPUART1_IRQHandler(void)
{
    os_interrupt_enter();

    uart_isr(&serial1);

    os_interrupt_leave();
}

void App_LpUartClkCfg(void)
{
    Sysctrl_SetRCLTrim(SysctrlRclFreq38400);
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
}

static os_err_t hc_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    uint16_t u16Scnt = 0;

    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    uart = (struct hc_uart *)serial->parent.user_data;

    stc_lpuart_sclk_sel_t stcSclk;
    stc_lpuart_config_t  stcConfig;
    stc_lpuart_irq_cb_t stcLPUartIrqCb;
    stc_lpuart_multimode_t stcMulti;
    stc_lpuart_baud_t stcBaud;

    DDL_ZERO_STRUCT(stcConfig);
    DDL_ZERO_STRUCT(stcLPUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    DDL_ZERO_STRUCT(stcSclk);

    App_LpUartClkCfg();  /* LPUART clock configuration, transfer clock select RCL */

    if (uart->uart_device == 0)
    {
        Sysctrl_SetPeripheralGate(SysctrlPeripheralLpUart0, TRUE);
    }
    if (uart->uart_device == 1)
    {
        Sysctrl_SetPeripheralGate(SysctrlPeripheralLpUart1, TRUE);
    }

    stc_gpio_config_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    stcGpioCfg.enDir =  GpioDirOut;
    Gpio_Init(uart->tx_port, uart->tx_pin, &stcGpioCfg);       /* TX */
    stcGpioCfg.enDir =  GpioDirIn;
    Gpio_Init(uart->rx_port, uart->rx_pin, &stcGpioCfg);       /* RX */
    Gpio_SetAfMode(uart->tx_port, uart->tx_pin, uart->tx_af);
    Gpio_SetAfMode(uart->rx_port, uart->rx_pin, uart->rx_af);

    stcLPUartIrqCb.pfnRxIrqCb   = RxIntCallback;
    stcLPUartIrqCb.pfnTxIrqCb   = TxIntCallback;
    stcLPUartIrqCb.pfnRxFEIrqCb = ErrIntCallback;
    stcLPUartIrqCb.pfnPEIrqCb   = PErrIntCallBack;
    stcLPUartIrqCb.pfnCtsIrqCb  = CtsIntCallBack;
    stcConfig.pstcIrqCb = &stcLPUartIrqCb;
    stcConfig.bTouchNvic = TRUE;
    if(TRUE == stcConfig.bTouchNvic)
    {
        EnableNvic(uart->irqn, IrqLevel3, TRUE);
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        stcConfig.enStopBit = LPUart1bit;
        break;
    case STOP_BITS_2:
        stcConfig.enStopBit = LPUart2bit;
        break;
    default:
        return OS_EINVAL;
    }

    stcConfig.enRunMode = LPUartMode1;
    stcSclk.enSclk_Prs = LPUart4Or8Div;
    stcSclk.enSclk_sel = LPUart_Rcl;  /* Transfer clock source RCL */
    stcConfig.pstcLpuart_clk = &stcSclk;

#if defined(BSP_USING_LPUART0) || defined(BSP_USING_LPUART1)
    if ((uart->uart_device == 0) || (uart->uart_device == 1))
    {
        stcMulti.enMulti_mode = LPUartNormal;
        LPUart_SetMultiMode(uart->lp_idx, &stcMulti);

        LPUart_Init(uart->lp_idx, &stcConfig);

        LPUart_SetClkDiv(uart->lp_idx, LPUart4Or8Div);

        stcBaud.u32Sclk = 38400;  /* RCL clock frequency 38400Hz */
        stcBaud.enRunMode = LPUartMode1;
        stcBaud.u16Baud = cfg->baud_rate;
        u16Scnt = LPUart_CalScnt(uart->lp_idx, &stcBaud);
        LPUart_SetBaud(uart->lp_idx, u16Scnt);

        LPUart_ClrStatus(uart->lp_idx, LPUartRC);
        LPUart_EnableFunc(uart->lp_idx, LPUartRx);
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
