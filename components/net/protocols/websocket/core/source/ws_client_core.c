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
 * @file        ws_client_core.c
 *
 * @brief       The websocket client core functions
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <sys/socket.h>
#include <os_errno.h>
#include <os_dbg_ext.h>
#include <os_assert.h>
#include <os_clock.h>
#include "core/include/ws_client_core.h"
#include "core/include/ws_common.h"

#ifdef NET_USING_WEBSOCKET_CLIENT

#define WS_CLIENT_BUFF_LENGTH      (MAX_WS_HEAD_LENGTH + WEBSOCKET_CLIENT_MAX_BUFF_DEP)
#define WS_CLIENT_TASK_STACK_DEPTH (WEBSOCKET_CLIENT_TASK_STACKSIZE + WS_CLIENT_BUFF_LENGTH)

typedef enum ws_errno
{
    WS_EOK,
    WS_ESEND,
    WS_EREC,
} ws_errno_t;

struct ws_conn_info
{
    char               url[MAX_URL_LENGTH + 1]; /* url string */
    os_bool_t          secure;                  /* ws or wss*/
    struct sockaddr_in sock_in;                 /* ws server ip infomation*/
};

struct ws_client
{
    ws_state_t          state;     /* websocket link status */
    struct ws_conn_info conn_info; /* connect infomation*/
    int                 socket;    /* socket fd */
    int                 err_code;  /* websocket error number */
    ws_event_t *        event;     /* websocket event response function */
    os_task_t *         task;      /* rece task point */
    os_uint8_t *        send_buf;  /* send queue buffer */
};

static struct ws_client ws_client_list[WEBSOCKET_CLIENT_TASK_MAX_INSTANCE];

#define get_client_subitem(fd, param, item)                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        OS_ASSERT(fd >= 0 && fd < WEBSOCKET_CLIENT_TASK_MAX_INSTANCE);                                                 \
        os_enter_critical();                                                                                           \
        param = ws_client_list[fd].item;                                                                               \
        os_exit_critical();                                                                                            \
    } while (0)

#define set_client_subitem(fd, param, item)                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        OS_ASSERT(fd >= 0 && fd < WEBSOCKET_CLIENT_TASK_MAX_INSTANCE);                                                 \
        os_enter_critical();                                                                                           \
        ws_client_list[fd].item = param;                                                                               \
        os_exit_critical();                                                                                            \
    } while (0)

