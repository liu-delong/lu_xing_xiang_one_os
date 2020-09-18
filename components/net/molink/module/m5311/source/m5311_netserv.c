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
 * @file        m5311_netserv.c
 *
 * @brief       m5311 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5311_netserv.h"
#include "m5311.h"

#include <stdlib.h>
#include <string.h>

#ifdef MOLINK_USING_IP
#include <mo_ipaddr.h>
#endif /* MOLINK_USING_IP */

#define DBG_EXT_TAG "m5311.netserv"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef M5311_USING_NETSERV_OPS

#define M5311_MIN_PING_PKG_LEN (8)
#define M5311_MAX_PING_PKG_LEN (1400)

os_err_t m5311_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CGATT=%d", attach_stat);
}

os_err_t m5311_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+CGATT:", "+CGATT:%d", attach_stat) <= 0)
    {
        LOG_EXT_E("Get %s module attach state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%d", reg_n);
}

os_err_t m5311_get_reg(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG:%d,%d", reg_n, reg_stat) <= 0)
    {
        LOG_EXT_E("Get %s module register state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CGACT=%d,%d", act_stat, cid);
}

os_err_t m5311_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGACT:", "+CGACT:%d,%d", cid, act_stat) <= 0)
    {
        LOG_EXT_E("Get %s module cgact state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CSQ:", "+CSQ:%d,%d", rssi, ber) <= 0)
    {
        LOG_EXT_E("Get %s module signal quality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_get_radio(mo_object_t *self, radio_info_t *radio_info)
{
#define M5311_NETSTAT_REGEX "*ENGINFOSC: %hd,%*[^\"]\"%[^\"]\"\n,%*d,%d,%*d,%hd,%*d,%*[^,],%hhu,"

    at_parser_t *parser = &self->parser;

    memset(radio_info, 0, sizeof(radio_info_t));
    
    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT*ENGINFO=0");
    if (result != OS_EOK)
    {
        goto __exit;
    }
    
    /* return eg: *ENGINFOSC: 3684,2,131,"85B0F15"\n,-107,-13,-93,-3,8,"5A21",0, */
    /* <sc_earfcn>,<sc_earfcn_offset>,<sc_pci>,<sc_cellid>,
    [<sc_rsrp>],[<sc_rsrq>],[<sc_rssi>],[<sc_snr>],<sc_band>,<sc_tac>,[<sc_ecl>],[<sc_tx_pwr>] */
    if (at_resp_get_data_by_kw(&resp, "*ENGINFOSC:", M5311_NETSTAT_REGEX, 
                                                        &radio_info->earfcn,
                                                        &radio_info->cell_id, 
                                                        &radio_info->rsrq, 
                                                        &radio_info->snr,
                                                        &radio_info->ecl) <= 0)
    {
        LOG_EXT_E("Get %s module radio_info failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

__exit:

    return result;
}

os_err_t m5311_get_ipaddr(mo_object_t *self, char ip[])
{
    at_parser_t *parser = &self->parser;
    os_int8_t    ucid   = -1;
    os_int8_t    len    = -1;

    char ipaddr[IP_SIZE] = {0};

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGPADDR=");
    if (result != OS_EOK)
    {
        LOG_EXT_E("Get ip address fail: AT+CGPADDR cmd exec fail.");
        goto __exit;
    }

    /* Response for ex: +CGPADDR:0,100.113.120.235 */
    if (at_resp_get_data_by_kw(&resp, "+CGPADDR:", "+CGPADDR:%d,\"%[^\"]", &ucid, ipaddr) <= 0)
    {
        LOG_EXT_E("Get ip address: parse resp fail.");
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(ipaddr);
    if ((len < MIN_IP_SIZE) || (len > IP_SIZE))
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

os_err_t m5311_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser          = &self->parser;
    os_err_t     result          = OS_EOK;
    os_int16_t   req_time        = -1;
    os_int16_t   ttl             = -1;
    char         ipaddr[IP_SIZE] = {0};
    char         ret_buff[36]    = {0};
    
    if (parser == OS_NULL)
    {
        LOG_EXT_E("M5311 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < M5311_MIN_PING_PKG_LEN) || (len > M5311_MAX_PING_PKG_LEN))
    {
        LOG_EXT_E("M5311 ping: ping package len[%d] is out of rang[%d, %d].",
                   len, M5311_MIN_PING_PKG_LEN, M5311_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    LOG_EXT_D("M5311 ping: %s, len: %d, timeout: %d", host, len, timeout);
    /* Need to wait for 4 lines response msg */
    char resp_buff[256] = {0};
    at_resp_t at_resp = {.buff      = resp_buff,
                        .buff_size  = sizeof(resp_buff),
                        .line_num   = 4,
                        .timeout    = 16 * OS_TICK_PER_SECOND};

    /* Exec commond "AT+PING="www.baidu.com",32,2000,1 and wait response */
    /* Return: success: +PING: 39.156.66.14,49,1206  fail: +PINGERR: 1 */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+PING=%s,%d,%d,1", host, len, timeout) < 0)
    {
        LOG_EXT_E("Ping: AT cmd exec fail: AT+PING=%s,%d,%d", host, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+PING", "+PING: %[^,],%d,%d", ipaddr, &ttl, &req_time) <= 0)
    {
        if (at_resp_get_data_by_kw(&at_resp, "+", "+%s", ret_buff) <= 0)
        {
            LOG_EXT_E("AT+PING resp prase \"+PINGERR\" fail.");
        }

        LOG_EXT_E("M5311 ping %s fail: %s, check network status and try to set a longer timeout.", host, ret_buff);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("M5311 ping: resp prase ip[%s], req_time[%d], ttl[%d]", ipaddr, req_time, ttl);
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
        LOG_EXT_D("M5311 ping return ip: %s", ipaddr);
        inet_aton(ipaddr, &(resp->ip_addr));
        resp->data_len = len;
        resp->ttl      = ttl;
        resp->time     = req_time;
    }

__exit:

    return result;
}

#endif /* M5311_USING_NETSERV_OPS */
