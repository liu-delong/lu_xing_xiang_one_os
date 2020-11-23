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
 * @file        ws_common.c
 *
 * @brief       The websocket common functions
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <time.h>
#include <os_errno.h>
#include <os_dbg_ext.h>
#include <os_assert.h>
#include "thirdpart/polarssl/include/sha1.h"
#include "core/include/ws_def.h"
#include "core/include/ws_common.h"

#ifdef NET_USING_WEBSOCKET_CLIENT

#if DEC("url parsing")
static int ws_url_header_parsing(const char *header, os_bool_t *secure)
{
    if (strcmp(header, "ws") == 0)
    {
        *secure = OS_FALSE;
    }
    else if (strcmp(header, "wss") == 0)
    {
        *secure = OS_TRUE;
    }
    else
    {
        return OS_ERROR;
    }

    return OS_EOK;
}
static int ws_url_body_parsing(const char *body, struct in_addr *sin_addr, os_uint16_t *sin_port)
{
    const char separator[]                 = ":";
    char       body_backup[MAX_URL_LENGTH] = {0};

    memcpy(body_backup, body, strlen(body));
    char *ip = strtok(body_backup, separator);
    if (ip == OS_NULL)
        return OS_ERROR;

    struct hostent *hostent = gethostbyname(ip);
    if (hostent == OS_NULL)
    {
        LOG_EXT_E("DNS resolution error!");
        return OS_ERROR;
    }

    if (hostent->h_addrtype != AF_INET)
    {
        LOG_EXT_E("not support ipv6!");
        return OS_ERROR;
    }

    if (*(hostent->h_addr_list) == OS_NULL)
    {
        LOG_EXT_E("DNS resolution error!");
        return OS_ERROR;
    }

    memcpy(&sin_addr->s_addr, *(hostent->h_addr_list), hostent->h_length);

    char *port = strtok(OS_NULL, separator);
    port       = strtok(port, "/");
    if (port == OS_NULL)
    {
        *sin_port = 0;
        return OS_EOK;
    }

    if (atol(port) > OS_UINT16_MAX)
    {
        LOG_EXT_E("port out of range!");
        return OS_ERROR;
    }

    *sin_port = (os_uint16_t)atol(port);

    return OS_EOK;
}

int ws_url_parsing(const char *url, os_bool_t *secure, struct in_addr *sin_addr, os_uint16_t *sin_port)
{
    const char separator[]                = "://";
    char       url_backup[MAX_URL_LENGTH] = {0};

    if (strlen(url) > MAX_URL_LENGTH)
    {
        LOG_EXT_E("URL length is too long!");
        return OS_ERROR;
    }

    memcpy(url_backup, url, strlen(url));
    char *header = strtok(url_backup, separator);
    if (header == OS_NULL)
    {
        LOG_EXT_E("URL parsing fail!");
        return OS_ERROR;
    }

    if (strlen(header) + strlen(separator) >= strlen(url))
    {
        LOG_EXT_E("URL parsing fail!");
        return OS_ERROR;
    }

    char *body = url_backup + strlen(header) + strlen(separator);

    if (ws_url_header_parsing(header, secure))
    {
        LOG_EXT_E("URL parsing fail!");
        return OS_ERROR;
    }

    if (ws_url_body_parsing(body, sin_addr, sin_port))
    {
        LOG_EXT_E("URL parsing fail!");
        return OS_ERROR;
    }

    return OS_EOK;
}
#endif /* url parsing */

#if DEC("package data parsing")
void ws_get_random_string(os_uint8_t *buf, os_size_t len)
{
#ifdef OS_USING_RTC
    srand((int)time(0));
#else
    static int seed = 0;
    srand(seed);
#endif
    for (os_size_t i = 0; i < len; i++)
    {
        os_uint8_t temp = (os_uint8_t)(rand() % 256);
        buf[i]          = temp ? temp : 128;
#ifndef OS_USING_RTC
        seed = (seed << 8) | temp;
#endif
    }
    return;
}

int ws_en_package(os_uint8_t    *in,
                  os_size_t      in_len,
                  os_uint8_t    *out,
                  os_size_t      out_len,
                  os_bool_t      mask,
                  ws_data_type_t type)
{
    os_size_t header_len = 2;

    if (mask)
    {
        mask = 1;
    }

    if (in_len > 0x7D)
    {
        header_len += 2;
    }

    if (out_len < (header_len + in_len + mask * 4))
    {
        return OS_ERROR;
    }

    memset(out, 0, out_len);

    os_size_t len = 0;

    out[len++] = type;
    out[len] |= (mask << 7);

    if (in_len < 126)
    {
        out[len++] |= in_len;
    }
    else if (in_len < 0xFFFF)    // websocket supports UINT64, but ipv4 only support UINT16
    {
        out[len++] |= 0x7E;
        out[len++] = (os_uint8_t)(in_len >> 8);
        out[len++] = (os_uint8_t)(in_len & 0xFF);
    }
    else
    {
        return OS_ERROR;
    }

    if (mask)
    {
        os_uint8_t mask_key[4] = {0};

        ws_get_random_string(mask_key, 4);
        memcpy(&out[len], mask_key, 4);
        len += 4;
        for (os_size_t i = 0; i < in_len; i++)
        {
            out[len++] = (os_uint8_t)(((~mask_key[i % 4]) & in[i]) | ((mask_key[i % 4]) & (~in[i])));
        }
    }
    else
    {
        memcpy(&out[len], in, in_len);
        len += in_len;
    }

    return len;
}

