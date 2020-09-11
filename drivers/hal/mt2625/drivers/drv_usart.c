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
 * @brief       This file implements usart driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_irq.h>
#include <os_memory.h>
#include <bus/bus.h>
#include "hal_uart_internal.h"
#include "os_idle.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

typedef struct _MT_RX_DATA_S
{
    struct os_serial_device *serial;
    uint8_t rx_flag;
    uint8_t *rxbuf;
    uint8_t rxlen;
}MT_RX_DATA_S;


typedef struct _MT_RX_HOOK_S
{
    uint8_t issethook;
    MT_RX_DATA_S sethook[HAL_UART_MAX];
}MT_RX_HOOK_S;

MT_RX_HOOK_S rx_hook = {0};

#define MT2625_DMA_FIFO_TX_BUF_SIZE (64)

ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t uart0_dma_fifo_tx_buf[MT2625_DMA_FIFO_TX_BUF_SIZE];

ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t uart1_dma_fifo_tx_buf[MT2625_DMA_FIFO_TX_BUF_SIZE];

ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t uart2_dma_fifo_tx_buf[MT2625_DMA_FIFO_TX_BUF_SIZE];
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t uart3_dma_fifo_tx_buf[MT2625_DMA_FIFO_TX_BUF_SIZE];

ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t uart2_dma_fifo_rx_buf[OS_SERIAL_RX_BUFSZ];

uint8_t *mt2625_dma_buf_sets[] = 
{
    uart0_dma_fifo_tx_buf, 
    uart1_dma_fifo_tx_buf, 
    uart2_dma_fifo_tx_buf, 
    uart3_dma_fifo_tx_buf,
    uart2_dma_fifo_rx_buf
};

struct mt_uart
{
    struct os_serial_device serial;

    UART_HandleTypeDef *huart;

    os_list_node_t list;
};

static os_list_node_t mt_uart_list = OS_LIST_INIT(mt_uart_list);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    struct mt_uart *uart;

    os_list_for_each_entry(uart, &mt_uart_list, struct mt_uart, list)
    {
        if (uart->huart == huart)
        {
            os_interrupt_enter();
            os_hw_serial_isr_txdone((struct os_serial_device *)uart);
            os_interrupt_leave();
            break;
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    struct mt_uart *uart;

    os_list_for_each_entry(uart, &mt_uart_list, struct mt_uart, list)
    {
        if (uart->huart == huart)
        {
            os_interrupt_enter();
            os_hw_serial_isr_rxdone((struct os_serial_device *)uart, huart->RxXferCount);
            os_interrupt_leave();
            break;
        }
    }
}

uint32_t mt_dma_recv(struct os_serial_device *serial)
{
    struct mt_uart *uart;
    UART_HandleTypeDef *huart;    
    int receive_bytes = 0;
    int left;
    int recvcnt = 0;
    uint8_t *buffer;
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);
    OS_ASSERT(uart != OS_NULL);
    
    huart = uart->huart;
    OS_ASSERT(huart != OS_NULL);

    receive_bytes = hal_uart_get_available_receive_bytes(huart->mt_uart->uart_port);
    if (receive_bytes > huart->RxXferSize)
    {
        receive_bytes = huart->RxXferSize;
    }

    left = receive_bytes;
    buffer = huart->pRxBuffPtr;
    while (left)
    {
        recvcnt = hal_uart_receive_dma(huart->mt_uart->uart_port, buffer, left);
        buffer += recvcnt;
        left -= recvcnt;
    }

    
   return receive_bytes;
}


static void user_uart_callback(hal_uart_callback_event_t status, void *user_data)
{
    struct os_serial_device *serial = (struct os_serial_device *)user_data;
    struct mt_uart *uart;
    UART_HandleTypeDef *huart;
    OS_ASSERT(serial != OS_NULL);
    
    uart = os_container_of(serial, struct mt_uart, serial);
    OS_ASSERT(uart != OS_NULL);

    huart = uart->huart;
    OS_ASSERT(huart != OS_NULL);

    if (status == HAL_UART_EVENT_READY_TO_WRITE)
    {
        HAL_UART_TxCpltCallback(huart);
    } 
    else if (status == HAL_UART_EVENT_READY_TO_READ)
    {
        huart->RxXferCount = mt_dma_recv(serial);
        HAL_UART_RxCpltCallback(huart);
    }
    
}

