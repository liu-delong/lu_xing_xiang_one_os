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
 * \@file        drv_uart.h
 *
 * \@brief       This file provides operation functions declaration for uart.
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_UART_H_
#define __DRV_UART_H_

#ifdef OS_USING_SERIAL

#if defined(BSP_USING_UART0)
#ifndef UART0_CONFIG
#define UART0_CONFIG    \
{\
    "uart0",              \
    UART0,                              \
    UART0CLK,   \
    UART0_IRQn  \
}
#endif
#endif

#if defined(BSP_USING_UART1)
#ifndef UART1_CONFIG
#define UART1_CONFIG    \
{\
    "uart1",              \
    UART1,                              \
    UART1CLK,   \
    UART1_IRQn  \
}
#endif
#endif

#if defined(BSP_USING_UART2)
#ifndef UART2_CONFIG
#define UART2_CONFIG    \
{\
    "uart2",              \
    UART2,                              \
    UART2CLK,   \
    UART2_IRQn  \
}
#endif
#endif

#if defined(BSP_USING_UART3)
#ifndef UART3_CONFIG
#define UART3_CONFIG    \
{\
    "uart3",              \
    UART3,                              \
    UART3CLK,   \
    UART3_IRQn  \
}
#endif
#endif

#if defined(BSP_USING_UART4)
#ifndef UART4_CONFIG
#define UART4_CONFIG    \
{\
    "uart4",              \
    UART4,                              \
    UART4CLK,   \
    UART4_IRQn  \
}
#endif
#endif

#if defined(BSP_USING_UART5)
#ifndef UART5_CONFIG
#define UART5_CONFIG    \
{\
    "uart5",              \
    UART5,                              \
    UART5CLK,   \
    UART5_IRQn  \
}
#endif
#endif

int os_hw_usart_init(void);

#endif /* OS_USING_SERIAL */

#endif /* __DRV_UART_H_ */
