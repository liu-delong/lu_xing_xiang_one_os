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
 * @file        ws_client.c
 *
 * @brief       The websocket client API
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <os_errno.h>
#include <os_dbg_ext.h>
#include "ws_client.h"
#include "core/include/ws_common.h"
#include "core/include/ws_client_core.h"

#ifdef NET_USING_WEBSOCKET_CLIENT

/**
 ***********************************************************************************************************************
 * @brief           This function is used to create a websocket client file descriptor.
 *
 * @param[in]       url             The websocket server's URL
 *
 * @return          Websocket client file descriptor
 * @retval          Less than 0     Fail.
 * @retval          Other           Successful.
 ***********************************************************************************************************************
 */
int ws_client_create(const char *url)
{
    /*
    url example:
    "ws://websocket.org"
    "ws://127.0.0.1:1234"
    */
    struct in_addr sin_addr;
    os_uint16_t    sin_port;
    os_bool_t      secure;

    if (url == OS_NULL || !strlen(url))
    {
        return OS_ERROR;
    }

    if (ws_url_parsing(url, &secure, &sin_addr, &sin_port))
    {
        LOG_EXT_E("url paresing error!");
        return OS_ERROR;
    }
    if (sin_port == 0)
    {
        sin_port = secure ? WSS_DEF_PORT : WS_DEF_PORT;
    }
    return websocket_client_create(secure, sin_addr, sin_port, url);
}

/**
 ***********************************************************************************************************************
 * @brief           This function is used to destroy a websocket client file descriptor and release resources.
 *
 * @param[in]       fd              Websocket client file descriptor.
 *
 * @return          None
 ***********************************************************************************************************************
 */
void ws_client_destroy(int fd)
{
    websocket_client_destroy(fd);
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function registers websocket callback events.
 *
 * @param[in]       fd              Websocket client file descriptor.
 * @param[in]       event           Pointer to user callback events.
 *
 * @return          Regist result.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
int ws_client_event_register(int fd, struct ws_event *event)
{
    return websocket_client_event_register(fd, event);
}

/**
 ***********************************************************************************************************************
 * @brief           This function query the current status of websocket client.
 *
 * @param[in]       fd              Websocket client file descriptor.
 *
 * @return          State.
 * @retval          WS_CLOSED       Websocket client is closed, can't been used.
 * @retval          WS_OPEN         Websocket client is opening, can been used.
 * @retval          WS_CONNECTING   Websocket client is connecting, can't been used.
 * @retval          WS_CLOSING      Websocket client is closing, can't been used.
 ***********************************************************************************************************************
 */
ws_state_t ws_client_get_state(int fd)
{
    return websocket_client_get_state(fd);
}

/**
 ***********************************************************************************************************************
 * @brief           This function start websocket client, including TCP connection and websocket handshake.
 *
 * @param[in]       fd              Websocket client file descriptor.
 * @param[in]       timeout         Time out in seconds, 0 means forever.
 * @param[in]       header          Websocket client handshake header, NULL means not handshake.
 *
 * @return          Start result.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
int ws_client_start(int fd, os_uint8_t time_out, char *header)
{
    return websocket_client_start(fd, time_out, header);
}

/**
 ***********************************************************************************************************************
 * @brief           This function send data to websocket server.
 *
 * @param[in]       fd              Websocket client file descriptor.
 * @param[in]       data            Pointer to user data to send.
 * @param[in]       len             Data pointer length, cannot exceed MAX_DATA_LENGTH.
 * @param[in]       type            Data type.
 *
 * @return          Send result.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
int ws_client_send(int fd, os_uint8_t *data, os_uint16_t len, ws_data_type_t type)
{
    return websocket_client_send(fd, data, len, type);
}

/**
 ***********************************************************************************************************************
 * @brief           This function stop websockt client, it will send the shutdown reason to the server.
 *
 * @param[in]       fd              Websocket client file descriptor.
 * @param[in]       code            Shutdown reason conding.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void ws_client_stop(int fd, os_uint16_t code)
{
    websocket_client_stop(fd, code);
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function query file descriptor by URL.
 *
 * @param[in]       url             The websocket server's URL
 *
 * @return          Websocket client file descriptor
 * @retval          Less than 0     Fail.
 * @retval          Other           Successful.
 ***********************************************************************************************************************
 */
int ws_client_get_by_url(const char *url)
{
    return websocket_client_get_by_url(url);
}

/**
 ***********************************************************************************************************************
 * @brief           This function generate default websocket clinet handshake header.
 *
 * @param[in]       fd              Websocket client file descriptor.
 *
 * @return          Websocket client handshake header
 * @retval          NULL            Fail.
 * @retval          Other           Successful.
 ***********************************************************************************************************************
 */
char *ws_client_get_default_header(int fd)
{
    return websocket_client_get_default_header(fd);
}

#endif /* NET_USING_WEBSOCKET_CLIENT */
