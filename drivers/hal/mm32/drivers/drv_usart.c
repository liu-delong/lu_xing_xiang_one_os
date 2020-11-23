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
 * @brief       This file implements usart driver for mm32
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

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

#include "bsp.h"

#ifdef OS_USING_SERIAL

struct mm32_uart
{
    struct os_serial_device serial;

    UART_HandleTypeDef *huart;

    os_size_t count;

    os_size_t size;

    os_int32_t state;

    os_list_node_t list;
};

static os_list_node_t mm32_uart_list = OS_LIST_INIT(mm32_uart_list);

static void mm32_uart_rcc_init(void)
{
#if defined(BSP_USING_UART1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_UART1, ENABLE);  
#endif

#if defined(BSP_USING_UART2)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART2, ENABLE); 
#endif
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);	
}

static void mm32_uart_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

#if defined(BSP_USING_UART1)		
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

#if defined(BSP_USING_UART2)

    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);		
#endif	
}
static void mm32_uart_nvic_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;	

#if defined(BSP_USING_UART1)	
    NVIC_InitStructure.NVIC_IRQChannel = UART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
    NVIC_Init(&NVIC_InitStructure);	
#endif

#if defined(BSP_USING_UART2)
    NVIC_InitStructure.NVIC_IRQChannel = UART2_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
    NVIC_Init(&NVIC_InitStructure);	
#endif


}
static os_err_t mm32_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct mm32_uart *uart;
    UART_InitTypeDef UART_InitStructure;
    UART_HandleTypeDef *m_uart;
    os_uint32_t        data_bits;
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);


    uart = os_container_of(serial, struct mm32_uart, serial);

    UART_InitStructure.UART_BaudRate     = cfg->baud_rate;	
    UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
    UART_InitStructure.UART_Mode = UART_Mode_Rx | UART_Mode_Tx;	  

    m_uart = uart->huart;
    switch (cfg->stop_bits)
    {
        case STOP_BITS_1:
            UART_InitStructure.UART_StopBits = UART_StopBits_1;
            break;
        case STOP_BITS_2:
            UART_InitStructure.UART_StopBits = UART_StopBits_2;
            break;
        default:
            return OS_EINVAL;
    }
    switch (cfg->parity)
    {
        case PARITY_NONE:
            UART_InitStructure.UART_Parity = UART_Parity_No;
            data_bits                = cfg->data_bits;
            break;
        default:
            return OS_EINVAL;
    }

    switch (data_bits)
    {
        case DATA_BITS_8:
            UART_InitStructure.UART_WordLength = UART_WordLength_8b;
            break;    
        default:
            return OS_EINVAL;
    }
    mm32_uart_rcc_init();
    mm32_uart_gpio_init();
    mm32_uart_nvic_init();	

    UART_Init(m_uart->dev, &UART_InitStructure);		

    UART_ITConfig(m_uart->dev, UART_IT_RXIEN, ENABLE);

    UART_Cmd(m_uart->dev, ENABLE);  

    NVIC_EnableIRQ(uart->huart->irq);	

    return OS_EOK;
}

static int mm32_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{   
    return 0;
}

static int mm32_uart_stop_send(struct os_serial_device *serial)
{
    return 0;
}

static int mm32_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct mm32_uart *uart;

    UART_HandleTypeDef *m_uart;				

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial); 

    OS_ASSERT(uart != OS_NULL);

    m_uart = uart->huart;	

    m_uart->buff = buff;

    uart->size = size;

    uart->count = 0;

    uart->state = SET;

    NVIC_EnableIRQ(m_uart->irq);

    UART_ITConfig(m_uart->dev, UART_IT_RXIEN, ENABLE);

    return 0;
}

static int mm32_uart_stop_recv(struct os_serial_device *serial)
{
    struct mm32_uart *uart;

    UART_HandleTypeDef *m_uart;				

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial); 

    OS_ASSERT(uart != OS_NULL);

    m_uart = uart->huart;	

    m_uart->buff = OS_NULL;		

    uart->count = 0;

    uart->state = RESET;

    NVIC_DisableIRQ(m_uart->irq);

    UART_ITConfig(m_uart->dev, UART_IT_RXIEN, DISABLE);

    return 0;
}

static int mm32_uart_recv_state(struct os_serial_device *serial)
{
    int state;
    struct mm32_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial);

    OS_ASSERT(uart != OS_NULL);

    state = uart->count;

    if(uart->state == RESET )
    {
        state |= OS_SERIAL_FLAG_RX_IDLE;
    }

    return state;
}

static int mm32_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct mm32_uart *uart;

    UART_HandleTypeDef *m_uart;		

    int i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial);

    m_uart = uart->huart;

    for (i = 0; i < size; i++)
    {
        level = os_hw_interrupt_disable();	
        while (UART_GetFlagStatus(m_uart->dev, UART_IT_TXIEN) == RESET);	
        UART_SendData(m_uart->dev, *(buff + i));
        os_hw_interrupt_enable(level);
    }

    return size;
}

