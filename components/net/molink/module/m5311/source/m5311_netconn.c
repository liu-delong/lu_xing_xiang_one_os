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
 * @file        m5311_netconn.c
 *
 * @brief       m5311 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5311_netconn.h"
#include "m5311.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "mo_lib.h"

#define DBG_EXT_TAG "m5311.netconn"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifndef AT_ECHO_MODE
#define AT_ECHO_MODE       OS_FALSE
#endif

#define M5311_TCP_SEND_MAX_SIZE (720)
#define M5311_UDP_SEND_MAX_SIZE (712)

#ifndef M5311_DATA_QUEUE_SIZE
#define M5311_DATA_QUEUE_SIZE   (5)
#endif

#ifdef M5311_USING_NETCONN_OPS

static os_err_t m5311_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_IPC_WAITING_FOREVER);
}

static os_err_t m5311_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *m5311_netconn_alloc(mo_object_t *module)
{
    mo_m5311_t *m5311 = os_container_of(module, mo_m5311_t, parent);
    os_int32_t current_connect_id = -1;
    for (int i = 0; i < M5311_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == m5311->netconn[i].stat)
        {
            /* reset netconn prevent reuse content */
            current_connect_id = m5311->netconn[i].connect_id;
            memset(&m5311->netconn[i], 0, sizeof(mo_netconn_t));
            m5311->netconn[i].connect_id = current_connect_id;
            LOG_EXT_I("Moduel %s NO[%d]:connect_id[%d]!", module->name, i, m5311->netconn[i].connect_id);
            
            return &m5311->netconn[i];
        }
    }

    LOG_EXT_E("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *m5311_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    mo_m5311_t *m5311 = os_container_of(module, mo_m5311_t, parent);

    for (int i = 0; i < M5311_NETCONN_NUM; i++)
    {
        if (connect_id == m5311->netconn[i].connect_id)
        {
            return &m5311->netconn[i];
        }
    }
    LOG_EXT_I("Moduel %s netconn all connect_id was occupied.", module->name);
    return OS_NULL;
}

os_err_t m5311_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_m5311_t *m5311 = os_container_of(module, mo_m5311_t, parent);

    info->netconn_array = m5311->netconn;
    info->netconn_nums  = sizeof(m5311->netconn) / sizeof(m5311->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *m5311_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_m5311_t   *m5311 = os_container_of(module, mo_m5311_t, parent);
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    m5311_lock(&m5311->netconn_lock);

    mo_netconn_t *netconn = m5311_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        m5311_unlock(&m5311->netconn_lock);
        return OS_NULL;
    }

    if (type != NETCONN_TYPE_TCP && type != NETCONN_TYPE_UDP)
    {
        m5311_unlock(&m5311->netconn_lock);
        return OS_NULL;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    result = at_parser_exec_cmd(parser, &resp, "AT+IPRCFG=1,0,1");
    
    if (OS_EOK != result)
    {
        LOG_EXT_E("Module %s set netconn autorcv data HEX format failed", module->name);
        m5311_unlock(&m5311->netconn_lock);
        return OS_NULL;
    }

    os_data_queue_init(&netconn->data_queue, M5311_DATA_QUEUE_SIZE, 0, OS_NULL);

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    m5311_unlock(&m5311->netconn_lock);
    return netconn;
}

os_err_t m5311_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;

    LOG_EXT_I("Module %s in %d netconnn status", module->name, netconn->stat);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    switch (netconn->stat)
    {
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+IPCLOSE=%d", netconn->connect_id);
        if (result != OS_EOK)
        {
            LOG_EXT_E("Module %s destroy %s netconn failed",
                       module->name,
                      (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
            return result;
        }
        break;
    default:        
        /* add handler when we need */
        break;
    }

    if (netconn->stat != NETCONN_STAT_NULL)
    {
        mo_netconn_data_queue_deinit(&netconn->data_queue);
    }

    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);
    
    LOG_EXT_I("Module %s netconnn_id:%d destroyed", module->name, netconn->connect_id);
    
    return OS_EOK;
}

