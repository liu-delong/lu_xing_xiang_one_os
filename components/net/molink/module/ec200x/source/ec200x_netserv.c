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

os_err_t ec200x_set_netstat(mo_object_t *self, os_uint8_t stat)
{
    mo_ec200x_t *m5310 = os_container_of(self, mo_ec200x_t, parent);

    m5310->netstat = stat;

    return OS_EOK;
}

os_err_t ec200x_get_netstat(mo_object_t *self, os_uint8_t *stat)
{
    mo_ec200x_t *m5310 = os_container_of(self, mo_ec200x_t, parent);

    *stat = m5310->netstat;

    return OS_EOK;
}

os_err_t ec200x_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CGATT=%d", attach_stat);
}

os_err_t ec200x_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(at_parser_get_data_by_kw(parser, "+CGATT:", "+CGATT: %d", attach_stat) <= 0)
    {
        LOG_EXT_E("Get %s module attach state failed", self->name);
        return OS_ERROR;
    }

    if (*attach_stat == ATTACHED)
    {
        ec200x_set_netstat(self, MO_NET_ATTACH);
    }
    else
    {
        ec200x_set_netstat(self, MO_NET_DETACH);
    }
    
    return OS_EOK;
}

os_err_t ec200x_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CEREG=%d", reg_n);
}

os_err_t ec200x_get_reg(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_parser_get_data_by_kw(parser, "+CEREG:", "+CEREG: %d,%d", reg_n, reg_stat) <= 0)
    {
        LOG_EXT_E("Get %s module register state failed", self->name);
        return OS_ERROR;
    }

    if ((*reg_stat == REG_HOME_NETWORK) || (*reg_stat == REG_ROAMING))
    {
        ec200x_set_netstat(self, MO_NET_EPS_REG_OK);
    }
    else
    {
        ec200x_set_netstat(self, MO_NET_EPS_REG_FAIL);
    }

    return OS_EOK;
}

os_err_t ec200x_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CGACT=%d,%d", act_stat, cid);
}

os_err_t ec200x_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_parser_get_data_by_kw(parser, "+CGACT:", "+CGACT: %d,%d", cid, act_stat) <= 0)
    {
        LOG_EXT_E("Get %s module cgact state failed", self->name);
        return OS_ERROR;
    }

    if (*act_stat == ACTIVATED)
    {
        ec200x_set_netstat(self, MO_NET_ACTIVATED);
    }
    else
    {
        ec200x_set_netstat(self, MO_NET_DEACTIVATED);
    }

    return OS_EOK;
}

os_err_t ec200x_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_parser_get_data_by_kw(parser, "+CSQ:", "+CSQ: %d,%d", rssi, ber) <= 0)
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

    at_parser_set_resp(parser, 128, 0, os_tick_from_ms(5000));
    os_err_t result = at_parser_exec_cmd(parser, "AT+CGPADDR=1");
    if (result != OS_EOK)
    {
        LOG_EXT_E("Get ip address fail: AT+CGPADDR cmd exec fail.");
        goto __exit;
    }

    /* Response for ex: +CGPADDR:0,100.113.120.235 */
    if (at_parser_get_data_by_kw(parser, "+CGPADDR:", "+CGPADDR:%*[^\"]\"%[^\"]", ipaddr) <= 0)
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
    
    if (result == OS_EOK)
    {
        ec200x_set_netstat(self, MO_NET_NETWORK_REG_OK);
    }
    else
    {
        ec200x_set_netstat(self, MO_NET_NETWORK_REG_FAIL);
    }
    
    at_parser_reset_resp(parser);

    return result;
}

os_err_t ec200x_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser          = &self->parser;
    os_err_t     result          = OS_EOK;
    os_uint8_t   stat            = 0;
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

    ec200x_get_netstat(self, &stat);
    if (stat != MO_NET_NETWORK_REG_OK)
    {
        LOG_EXT_E("EC200X ping: network isn't registered OK yet, please register OK before ping.");
        return OS_ERROR;
    }

    LOG_EXT_D("EC200X ping: %s, len: %d, timeout: %d", host, len, timeout);
    
    /* Need to wait for 4 lines response msg */
    at_parser_set_resp(parser, 256, 4, 16000);

    /* REF: EC200S QPING */
    if (at_parser_exec_cmd(parser, "AT+QPING=1,%s,%d,1", host, timeout / OS_TICK_PER_SECOND) < 0)
    {
        LOG_EXT_E("Ping: AT cmd exec fail: AT+QPING=1,%s,%d,1", host, timeout / OS_TICK_PER_SECOND);
        result = OS_ERROR;
        goto __exit;
    }

    /* Received the ping response from the server */
    at_parser_get_data_by_kw(parser, "+QPING:", "+QPING:%d", &response);
    if (response == 0)
    {
        if (at_parser_get_data_by_kw(parser,
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
    
    at_parser_reset_resp(parser);

    return result;
}

#endif /* EC200X_USING_NETSERV_OPS */