static inline int ws_fd_is_invalid(int fd)
{
    if (fd < 0 || fd >= WEBSOCKET_CLIENT_TASK_MAX_INSTANCE)
    {
        return OS_ERROR;
    }

    os_enter_critical();
    char *url = ws_client_list[fd].conn_info.url;
    os_exit_critical();

    if (!strlen(url))
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static int ws_insert_client_list(struct ws_client *ws)
{
    int fd;

    OS_ASSERT(ws != OS_NULL);

    if (!strlen(ws->conn_info.url) || websocket_client_get_by_url(ws->conn_info.url) >= 0)
    {
        return OS_ERROR;
    }

    for (fd = 0; fd < WEBSOCKET_CLIENT_TASK_MAX_INSTANCE; fd++)
    {
        os_enter_critical();
        char *url = ws_client_list[fd].conn_info.url;
        os_exit_critical();

        if (!strlen(url))
        {
            break;
        }
    }

    if (fd == WEBSOCKET_CLIENT_TASK_MAX_INSTANCE)
    {
        return OS_ERROR;
    }

    os_enter_critical();
    memcpy(&ws_client_list[fd], ws, sizeof(struct ws_client));
    os_exit_critical();

    return fd;
}

static int ws_delete_client_list(int fd)
{
    if (ws_fd_is_invalid(fd))
    {
        LOG_EXT_E("websocket fd is invalid!");
        return OS_ERROR;
    }

    os_enter_critical();
    memset(&ws_client_list[fd], 0, sizeof(struct ws_client));
    os_exit_critical();

    return OS_EOK;
}

static int ws_shake_key_compare(os_uint8_t *key, os_uint8_t *respond)
{
    if (!strlen((const char *)key) || !strlen((const char *)respond))
    {
        LOG_EXT_E("websocket respond key error!");
        return (-1);
    }

    os_uint8_t exp[32];
    if (ws_generate_respond_shake_key(key, strlen((const char *)key), exp, sizeof(exp)) < 0)
    {
        LOG_EXT_E("websocket respond key error!");
        return (-1);
    }

    if (!strcmp((const char *)exp, (const char *)respond))
    {
        return OS_EOK;
    }

    LOG_EXT_E("websocket respond key error!");
    return (-1);
}

static int ws_shake_process(int socket, os_tick_t tick_out, char *header)
{
    os_uint8_t *p             = OS_NULL;
    os_uint8_t  shake_key[25] = {0};
    os_size_t   header_len    = strlen((const char *)header);
    os_tick_t   start         = os_tick_get();

    if ((p = (os_uint8_t *)strstr((const char *)header, "Sec-WebSocket-Key: ")) != NULL)
    {
        p += strlen("Sec-WebSocket-Key: ");
        memcpy(shake_key, p, sizeof(shake_key) - 1);
    }
    else
    {
        LOG_EXT_E("websocket shake header error!");
        return OS_ERROR;
    }

    os_uint8_t *rece_buf = (os_uint8_t *)malloc(WS_CLIENT_BUFF_LENGTH);
    if (rece_buf == OS_NULL)
    {
        LOG_EXT_E("Out of memory!");
        return OS_ERROR;
    }

    while (1)
    {
        send(socket, header, header_len, 0);
        int ret = recv(socket, rece_buf, WS_CLIENT_BUFF_LENGTH, 0);

        if (ret > 0)
        {
            if (strncmp((const char *)rece_buf, "HTTP", strlen("HTTP")) == 0)
            {
                if ((p = (os_uint8_t *)strstr((const char *)rece_buf, "Sec-WebSocket-Accept: ")) != NULL)
                {
                    p += strlen("Sec-WebSocket-Accept: ");
                    sscanf((const char *)p, "%s\r\n", p);
                    if (ws_shake_key_compare(shake_key, p) == 0)
                    {
                        free(rece_buf);
                        return OS_EOK;
                    }
                }
            }
        }
        else
        {
            os_task_msleep(100);
        }
        if (tick_out)
        {
            os_tick_t now           = os_tick_get();
            os_tick_t time_interval = now > start ? (now - start) : (OS_UINT32_MAX - start + now);
            if (time_interval >= tick_out)
            {
                LOG_EXT_E("shake time out!");
                free(rece_buf);
                return OS_ERROR;
            }
        }
    }
}

static int ws_client_task_prepare(int fd, os_uint8_t time_out, char *header)
{
    struct ws_conn_info conn_info;
    int                 socket_fd;
    ws_event_t *        event;
    os_tick_t           start = os_tick_get();

    if (ws_fd_is_invalid(fd))
    {
        LOG_EXT_E("websocket fd is invalid!");
        return (-1);
    }

    get_client_subitem(fd, conn_info, conn_info);
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        LOG_EXT_E("socket create fail:%s(%d)!", strerror(errno), errno);
        return (-1);
    }

    struct timeval time_val = {1, 0};
    if (setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&time_val, sizeof(struct timeval)) == -1)
    {
        LOG_EXT_E("set socket send time out fail:%s(%d)!", strerror(errno), errno);
        return (-1);
    }
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_val, sizeof(struct timeval)) == -1)
    {
        LOG_EXT_E("set socket recv time out fail:%s(%d)!", strerror(errno), errno);
        return (-1);
    }

    set_client_subitem(fd, WS_CONNECTING, state);

    os_tick_t tick_out = OS_TICK_PER_SECOND * time_out;
    os_tick_t time_interval;
    os_tick_t now;

    while (connect(socket_fd, (struct sockaddr *)(&conn_info.sock_in), sizeof(struct sockaddr)) == -1)
    {
        if (tick_out)
        {
            now           = os_tick_get();
            time_interval = now > start ? (now - start) : (OS_UINT32_MAX - start + now);
            if (time_interval >= tick_out)
            {
                LOG_EXT_E("connect time out!");
                return (-1);
            }
        }
        os_task_msleep(100);
    }

    if (tick_out)
    {
        now           = os_tick_get();
        time_interval = now > start ? (now - start) : (OS_UINT32_MAX - start + now);
        if (time_interval >= tick_out)
        {
            return (-1);
        }
        tick_out -= time_interval;
    }

    if (header != OS_NULL && ws_shake_process(socket_fd, tick_out, header) != OS_EOK)
    {
        /*release socket_fd*/
        closesocket(socket_fd);
        return (-1);
    }

    set_client_subitem(fd, WS_OPEN, state);
    get_client_subitem(fd, event, event);

    if (event != OS_NULL && event->onOpen != OS_NULL)
    {
        event->onOpen();
    }

    return socket_fd;
}

