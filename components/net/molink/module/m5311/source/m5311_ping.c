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
 * @file        m5311_ping.c
 *
 * @brief       m5311 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5311_ping.h"

#define DBG_EXT_TAG "m5311.ping"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define M5311_MIN_PING_PKG_LEN (8)
#define M5311_MAX_PING_PKG_LEN (1400)

#ifdef M5311_USING_PING_OPS

os_err_t m5311_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser          = &self->parser;
    os_err_t     result          = OS_EOK;
    os_uint32_t  req_time        = 0;
    os_int16_t   ttl             = -1;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    char ret_buff[36]                   = {0};

    if (parser == OS_NULL)
    {
        LOG_EXT_E("M5311 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < M5311_MIN_PING_PKG_LEN) || (len > M5311_MAX_PING_PKG_LEN))
    {
        LOG_EXT_E("M5311 ping: ping package len[%hu] is out of rang[%d, %d].",
                   len, M5311_MIN_PING_PKG_LEN, M5311_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    /* M5311 timeout range not define */

    LOG_EXT_D("M5311 ping: %s, len: %hu, timeout: %ums", host, len, timeout);
    /* Need to wait for 4 lines response msg */
    char resp_buff[256] = {0};
    at_resp_t at_resp = {.buff      = resp_buff,
                        .buff_size  = sizeof(resp_buff),
                        .line_num   = 4,
                        .timeout    = (5 + timeout / 1000) * OS_TICK_PER_SECOND};

    /* Exec commond "AT+PING="www.baidu.com",32,2000,1 and wait response */
    /* Return: success: +PING: 39.156.66.14,49,1206  fail: +PINGERR: 1 */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+PING=%s,%hu,%u,1", host, len, timeout) < 0)
    {
        LOG_EXT_E("Ping: AT cmd exec fail: AT+PING=%s,%hu,%u", host, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+PING", "+PING: %[^,],%hd,%u", ipaddr, &ttl, &req_time) <= 0)
    {
        if (at_resp_get_data_by_kw(&at_resp, "+", "+%s", ret_buff) <= 0)
        {
            LOG_EXT_E("AT+PING resp prase \"+NPINGERR\" fail.");
        }

        LOG_EXT_E("M5311 ping %s fail: %s, check network status and try to set a longer timeout.", host, ret_buff);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("M5311 ping: resp prase ip[%s], req_time[%u], ttl[%hd]", ipaddr, req_time, ttl);
        if (ttl <= 0)
        {
            result = OS_ETIMEOUT;
        }
        else
        {
            result = OS_EOK;
        }
    }

    if (0 != req_time)
    {
        LOG_EXT_D("M5311 ping return ip: %s", ipaddr);
        inet_aton(ipaddr, &(resp->ip_addr));
        resp->data_len = len;
        resp->ttl      = ttl;
        resp->time     = req_time;
    }

__exit:

    return result;
}

#endif /* M5311_USING_PING_OPS */
