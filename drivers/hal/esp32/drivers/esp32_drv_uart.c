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
 * @file        esp32_drv_uart.c
 *
 * @brief       This file implements usart driver for esp32
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <bus/bus.h>
#include <serial.h>
#include "os_kernel.h"
#include "string.h"

#include "esp_types.h"
#include "esp_attr.h"
#include "esp_intr.h"
#include "esp_log.h"

#include "gpio.h"
#include "uart.h"
#include "soc/uart_reg.h"
#include "soc/dport_reg.h"
#include "soc/uart_struct.h"


#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

#define CPUID 0

#define UART_FULL_THRESH_DEFAULT  (120)
#define UART_TOUT_THRESH_DEFAULT   (10)

#define UART_FIFO_LEN           (128)        /* Length of the hardware FIFO buffers */
#define UART_INTR_MASK          0x1ff        /* mask of all UART interrupts */
#define UART_LINE_INV_MASK      (0x3f << 19) /* TBD */
#define UART_BITRATE_MAX        5000000      /* Max bit rate supported by UART */
#define UART_PIN_NO_CHANGE      (-1)         /* Constant for uart_set_pin function which indicates that UART pin should not be changed */

#define UART_INVERSE_DISABLE  (0x0)            /* Disable UART signal inverse*/
#define UART_INVERSE_RXD   (UART_RXD_INV_M)    /* UART RXD input inverse*/
#define UART_INVERSE_CTS   (UART_CTS_INV_M)    /* UART CTS input inverse*/
#define UART_INVERSE_TXD   (UART_TXD_INV_M)    /* UART TXD output inverse*/
#define UART_INVERSE_RTS   (UART_RTS_INV_M)    /* UART RTS output inverse*/

#define UART_INTR_NUM_0           17
#define UART_INTR_NUM_1           18
#define UART_INTR_NUM_2           19

struct esp32_uart
{
    struct os_serial_device serial_dev;
    uart_dev_t *uart_idf;
    os_int32_t irq;
    os_int32_t num;
    //for rtt
    os_bool_t   rx_isr_enabled;
    os_uint8_t *buff;
    os_size_t   size;
    os_size_t   count;
};

static DRAM_ATTR struct esp32_uart uart0;

static void IRAM_ATTR uart_rx_intr_handler_default(void *param)
{
    struct esp32_uart* uart_p = (struct esp32_uart *)param;
    uint32_t uart_intr_status = uart_p->uart_idf->int_st.val;

    while(uart_intr_status != 0x0)
    {
        if((uart_intr_status & UART_RXFIFO_TOUT_INT_ST_M)
            || (uart_intr_status & UART_RXFIFO_FULL_INT_ST_M))
        {
            uart_p->uart_idf->int_clr.rxfifo_tout = 1;
            uart_p->uart_idf->int_clr.rxfifo_full = 1;
        }else if(uart_intr_status & UART_RXFIFO_OVF_INT_ST_M)
        {
            uart_p->uart_idf->conf0.rxfifo_rst = 1;
            uart_p->uart_idf->conf0.rxfifo_rst = 0;
            uart_p->uart_idf->int_clr.rxfifo_ovf = 1;
        }else if(uart_intr_status & UART_BRK_DET_INT_ST_M)
        {
            uart_p->uart_idf->int_clr.brk_det = 1;
        }else if(uart_intr_status & UART_PARITY_ERR_INT_ST_M )
        {
            uart_p->uart_idf->int_clr.parity_err = 1;
        }else if(uart_intr_status & UART_FRM_ERR_INT_ST_M)
        {
            uart_p->uart_idf->int_clr.frm_err = 1;
        }else 
        {
            /* simply clear all other intr status */
            uart_p->uart_idf->int_clr.val = uart_intr_status;
        }
        uart_intr_status = uart_p->uart_idf->int_st.val;
    }

}