static int ws_client_message_process(int socket, ws_event_t *event)
{

    os_uint8_t rec_buf[WS_CLIENT_BUFF_LENGTH];

    if (event == OS_NULL)
    {
        os_task_msleep(1);
        return OS_ERROR;
    }

    int len = recv(socket, rec_buf, WS_CLIENT_BUFF_LENGTH, 0);
    if (len == 0)
    {
        /* socket disconnect */
        LOG_EXT_I("socket disconnect!");
        if (event->onClose != OS_NULL)
        {
            event->onClose(WS_CLOSE_NORMAL);
        }
        return (int)WS_DATA_DIS_CONN;
    }
    else if (len < 0)
    {
        /* time out */
        os_task_msleep(10);
        return OS_EOK;
    }

    ws_data_type_t type;
    int            data_len = ws_de_package(rec_buf, len, &type);
    if (data_len < 0)
    {
        LOG_EXT_E("de package error!");
        return OS_ERROR;
    }

    switch (type)
    {
    case WS_DATA_ERR:
        LOG_EXT_E("ws rece error data!");
        break;
    case WS_DATA_MID_DATA:
    case WS_DATA_TXT_DATA:
    case WS_DATA_BIN_DATA:
    case WS_DATA_PING:
    case WS_DATA_PONG:
        if (event->onMessage != OS_NULL)
            event->onMessage(rec_buf, data_len, type);
        break;
    case WS_DATA_DIS_CONN:
        if (event->onClose != OS_NULL)
        {
            os_uint16_t colseCode = WS_CLOSE_NORMAL;
            if (data_len > 1)
                colseCode = (rec_buf[0] << 8) | rec_buf[1];
            event->onClose(colseCode);
        }
        break;
    default:
        LOG_EXT_I("ws rece unknown type data!");
        break;
    }
    return (int)type;
}

static void ws_client_task(void *parameter)
{
    const int   fd = *(int *)parameter;
    int         socket;
    ws_event_t *event;

    if (ws_fd_is_invalid(fd))
    {
        LOG_EXT_E("websocket fd is invalid!");
        return;
    }
    get_client_subitem(fd, socket, socket);
    get_client_subitem(fd, event, event);

    if (event == OS_NULL)
    {
        LOG_EXT_E("websocket has been not registered event!");
        return;
    }

    while (1)
    {
        /* message process */
        int ret = ws_client_message_process(socket, event);
        if (ret == (int)WS_DATA_DIS_CONN)
        {
            LOG_EXT_I("websocket receive disconnect message, task stop!");
            break;
        }
    }

    set_client_subitem(fd, OS_NULL, task);
    set_client_subitem(fd, WS_CLOSED, state);
}

static int ws_secure_client_task_prepare(int fd, os_uint8_t time_out, char *header)
{
    /*To Do*/
    return (-1);
}

static void ws_secure_client_task(void *parameter)
{
    /*To Do*/
    return;
}

