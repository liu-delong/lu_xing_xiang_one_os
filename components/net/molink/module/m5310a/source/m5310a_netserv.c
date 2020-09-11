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
 * @file        m5310a_netserv.c
 *
 * @brief       m5310-a module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5310a_netserv.h"
#include "m5310a.h"

#include <stdlib.h>
#include <string.h>

#ifdef MOLINK_USING_IP
#include <mo_ipaddr.h>
#endif /* MOLINK_USING_IP */

#define DBG_EXT_TAG "m5310a.netserv"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef M5310A_USING_NETSERV_OPS

#define M5310A_MIN_PING_PKG_LEN (8)
#define M5310A_MAX_PING_PKG_LEN (1460)
#define M5310A_RESOLVE_RETRY    (3)

os_err_t m5310a_set_netstat(mo_object_t *self, os_uint8_t stat)
{
    mo_m5310a_t *m5310 = os_container_of(self, mo_m5310a_t, parent);

    m5310->netstat = stat;

    return OS_EOK;
}

os_err_t m5310a_get_netstat(mo_object_t *self, os_uint8_t *stat)
{
    mo_m5310a_t *m5310 = os_container_of(self, mo_m5310a_t, parent);

    *stat = m5310->netstat;

    return OS_EOK;
}

os_err_t m5310a_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CGATT=%d", attach_stat);
}

os_err_t m5310a_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(at_parser_get_data_by_kw(parser, "+CGATT:", "+CGATT:%d", attach_stat) <= 0)
    {
        LOG_EXT_E("Get %s module attach state failed", self->name);
        return OS_ERROR;
    }

    if (*attach_stat == ATTACHED)
    {
        m5310a_set_netstat(self, MO_NET_ATTACH);
    }
    else
    {
        m5310a_set_netstat(self, MO_NET_DETACH);
    }
    
    return OS_EOK;
}

os_err_t m5310a_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CEREG=%d", reg_n);
}

os_err_t m5310a_get_reg(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_parser_get_data_by_kw(parser, "+CEREG:", "+CEREG:%d,%d", reg_n, reg_stat) <= 0)
    {
        LOG_EXT_E("Get %s module register state failed", self->name);
        return OS_ERROR;
    }

    if ((*reg_stat == REG_HOME_NETWORK) || (*reg_stat == REG_ROAMING))
    {
        m5310a_set_netstat(self, MO_NET_EPS_REG_OK);
    }
    else
    {
        m5310a_set_netstat(self, MO_NET_EPS_REG_FAIL);
    }

    return OS_EOK;
}

os_err_t m5310a_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CGACT=%d,%d", act_stat, cid);
}

os_err_t m5310a_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_parser_get_data_by_kw(parser, "+CGACT:", "+CGACT:%d,%d", cid, act_stat) <= 0)
    {
        LOG_EXT_E("Get %s module cgact state failed", self->name);
        return OS_ERROR;
    }

    if (*act_stat == ACTIVATED)
    {
        m5310a_set_netstat(self, MO_NET_ACTIVATED);
    }
    else
    {
        m5310a_set_netstat(self, MO_NET_DEACTIVATED);
    }

    return OS_EOK;
}

