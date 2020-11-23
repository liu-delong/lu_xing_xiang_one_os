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
 * @file        ws_client_app.c
 *
 * @brief       The websocket client application
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <os_errno.h>
#include <os_dbg_ext.h>
#include <os_task.h>
#include "ws_client.h"
#include "core/include/ws_common.h"

#if ((defined NET_USING_WEBSOCKET_CLIENT) && (defined WEBSOCKET_CLIENT_EXAMPLE))

static os_bool_t gs_ws_flag = OS_TRUE;

static void ws_onOpen(void)
{
    printf("%s\n", __FUNCTION__);
}
static void ws_onClose(os_uint16_t close_code)
{
    printf("%s:code=%d\n", __FUNCTION__, close_code);
    gs_ws_flag = OS_FALSE;
}
static void ws_onError(os_uint16_t err_code)
{
    printf("%s:code=%d\n", __FUNCTION__, err_code);
    gs_ws_flag = OS_FALSE;
}
static void ws_onMessage(os_uint8_t *data, os_size_t len, ws_data_type_t type)
{
    printf("%s:type=%d,len=%ld\n", __FUNCTION__, type, len);
    for (os_size_t i = 0; i < len; i++)
    {
        printf("%c", data[i]);
    }
    printf("\n");
}
struct ws_event event = {
    &ws_onOpen,
    &ws_onClose,
    &ws_onError,
    &ws_onMessage,
};

static char *header = "GET / HTTP/1.1\r\n"
                      "Host: 127.0.0.1:9999\r\n"
                      "Connection: Upgrade\r\n"
                      "Pragma: no-cache\r\n"
                      "Cache-Control: no-cache\r\n"
                      "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) "
                      "Chrome/83.0.4103.116 Safari/537.36\r\n"
                      "Upgrade: websocket\r\n"
                      "Origin: file://\r\n"
                      "Sec-WebSocket-Version: 13\r\n"
                      "Accept-Encoding: gzip, deflate, br\r\n"
                      "Accept-Language: zh-CN,zh;q=0.9\r\n"
                      "Sec-WebSocket-Key: ZCt0L2xCipxRBAGgt+AaYg==\r\n"
                      "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n"
                      "\r\n";       /* define by user */

char url[128] = {0};

static void task_entry(void *parameter)
{
    gs_ws_flag = OS_TRUE;

    int fd = ws_client_create(url);

    if (fd < 0)
    {
        LOG_EXT_E("client create fail!");
        return;
    }

    printf("ws satate:%d\n", ws_client_get_state(fd));

    int ret = ws_client_event_register(fd, &event);
    if (ret != OS_EOK)
    {
        LOG_EXT_E("ws event registe fail!");
        return;
    }

    if (strcmp(url, "ws://192.1.1.5:9999"))
    {
        ret = ws_client_start(fd, 3, ws_client_get_default_header(fd));
    }
    else
    {
        ret = ws_client_start(fd, 3, header);
    }

    if (ret != OS_EOK)
    {
        LOG_EXT_E("ws client start fail!");
        return;
    }

    while (gs_ws_flag)
    {
        os_uint8_t tx_buf[WEBSOCKET_CLIENT_MAX_BUFF_DEP / 4 * 3];
        os_uint8_t out[WEBSOCKET_CLIENT_MAX_BUFF_DEP];

        ws_get_random_string(tx_buf, sizeof(tx_buf));
        ret = base64_encode(tx_buf, sizeof(tx_buf), out, sizeof(out));

        ws_client_send(fd, out, ret, WS_DATA_TXT_DATA);
        os_task_msleep(5000);
        ws_client_send(fd, out, 4, WS_DATA_PING);
    }

    ws_client_stop(fd, 0);
    ws_client_destroy(fd);
    return;
}

static int ws_client(char argc, char **argv)
{
    os_uint8_t start = 0xFF;
    if (argc < 2)
    {
        printf("example:\n"
               "ws_client start ws://192.168.1.2 or\n"
               "ws_client stop\n");
        return (-1);
    }
    if (!strncmp((const char *)argv[1], "start", strlen("start")))
    {
        start = OS_TRUE;
    }
    else if (!strncmp((const char *)argv[1], "stop", strlen("stop")))
    {
        start = OS_FALSE;
    }

    if (start == OS_TRUE)
    {
        if (argc < 3)
        {
            printf("pls input valid url!");
            return (-1);
        }
        os_size_t len = strlen(argv[2]);

        memset(url, 0, 128);
        memcpy(url, argv[2], len > 128 ? 128 : len);
    }
    if (start == OS_FALSE)
    {
        gs_ws_flag = OS_FALSE;
    }
    else if (start == OS_TRUE)
    {
        static os_uint8_t times = 0;
        char              task_name[32];

        snprintf(task_name, sizeof(task_name), "ws_app%d", times++);
        os_task_t *task = os_task_create(task_name, task_entry, OS_NULL, 8096, SHELL_TASK_PRIORITY - 1, 3);
        if (task != OS_NULL)
        {
            os_task_startup(task);
        }
    }
    return 0;
}

#ifdef OS_USING_SHELL

#include <shell.h>

SH_CMD_EXPORT(ws_client, ws_client, "start/stop websocket client");

#endif /* OS_USING_SHELL */

#endif /* NET_USING_WEBSOCKET_CLIENT */
