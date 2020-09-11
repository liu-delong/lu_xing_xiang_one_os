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
 * @file        bc28_netserv.c
 *
 * @brief       bc28 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc28_netserv.h"
#include "bc28.h"

#include <stdlib.h>
#include <string.h>

#ifdef MOLINK_USING_IP
#include <mo_ipaddr.h>
#endif /* MOLINK_USING_IP */

#define DBG_EXT_TAG "bc28.netserv"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef BC28_USING_NETSERV_OPS

#define BC28_MIN_PING_PKG_LEN (12)
#define BC28_MAX_PING_PKG_LEN (1500)

os_err_t bc28_set_netstat(mo_object_t *self, os_uint8_t stat)
{
    mo_bc28_t *bc28 = os_container_of(self, mo_bc28_t, parent);

    bc28->netstat = stat;

    return OS_EOK;
}

os_err_t bc28_get_netstat(mo_object_t *self, os_uint8_t *stat)
{
    mo_bc28_t *bc28 = os_container_of(self, mo_bc28_t, parent);

    *stat = bc28->netstat;

    return OS_EOK;
}

os_err_t bc28_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CGATT=%d", attach_stat);
}

os_err_t bc28_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
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
        bc28_set_netstat(self, MO_NET_ATTACH);
    }
    else
    {
        bc28_set_netstat(self, MO_NET_DETACH);
    }
    
    return OS_EOK;
}

os_err_t bc28_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CEREG=%d", reg_n);
}

os_err_t bc28_get_reg(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat)
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
        bc28_set_netstat(self, MO_NET_EPS_REG_OK);
    }
    else
    {
        bc28_set_netstat(self, MO_NET_EPS_REG_FAIL);
    }

    return OS_EOK;
}

os_err_t bc28_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CGACT=%d,%d", act_stat, cid);
}

os_err_t bc28_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
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
        bc28_set_netstat(self, MO_NET_ACTIVATED);
    }
    else
    {
        bc28_set_netstat(self, MO_NET_DEACTIVATED);
    }

    return OS_EOK;
}

os_err_t bc28_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    /* bc28 dosen't support ber, 99 will be set forever */
    if (at_parser_get_data_by_kw(parser, "+CSQ:", "+CSQ:%d,%d", rssi, ber) <= 0)
    {
        LOG_EXT_E("Get %s module signal quality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc28_get_radio(mo_object_t *self, radio_info_t *radio_info)
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

os_err_t bc28_get_ipaddr(mo_object_t *self, char ip[])
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

    /* Response for ex: +CGPADDR:0,100.113.120.235 */
    if (at_parser_get_data_by_kw(parser, "+CGPADDR:", "+CGPADDR:%d,%[^,]", &ucid, ipaddr) <= 0)
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
        bc28_set_netstat(self, MO_NET_NETWORK_REG_OK);
    }
    else
    {
        bc28_set_netstat(self, MO_NET_NETWORK_REG_FAIL);
    }
    
    at_parser_reset_resp(parser);

    return result;
}

os_err_t bc28_set_dnsserver(mo_object_t *self, dns_server_t dns)
{
    /* BC28 must set usable dns server befor QDNS(gethostbyname), 
       otherwise both func will not be reachable before reboot! */
    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;

    if (OS_NULL == parser)
    {
        LOG_EXT_E("BC28 %s: at parser is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    /* it appears that module will reset dns if secondary dns is "0" */
    if (0 == strlen(dns.primary_dns) || 0 == strcmp(dns.secondary_dns, "0"))
    {
        LOG_EXT_E("BC28 %s: with invalid param.", __FUNCTION__);
        return OS_ERROR;
    }

    if (0 == strlen(dns.secondary_dns))
    {
        result = at_parser_exec_cmd(parser, "AT+QIDNSCFG=\"%s\"", dns.primary_dns);
    }
    else
    {
        result = at_parser_exec_cmd(parser, "AT+QIDNSCFG=\"%s\",\"%s\"", dns.primary_dns, dns.secondary_dns);
    }
    
    return result;
}

os_err_t bc28_get_dnsserver(mo_object_t *self, dns_server_t *dns)
{
    at_parser_t *parser         = &self->parser;
    os_err_t     result         = OS_EOK;

    char primary_dns[IP_SIZE]   = {0};
    char secondary_dns[IP_SIZE] = {0};

    if (OS_NULL == parser)
    {
        LOG_EXT_E("BC28 %s: at parser is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    if (OS_NULL == dns)
    {
        LOG_EXT_E("BC28 %s: dns is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    at_parser_set_resp(parser, 256, 4, 16000);

    result = at_parser_exec_cmd(parser, "AT+QIDNSCFG?");

    /* return eg. PrimaryDns: 218.4.4.4\r\n SecondaryDns: 208.67.222.222 */
    if (at_parser_get_data_by_kw(parser, "PrimaryDns:", "PrimaryDns: %s", primary_dns) <= 0)
    {
        LOG_EXT_E("BC28 %s: get primary dns failed.", __FUNCTION__);
        result = OS_ERROR;
        goto __exit;
    }
    
    if (at_parser_get_data_by_kw(parser, "PrimaryDns:", "PrimaryDns: %s", secondary_dns) <= 0)
    {
        LOG_EXT_E("BC28 %s: get primary dns failed.", __FUNCTION__);
        result = OS_ERROR;
        goto __exit;
    }

    strcpy(dns->primary_dns, primary_dns);
    strcpy(dns->secondary_dns, secondary_dns);

    LOG_EXT_D("BC28 %s: primary_dns[%s],secondary_dns[%s]", __FUNCTION__, dns.primary_dns, dns.secondary_dns);

__exit:
    return result;
}

os_err_t bc28_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp)
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
        LOG_EXT_E("BC28 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < BC28_MIN_PING_PKG_LEN) || (len > BC28_MAX_PING_PKG_LEN))
    {
        LOG_EXT_E("BC28 ping: ping package len[%d] is out of rang[8, 1460].", len);
        return OS_ERROR;
    }

    bc28_get_netstat(self, &stat);
    if (stat != MO_NET_NETWORK_REG_OK)
    {
        LOG_EXT_E("BC28 ping: network isn't registered OK yet, please register OK before ping.");
        return OS_ERROR;
    }

    LOG_EXT_D("BC28 ping: %s, len: %d, timeout: %d", host, len, timeout);
    /* Need to wait for 4 lines response msg */
    at_parser_set_resp(parser, 256, 4, 16000);

    /* App config is ignored and tools default set timeout to 10000ms */
    /* Exec commond "AT+NPING=183.232.231.174,64,5000 and wait response */
    /* Return: success: +NPING:183.232.231.174,54,1974  fail: +NPINGERR:1 */
    /* module only support IPV4, BC28 only support Dotted Dec/Hex/Oct Notation */
    if (at_parser_exec_cmd(parser, "AT+NPING=%s,%d,%d", host, len, timeout) < 0)
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

        LOG_EXT_E("BC28 ping %s fail: %s, check network status and try to set a longer timeout.", host, ret_buff);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("BC28 ping: resp prase ip[%s], req_time[%d], ttl[%d]", ipaddr, req_time, ttl);
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

#endif /* BC28_USING_NETSERV_OPS */