os_err_t m5310a_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_parser_get_data_by_kw(parser, "+CSQ:", "+CSQ:%d,%d", rssi, ber) <= 0)
    {
        LOG_EXT_E("Get %s module signal quality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5310a_get_radio(mo_object_t *self, radio_info_t *radio_info)
{
    at_parser_t *parser = &self->parser;

    memset(radio_info, 0, sizeof(radio_info_t));

    at_parser_set_resp(parser, 256, 0, os_tick_from_ms(1000));

    os_err_t result = at_parser_exec_cmd(parser, "AT+NUESTATS");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser, "Signal power:", "Signal power:%d", &radio_info->signal_power) <= 0)
    {
        LOG_EXT_E("Get %s module signal power failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser, "Cell ID:", "Cell ID:%s", &radio_info->cell_id) <= 0)
    {
        LOG_EXT_E("Get %s module cell id failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser, "ECL:", "ECL:%d", &radio_info->ecl) <= 0)
    {
        LOG_EXT_E("Get %s module ECL failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser, "SNR:", "SNR:%d", &radio_info->snr) <= 0)
    {
        LOG_EXT_E("Get %s module SNR failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser, "EARFCN:", "EARFCN:%d", &radio_info->earfcn) <= 0)
    {
        LOG_EXT_E("Get %s module EARFCN failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser, "RSRQ:", "RSRQ:%d", &radio_info->rsrq) <= 0)
    {
        LOG_EXT_E("Get %s module RSRQ failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

__exit:

    at_parser_reset_resp(parser);

    return result;
}

os_err_t m5310a_get_ipaddr(mo_object_t *self, char ip[])
{
    at_parser_t *parser = &self->parser;
    os_int8_t    ucid   = -1;
    os_int8_t    len    = -1;

    char ipaddr[IP_SIZE] = {0};

    at_parser_set_resp(parser, 128, 0, os_tick_from_ms(5000));
    os_err_t result = at_parser_exec_cmd(parser, "AT+CGPADDR");
    if (result != OS_EOK)
    {
        LOG_EXT_E("Get ip address fail: AT+CGPADDR cmd exec fail.");
        goto __exit;
    }

    /* Response for ex: +CGPADDR:0,10.208.88.25,2409:8962:B516:123A:1:0:4C7:E50F */
    if (at_parser_get_data_by_kw(parser, "+CGPADDR:", "+CGPADDR:%d,%[^,]", &ucid, ipaddr) <= 0)
    {
        LOG_EXT_E("Get ip address: parse resp fail.");
        result = OS_ERROR;
        goto __exit;
    }
    /* Delete the last useless byte of data --OD */
    ipaddr[strlen(ipaddr) - 1] = 0;

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
        m5310a_set_netstat(self, MO_NET_NETWORK_REG_OK);
    }
    else
    {
        m5310a_set_netstat(self, MO_NET_NETWORK_REG_FAIL);
    }
    
    at_parser_reset_resp(parser);

    return result;
}

os_err_t m5310a_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser          = &self->parser;
    os_err_t     result          = OS_EOK;
    os_int16_t   req_time        = -1;
    os_int16_t   ttl             = -1;
    os_uint8_t   stat            = 0;
    char         ipaddr[IP_SIZE] = {0};
    char         ret_buff[36]    = {0};
    
    if (parser == OS_NULL)
    {
        LOG_EXT_E("M5310-A ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < M5310A_MIN_PING_PKG_LEN) || (len > M5310A_MAX_PING_PKG_LEN))
    {
        LOG_EXT_E("M5310-A ping: ping package len[%d] is out of rang[8, 1460].", len);
        return OS_ERROR;
    }

    m5310a_get_netstat(self, &stat);
    if (stat != MO_NET_NETWORK_REG_OK)
    {
        LOG_EXT_E("M5310-A ping: network isn't registered OK yet, please register OK before ping.");
        return OS_ERROR;
    }

    LOG_EXT_D("M5310-A ping: %s, len: %d, timeout: %d", host, len, timeout);
    /* Need to wait for 4 lines response msg */
    at_parser_set_resp(parser, 256, 4, 16000);

    /* App config is ignored and tools default set timeout to 5000ms */
    /* It is found that the ping packet of M5310-A takes 4 seconds to return */
    /* Exec commond "AT+NPING=www.baidu.com,64,5000,4 and wait response */
    /* Return: success: +NPING:183.232.231.174,54,1974  fail: +NPINGERR:1 */
    if (at_parser_exec_cmd(parser, "AT+NPING=%s,%d,%d,1", host, len, timeout) < 0)
    {
        LOG_EXT_E("Ping: AT cmd exec fail: AT+NPING=%s,%d,%d", host, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser, "+NPING", "+NPING:%[^,],%d,%d", ipaddr, &ttl, &req_time) <= 0)
    {
        if (at_parser_get_data_by_kw(parser, "+", "+%s", ret_buff) <= 0)
        {
            LOG_EXT_E("AT+NPING resp prase \"+NPINGERR\" fail.");
        }

        LOG_EXT_E("M5310-A ping %s fail: %s, check network status and try to set a longer timeout.", host, ret_buff);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("M5310-A ping: resp prase ip[%s], req_time[%d], ttl[%d]", ipaddr, req_time, ttl);
        if (ttl <= 0)
        {
            result = OS_ETIMEOUT;
        }
        else
        {
            result = OS_EOK;
        }
    }

    if (req_time)
    {
        inet_aton(ipaddr, &(resp->ip_addr));
        resp->data_len = len;
        resp->ttl      = ttl;
        resp->time     = req_time;
    }

__exit:

    at_parser_reset_resp(parser);

    return result;
}

#endif /* M5310A_USING_NETSERV_OPS */
