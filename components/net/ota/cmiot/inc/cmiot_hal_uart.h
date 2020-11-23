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
 * @file        cmiot_hal_uart.h
 *
 * @brief       The hal uart header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_HAL_UART__
#define __CMIOT_HAL_UART__

#include "cmiot_stdlib.h"
#include "cmiot_at_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CMIOT_TIMERS
#include "cmiot_timer.h"
#endif

#ifdef CMIOT_TIMERS
cmiot_extern cmiot_timer_handle_t cmiot_atp_timer;
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG NULL
#endif

cmiot_extern void cmiot_bl_main_printf(cmiot_char *data);

#ifdef CMIOT_BL_PRINT_MAXLEN
#define cmiot_bl_debug_printf   cmiot_kprintf
#define cmiot_full_debug_printf cmiot_kprintf
#define cmiot_bl_full_printf    cmiot_kprintf
#define cmiot_bl_debug_printf   cmiot_kprintf
#define cmiot_bl_debug_print(tag, content, ...)                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        cmiot_kprintf(content, ##__VA_ARGS__);                                                                         \
    } while (0);
#define peer_logout cmiot_kprintf
#else
#define cmiot_bl_debug_printf(...)
#define cmiot_full_debug_printf(...)
#define cmiot_bl_full_printf(...)
#define cmiot_bl_debug_printf(...)
#define cmiot_bl_debug_print(...)
#define peer_logout(...)
#endif

#ifdef CMIOT_UARTTX_MAXLEN
cmiot_extern cmiot_char at_buf[512];
#endif

#if (CMIOT_APP_DEBUG == 1)
#define CMIOT_USART_DEBUG_AT
#endif

#if !defined(CMIOT_USART_DEBUG_BAUDRATE)
#define CMIOT_USART_DEBUG_BAUDRATE 115200
#endif

#ifdef CMIOT_USART_DEBUG_AT
#define CMIOT_UARTRX_DEBUGLEN 255
cmiot_extern cmiot_uint8  cmiot_debug_rxbuf[CMIOT_UARTRX_DEBUGLEN];
cmiot_extern cmiot_uint16 cmiot_debug_rxbuf_len;
cmiot_extern cmiot_char * cmiot_get_cmiot_debug_rxbuf(void);
cmiot_extern cmiot_uint16 cmiot_get_cmiot_debug_rxbuf_len(void);
#endif

#ifdef CMIOT_DEBUG_MODE
#define cmiot_debug_printf cmiot_kprintf
#define cmiot_debug_override_function()                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        static cmiot_bool first = cmiot_true;                                                                          \
        if (first)                                                                                                     \
        {                                                                                                              \
            CMIOT_DEBUG(__func__, "function can be rewritten");                                                        \
            first = cmiot_false;                                                                                       \
        }                                                                                                              \
    } while (0)
#else
#define cmiot_debug_printf(...)
#define cmiot_debug_override_function(...)
#endif

#define CMIOT_TAG "CMIOT"
#define CMIOT_INFO(tag, content, ...)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        cmiot_kprintf("[%s] " content "\r\n", tag, ##__VA_ARGS__);                                                     \
    } while (0)
#define CMIOT_DEBUG(tag, content, ...)                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        cmiot_debug_printf("\r\n[%s] " content "\r\n", tag, ##__VA_ARGS__);                                            \
    } while (0)
cmiot_extern void      cmiot_kprintf(const char *fmt, ...);
cmiot_extern cmiot_int cmiot_at_send_cmd(const cmiot_char *fmt, ...);
cmiot_extern void      cmiot_at_send_data(cmiot_char *data, cmiot_int16 len);
cmiot_extern cmiot_char * cmiot_get_at_buf(void);
cmiot_extern cmiot_uint16 cmiot_get_at_buf_len(void);
cmiot_extern cmiot_uint16 cmiot_get_cmiot_at_rxbuf_len(void);

#ifdef __cplusplus
}
#endif

#endif
