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
 * @file        bc95_netserv.c
 *
 * @brief       bc95 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc95_netserv.h"
#include "bc95.h"

#include <stdlib.h>
#include <string.h>

#ifdef MOLINK_USING_IP
#include <mo_ipaddr.h>
#endif /* MOLINK_USING_IP */

#define DBG_EXT_TAG "bc95.netserv"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef BC95_USING_NETSERV_OPS

#define BC95_MIN_PING_PKG_LEN (12)
#define BC95_MAX_PING_PKG_LEN (1500)

os_err_t bc95_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CGATT=%d", attach_stat);
}

os_err_t bc95_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
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

os_err_t bc95_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%d", reg_n);
}

os_err_t bc95_get_reg(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat)
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

os_err_t bc95_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CGACT=%d,%d", act_stat, cid);
}

os_err_t bc95_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
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

os_err_t bc95_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    /* bc95 dosen't support ber, 99 will be set forever */
    if (at_resp_get_data_by_kw(&resp, "+CSQ:", "+CSQ:%d,%d", rssi, ber) <= 0)
    {
        LOG_EXT_E("Get %s module signal quality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc95_get_radio(mo_object_t *self, radio_info_t *radio_info)
{
    at_parser_t *parser = &self->parser;

    memset(radio_info, 0, sizeof(radio_info_t));

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 1 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+NUESTATS");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "Signal power:", "Signal power:%d", &radio_info->signal_power) <= 0)
    {
        LOG_EXT_E("Get %s module signal power failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "Cell ID:", "Cell ID:%s", &radio_info->cell_id) <= 0)
    {
        LOG_EXT_E("Get %s module cell id failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "ECL:", "ECL:%d", &radio_info->ecl) <= 0)
    {
        LOG_EXT_E("Get %s module ECL failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "SNR:", "SNR:%d", &radio_info->snr) <= 0)
    {
        LOG_EXT_E("Get %s module SNR failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "EARFCN:", "EARFCN:%d", &radio_info->earfcn) <= 0)
    {
        LOG_EXT_E("Get %s module EARFCN failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "RSRQ:", "RSRQ:%d", &radio_info->rsrq) <= 0)
    {
        LOG_EXT_E("Get %s module RSRQ failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

__exit:

    return result;
}

os_err_t bc95_get_ipaddr(mo_object_t *self, char ip[])
{
    at_parser_t *parser = &self->parser;
    os_int8_t    ucid   = -1;
    os_int8_t    len    = -1;

    char ipaddr[IP_SIZE] = {0};

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGPADDR");
    if (result != OS_EOK)
    {
        LOG_EXT_E("Get ip address fail: AT+CGPADDR cmd exec fail.");
        goto __exit;
    }

    /* Response for ex: +CGPADDR:0,100.113.120.235 */
    if (at_resp_get_data_by_kw(&resp, "+CGPADDR:", "+CGPADDR:%d,%[^,]", &ucid, ipaddr) <= 0)
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

os_err_t bc95_set_dnsserver(mo_object_t *self, dns_server_t dns)
{
    /* BC95 must set usable dns server befor QDNS(gethostbyname), 
       otherwise both func will not be reachable before reboot! */
    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    if (OS_NULL == parser)
    {
        LOG_EXT_E("BC95 %s: at parser is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    /* it appears that module will reset dns if secondary dns is "0" */
    if (0 == strlen(dns.primary_dns) || 0 == strcmp(dns.secondary_dns, "0"))
    {
        LOG_EXT_E("BC95 %s: with invalid param.", __FUNCTION__);
        return OS_ERROR;
    }

    if (0 == strlen(dns.secondary_dns))
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+QIDNSCFG=\"%s\"", dns.primary_dns);
    }
    else
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+QIDNSCFG=\"%s\",\"%s\"", dns.primary_dns, dns.secondary_dns);
    }
    
    return result;
}

os_err_t bc95_get_dnsserver(mo_object_t *self, dns_server_t *dns)
{
    at_parser_t *parser         = &self->parser;
    os_err_t     result         = OS_EOK;

    char primary_dns[IP_SIZE]   = {0};
    char secondary_dns[IP_SIZE] = {0};

    if (OS_NULL == parser)
    {
        LOG_EXT_E("BC95 %s: at parser is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    if (OS_NULL == dns)
    {
        LOG_EXT_E("BC95 %s: dns is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    char resp_buff[256] = {0};

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 4,
                         .timeout   = 16 * OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &at_resp, "AT+QIDNSCFG?");

    /* return eg. PrimaryDns: 218.4.4.4\r\n SecondaryDns: 208.67.222.222 */
    if (at_resp_get_data_by_kw(&at_resp, "PrimaryDns:", "PrimaryDns: %s", primary_dns) <= 0)
    {
        LOG_EXT_E("BC95 %s: get primary dns failed.", __FUNCTION__);
        result = OS_ERROR;
        goto __exit;
    }
    
    if (at_resp_get_data_by_kw(&at_resp, "PrimaryDns:", "PrimaryDns: %s", secondary_dns) <= 0)
    {
        LOG_EXT_E("BC95 %s: get primary dns failed.", __FUNCTION__);
        result = OS_ERROR;
        goto __exit;
    }

    strcpy(dns->primary_dns, primary_dns);
    strcpy(dns->secondary_dns, secondary_dns);

    LOG_EXT_D("BC95 %s: primary_dns[%s],secondary_dns[%s]", __FUNCTION__, dns.primary_dns, dns.secondary_dns);

__exit:
    return result;
}

os_err_t bc95_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser          = &self->parser;
    os_err_t     result          = OS_EOK;
    os_int16_t   req_time        = -1;
    os_int16_t   ttl             = -1;
    char         ipaddr[IP_SIZE] = {0};
    char         ret_buff[36]    = {0};
    
    if (parser == OS_NULL)
    {
        LOG_EXT_E("BC95 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < BC95_MIN_PING_PKG_LEN) || (len > BC95_MAX_PING_PKG_LEN))
    {
        LOG_EXT_E("BC95 ping: ping package len[%d] is out of rang[8, 1460].", len);
        return OS_ERROR;
    }

    LOG_EXT_D("BC95 ping: %s, len: %d, timeout: %d", host, len, timeout);
    /* Need to wait for 4 lines response msg */
    char resp_buff[256] = {0};

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 4,
                         .timeout   = 16 * OS_TICK_PER_SECOND};

    /* App config is ignored and tools default set timeout to 10000ms */
    /* Exec commond "AT+NPING=183.232.231.174,64,5000 and wait response */
    /* Return: success: +NPING:183.232.231.174,54,1974  fail: +NPINGERR:1 */
    /* module only support IPV4, BC95 only support Dotted Dec/Hex/Oct Notation */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+NPING=%s,%d,%d", host, len, timeout) < 0)
    {
        LOG_EXT_E("Ping: AT cmd exec fail: AT+NPING=%s,%d,%d", host, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+NPING", "+NPING:%[^,],%d,%d", ipaddr, &ttl, &req_time) <= 0)
    {
        if (at_resp_get_data_by_kw(&at_resp, "+", "+%s", ret_buff) <= 0)
        {
            LOG_EXT_E("AT+NPING resp prase \"+NPINGERR\" fail.");
        }

        LOG_EXT_E("BC95 ping %s fail: %s, check network status and try to set a longer timeout.", host, ret_buff);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("BC95 ping: resp prase ip[%s], req_time[%d], ttl[%d]", ipaddr, req_time, ttl);
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

    return result;
}

#endif /* BC95_USING_NETSERV_OPS */
