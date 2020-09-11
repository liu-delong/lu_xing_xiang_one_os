/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * \@file        drv_uart.c
 *
 * \@brief       This file implements uart driver for FM33A0xx.
 *
 * \@revision
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

#include "define_all.h"
#include "drv_common.h"
#include "drv_uart.h"
#include <drv_log.h>

#ifdef OS_USING_SERIAL

/* AM uart driver */
struct fm_uart
{
    const char *uart_name;
    UARTx_Type *uartxx;
    os_uint32_t uart_ClockSrc;
    IRQn_Type IRQn;
};


static os_err_t fm_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct fm_uart *     uart;
    UART_SInitTypeDef UART_para;
    RCC_ClocksType RCC_Clocks;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = (struct fm_uart *)serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    RCC_PERCLK_SetableEx(uart->uart_ClockSrc, ENABLE);
    NVIC_DisableIRQ(uart->IRQn);
    NVIC_SetPriority(uart->IRQn, FM_IRQ_PRI_UART);
    NVIC_EnableIRQ(uart->IRQn);

    UART_para.BaudRate = cfg->baud_rate;

    switch (cfg->data_bits)
    {
    case DATA_BITS_7:
        UART_para.DataBit = Seven7Bit;
        break;
    case DATA_BITS_8:
        UART_para.DataBit = Eight8Bit;
        break;
    case DATA_BITS_9:
        UART_para.DataBit = Nine9Bit;
        break;
    default:
        return OS_EINVAL;
    }

    switch (cfg->parity)
    {
    case PARITY_NONE:
        UART_para.ParityBit = NONE;
        break;
    case PARITY_ODD:
        UART_para.ParityBit = ODD;
        break;
    case PARITY_EVEN:
        UART_para.ParityBit = EVEN;
        break;
    default:
        return OS_EINVAL;
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        UART_para.StopBit = OneBit;
        break;
    case STOP_BITS_2:
        UART_para.StopBit = TwoBit;
        break;
    default:
        return OS_EINVAL;
    }

    RCC_GetClocksFreq(&RCC_Clocks);
    UART_SInit(uart->uartxx, &UART_para, RCC_Clocks.APBCLK_Frequency);
    UARTx_RXSTA_RXEN_Setable(uart->uartxx, ENABLE);
    UARTx_TXSTA_TXEN_Setable(uart->uartxx, ENABLE);

    return OS_EOK;
}

static os_err_t fm_control(struct os_serial_device *serial, int cmd, void *arg)
{
    struct fm_uart *uart;
    unsigned long irq_type = (unsigned long)arg;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct fm_uart *)serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    switch (cmd)
    {
    /* disable interrupt */
    case OS_DEVICE_CTRL_CLR_INT:
        if (irq_type == OS_DEVICE_FLAG_INT_RX)
        {
            /* disable rx irq */
            UART_UARTIE_RxTxIE_SetableEx(uart->uartxx, RxInt, DISABLE);
        }
        else if (irq_type == OS_DEVICE_FLAG_INT_TX)
        {
            /* disable tx irq */
            UART_UARTIE_RxTxIE_SetableEx(uart->uartxx, TxInt, DISABLE);
        }
        else
        {
            LOG_EXT_D("invalide irq type %d\r\n", (int)irq_type);
            return OS_ERROR;
        }
        break;
    /* enable interrupt */
    case OS_DEVICE_CTRL_SET_INT:
        /* Enable the uart interrupt in the NVIC */
        if (irq_type == OS_DEVICE_FLAG_INT_RX)
        {
            /* enable rx irq */
            UART_UARTIE_RxTxIE_SetableEx(uart->uartxx, RxInt, ENABLE);
        }
        else if (irq_type == OS_DEVICE_FLAG_INT_TX)
        {
            /* enable tx irq */
            UART_UARTIE_RxTxIE_SetableEx(uart->uartxx, TxInt, ENABLE);
        }
        else
        {
            LOG_EXT_D("invalide irq type %d\r\n", (int)irq_type);
            return OS_ERROR;
        }
        break;
    /* UART config */
    case OS_DEVICE_CTRL_CONFIG:
        break;
    }

    return OS_EOK;
}

static int fm_putc(struct os_serial_device *serial, char c)
{
    struct fm_uart *uart;
    os_base_t          level;

    OS_ASSERT(serial != OS_NULL);

    uart = (struct fm_uart *)serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    while(SET == UARTx_TXBUFSTA_TXFF_Chk(uart->uartxx));

    level = os_hw_interrupt_disable();
    UARTx_TXREG_Write(uart->uartxx, c);
    os_hw_interrupt_enable(level);

    return 1;
}

static int fm_getc(struct os_serial_device *serial)
{
    int             ch;
    struct fm_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = (struct fm_uart *)serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    ch = -1;

    if(SET == UARTx_RXBUFSTA_RXFF_Chk(uart->uartxx))
    {
        ch = UARTx_RXREG_Read(uart->uartxx);
    }

    return ch;
}