static os_err_t mt2625_dma_configure(struct os_serial_device *serial)
{    
    struct mt_uart *uart;
    struct mt2625_uart *mt_uart;    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);
    OS_ASSERT(uart != OS_NULL);
    
    mt_uart = uart->huart->mt_uart;
    OS_ASSERT(mt_uart != OS_NULL);


    hal_uart_dma_config_t dma_config;
    dma_config.receive_vfifo_alert_size = 32;
    dma_config.receive_vfifo_buffer = (uint8_t *)uart2_dma_fifo_rx_buf; 
    /* default 64 bytes */
    dma_config.receive_vfifo_buffer_size = OS_SERIAL_RX_BUFSZ;

    dma_config.receive_vfifo_threshold_size = 32;
    dma_config.send_vfifo_buffer = (uint8_t *)mt2625_dma_buf_sets[mt_uart->uart_port];
    dma_config.send_vfifo_buffer_size = MT2625_DMA_FIFO_TX_BUF_SIZE;
    dma_config.send_vfifo_threshold_size = 8;

    hal_uart_set_dma(mt_uart->uart_port, &dma_config);
    hal_uart_register_callback(mt_uart->uart_port, user_uart_callback, (void *)serial);
    
    return OS_EOK;
}

static os_err_t mt_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct mt_uart *uart;
    struct mt2625_uart *mt_uart;
    hal_uart_config_t *config;
    hal_uart_status_t ret;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);
    mt_uart = uart->huart->mt_uart;
    config= &uart->huart->Init;
        
    hal_uart_deinit(mt_uart->uart_port);
    /* init device uart */
    hal_gpio_init(mt_uart->uart_tx_pin);
    hal_pinmux_set_function(mt_uart->uart_tx_pin, mt_uart->uart_tx_pinmux);
    hal_gpio_init(mt_uart->uart_rx_pin);
    hal_pinmux_set_function(mt_uart->uart_rx_pin, mt_uart->uart_rx_pinmux);

    switch (cfg->baud_rate)
    {
        case BAUD_RATE_9600:
            config->baudrate = HAL_UART_BAUDRATE_9600;
            break;
        case BAUD_RATE_115200:
            config->baudrate = HAL_UART_BAUDRATE_115200;
            break;
        case BAUD_RATE_921600:
            config->baudrate = HAL_UART_BAUDRATE_921600;
            break;
        default:
            return OS_EINVAL;
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        config->stop_bit = HAL_UART_STOP_BIT_1;
        break;
    case STOP_BITS_2:
        config->stop_bit = HAL_UART_STOP_BIT_2;
        break;
    default:
        return OS_EINVAL;
    }
    
    switch (cfg->parity)
    {
    case PARITY_NONE:
        config->parity = HAL_UART_PARITY_NONE;
        break;
    case PARITY_ODD:
        config->parity = HAL_UART_PARITY_ODD;
        break;
    case PARITY_EVEN:
        config->parity = HAL_UART_PARITY_EVEN;
        break;
    default:
        return OS_EINVAL;
    }

    switch (cfg->data_bits)
    {
    case DATA_BITS_7:
        config->word_length = HAL_UART_WORD_LENGTH_7;
        break;
    case DATA_BITS_8:
        config->word_length = HAL_UART_WORD_LENGTH_8;
        break;
    default:
        return OS_EINVAL;
    }

    ret = hal_uart_init(mt_uart->uart_port, config);
    if (ret != HAL_UART_STATUS_OK)
    {
        return OS_ERROR;
    }

    if (mt_uart->hdmatx)
    {
        mt2625_dma_configure(serial);
    }

    return OS_EOK;
}