char *websocket_client_get_default_header(int fd)
{
    static char *default_header = OS_NULL;
    const char   demo_header[]  = "GET / HTTP/1.1\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Host: %s:%hu\r\n"
                               "Sec-WebSocket-Key: %s\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "\r\n";
    os_uint8_t          header[200] = {0};
    os_uint8_t          shake_key[25];
    os_uint8_t          random_data[16];
    struct ws_conn_info conn_info;

    if (ws_fd_is_invalid(fd))
    {
        LOG_EXT_E("websocket fd is invalid!");
        return OS_NULL;
    }

    get_client_subitem(fd, conn_info, conn_info);
    ws_get_random_string(random_data, sizeof(random_data));
    base64_encode(random_data, sizeof(random_data), shake_key, sizeof(shake_key));
    snprintf((char *)header,
             sizeof(header),
             demo_header,
             inet_ntoa(conn_info.sock_in.sin_addr),
             ntohs(conn_info.sock_in.sin_port),
             shake_key);
    os_size_t header_len = strlen((const char *)header);

    if (default_header != OS_NULL)
    {
        free(default_header);
        default_header = OS_NULL;
    }

    if ((default_header = (char *)malloc(header_len + 1)) == OS_NULL)
    {
        LOG_EXT_E("Out of memory!");
        return OS_NULL;
    }

    memcpy(default_header, header, header_len);
    default_header[header_len] = '\0';

    return default_header;
}

int websocket_client_get_by_url(const char *url)
{
    if (url == OS_NULL || !strlen(url))
    {
        LOG_EXT_E("url input error!");
        return (-1);
    }

    for (int fd = 0; fd < WEBSOCKET_CLIENT_TASK_MAX_INSTANCE; fd++)
    {
        os_enter_critical();
        char *url_c = ws_client_list[fd].conn_info.url;
        os_exit_critical();

        if (url_c != OS_NULL && !strcmp(url, url_c))
        {
            return fd;
        }
    }

    return (-1);
}

int websocket_client_create(os_bool_t secure, struct in_addr sin_addr, os_uint16_t sin_port, const char *url)
{
    struct ws_client ws;

    if (url == OS_NULL || !strlen(url))
    {
        LOG_EXT_E("url input error!");
        return OS_ERROR;
    }

    int fd = websocket_client_get_by_url(url);
    if (!ws_fd_is_invalid(fd))
    {
        LOG_EXT_I("WebSocket has been created.");
        return fd;
    }

    os_uint8_t *send_buf = (os_uint8_t *)malloc(WS_CLIENT_BUFF_LENGTH);
    if (send_buf == OS_NULL)
    {
        LOG_EXT_E("Out of memory!");
        return OS_ERROR;
    }

    memset(&ws, 0, sizeof(struct ws_client));
    memcpy(ws.conn_info.url, url, strlen(url));

    ws.send_buf                     = send_buf;
    ws.conn_info.secure             = secure;
    ws.conn_info.sock_in.sin_addr   = sin_addr;
    ws.conn_info.sock_in.sin_family = AF_INET;
    ws.conn_info.sock_in.sin_port   = htons(sin_port);

    return ws_insert_client_list(&ws);
}

void websocket_client_destroy(int fd)
{
    os_task_t *task = OS_NULL;

    if (ws_fd_is_invalid(fd))
    {
        LOG_EXT_E("websocket fd is invalid!");
        return;
    }

    get_client_subitem(fd, task, task);
    if (task != OS_NULL)
    {
        websocket_client_stop(fd, WS_CLOSE_NORMAL); /*stop task, close socket*/
    }

    os_uint8_t *send_buf;
    get_client_subitem(fd, send_buf, send_buf);
    OS_ASSERT(send_buf != OS_NULL);
    free(send_buf);

    ws_delete_client_list(fd);
    return;
}

int websocket_client_event_register(int fd, struct ws_event *event)
{
    if (event == OS_NULL)
    {
        LOG_EXT_E("event is NULL!");
        return OS_ERROR;
    }

    if (ws_fd_is_invalid(fd))
    {
        LOG_EXT_E("websocket fd is invalid!");
        return OS_ERROR;
    }

    set_client_subitem(fd, event, event);
    return OS_EOK;
}

ws_state_t websocket_client_get_state(int fd)
{
    ws_state_t state;

    if (ws_fd_is_invalid(fd))
    {
        LOG_EXT_E("websocket fd is invalid!");
        return WS_CLOSED;
    }
    get_client_subitem(fd, state, state);
    return state;
}