static int mm32_uart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    return 0;
}

static const struct os_uart_ops mm32_uart_ops = {
    .configure    = mm32_configure,

    .start_send   = mm32_uart_start_send,
    .stop_send    = mm32_uart_stop_send,

    .start_recv   = mm32_uart_start_recv,
    .stop_recv    = mm32_uart_stop_recv,
    .recv_state   = mm32_uart_recv_state,

    .poll_send    = mm32_uart_poll_send,
    .poll_recv    = mm32_uart_poll_recv,
};
static void uart_isr(struct os_serial_device *serial)
{    
    struct mm32_uart *uart;
    UART_HandleTypeDef *m_uart;
    os_uint8_t rev;

    OS_ASSERT(serial != OS_NULL);
    uart = os_container_of(serial, struct mm32_uart, serial);
    OS_ASSERT(uart != OS_NULL);

    m_uart = uart->huart;		
    OS_ASSERT(uart != OS_NULL);	

    if (UART_GetITStatus(m_uart->dev, UART_IT_RXIEN) != RESET)
    {
        UART_ClearITPendingBit(m_uart->dev, UART_IT_RXIEN);

        rev = (os_uint8_t) UART_ReceiveData(m_uart->dev);	

        *(m_uart->buff + uart->count) = rev;

        uart->count++;

        if(uart->count == uart->size)
        {					
            uart->state = RESET;

            UART_ITConfig(m_uart->dev, UART_IT_RXIEN, DISABLE);
        }


    }
    if (UART_GetITStatus(m_uart->dev, UART_IT_TXIEN) != RESET)
    {
        /* clear interrupt */
        UART_ClearITPendingBit(m_uart->dev, UART_IT_TXIEN);
    } 

}
#if defined(BSP_USING_UART1)
/* UART1 device driver structure */
void UART1_IRQHandler(void)
{
    struct mm32_uart *uart;

    /* enter interrupt */
    os_interrupt_enter();

    os_list_for_each_entry(uart, &mm32_uart_list, struct mm32_uart, list)
    {
        if (uart->huart == &huart1)
        {
            /* enter interrupt */
            os_interrupt_enter();
            uart_isr(&uart->serial);
            /* leave interrupt */
            os_interrupt_leave();
            break;
        }
    }
    /* leave interrupt */
    os_interrupt_leave();
}
#endif /* BSP_USING_UART1 */

#if defined(BSP_USING_UART2)
/* UART2 device driver structure */
void UART2_IRQHandler(void)
{
    struct mm32_uart *uart;

    /* enter interrupt */
    os_interrupt_enter();

    os_list_for_each_entry(uart, &mm32_uart_list, struct mm32_uart, list)
    {
        if (uart->huart == &huart2)
        {
            /* enter interrupt */
            os_interrupt_enter();
            uart_isr(&uart->serial);
            /* leave interrupt */
            os_interrupt_leave();
            break;
        }
    }
    /* leave interrupt */
    os_interrupt_leave();
}
#endif /* BSP_USING_UART2 */


static int mm32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

    os_err_t    result  = 0;
    os_base_t   level;

    struct mm32_uart *uart = os_calloc(1, sizeof(struct mm32_uart));		

    OS_ASSERT(uart);

    uart->huart = (UART_HandleTypeDef *)dev->info;

    struct os_serial_device *serial = &uart->serial;	

    serial->ops    = &mm32_uart_ops;
    serial->config = config;				

    level = os_hw_interrupt_disable();
    os_list_add_tail(&mm32_uart_list, &uart->list);
    os_hw_interrupt_enable(level);

    result = os_hw_serial_register(serial, dev->name, OS_DEVICE_FLAG_RDWR, NULL);    

    OS_ASSERT(result == OS_EOK);

    return result;
}

#else   /* OS_USING_SERIAL */

static UART_HandleTypeDef *console_uart;

void os_hw_console_output(const char *str)
{
    if (console_uart == OS_NULL)
        return;

    while (*str)
    {
        if (*str == '\n')
        {
            HAL_UART_Transmit(console_uart, "\r", 1, STM32_UART_POLL_TIMEOUT_MS_PER_BYTE);
        }

        HAL_UART_Transmit(console_uart, (uint8_t *)str, 1, STM32_UART_POLL_TIMEOUT_MS_PER_BYTE);
        str++;
    }
}

static int mm32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    console_uart = (UART_HandleTypeDef *)dev->info;
    return 0;
}

#endif

OS_DRIVER_INFO mm32_usart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = mm32_usart_probe,
};

OS_DRIVER_DEFINE(mm32_usart_driver, "0.end.0");