static int mt_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct mt_uart *uart;
    int ret = 0;
    struct mt2625_uart *mt_uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);
    mt_uart = uart->huart->mt_uart;
    
    if (mt_uart->hdmatx)
    {
        ret = hal_uart_send_dma(mt_uart->uart_port, (uint8_t *)buff, size);
    }
    else
    {
        LOG_EXT_E("[%s]-[%d], not support IT recv!!please DMA or polling!!\r\n", __FILE__, __LINE__, ret);
        ret = OS_ERROR;
    }

    return ret;
}

static int mt_uart_stop_send(struct os_serial_device *serial)
{
    return 0;
}

int mt_char_rtcv(hal_uart_port_t uart_port)
{
    return (int)hal_uart_get_char_unblocking(uart_port);
}
uint32_t mt_uart_recv(hal_uart_port_t uart_port, uint8_t *buffer, uint32_t size)
{
    uint32_t len = size;
    uint32_t recvlen = 0;
    int ch = -1;
    
    
    while (len--)
    {
            ch = mt_char_rtcv(uart_port);
            if (ch < 0)
                break;
            buffer[recvlen] = ch;
            recvlen++;
    }

    return recvlen;
}

void mt_uart_rx(hal_uart_port_t uart_port, MT_RX_DATA_S *mt_rx)
{
    uint32_t ret;
    
    ret = mt_uart_recv(uart_port, (uint8_t *)mt_rx->rxbuf, mt_rx->rxlen);

    if (ret > 0)
    {
        os_hw_serial_isr_rxdone(mt_rx->serial, ret);
    }
}

void mt_uart_idle_rx(void)
{
    os_base_t level;
    int i;
    MT_RX_DATA_S *uart_hook;

    for (i = 0; i < HAL_UART_MAX; i++)
    {
        level = os_hw_interrupt_disable();
        uart_hook = &rx_hook.sethook[i];
        os_hw_interrupt_enable(level);
        if (uart_hook->rx_flag == 1)
        {
            mt_uart_rx(i, uart_hook);
        }
    }
}

int mt_uart_dma_recv_start(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct mt_uart *uart;
    UART_HandleTypeDef *huart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);    
    huart = uart->huart;
    huart->pRxBuffPtr = buff;
    huart->RxXferSize = size;

    return size;
}

static int mt_uart_it_recv_start(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct mt_uart *uart;
    struct mt2625_uart *mt_uart;
    os_err_t ret2;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);
    OS_ASSERT(uart != OS_NULL);
    mt_uart = uart->huart->mt_uart;
    OS_ASSERT(mt_uart != OS_NULL);
    OS_ASSERT(mt_uart->uart_port < HAL_UART_MAX);

    if (rx_hook.issethook == 0)
    {
        ret2 = os_idle_task_set_hook(mt_uart_idle_rx);
        if (OS_EOK != ret2)
        {
            return OS_ERROR;
        }
        rx_hook.issethook = 1;
    }
    
    level = os_hw_interrupt_disable();
    rx_hook.sethook[mt_uart->uart_port].rx_flag = 1;
    rx_hook.sethook[mt_uart->uart_port].rxbuf = buff;
    rx_hook.sethook[mt_uart->uart_port].rxlen = size;
    rx_hook.sethook[mt_uart->uart_port].serial = serial;
    os_hw_interrupt_enable(level);

    return size;
}

static int mt_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    int  ret = 0;
    struct mt_uart *uart;
    struct mt2625_uart *mt_uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);    
    mt_uart = uart->huart->mt_uart;

    if (mt_uart->hdmatx)
    {
        ret = mt_uart_dma_recv_start(serial, buff, size);
    }
    else
    {
        ret = mt_uart_it_recv_start(serial, buff, size);
    }
    
    uart->huart->RxState = HAL_UART_STATE_BUSY_RX;

    return ((ret > 0) ? ret:0);
}