static os_err_t esp32_uart_config(struct os_serial_device *serial, struct serial_configure *cfg)
{
    os_base_t level;
    os_uint32_t clk_div, parity, stopbit;
    struct esp32_uart *uart_p = serial->parent.user_data;

    level = os_hw_interrupt_disable();

    clk_div = (((UART_CLK_FREQ) << 4) / cfg->baud_rate);
    uart_p->uart_idf->clk_div.div_int = clk_div >> 4;
    uart_p->uart_idf->clk_div.div_frag = clk_div & 0xf;

    os_uint32_t databits = UART_DATA_8_BITS;
    switch (cfg->data_bits)
    {
    case DATA_BITS_7:
        databits = UART_DATA_7_BITS;
        break;
    }
    uart_p->uart_idf->conf0.bit_num = databits;

    parity = UART_PARITY_DISABLE;
    switch (cfg->parity)
    {
    case PARITY_EVEN:
        parity = UART_PARITY_EVEN;
        break;
    case PARITY_ODD:
        parity = UART_PARITY_ODD;
        break;
    }
    uart_p->uart_idf->conf0.parity = parity & 0x1;
    uart_p->uart_idf->conf0.parity_en = (parity >> 1) & 0x1;

    stopbit = UART_STOP_BITS_1;
    switch (cfg->stop_bits)
    {
    case STOP_BITS_2:
        stopbit = UART_STOP_BITS_2;
        break;
    }
    uart_p->uart_idf->conf0.stop_bit_num = stopbit;

    CLEAR_PERI_REG_MASK(UART_CONF0_REG(uart_p->num), UART_LINE_INV_MASK);
    SET_PERI_REG_MASK(UART_CONF0_REG(uart_p->num), UART_INVERSE_DISABLE);

    uart_p->uart_idf->conf1.rx_flow_thrhd = UART_FULL_THRESH_DEFAULT;
    if(cfg->reserved & UART_HW_FLOWCTRL_RTS)
        uart_p->uart_idf->conf1.rx_flow_en = 1;
    else
        uart_p->uart_idf->conf1.rx_flow_en = 0;
    if(cfg->reserved & UART_HW_FLOWCTRL_CTS)
        uart_p->uart_idf->conf0.tx_flow_en = 1;
    else
        uart_p->uart_idf->conf0.tx_flow_en = 0;

    uart_p->uart_idf->conf0.tick_ref_always_on = 1;
    ESP_INTR_DISABLE(uart_p->irq);

    os_hw_interrupt_enable(level);
    
    return OS_EOK;
}

static int esp32_rtt_uart_recv_start(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct esp32_uart *uart;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);
    uart = serial->parent.user_data;

    level = os_hw_interrupt_disable();

    uart->buff  = buff;
    uart->size  = size;
    uart->count = 0;

    uart->rx_isr_enabled = OS_TRUE;

    os_hw_interrupt_enable(level);
    
    return 0;
}

static int esp32_rtt_uart_recv_stop(struct os_serial_device *serial)
{
    struct esp32_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = serial->parent.user_data;

    uart->rx_isr_enabled = OS_FALSE;
    
    return 0;
}

static void esp32_rtt_rx_process(struct esp32_uart *uart)
{
    os_int32_t count, i;

    count = uart->uart_idf->status.rxfifo_cnt;
    if(uart->count + count >= uart->size)
        count = uart->size - uart->count;
    
    for(i=0; i<count; i++)
        uart->buff[uart->count++] = uart->uart_idf->fifo.rw_byte;
}

static int esp32_rtt_uart_recv_state(struct os_serial_device *serial)
{    
    struct esp32_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = serial->parent.user_data;

    if (uart->rx_isr_enabled == OS_TRUE)
    {
        esp32_rtt_rx_process(uart);
        return OS_SERIAL_FLAG_RX_IDLE | uart->count;
    }
    else
    {
        return OS_SERIAL_FLAG_RX_IDLE;
    }
}

