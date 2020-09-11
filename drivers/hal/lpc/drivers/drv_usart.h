/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-07-15     Magicoe      The first version for LPC55S6x
 */

#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include "fsl_usart.h"
#include "fsl_usart_dma.h"
#include "fsl_dma.h"

/****** USART Event *****/
#define ARM_USART_EVENT_SEND_COMPLETE       (1UL << 0)  ///< Send completed; however USART may still transmit data
#define ARM_USART_EVENT_RECEIVE_COMPLETE    (1UL << 1)  ///< Receive completed
#define ARM_USART_EVENT_TRANSFER_COMPLETE   (1UL << 2)  ///< Transfer completed
#define ARM_USART_EVENT_TX_COMPLETE         (1UL << 3)  ///< Transmit completed (optional)
#define ARM_USART_EVENT_TX_UNDERFLOW        (1UL << 4)  ///< Transmit data not available (Synchronous Slave)
#define ARM_USART_EVENT_RX_OVERFLOW         (1UL << 5)  ///< Receive data overflow
#define ARM_USART_EVENT_RX_TIMEOUT          (1UL << 6)  ///< Receive character timeout (optional)
#define ARM_USART_EVENT_RX_BREAK            (1UL << 7)  ///< Break detected on receive
#define ARM_USART_EVENT_RX_FRAMING_ERROR    (1UL << 8)  ///< Framing error detected on receive
#define ARM_USART_EVENT_RX_PARITY_ERROR     (1UL << 9)  ///< Parity error detected on receive
#define ARM_USART_EVENT_CTS                 (1UL << 10) ///< CTS state changed (optional)
#define ARM_USART_EVENT_DSR                 (1UL << 11) ///< DSR state changed (optional)
#define ARM_USART_EVENT_DCD                 (1UL << 12) ///< DCD state changed (optional)
#define ARM_USART_EVENT_RI                  (1UL << 13) ///< RI  state changed (optional)

#define USART_RX_DMA_CHANNEL      4
#define USART_TX_DMA_CHANNEL      5
#define USART_DMA_BASEADDR DMA0

typedef struct _hal_usart_handle
{
    USART_Type *uart_base;
    const usart_config_t *uart_config;
    usart_dma_handle_t *uartDmaHandle;
} hal_usart_handle_t;

typedef struct _hal_uart_receive_state
{
    volatile uint8_t *buffer;
    volatile uint32_t bufferLength;
    volatile uint32_t bufferSofar;
} hal_uart_receive_state_t;

/*! @brief uart TX state structure. */
typedef struct _hal_uart_send_state
{
    volatile uint8_t *buffer;
    volatile uint32_t bufferLength;
    volatile uint32_t bufferSofar;
} hal_uart_send_state_t;

typedef void (*hal_uart_transfer_callback_t)(USART_Type *base, usart_handle_t *handle, status_t status, void *userData);

typedef struct _hal_uart_transfer
{
    uint8_t *data;   /*!< The buffer of data to be transfer.*/
    size_t dataSize; /*!< The byte count to be transfer. */
} hal_uart_transfer_t;

typedef struct _hal_uart_state
{
    hal_uart_transfer_callback_t callback;
    void *callbackParam;
    
    usart_handle_t hardwareHandle;
    hal_uart_receive_state_t rx;
    hal_uart_send_state_t tx;
    
    uint8_t instance;
} hal_uart_state_t;


int os_hw_uart_init(void);

#define USART0_GetFreq(void) CLOCK_GetFlexCommClkFreq(0U);

#endif /* __DRV_UART_H__ */
