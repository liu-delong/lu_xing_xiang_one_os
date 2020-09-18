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
 * @file        ec200x.c
 *
 * @brief       ec200x module link kit netserv api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ec200x_netserv.h"
#include "ec200x.h"

#include <stdlib.h>
#include <string.h>

#ifdef MOLINK_USING_IP
#include <mo_ipaddr.h>
#endif /* MOLINK_USING_IP */

#define DBG_EXT_TAG "ec200x.netserv"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef EC200X_USING_NETSERV_OPS

#define EC200X_MIN_PING_TIME   (1 * OS_TICK_PER_SECOND)
#define EC200X_MAX_PING_TIME   (255 * OS_TICK_PER_SECOND)

os_err_t ec200x_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CGATT=%d", attach_stat);
}

os_err_t ec200x_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+CGATT:", "+CGATT: %d", attach_stat) <= 0)
    {
        LOG_EXT_E("Get %s module attach state failed", self->name);
        return OS_ERROR;
    }
    
    return OS_EOK;
}

os_err_t ec200x_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%d", reg_n);
}

os_err_t ec200x_get_reg(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG: %d,%d", reg_n, reg_stat) <= 0)
    {
        LOG_EXT_E("Get %s module register state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t ec200x_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CGACT=%d,%d", act_stat, cid);
}

os_err_t ec200x_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGACT:", "+CGACT: %d,%d", cid, act_stat) <= 0)
    {
        LOG_EXT_E("Get %s module cgact state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t ec200x_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CSQ:", "+CSQ: %d,%d", rssi, ber) <= 0)
    {
        LOG_EXT_E("Get %s module signal quality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t ec200x_get_ipaddr(mo_object_t *self, char ip[])
{
    at_parser_t *parser = &self->parser;
    os_int8_t    len    = -1;

    char ipaddr[IP_SIZE] = {0};

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGPADDR=1");
    if (result != OS_EOK)
    {
        LOG_EXT_E("Get ip address fail: AT+CGPADDR cmd exec fail.");
        goto __exit;
    }

    /* Response for ex: +CGPADDR:0,100.113.120.235 */
    if (at_resp_get_data_by_kw(&resp, "+CGPADDR:", "+CGPADDR:%*[^\"]\"%[^\"]", ipaddr) <= 0)
    {
        LOG_EXT_E("Get ip address: parse resp fail.");
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(ipaddr);
    if ((len < MIN_IP_SIZE) || (len >= IP_SIZE))
    {
        LOG_EXT_E("IP address size [%d] error.", len);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        strcpy(ip, ipaddr);
        LOG_EXT_D("IP address: %s", ip);
    }

__exit:
    
    return result;
}

os_err_t ec200x_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser          = &self->parser;
    os_err_t     result          = OS_EOK;
    os_int32_t   response        = -1;
    os_uint16_t  recv_data_len   = 0;
    os_uint32_t  ping_time       = 0;
    os_int16_t   ttl             = -1;

    char ip_addr[IP_SIZE] = {0};

    if (parser == OS_NULL)
    {
        LOG_EXT_E("EC200X ping: at parser is NULL.");
        return OS_ERROR;
    }

    /* ec200s-cn ping timeout range: 1-999 */
    if ((timeout < EC200X_MIN_PING_TIME) || (timeout > EC200X_MAX_PING_TIME))
    {
        LOG_EXT_E("EC200X ping: ping timeout[%d] is out of rang[%d, %d].",
                  timeout, EC200X_MIN_PING_TIME, EC200X_MAX_PING_TIME);
        return OS_ERROR;
    }

    LOG_EXT_D("EC200X ping: %s, len: %d, timeout: %d", host, len, timeout);

    char resp_buff[256] = {0};

    /* Need to wait for 4 lines response msg */
    at_resp_t at_resp = {.buff      = resp_buff,
                        .buff_size = sizeof(resp_buff),
                        .line_num  = 4,
                        .timeout   = 16 * OS_TICK_PER_SECOND};

    /* REF: EC200S QPING */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+QPING=1,%s,%d,1", host, timeout / OS_TICK_PER_SECOND) < 0)
    {
        LOG_EXT_E("Ping: AT cmd exec fail: AT+QPING=1,%s,%d,1", host, timeout / OS_TICK_PER_SECOND);
        result = OS_ERROR;
        goto __exit;
    }

    /* Received the ping response from the server */
    at_resp_get_data_by_kw(&at_resp, "+QPING:", "+QPING:%d", &response);
    if (response == 0)
    {
        if (at_resp_get_data_by_kw(&at_resp,
                                   "+QPING:",
                                   "+QPING:%d,\"%[^\"]\",%d,%d,%d",
                                   &response,
                                   ip_addr,
                                   &recv_data_len,
                                   &ping_time,
                                   &ttl) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }
    }

    /* prase response number */
    switch (response)
    {
    case 0:
        inet_aton(ip_addr, &(resp->ip_addr));

        resp->data_len = recv_data_len;
        resp->time     = ping_time;
        resp->ttl      = ttl;

        result = OS_EOK;
        break;

    case 569:
        result = OS_ETIMEOUT;
        break;

    default:
        result = OS_ERROR;
        break;
    }

__exit:

    return result;
}

#endif /* EC200X_USING_NETSERV_OPS */