static int esp32_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    struct esp32_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = serial->parent.user_data;
 
    while(1)
    {
        uint8_t tx_fifo_cnt = uart->uart_idf->status.txfifo_cnt;
        if (tx_fifo_cnt < UART_FIFO_LEN)
            break;
    }

    for(i=0; i<size; i++)
        WRITE_PERI_REG(UART_FIFO_AHB_REG(uart->num), buff[i]);
    
    return size;
}


static const struct os_uart_ops esp32_uart_ops = {
    .configure    = esp32_uart_config,

    .start_send   = OS_NULL,
    .stop_send    = OS_NULL,

    .start_recv   = esp32_rtt_uart_recv_start,
    .stop_recv    = esp32_rtt_uart_recv_stop,
    .recv_state   = esp32_rtt_uart_recv_state,
    
    .poll_send    = esp32_uart_poll_send,
    .poll_recv    = OS_NULL,
};

int esp32_uart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    struct esp32_uart *uart_p;

    if(!strcmp(dev->name, "uart0"))
    {
        uart_p = &uart0;
        uart0.uart_idf = &UART0;
        uart0.irq = UART_INTR_NUM_0;
        uart0.num = UART_NUM_0;
        /* wait for fifo empty */
        while (uart0.uart_idf->status.txfifo_cnt);
        uart_tx_wait_idle(0);

        PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[GPIO_NUM_1], PIN_FUNC_GPIO);
        gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLUP_ONLY);
        gpio_set_direction(GPIO_NUM_1, GPIO_MODE_OUTPUT);
        gpio_matrix_out(GPIO_NUM_1, U0TXD_OUT_IDX, 0, 0);
        PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[GPIO_NUM_3], PIN_FUNC_GPIO);
        gpio_set_pull_mode(GPIO_NUM_3, GPIO_PULLUP_ONLY);
        gpio_set_direction(GPIO_NUM_3, GPIO_MODE_INPUT);
        gpio_matrix_in(GPIO_NUM_3, U0RXD_IN_IDX, 0);
        PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[GPIO_NUM_23], PIN_FUNC_GPIO);
        gpio_set_pull_mode(GPIO_NUM_23, GPIO_PULLUP_ONLY);
        gpio_set_level(GPIO_NUM_23, 1);
        gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
        gpio_matrix_out(GPIO_NUM_23, SIG_GPIO_OUT_IDX, 0, 0);
        config.reserved = UART_HW_FLOWCTRL_DISABLE;

        intr_matrix_set(CPUID, ETS_UART0_INTR_SOURCE, uart0.irq);
        xt_set_interrupt_handler(uart0.irq, uart_rx_intr_handler_default, &uart0);
        uart0.uart_idf->int_clr.val = UART_INTR_MASK;
        uart0.uart_idf->conf1.rx_tout_thrhd = UART_TOUT_THRESH_DEFAULT;
        uart0.uart_idf->conf1.rx_tout_en = 1;
        uart0.uart_idf->conf1.rxfifo_full_thrhd = UART_FULL_THRESH_DEFAULT;
        uart0.uart_idf->int_ena.val = UART_RXFIFO_FULL_INT_ENA_M
                                | UART_RXFIFO_TOUT_INT_ENA_M
                                | UART_RXFIFO_OVF_INT_ENA_M
                                | UART_FRM_ERR_INT_ENA_M
                                | UART_BRK_DET_INT_ENA_M
                                | UART_PARITY_ERR_INT_ENA_M;
        ESP_INTR_DISABLE(uart0.irq);

    }else
    {
        return OS_ERROR;
    }


    uart_p->serial_dev.ops = &esp32_uart_ops;
    uart_p->serial_dev.config = config;
    return os_hw_serial_register(&(uart_p->serial_dev), dev->name, OS_DEVICE_FLAG_RDWR, uart_p);
}

OS_DRIVER_INFO esp32_uart_driver = {
    .name   = "ESP32_UART_DRIVER",
    .probe  = esp32_uart_probe,
};

OS_DRIVER_DEFINE(esp32_uart_driver, "0.end.0");