static int mt_uart_stop_recv_IT(struct os_serial_device *serial)
{
    struct mt_uart *uart;
    struct mt2625_uart *mt_uart;
    os_base_t level;
    int i;
    int flag = 0;
    os_err_t ret2;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);
    mt_uart = uart->huart->mt_uart;

    if (mt_uart->uart_port >= HAL_UART_MAX)
    {
        return OS_ERROR;
    }

    level = os_hw_interrupt_disable();
    rx_hook.sethook[mt_uart->uart_port].rx_flag = 0;
    rx_hook.sethook[mt_uart->uart_port].rxbuf = NULL;
    rx_hook.sethook[mt_uart->uart_port].rxlen = 0;
    rx_hook.sethook[mt_uart->uart_port].serial = NULL;
    os_hw_interrupt_enable(level);

    for (i = 0; i < HAL_UART_MAX; i++)
    {
        flag |= rx_hook.sethook[i].rx_flag;
    }

    /*  */
    if (flag == 0)
    {
        ret2 = os_idle_task_del_hook(mt_uart_idle_rx);
        if (OS_EOK != ret2)
        {
            return OS_ERROR;
        }
        
        rx_hook.issethook = 0;
    }

    return OS_EOK;
}

static int mt_uart_stop_recv(struct os_serial_device *serial)
{
    struct mt_uart *uart;
    struct mt2625_uart *mt_uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);
    OS_ASSERT(uart != OS_NULL);
    mt_uart = uart->huart->mt_uart;
    OS_ASSERT(mt_uart != OS_NULL);
    
    if (mt_uart->hdmatx)
    {
        ;
    }
    else
    {
        mt_uart_stop_recv_IT(serial);
    }

    uart->huart->RxState = HAL_UART_STATE_READY;
    //hal_uart_deinit(mt_uart->uart_port);

    return 0;
}

static int mt_uart_recv_state(struct os_serial_device *serial)
{
    int state = 0;
    struct mt_uart *uart;
    struct mt2625_uart *mt_uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);
    mt_uart = uart->huart->mt_uart;

    if (mt_uart->hdmatx)
    {
        state = hal_uart_get_available_receive_bytes(mt_uart->uart_port);
    }

    if (uart->huart->RxState == HAL_UART_STATE_READY)
    {
        state |= OS_SERIAL_FLAG_RX_IDLE;
    }
    
    return state;
}

static int mt_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct mt_uart *uart;
    os_uint32_t  ret = 0;
    struct mt2625_uart *mt_uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mt_uart, serial);
    mt_uart = uart->huart->mt_uart;
    ret = hal_uart_send_polling(mt_uart->uart_port, buff, size);

    return ret;
}

static int mt_uart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct mt_uart *uart;
    os_uint32_t  ret;
    struct mt2625_uart *mt_uart;
    
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(size == 1);

    uart = os_container_of(serial, struct mt_uart, serial);
    OS_ASSERT(uart != OS_NULL);
    mt_uart = uart->huart->mt_uart;
    OS_ASSERT(mt_uart != OS_NULL);

    ret = hal_uart_receive_polling(mt_uart->uart_port, (uint8_t *)buff, size);

    return ret;
}

static const struct os_uart_ops mt_uart_ops = {
    .configure    = mt_configure,

    .start_send   = mt_uart_start_send,
    .stop_send    = mt_uart_stop_send,

    .start_recv   = mt_uart_start_recv,
    .stop_recv    = mt_uart_stop_recv,
    .recv_state   = mt_uart_recv_state,
    
    .poll_send    = mt_uart_poll_send,
    .poll_recv    = mt_uart_poll_recv,
};


static int mt_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    os_err_t    result  = 0;
    os_base_t   level;

    struct mt_uart *uart = os_calloc(1, sizeof(struct mt_uart));

    OS_ASSERT(uart);

    uart->huart = (UART_HandleTypeDef *)dev->info;

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &mt_uart_ops;
    serial->config = config;
    uart->huart->RxState = HAL_UART_STATE_READY;

    level = os_hw_interrupt_disable();
    os_list_add_tail(&mt_uart_list, &uart->list);
    os_hw_interrupt_enable(level);
    
    result = os_hw_serial_register(serial, dev->name, OS_DEVICE_FLAG_RDWR, NULL);

    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO mt_usart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = mt_usart_probe,
};

OS_DRIVER_DEFINE(mt_usart_driver, "1");
