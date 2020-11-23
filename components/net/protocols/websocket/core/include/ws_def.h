/*
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
 * @file        ws_def.h
 *
 * @brief       The websocket define header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __WS_DEF_H__
#define __WS_DEF_H__

#include <oneos_config.h>
#include <os_types.h>
#include <sys/socket.h>

#ifdef NET_USING_WEBSOCKET_CLIENT

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef DEC
#define DEC(X) 1
#endif

#define MAX_WS_HEAD_LENGTH      8
#define MAX_URL_LENGTH          63
#define MAX_SUB_PROTOCOL_LENGTH 63
#define WS_DEF_PORT             80
#define WSS_DEF_PORT            443

typedef enum ws_state
{
    WS_CLOSED,
    WS_OPEN,
    WS_CONNECTING,
    WS_CLOSING,
} ws_state_t;

typedef enum ws_close_type
{
    WS_CLOSE_NORMAL           = 1000,
    WS_CLOSE_GOING_AWAY       = 1001,
    WS_CLOSE_PROTOCOL_ERROR   = 1002,
    WS_CLOSE_UNSUPPORT_DATA   = 1003,
    WS_CLOSE_RESERVE1         = 1004,
    WS_CLOSE_NO_STATE         = 1005,
    WS_CLOSE_ABNORMAL         = 1006,
    WS_INVALID_DATA           = 1007,
    WS_CLOSE_POLICE_VIOLATION = 1008,
    WS_CLOSE_TOO_LARGE        = 1009,
    WS_CLOSE_MISS_EXTENSION   = 1010,
    WS_CLOSE_INTERNAL_ERROR   = 1011,
    WS_CLOSE_SERVICE_RESTART  = 1012,
    WS_CLOSE_TRY_AGAIN_LATER  = 1013,
    WS_CLOSE_RESERVE2         = 1014,
    WS_CLOSE_TLS_HANDSHAKE    = 1015,
} ws_close_type_t;

typedef enum ws_data_type
{
    WS_DATA_ERR      = -1,
    WS_DATA_MID_DATA = 0,
    WS_DATA_TXT_DATA = 129,
    WS_DATA_BIN_DATA = 130,
    WS_DATA_DIS_CONN = 136,
    WS_DATA_PING     = 137,
    WS_DATA_PONG     = 138,
} ws_data_type_t;

typedef void (*open_event)(void);
typedef void (*close_event)(os_uint16_t close_code);
typedef void (*error_event)(os_uint16_t err_code);
typedef void (*message_event)(os_uint8_t *data, os_size_t len, ws_data_type_t type);

typedef struct ws_event
{
    open_event    onOpen;    /* Socket open event response function */
    close_event   onClose;   /* Socket close event response function */
    error_event   onError;   /* Socket error event response function */
    message_event onMessage; /* Socket receive message event response function */
} ws_event_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NET_USING_WEBSOCKET_CLIENT */

#endif /* __WS_DEF_H__ */