os_err_t m5311_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    at_parser_t *parser = &self->parser;

	char recvip[IPADDR_MAX_STR_LEN + 1] = {0};
	
    char resp_buff[256] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 4,
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CMDNS=\"%s\"", domain_name);
    if (result < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    /* AT+CMDNS="www.baidu.com" return: OK \r\n  +CMDNS:183.232.231.172 \r\n */
    /* AT+CMDNS="8.8.8.8" return: +CMDNS:8.8.8.8 \r\n  OK */
    if (at_resp_get_data_by_kw(&resp, "+CMDNS:", "+CMDNS:%s", recvip) <= 0)
    {
        LOG_EXT_E("M5311 domain resolve: resp parse fail, try again, host: %s", domain_name);
        result = OS_ERROR;
        /* If resolve failed, maybe receive an URC CRLF */
        goto __exit;
    }

    if (strlen(recvip) < IPADDR_MIN_STR_LEN)
    {
        LOG_EXT_E("M5311 domain resolve: recvip len < IPADDR_MIN_STR_LEN, len = %d", strlen(recvip));
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("M5311 domain resolve: \"%s\" domain ip is %s, addrlen %d", domain_name, recvip, strlen(recvip));
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

static os_err_t m5311_tcp_connect(at_parser_t *parser, os_int32_t connect_id, char *ip_addr, os_uint16_t port)
{
	char buf[16]        = {0};
	char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 4,
                      .timeout   = 41 * OS_TICK_PER_SECOND}; /* about 40s return failed */

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+IPSTART=%d,\"TCP\",%s,%hu", connect_id, ip_addr, port);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "CONNECT", "CONNECT %s", buf) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    if (strcmp(buf, "OK"))
    {
        LOG_EXT_I("Module connect[%d]:%s!", connect_id, buf);
        result = OS_ERROR;
        goto __exit;
    }

__exit:

    return result;
}

static os_err_t m5311_udp_connect(at_parser_t *parser, os_int32_t connect_id, char *ip_addr, os_uint16_t port)
{
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};
    
    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+IPSTART=%d,\"UDP\",%s,%hu", connect_id, ip_addr, port);

    return result;
}

os_err_t m5311_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = m5311_tcp_connect(parser, netconn->connect_id, remote_ip, port);
        break;
    case NETCONN_TYPE_UDP:
        result = m5311_udp_connect(parser, netconn->connect_id, remote_ip, port);
        break;
    default:
        result = OS_ERROR;
        break;
    }

    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s connect to %s:%hu failed!", module->name, remote_ip, port);
        return result;
    }

    ip_addr_copy(netconn->remote_ip, addr);
    netconn->remote_port = port;
    netconn->stat        = NETCONN_STAT_CONNECT;

    LOG_EXT_D("Module %s connect to %s:%hu successfully!", module->name, remote_ip, port);

    return OS_EOK;
}

static os_size_t m5311_hexdata_send(at_parser_t *parser, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    os_err_t   result        = OS_EOK;
    os_size_t  sent_size     = 0;
    os_size_t  cur_pkt_size  = 0;
    os_int32_t connect_id    = -1;
    os_size_t  cnt           = 0; 
    char prefix_send_cmd[30] = {0};
    char suffix_send_cmd[30] = {0};
    char remote_ip[IPADDR_MAX_STR_LEN + 1]  = {0};

    /* M5311 UDP send size <= 712, different from the spec size 720 */
    const os_int32_t send_max_size = netconn->type == NETCONN_TYPE_TCP ? M5311_TCP_SEND_MAX_SIZE : M5311_UDP_SEND_MAX_SIZE;
    
    char *resp_buff = calloc(1, send_max_size + 60);
    if (OS_NULL == resp_buff)
    {
        fprintf(stdout, "No enough memory!\n");
        LOG_EXT_E("Module %s %s no enough memory!", parser->name, __FUNCTION__);
        return 0;
    }

    at_resp_t resp = {.buff = resp_buff, .buff_size = send_max_size + 60, .timeout =  10 * OS_TICK_PER_SECOND};

    strncpy(remote_ip, inet_ntoa(netconn->remote_ip), IPADDR_MAX_STR_LEN);

    while (sent_size < size)
    {
        if (size - sent_size < send_max_size * 2)
        {
            cur_pkt_size = size - sent_size;
        }
        else
        {
            cur_pkt_size = send_max_size * 2;
        }

        snprintf(prefix_send_cmd, sizeof(prefix_send_cmd), "AT+IPSEND=%d,%d,", netconn->connect_id, (int)cur_pkt_size / 2);
        snprintf(suffix_send_cmd, sizeof(suffix_send_cmd), ",%s,%hu", remote_ip, netconn->remote_port);
        /* send cmd */
        at_parser_exec_lock(parser);
        
        if (at_parser_send(parser, prefix_send_cmd, strlen(prefix_send_cmd)) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        /* send data */
        if (at_parser_send(parser, data + sent_size, cur_pkt_size) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        /* UDP needs tail for addr&port */
        if (netconn->type == NETCONN_TYPE_UDP)
        {
            if (at_parser_send(parser, suffix_send_cmd, strlen(suffix_send_cmd)) <= 0)
            {
                result = OS_ERROR;
                goto __exit;
            }
        }
            
        result = at_parser_exec_cmd(parser, &resp, "");
        if (result != OS_EOK)
        {
            goto __exit;
        }

        if (at_resp_get_data_by_kw(&resp, "+IPSEND:", "+IPSEND: %d,%d", &connect_id, &cnt) <= 0 || cnt != cur_pkt_size / 2)
        {
            result = OS_ERROR;
            goto __exit;
        }
        
        at_parser_exec_unlock(parser);

        sent_size += cur_pkt_size;
    }

__exit:
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn %d send %d bytes data failed!",
                  parser->name,
                  netconn->connect_id,
                  cur_pkt_size / 2);
                  
        at_parser_exec_unlock(parser);
    }
    

    return sent_size / 2;
}

