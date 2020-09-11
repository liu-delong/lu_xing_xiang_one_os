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
 * @file        ml302_netserv.c
 *
 * @brief       ml302 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ml302_netserv.h"
#include "ml302.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef MOLINK_USING_IP
#include <mo_ipaddr.h>
#endif /* MOLINK_USING_IP */

#define DBG_EXT_TAG "ml302.netserv"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef ML302_USING_NETSERV_OPS

#define ML302_MIN_PING_PKG_LEN (36)
#define ML302_MAX_PING_PKG_LEN (1500)
#define ML302_MAX_PING_TIMEOUT (255)
#define ML302_MIN_PING_TIMEOUT (1)
#define ML302_RESOLVE_RETRY    (3)

os_err_t ml302_set_netstat(mo_object_t *self, os_uint8_t stat)
{
    mo_ml302_t *ml302 = os_container_of(self, mo_ml302_t, parent);

    ml302->netstat = stat;

    return OS_EOK;
}

os_err_t ml302_get_netstat(mo_object_t *self, os_uint8_t *stat)
{
    mo_ml302_t *ml302 = os_container_of(self, mo_ml302_t, parent);

    *stat = ml302->netstat;

    return OS_EOK;
}

os_err_t ml302_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CGATT=%d", attach_stat);
}

os_err_t ml302_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
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
        ml302_set_netstat(self, MO_NET_ATTACH);
    }
    else
    {
        ml302_set_netstat(self, MO_NET_DETACH);
    }
    
    return OS_EOK;
}

os_err_t ml302_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CEREG=%d", reg_n);
}

os_err_t ml302_get_reg(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat)
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
        ml302_set_netstat(self, MO_NET_EPS_REG_OK);
    }
    else
    {
        ml302_set_netstat(self, MO_NET_EPS_REG_FAIL);
    }

    return OS_EOK;
}

os_err_t ml302_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CGACT=%d,%d", act_stat, cid);
}

os_err_t ml302_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
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

    if ((*act_stat == REG_HOME_NETWORK) || (*act_stat == REG_ROAMING))
    {
        ml302_set_netstat(self, MO_NET_ACTIVATED);
    }
    else
    {
        ml302_set_netstat(self, MO_NET_DEACTIVATED);
    }

    return OS_EOK;
}

os_err_t ml302_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
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


/*  The function ml302_get_radio dot support in ml302 module */
/*
os_err_t ml302_get_radio(mo_object_t *self, radio_info_t *radio_info)
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
*/
os_err_t ml302_get_ipaddr(mo_object_t *self, char ip[])
{
    at_parser_t *parser = &self->parser;
    os_int8_t    ucid   = -1;
    os_int8_t    len    = -1;

    char ipaddr[IP_SIZE] = {0};

    at_parser_set_resp(parser, 256, 0, os_tick_from_ms(5000));
    os_err_t result = at_parser_exec_cmd(parser, "AT+CGPADDR");
    if (result != OS_EOK)
    {
        LOG_EXT_E("Get ip address fail: AT+CGPADDR cmd exec fail.");
        goto __exit;
    }
    /* Response for ex: +CGPADDR:0,100.113.120.235 */
    if (at_parser_get_data_by_line(parser, parser->resp.line_counts - 2, "+CGPADDR:%d,\"%[^\"]", &ucid, ipaddr) <= 0)
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
        ml302_set_netstat(self, MO_NET_NETWORK_REG_OK);
    }
    else
    {
        ml302_set_netstat(self, MO_NET_NETWORK_REG_FAIL);
    }
    
    at_parser_reset_resp(parser);

    return result;
}



