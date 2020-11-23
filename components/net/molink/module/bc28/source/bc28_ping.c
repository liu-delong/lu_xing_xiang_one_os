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
 * @file        bc28_ping.c
 *
 * @brief       bc28 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc28_ping.h"
#include <string.h>

#define DBG_EXT_TAG "bc28.ping"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define BC28_MIN_PING_PKG_LEN (12)
#define BC28_MAX_PING_PKG_LEN (1500)
#define BC28_MIN_PING_TIMEOUT (10)
#define BC28_MAX_PING_TIMEOUT (600000)

#ifdef BC28_USING_PING_OPS

static os_err_t bc28_ping_dns(mo_object_t *self, const char *domain_name, ip_addr_t *addr);

os_err_t bc28_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser          = &self->parser;
    os_err_t     result          = OS_EOK;
    os_uint32_t  req_time        = 0;
    os_int16_t   ttl             = -1;
    ip_addr_t    addr            = {0};

    char ipsend[IPADDR_MAX_STR_LEN + 1] = {0};
    char iprecv[IPADDR_MAX_STR_LEN + 1] = {0};
    char ret_buff[36]                   = {0};
    char resp_buff[256]                 = {0};

    if (parser == OS_NULL)
    {
        LOG_EXT_E("BC28 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < BC28_MIN_PING_PKG_LEN) || (len > BC28_MAX_PING_PKG_LEN))
    {
        LOG_EXT_E("BC28 ping: ping package len[%d] is out of rang[%d, %d].", 
                  len, BC28_MIN_PING_PKG_LEN, BC28_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout < BC28_MIN_PING_TIMEOUT) || (timeout > BC28_MAX_PING_TIMEOUT))
    {
        LOG_EXT_E("BC28 ping: user set ping timeout value %ums is out of rang[%dms, %dms].", 
                  timeout, BC28_MIN_PING_TIMEOUT, BC28_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    LOG_EXT_D("BC28 ping: %s, len: %u, timeout: %ums", host, len, timeout);
    
    /* DNS: bc28 dosen't support ping domain name, do dns before ping */
    result = bc28_ping_dns(self, host, &addr);
    if (OS_EOK != result)
    {
        LOG_EXT_E("BC28 ping: dns error, host: %s", host);
        return result;
    }

    strncpy(ipsend, inet_ntoa(addr), IPADDR_MAX_STR_LEN);
    LOG_EXT_D("BC28 ping: domain ip is %s, addrlen %d", ipsend, strlen(ipsend));
    /* DNS: finished */

    /**
     * Exec commond "AT+NPING=183.232.231.174,64,5000 and wait response
     * Return: success: +NPING:183.232.231.174,54,1974  fail: +NPINGERR:1
     * Module only support IPV4, BC28 only support Dotted Dec/Hex/Oct Notation
     * Because unable to synchronize time between board & module, 5 seconds reserved 
     *  **/
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 4,
                         .timeout   = (5 + timeout / 1000) * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &at_resp, "AT+NPING=%s,%hd,%u", ipsend, len, timeout) < 0)
    {
        LOG_EXT_E("Ping: AT cmd exec fail: AT+NPING=%s,%u,%u", ipsend, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+NPING", "+NPING:%[^,],%hd,%u", iprecv, &ttl, &req_time) <= 0)
    {
        if (at_resp_get_data_by_kw(&at_resp, "+", "+%s", ret_buff) <= 0)
        {
            LOG_EXT_E("AT+NPING resp prase \"+NPINGERR\" fail.");
        }

        LOG_EXT_E("BC28 ping %s fail: %s, check network status and try to set a longer timeout.", host, ret_buff);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("BC28 ping: resp prase ip[%s], req_time[%u], ttl[%d]", iprecv, req_time, ttl);
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
        inet_aton(iprecv, &(resp->ip_addr));
        resp->data_len = len;
        resp->ttl      = ttl;
        resp->time     = req_time;
    }

__exit:

    return result;
}

static os_err_t bc28_ping_dns(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_ERROR;

	char recvip[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[256]                 = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 4,
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &resp, "AT+QDNS=0,\"%s\"", domain_name);
    if (OS_EOK != result)
    {
        LOG_EXT_E("%s: execute error, host: %s", __FUNCTION__, domain_name);
        goto __exit;
    }

    /* AT+CMDNS="www.baidu.com" return: OK \r\n  +CMDNS:183.232.231.172 \r\n */
    /* AT+CMDNS="8.8.8.8" return: +CMDNS:8.8.8.8 \r\n  OK */
    if (at_resp_get_data_by_kw(&resp, "+QDNS:", "+QDNS:%s", recvip) <= 0)
    {
        LOG_EXT_E("%s: resp parse fail, try again, host: %s", __FUNCTION__, domain_name);
        result = OS_ERROR;
        /* If resolve failed, maybe receive an URC CRLF */
        goto __exit;
    }

    if (strlen(recvip) < IPADDR_MIN_STR_LEN)
    {
        LOG_EXT_E("%s: recvip len < IPADDR_MIN_STR_LEN, len = %d", __FUNCTION__, strlen(recvip));
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("%s: \"%s\" domain ip is %s, addrlen %d", __FUNCTION__, domain_name, recvip, strlen(recvip));
        inet_aton(recvip, addr);

        if (IPADDR_ANY == addr->addr || IPADDR_LOOPBACK == addr->addr)
        {
            ip_addr_set_zero(addr);
            result = OS_ERROR;
            goto __exit;
        }
        
        result = OS_EOK;
    }

__exit:

    return result;
}

#endif /* BC28_USING_PING_OPS */
