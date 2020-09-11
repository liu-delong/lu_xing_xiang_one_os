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
 * @brief       The driver file for uart.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <rtl8710b.h>
#include <serial_api.h>
#include "board.h"
#include "drv_uart.h"
#include "serial.h"

#ifdef OS_USING_SERIAL

#define UART0_TX PA_23
#define UART0_RX PA_18

#define UART2_TX PA_30
#define UART2_RX PA_19

extern int LOGUART_SetBaud(u32 BaudRate);

struct ameba_uart
{
    serial_t                serial;
    os_uint32_t             irqno;
    struct os_serial_device serial_dev;
};

static struct ameba_uart uart0;
static struct ameba_uart loguart;

static void ameba_uart_irq(uint32_t id, SerialIrq event)
{
    struct os_serial_device *serial = (struct os_serial_device *)id;
    if (event == RxIrq)
    {
        os_hw_serial_isr(serial, OS_SERIAL_EVENT_RX_IND);
    }
    else if (event == TxIrq)
    {
    }
}

static os_err_t ameba_uart_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct ameba_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    serial->config = *cfg;

    uart = serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    /* Set databits, stopbits and parity. (8-bit data, 1 stopbit, no parity) */
    serial_format(&uart->serial, 8, ParityNone, 1);

    /* set baudrate */
    serial_baud(&uart->serial, 115200);

    return (OS_EOK);
}

static os_err_t ameba_uart_control(struct os_serial_device *serial, int cmd, void *arg)
{
    struct ameba_uart *uart;

    uart = serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    switch (cmd)
    {
    case OS_DEVICE_CTRL_CLR_INT:
        /* Disable the UART Interrupt */
        serial_irq_set(&uart->serial, RxIrq, 0);
        serial_irq_handler(&uart->serial, OS_NULL, 0);
        break;

    case OS_DEVICE_CTRL_SET_INT:
        /* install interrupt */
        serial_irq_handler(&uart->serial, ameba_uart_irq, (uint32_t)serial);

        /* Enable the UART Interrupt */
        serial_irq_set(&uart->serial, RxIrq, 1);
        break;
    }

    return (OS_EOK);
}

static int ameba_uart_putc(struct os_serial_device *serial, char c)
{
    struct ameba_uart *uart;

    uart = serial->parent.user_data;

    /* FIFO status, contain valid data */
    /* write data */
    serial_putc(&uart->serial, c);

    return (1);
}

static int ameba_uart_getc(struct os_serial_device *serial)
{
    struct ameba_uart *uart = serial->parent.user_data;

    if (!serial_readable(&uart->serial))
        return -1;

    /* Receive Data Available */
    return serial_getc(&uart->serial);
}

static os_size_t ameba_uart_dma_transmit(struct os_serial_device *serial, os_uint8_t *buf, os_size_t size, int direction)
{
    return (0);
}

static const struct os_uart_ops uart_ops = {ameba_uart_configure,
                                            ameba_uart_control,
                                            ameba_uart_putc,
                                            ameba_uart_getc,
                                            ameba_uart_dma_transmit};

static os_err_t ameba_loguart_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    LOGUART_SetBaud(115200);
    return OS_EOK;
}

void ameba_loguart_irq_handler(void *data)
{
    u32 IrqEn = DiagGetIsrEnReg();

    DiagSetIsrEnReg(0);

    os_hw_serial_isr(&loguart.serial_dev, OS_SERIAL_EVENT_RX_IND);

    DiagSetIsrEnReg(IrqEn);
}

static os_err_t ameba_loguart_control(struct os_serial_device *serial, int cmd, void *arg)
{
    switch (cmd)
    {
    case OS_DEVICE_CTRL_CLR_INT:
        /* Disable the UART Interrupt */
        NVIC_DisableIRQ(UART_LOG_IRQ); /* this is rom_code_patch */
        break;

    case OS_DEVICE_CTRL_SET_INT:
        /* install interrupt */
        DIAG_UartReInit((IRQ_FUN)ameba_loguart_irq_handler);
        /* Enable the UART Interrupt */
        NVIC_SetPriority(UART_LOG_IRQ, 10); /* this is rom_code_patch */
        break;
    }

    return (OS_EOK);
}

static int ameba_loguart_putc(struct os_serial_device *serial, char c)
{
    DiagPutChar(c);

    return 1;
};

static int ameba_loguart_getc(struct os_serial_device *serial)
{
    int c = -1;

    if (!UART_Readable(UART2_DEV))
        return -1;

    c = DiagGetChar(_FALSE);

    return c;
}

static const struct os_uart_ops loguart_ops = 
{
    ameba_loguart_configure,
    ameba_loguart_control,
    ameba_loguart_putc,
    ameba_loguart_getc,
    OS_NULL,
};

int os_hw_uart_init(void)
{
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    {
        uart0.irqno             = UART0_IRQ;
        uart0.serial_dev.ops    = &uart_ops;
        uart0.serial_dev.config = config;

        /* Init UART Hardware */
        serial_init(&uart0.serial, UART0_TX, UART0_RX);

        os_hw_serial_register(&uart0.serial_dev, "uart0", OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_INT_RX, &uart0);
    }

    {
        loguart.irqno             = UART_LOG_IRQ;
        loguart.serial_dev.ops    = &loguart_ops;
        loguart.serial_dev.config = config;

        /* Init UART Hardware */
        serial_init(&loguart.serial, UART2_TX, UART2_RX);

        os_hw_serial_register(&loguart.serial_dev,
                              OS_CONSOLE_DEVICE_NAME,
                              OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_INT_RX,
                              &loguart);
    }
    return 0;
}
#endif /* OS_USING_SERIAL */