static void uart_isr(struct os_serial_device *serial)
{
    struct fm_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = (struct fm_uart *)serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    if((ENABLE == UART_UARTIE_RxTxIE_GetableEx(uart->uartxx, RxInt))
            &&(SET == UART_UARTIF_RxTxIF_ChkEx(uart->uartxx, RxInt)))
    {
        os_hw_serial_isr(serial, OS_SERIAL_EVENT_RX_IND);
    }

    if((ENABLE == UART_UARTIE_RxTxIE_GetableEx(uart->uartxx, TxInt))
            &&(SET == UART_UARTIF_RxTxIF_ChkEx(uart->uartxx, TxInt)))
    {
        os_hw_serial_isr(serial, OS_SERIAL_EVENT_TX_DONE);
    }

}

static const struct os_uart_ops fm_uart_ops =
{
    fm_configure,
    fm_control,
    fm_putc,
    fm_getc,
};


struct fm_uart uart_cfg[] =
{
#if defined(BSP_USING_UART0)
    UART0_CONFIG,
#endif
#if defined(BSP_USING_UART1)
    UART1_CONFIG,
#endif

#if defined(BSP_USING_UART2)
    UART2_CONFIG,
#endif

#if defined(BSP_USING_UART3)
    UART3_CONFIG,
#endif

#if defined(BSP_USING_UART4)
    UART4_CONFIG,
#endif

#if defined(BSP_USING_UART5)
    UART5_CONFIG,
#endif

};

#define UART_CFG_NUM sizeof(uart_cfg)/sizeof(uart_cfg[0])

static struct os_serial_device fm_serial[UART_CFG_NUM];

#if defined(BSP_USING_UART0)
void UART0_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    uart_isr(&fm_serial[0]);

    /* leave interrupt */
    os_interrupt_leave();
}
#endif /* BSP_USING_UART0 */

#if defined(BSP_USING_UART1)
void UART1_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    uart_isr(&fm_serial[1]);

    /* leave interrupt */
    os_interrupt_leave();
}
#endif /* BSP_USING_UART1 */

#if defined(BSP_USING_UART2)
void UART2_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    uart_isr(&fm_serial[2]);

    /* leave interrupt */
    os_interrupt_leave();
}
#endif /* BSP_USING_UART2 */

#if defined(BSP_USING_UART3)
void UART3_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    uart_isr(&fm_serial[3]);

    /* leave interrupt */
    os_interrupt_leave();
}
#endif /* BSP_USING_UART3 */

#if defined(BSP_USING_UART4)
void UART4_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    uart_isr(&fm_serial[4]);

    /* leave interrupt */
    os_interrupt_leave();
}
#endif /* BSP_USING_UART4 */

#if defined(BSP_USING_UART5)
void UART5_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    uart_isr(&fm_serial[5]);

    /* leave interrupt */
    os_interrupt_leave();
}
#endif /* BSP_USING_UART5 */


static void GPIO_Configuration(void)
{
#if defined(BSP_USING_UART0)
    /* Make sure the UART RX and TX pins are enabled */
    AltFunIO(GPIOF, GPIO_Pin_3, 0);	
    AltFunIO(GPIOF, GPIO_Pin_4, 0);	
#endif /* BSP_USING_UART0 */

#if defined(BSP_USING_UART1)
    /* Make sure the UART RX and TX pins are enabled */
    AltFunIO(GPIOB, GPIO_Pin_0, 0);
    AltFunIO(GPIOB, GPIO_Pin_1, 0);
#endif /* BSP_USING_UART1 */

#if defined(BSP_USING_UART2)
    /* Make sure the UART RX and TX pins are enabled */
    AltFunIO(GPIOB, GPIO_Pin_2, 0);		
    AltFunIO(GPIOB, GPIO_Pin_3, 0);		
#endif /* BSP_USING_UART2 */

#if defined(BSP_USING_UART3)
    /* Make sure the UART RX and TX pins are enabled */
    AltFunIO(GPIOC, GPIO_Pin_10, 0);		
    AltFunIO(GPIOC, GPIO_Pin_11, 0);		
#endif /* BSP_USING_UART3 */

#if defined(BSP_USING_UART4)
    /* Make sure the UART RX and TX pins are enabled */
    AltFunIO(GPIOD, GPIO_Pin_0, 0);		
    AltFunIO(GPIOD, GPIO_Pin_1, 0);		
#endif /* BSP_USING_UART4 */

#if defined(BSP_USING_UART5)
    /* Make sure the UART RX and TX pins are enabled */
    AltFunIO(GPIOC, GPIO_Pin_4, 0);		
    AltFunIO(GPIOC, GPIO_Pin_5, 0);		
#endif /* BSP_USING_UART5 */

}

int os_hw_usart_init(void)
{
    os_uint8_t i;
    struct fm_uart *        uart;
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    RCC_PERCLK_SetableEx(UARTCOMCLK, ENABLE);
    GPIO_Configuration();

    for (i =0; i < UART_CFG_NUM; i++)
    {
        uart = &uart_cfg[i];

        fm_serial[i].ops    = &fm_uart_ops;
        fm_serial[i].config = config;

        /* register UART0 device */
        os_hw_serial_register(&fm_serial[i], uart->uart_name, OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_INT_RX | OS_DEVICE_FLAG_INT_TX, uart);
    }

    return 0;
}

#endif  /* OS_USING_SERIAL */