int ws_de_package(os_uint8_t *data, os_size_t len, ws_data_type_t *type)
{
    os_uint8_t mask_key[4] = {0};
    os_bool_t  mask;

    switch (data[0])
    {
    case WS_DATA_MID_DATA:
        *type = WS_DATA_MID_DATA;
        break;
    case WS_DATA_TXT_DATA:
        *type = WS_DATA_TXT_DATA;
        break;
    case WS_DATA_BIN_DATA:
        *type = WS_DATA_BIN_DATA;
        break;
    case WS_DATA_DIS_CONN:
        *type = WS_DATA_DIS_CONN;
        break;
    case WS_DATA_PING:
        *type = WS_DATA_PING;
        break;
    case WS_DATA_PONG:
        *type = WS_DATA_PONG;
        break;
    default:
        LOG_EXT_E("Unrecognized data type!");
        return OS_ERROR;
    }

    os_size_t data_len   = 0;
    os_size_t header_len = 2;

    mask = (data[1] >> 7) ? 1 : 0;
    if (((data[1]) & 0x7F) == 0x7F)    // websocket supports UINT64, but ipv4 only support UINT16
    {
        return OS_ERROR;
    }
    else if (((data[1]) & 0x7F) == 0x7E)
    {
        header_len += 2;
        if (len < header_len)
            return OS_ERROR;
        data_len = (data[2] << 8) | data[3];
    }
    else
    {
        data_len = data[1] & 0x7F;
    }

    if (len != (header_len + mask * 4 + data_len))
    {
        return OS_ERROR;
    }

    os_size_t i;
    os_size_t j;
    if (mask)
    {
        memcpy(mask_key, &data[header_len], 4);
        for (i = header_len + mask * 4, j = 0; i < len; i++, j++)
        {
            data[j] = (os_uint8_t)(((~mask_key[j % 4]) & data[i]) | (mask_key[j % 4] & (~data[i])));
        }
    }
    else
    {
        for (i = header_len, j = 0; i < len; i++, j++)
        {
            data[j] = data[i];
        }
    }

    data[j] = 0;

    return data_len;
}
#endif /* package data parsing */

#if DEC("base64 code")
int base64_encode(os_uint8_t *in, os_size_t in_len, os_uint8_t *out, os_size_t out_len)
{
    os_uint8_t *base64_table = (os_uint8_t *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    os_size_t   len;

    if (in_len % 3 == 0)
    {
        len = in_len / 3 * 4;
    }
    else
    {
        len = (in_len / 3 + 1) * 4;
    }

    if (out_len < len)
    {
        return (-1);
    }

    memset(out, 0, out_len);

    int i;
    int j;
    for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
    {
        out[i]     = base64_table[in[j] >> 2];
        out[i + 1] = base64_table[(in[j] & 0x3) << 4 | (in[j + 1] >> 4)];
        out[i + 2] = base64_table[(in[j + 1] & 0xf) << 2 | (in[j + 2] >> 6)];
        out[i + 3] = base64_table[in[j + 2] & 0x3f];
    }

    switch (in_len % 3)
    {
    case 1:
        out[i - 2] = '=';
        out[i - 1] = '=';
        break;
    case 2:
        out[i - 1] = '=';
        break;
    }

    return len;
}

int base64_decode(os_uint8_t *in, os_size_t in_len, os_uint8_t *out, os_size_t out_len)
{
    os_uint8_t table[] = {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                          0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                          0,  62, 0,  0,  0,  63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,
                          0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
                          19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  0,  0,  26, 27, 28, 29, 30, 31, 32, 33,
                          34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
    size_t     str_len;

    if (strstr((const char *)in, "=="))
    {
        str_len = in_len / 4 * 3 - 2;
    }
    else if (strstr((const char *)in, "="))
    {
        str_len = in_len / 4 * 3 - 1;
    }
    else
    {
        str_len = in_len / 4 * 3;
    }

    if (out_len < str_len)
        return 0;

    memset(out, 0, out_len);

    int i;
    int j;
    for (i = 0, j = 0; i < in_len - 2; j += 3, i += 4)
    {
        out[j]     = (table[in[i]] << 2) | (table[in[i + 1]] >> 4);
        out[j + 1] = (table[in[i + 1]] << 4) | (table[in[i + 2]] >> 2);
        out[j + 2] = (table[in[i + 2]] << 6) | (table[in[i + 3]]);
    }

    return (j + 2);
}
#endif /* base64 code */

#if DEC("common function")
int ws_send_ping_packet(int socket, os_uint8_t *buf, os_size_t len)
{
    os_uint8_t send_buf[8];

    int data_len = ws_en_package(buf, len, send_buf, sizeof(send_buf), OS_FALSE, WS_DATA_PING);
    OS_ASSERT(data_len > 0);

    int ret = send(socket, send_buf, data_len, 0);
    if (ret < data_len)
    {
        LOG_EXT_I("socket create fail:%s(%d)!", strerror(errno), errno);
        return OS_ERROR;
    }

    return OS_EOK;
}

int ws_generate_respond_shake_key(os_uint8_t *in, os_size_t in_len, os_uint8_t *out, os_size_t out_len)
{
    const os_uint8_t magic_key[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    os_uint8_t *in_key = malloc(in_len + sizeof(magic_key) + 1);
    if (in_key == OS_NULL)
    {
        return (-1);
    }

    memset(out, 0, out_len);
    memcpy(in_key, in, in_len);
    memcpy(&in_key[in_len], magic_key, sizeof(magic_key));
    in_key[in_len + sizeof(magic_key)] = '\0';

    os_uint8_t sha_data[20];
    ws_sha1(in_key, strlen((const char *)in_key), sha_data);
    free(in_key);

    return base64_encode(sha_data, sizeof(sha_data), out, out_len);
}

#endif /* common function */

#endif /* NET_USING_WEBSOCKET_CLIENT */