os_size_t m5311_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_size_t    sent_size = 0;
    os_err_t     result    = OS_EOK;
    mo_m5311_t  *m5311     = os_container_of(module, mo_m5311_t, parent);

    char *hexstr = calloc(1, size * 2 + 1);
    if (OS_NULL == hexstr)
    {
        LOG_EXT_E("Moudle %s netconn %d calloc %d bytes memory failed!",
                  module->name,
                  netconn->connect_id,
                  size * 2 + 1);
        return sent_size;
    }

    if (OS_EOK != m5311_lock(&m5311->netconn_lock))
    {
        LOG_EXT_E("Moudle %s netconn %d send lock failed.", module->name, netconn->connect_id);
        free(hexstr);
        return sent_size;
    }
    
    bytes_to_hexstr(data, hexstr, size);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        sent_size = m5311_hexdata_send(parser, netconn, hexstr, strlen(hexstr));
        break;
    case NETCONN_TYPE_UDP:
		sent_size = m5311_hexdata_send(parser, netconn, hexstr, strlen(hexstr));
        break;
    default:
        break;
    }

    result = m5311_unlock(&m5311->netconn_lock);
    if (OS_EOK != result)
    {
        LOG_EXT_E("Moudle %s netconn %d send unlock failed.", module->name, netconn->connect_id);
    }

    free(hexstr);

    return sent_size;
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = -1;

    sscanf(data, "+IPCLOSE: %d", &connect_id);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_netconn_t *netconn = m5311_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        LOG_EXT_E("Module %s get netconn error, when receive urc close code of conn_id:%d", module->name, connect_id);
        return;
    }

    LOG_EXT_W("Module %s receive close urc data of connect %d", module->name, connect_id);

    mo_netconn_pasv_close_notice(netconn);

    return;
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = -1;
    os_int32_t data_size  = 0;

    sscanf(data, "+IPRD: %d,%d,", &connect_id, &data_size);
    LOG_EXT_I("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = m5311_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        LOG_EXT_E("Module %s request receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    /*  bufflen >= strsize + 1 */
    char *recv_buff = calloc(1, data_size * 2 + 1);
    if (recv_buff == OS_NULL)
    {
        LOG_EXT_E("Calloc recv buff %d bytes fail, no enough memory", data_size * 2 + 1);
        return;
    }

    /* Get receive data to receive buffer */
    /* Alert! if using sscanf stores strings, be rember allocating enouth memory! */
    sscanf(data, "+IPRD: %*d,%*d,%s", recv_buff);

    char *recv_str = calloc(1, data_size + 1);
    if (recv_str == OS_NULL)
    {
        LOG_EXT_E("Calloc recv str %d bytes fail, no enough memory", data_size + 1);
        return;
    }

    /* from mo_lib */
    hexstr_to_bytes(recv_buff, recv_str, data_size);
    
    mo_netconn_data_recv_notice(netconn, recv_str, data_size);
    
    return;
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+IPCLOSE:", .suffix = "\r\n", .func = urc_close_func},
    {.prefix = "+IPRD:",  .suffix = "\r\n", .func = urc_recv_func},
};

void m5311_netconn_init(mo_m5311_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < M5311_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = i;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* M5311_USING_NETCONN_OPS */