os_err_t ml302_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser          = &self->parser;
    os_err_t     result          = OS_EOK;
    os_int16_t   req_time        = -1;
    os_int16_t   ttl             = -1;
    os_uint8_t   stat            = 0;
    char         ip_addr[IP_SIZE] = {0};
    char         ret_buff[36]    = {0};
    
    if (parser == OS_NULL)
    {
        LOG_EXT_E("ML302 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < ML302_MIN_PING_PKG_LEN) || (len > ML302_MAX_PING_PKG_LEN))
    {
        LOG_EXT_E("ML302 ping: ping package len[%d] is out of rang[8, 1460].", len);
        return OS_ERROR;
    }
		
	  if ((timeout < ML302_MIN_PING_TIMEOUT) || (len > ML302_MAX_PING_TIMEOUT))
    {
        LOG_EXT_E("ML302 ping: ping timeout [%d] is out of rang[8, 1460].", timeout);
        return OS_ERROR;
    }
		

    ml302_get_netstat(self, &stat);
    if (stat != MO_NET_NETWORK_REG_OK)
    {
        LOG_EXT_E("ML302 ping: network isn't registered OK yet, please register OK before ping.");
        return OS_ERROR;
    }

		LOG_EXT_D("ML302 ping: %s, len: %d, timeout: %d", host, len, timeout);
//    if (ml302_get_ipaddr(self, ip_addr) != OS_EOK)
//    {
//        LOG_EXT_E("Ping: ml302_at_domain_resolve %s failed.", host);
//        result = OS_ERROR;
//        goto __exit;
//    }

//    LOG_EXT_D("ML302 ping: %s, len: %d, timeout: %d", host, len, timeout);
    /* Need to wait for 4 lines response msg */
    at_parser_set_resp(parser, 256, 5, 16000);

	/* Send commond "AT+MPING=<domain name> or ip addr" and wait response */
	/* Exec AT+MPING="www.baidu.com",2,1,64 and return: 0 183.232.231.172: bytes = 36 time = 96(ms), TTL = 255 */

    if (at_parser_exec_cmd(parser, "AT+MPING=\"%s\",%d,1,%d", host, timeout, len) < 0)
    {
        LOG_EXT_E("Ping: AT cmd exec fail: AT+MPING=%s,%d,1,%d", host, timeout, len);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser,
                                 "TTL",
                                 "0 %[^:]: bytes = %d time = %d(ms), TTL = %d",
                                 ip_addr,
                                 &len,
                                 &req_time,
                                 &ttl) <= 0)
    {
        if (at_parser_get_data_by_kw(parser, "+", "+%s", ret_buff) <= 0)
        {
            LOG_EXT_E("AT+NPING resp prase \"+NPINGERR\" fail.");
        }

        LOG_EXT_E("ML302 ping %s fail: %s, check network status and try to set a longer timeout.", host, ret_buff);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("ML302 ping: resp prase ip[%s], req_time[%d], ttl[%d]", ipaddr, req_time, ttl);
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
        inet_aton(ip_addr, &(resp->ip_addr));
        resp->data_len = len;
        resp->ttl      = ttl;
        resp->time     = req_time;
    }

__exit:

    at_parser_reset_resp(parser);

    return result;
}




#define GET_ML302_RSSI(rxlev)   (0 - (63 - rxlev + 48))
#define ML302_MODULE_NET_TYPE   5

