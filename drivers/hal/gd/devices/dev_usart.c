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
 
#include "drv_usart.h"

#ifdef BSP_USING_USART0
struct gd32_usart_info usart0_info = {.husart = EVAL_COM0, .usart_clk = EVAL_COM0_CLK
                                        ,.baud_rate = EVAL_COM0_BAUDRATE, .irq = USART0_IRQn
                                        ,.tx_pin = EVAL_COM0_TX_PIN, .rx_pin = EVAL_COM0_RX_PIN
                                        ,.pin_port = EVAL_COM0_GPIO_PORT, .pin_clk = EVAL_COM0_GPIO_CLK
                                        ,.gpio_af_idx = EVAL_COM0_GPIO_AF_IDX
                                    };
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart0", usart0_info);
#endif

#ifdef BSP_USING_USART1
struct gd32_usart_info usart1_info = {.husart = EVAL_COM1, .usart_clk = EVAL_COM1_CLK
                                        ,.baud_rate = EVAL_COM1_BAUDRATE, .irq = USART1_IRQn
                                        ,.tx_pin = EVAL_COM1_TX_PIN, .rx_pin = EVAL_COM1_RX_PIN
                                        ,.pin_port = EVAL_COM1_GPIO_PORT, .pin_clk = EVAL_COM1_GPIO_CLK
                                        ,.gpio_af_idx = EVAL_COM1_GPIO_AF_IDX
                                    };
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart1", usart1_info);
#endif  

#ifdef BSP_USING_USART2
struct gd32_usart_info usart2_info = {.husart = EVAL_COM2, .usart_clk = EVAL_COM2_CLK
                                        ,.baud_rate = EVAL_COM2_BAUDRATE, .irq = USART2_IRQn
                                        ,.tx_pin = EVAL_COM2_TX_PIN, .rx_pin = EVAL_COM2_RX_PIN
                                        ,.pin_port = EVAL_COM2_GPIO_PORT, .pin_clk = EVAL_COM2_GPIO_CLK
                                        ,.gpio_af_idx = EVAL_COM2_GPIO_AF_IDX
                                    };
OS_HAL_DEVICE_DEFINE("Usart_Type", "usart2", usart2_info);
#endif 

#ifdef BSP_USING_UART6
struct gd32_usart_info uart6_info = {.husart = EVAL_COM6, .usart_clk = EVAL_COM6_CLK
                                        ,.baud_rate = EVAL_COM6_BAUDRATE, .irq = UART6_IRQn
                                        ,.tx_pin = EVAL_COM6_TX_PIN, .rx_pin = EVAL_COM6_RX_PIN
                                        ,.pin_port = EVAL_COM6_GPIO_PORT, .pin_clk = EVAL_COM6_GPIO_CLK
                                        ,.gpio_af_idx = EVAL_COM6_GPIO_AF_IDX
                                    };
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart6", uart6_info);                              
#endif