int websocket_client_start(int fd, os_uint8_t time_out, char *header)
{
    os_task_t *         task = OS_NULL;
    char                name[OS_NAME_MAX];
    struct ws_conn_info conn_info;

    if (ws_fd_is_invalid(fd))
    {
        LOG_EXT_E("websocket fd is invalid!");
        return OS_ERROR;
    }
    get_client_subitem(fd, task, task);
    if (task != OS_NULL)
    {
        LOG_EXT_E("task has been create!");
        return OS_ERROR;
    }
    get_client_subitem(fd, conn_info, conn_info);

    int socket;
    if (conn_info.secure)
    {
        socket = ws_secure_client_task_prepare(fd, time_out, header);
    }
    else
    {
        socket = ws_client_task_prepare(fd, time_out, header);
    }

    if (socket < 0)
    {
        return OS_ERROR;
    }
    set_client_subitem(fd, socket, socket);

    snprintf(name, sizeof(name), "ws_c_%d", fd);
    if (conn_info.secure)
    {
        task = os_task_create(name,
                              ws_secure_client_task,
                              (void *)(&fd),
                              WS_CLIENT_TASK_STACK_DEPTH,
                              WEBSOCKET_CLIENT_TASK_PRIORITY,
                              WEBSOCKET_CLIENT_TASK_TICK);
    }
    else
    {
        task = os_task_create(name,
                              ws_client_task,
                              (void *)(&fd),
                              WS_CLIENT_TASK_STACK_DEPTH,
                              WEBSOCKET_CLIENT_TASK_PRIORITY,
                              WEBSOCKET_CLIENT_TASK_TICK);
    }

    OS_ASSERT(task);
    set_client_subitem(fd, task, task);
    os_task_startup(task);

    return OS_EOK;
}

int websocket_client_send(int fd, os_uint8_t *data, os_uint16_t len, ws_data_type_t type)
{
    if (ws_fd_is_invalid(fd))
    {
        LOG_EXT_E("websocket fd is invalid!");
        return OS_ERROR;
    }

    if (len > WEBSOCKET_CLIENT_MAX_BUFF_DEP)
    {
        LOG_EXT_E("tx buff length is invalid!");
        return OS_ERROR;
    }

    os_uint8_t *send_buf = OS_NULL;
    get_client_subitem(fd, send_buf, send_buf);
    OS_ASSERT(send_buf != OS_NULL);

    int send_len = ws_en_package(data, len, send_buf, WS_CLIENT_BUFF_LENGTH, OS_TRUE, type);
    OS_ASSERT(send_len > 0);

    int socket;
    get_client_subitem(fd, socket, socket);

    int attempts = 3;
    int ret      = 0;
    while (attempts--)
    {
        if (ret < send_len)
        {
            ret += send(socket, send_buf + ret, send_len - ret, 0);
        }
        else
        {
            set_client_subitem(fd, WS_EOK, err_code);
            return OS_EOK;
        }
    }

    int err_code;
    get_client_subitem(fd, err_code, err_code);

    if (err_code != WS_EOK)
    {
        ws_event_t *event;
        get_client_subitem(fd, event, event);
        if (event != OS_NULL && event->onError != OS_NULL)
        {
            event->onError(err_code);
        }
    }

    set_client_subitem(fd, WS_ESEND, err_code);
    return OS_ERROR;
}

void websocket_client_stop(int fd, os_uint16_t code)
{
    os_task_t *task = OS_NULL;

    if (ws_fd_is_invalid(fd))
    {
        LOG_EXT_E("websocket fd is invalid!");
        return;
    }
    set_client_subitem(fd, WS_CLOSING, state);

    /* exit task */
    get_client_subitem(fd, task, task);
    if (task != OS_NULL && os_task_destroy(task) != OS_EOK)
    {
        LOG_EXT_E("task exit error!");
    }
    set_client_subitem(fd, OS_NULL, task);

    /* dis connect socket */
    int socket;
    get_client_subitem(fd, socket, socket);
    if (socket < 0)
    {
        LOG_EXT_E("query socket fd error!");
        return;
    }

    websocket_client_send(fd, (os_uint8_t *)&code, sizeof(code), WS_DATA_DIS_CONN);
    os_task_msleep(10);
    closesocket(socket);

    set_client_subitem(fd, (-1), socket);
    set_client_subitem(fd, WS_CLOSED, state);
    return;
}

#endif /* NET_USING_WEBSOCKET_CLIENT */