os_err_t ml302_get_cell_info(mo_object_t *self, onepos_cell_info_t* onepos_cell_info)
{
    os_err_t                ret = OS_EOK;
    os_size_t               i = 0;
    cell_info_t            *cell_info = OS_NULL;
    char*                   temp_buff = OS_NULL;
    os_uint32_t             cell_num = 0;
    os_uint32_t             frequency = 0;
    os_uint32_t             rsrp = 0;
    os_uint32_t             rsrq = 0;
    os_uint32_t             rxlev = 0;
    os_uint32_t             pcid = 0;
    char                    imsi[20];
    unsigned int            earfcn[10];
    os_uint32_t             roming_flag;
    os_uint32_t             bandinfo;
    char                    bandwith;
    at_parser_t *parser          = &self->parser;
    
    if (parser == OS_NULL)
    {
        LOG_EXT_E("ML302 ping: at parser is NULL.");
        return OS_ERROR;
    }   
    if(!onepos_cell_info)
    {
        LOG_EXT_E("input param is error!");
        return OS_ERROR;
    }
    at_parser_set_resp(parser, 1024, 0, os_tick_from_ms(50 * OS_TICK_PER_SECOND));
		
    /* neighbor cell */
    if (at_parser_exec_cmd(parser,"AT+CCED=0,2") < 0)
    {
        os_free(cell_info);
        ret = OS_ERROR;
        LOG_EXT_E("AT cmd exec fail: AT+CCED=0,2\n");
        goto __exit;
    }
    LOG_EXT_D("resp->line_counts : %d\r\n", parser->resp.line_counts);
    
    cell_info = calloc((parser->resp.line_counts - 4 + 1), sizeof(cell_info_t));
    if(NULL == cell_info)
    {
        LOG_EXT_E("malloc cell_info is null");
        ret = OS_ENOMEM;
        return ret;
    }
    
    LOG_EXT_I("                MCC         MNC          CID         LAC       RSSI\n");
    LOG_EXT_I("           ------------ ------------ ------------ ------------ ----\n");
    
    for (i = 2; i < parser->resp.line_counts - 2; i++)
    {
        temp_buff = (char*)at_parser_get_line(parser, i);
        if(strlen(temp_buff) > 25){
            sscanf(temp_buff,
                "+CCED:LTE neighbor cell:%u,%u,%u,%u,%u,%u,%u,%u,%u",
                &cell_info[cell_num].mcc,
                &cell_info[cell_num].mnc,
                &frequency,
                &cell_info[cell_num].cid,
                &rsrp,
                &rsrq,
                &cell_info[cell_num].lac,
                &rxlev,
                &pcid);

            cell_info[cell_num].ss = GET_ML302_RSSI(rxlev);
            if(cell_info[cell_num].mcc)
            {
                LOG_EXT_I("cell_info: %-12u %-12u %-12u %-12u %-4d\n", 
                                cell_info[cell_num].mcc, cell_info[cell_num].mnc, cell_info[cell_num].cid, 
                                cell_info[cell_num].lac, cell_info[cell_num].ss);
                cell_num ++;
            }
       }
    }
    /* main cell */
    if (at_parser_exec_cmd(parser,"AT+CCED=0,1") < 0)
    {
        ret = OS_ERROR;
        os_free(cell_info);
        LOG_EXT_E("AT cmd exec fail: AT+CCED=0,1\n");
        goto __exit;
    }
    LOG_EXT_D("resp->line_counts : %d\r\n", resp->line_counts);

    temp_buff = (char*)at_parser_get_line(parser, 2);
    if(strlen(temp_buff) > 25)
    {
        sscanf(temp_buff,
            "+CCED:LTE current cell:%u,%u,%[^,],%u,%u,%[^,],%u,%u,%u,%u,%u,%u,%u",
            &cell_info[cell_num].mcc,
            &cell_info[cell_num].mnc,
            imsi,
            &roming_flag,
            &bandinfo,
            &bandwith,
            earfcn,
            &cell_info[cell_num].cid,
            &rsrp,
            &rsrq,
            &cell_info[cell_num].lac,
            &rxlev,
            &pcid);

        cell_info[cell_num].ss = GET_ML302_RSSI(rxlev);
        LOG_EXT_I("cell_info: %-12u %-12u %-12u %-12u %-4d\n", 
                            cell_info[cell_num].mcc, cell_info[cell_num].mnc, cell_info[cell_num].cid, 
                            cell_info[cell_num].lac, cell_info[cell_num].ss);

    }
    else
    {
        cell_num --;
    }
    LOG_EXT_I("           ------------ ------------ ------------ ------------ ----\n");
                        
    onepos_cell_info->cell_num = ++cell_num;
    onepos_cell_info->cell_info = cell_info;
    onepos_cell_info->net_type = ML302_MODULE_NET_TYPE;
                            
__exit:
     at_parser_reset_resp(parser);
    return ret;
}




#endif /* ml302_USING_NETSERV_OPS */

