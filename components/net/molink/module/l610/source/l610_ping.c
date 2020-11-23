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
 * @file        l610_ping.c
 *
 * @brief       l610 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "l610_ping.h"

#define DBG_EXT_TAG "l610.ping"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define L610_MIN_PING_PKG_LEN (0)
#define L610_MAX_PING_PKG_LEN (1372)
#define L610_MIN_PING_TIMEOUT (1)
#define L610_MAX_PING_TIMEOUT (65535)

#ifdef L610_USING_PING_OPS

os_err_t l610_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    if (len > L610_MAX_PING_PKG_LEN)
    {
        LOG_EXT_E("L610 ping: ping package len[%d] is out of rang[%d, %d].", 
                  len, L610_MIN_PING_PKG_LEN, L610_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout < L610_MIN_PING_TIMEOUT) || (timeout > L610_MAX_PING_TIMEOUT))
    {
        LOG_EXT_E("L610 ping: ping timeout %us is out of rang[%ds, %ds].", 
                  timeout, L610_MIN_PING_TIMEOUT, L610_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    LOG_EXT_D("L610 ping: %s, len: %d, timeout: %us", host, len, timeout);

    at_parser_t *parser = &self->parser;

    char resp_buff[256] = {0};

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 6,
                         .timeout   = os_tick_from_ms(1000 + timeout),
                        };

    /* +MPING=<mode>[,<Destination_IP/hostname>[,<count>[,<size>[,<TTL>[,<TOS>[,<TimeOut>]]]]]] */
    os_err_t result = at_parser_exec_cmd(parser, &at_resp, "AT+MPING=1,\"%s\",1,%d,255,0,%d", host, len, timeout);
    if (result != OS_EOK)
    {
        return result;
    }

    char ip_addr[IPADDR_MAX_STR_LEN + 1] = {0};

    os_uint32_t req_time = 0;

    /* +MPINGSTAT: 0,"39.156.66.18",1,1,168 */
    if (at_resp_get_data_by_kw(&at_resp,
                               "+MPINGSTAT:",
                               "+MPINGSTAT: %*d,\"%[^\"]\",%*d,%*d,%hd",
                               ip_addr,
                               &req_time) <= 0)
    {
        LOG_EXT_E("L610 ping %s fail, check network status and try to set a longer timeout.", host);
        result = OS_ERROR;
        return result;
    }
    else
    {
        LOG_EXT_D("L610 ping: resp prase ip[%s], req_time[%u], ttl[%d]", ip_addr, req_time, ttl);
        if (req_time <= 0)
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
        inet_aton(ip_addr, &(resp->ip_addr));
        resp->data_len = len;
        resp->ttl      = 255;
        resp->time     = req_time;
    }

    return result;
}

#endif /* L610_USING_PING_OPS */
