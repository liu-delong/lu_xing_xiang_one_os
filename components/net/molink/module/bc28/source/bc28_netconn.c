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
 * @file        bc28_netconn.c
 *
 * @brief       bc28 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc28_netconn.h"
#include "bc28.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DBG_EXT_TAG "bc28.netconn"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define PROTOCOL_TYPE_TCP (6)
#define PROTOCOL_TYPE_UDP (17)

#define SEND_DATA_MAX_SIZE (1358)

#ifndef BC28_DATA_QUEUE_SIZE
#define BC28_DATA_QUEUE_SIZE (5)
#endif

#define BC28_CONN_ID_NULL  (-1)

#ifdef BC28_USING_NETCONN_OPS

static os_err_t bc28_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_IPC_WAITING_FOREVER);
}

static os_err_t bc28_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *bc28_netconn_alloc(mo_object_t *module)
{
    mo_bc28_t *bc28 = os_container_of(module, mo_bc28_t, parent);

    for (int i = 0; i < BC28_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == bc28->netconn[i].stat)
        {
            return &bc28->netconn[i];
        }
    }

    LOG_EXT_E("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *bc28_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    mo_bc28_t *bc28 = os_container_of(module, mo_bc28_t, parent);

    for (int i = 0; i < BC28_NETCONN_NUM; i++)
    {
        if (connect_id == bc28->netconn[i].connect_id)
        {
            return &bc28->netconn[i];
        }
    }
    
    return OS_NULL;
}

os_err_t bc28_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_bc28_t *bc28 = os_container_of(module, mo_bc28_t, parent);

    info->netconn_array = bc28->netconn;
    info->netconn_nums  = sizeof(bc28->netconn) / sizeof(bc28->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *bc28_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_netconn_t *netconn = bc28_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        return OS_NULL;
    }

    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    /* enable user data URC eg. +NSONMI: <socket>,<length>,<data>. */
    result = at_parser_exec_cmd(parser, &resp, "AT+NSONMI=2");
    if (OS_EOK != result)
    {
        LOG_EXT_E("Module %s set enable netconn urc autorecv data failed", module->name);
        return OS_NULL;
    }
    
    switch (type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser, &resp, "AT+NSOCR=STREAM,%d,0,1", PROTOCOL_TYPE_TCP);
        break;
    case NETCONN_TYPE_UDP:
        /* BC28 shown that can only create 1 UDP socket */
        result = at_parser_exec_cmd(parser, &resp, "AT+NSOCR=DGRAM,%d,0,1", PROTOCOL_TYPE_UDP);
        break;
    default:
        result = OS_ERROR;
        break;
    }

    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s create %s netconn failed", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        return OS_NULL;
    }

    if (at_resp_get_data_by_line(&resp, 2, "%d", &netconn->connect_id) <= 0)
    {
        LOG_EXT_E("Module %s get %s netconn id failed", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        return OS_NULL;
    }

    os_data_queue_init(&netconn->data_queue, BC28_DATA_QUEUE_SIZE, 0, OS_NULL);

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    return netconn;
}

os_err_t bc28_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;
    os_int32_t   connid = BC28_CONN_ID_NULL;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    LOG_EXT_I("Module %s in %d netconnn status", module->name, netconn->stat);

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+NSOCL=%d", netconn->connect_id);
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

    if (netconn->data_queue.queue != OS_NULL)
    {
        os_data_queue_deinit(&netconn->data_queue);
    }
    connid              = netconn->connect_id;
    netconn->connect_id = BC28_CONN_ID_NULL;
    netconn->stat       = NETCONN_STAT_NULL;
    netconn->type       = NETCONN_TYPE_NULL;
    
    LOG_EXT_I("Module %s netconnn_id: %d destroyed", module->name, connid);

    return OS_EOK;
}

os_err_t bc28_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    at_parser_t *parser = &self->parser;

	char recvip[IPADDR_MAX_STR_LEN] = {0};
	
    char resp_buff[256] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 4,
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+QDNS=0,\"%s\"", domain_name);
    if (result < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    /* AT+CMDNS="www.baidu.com" return: OK \r\n  +CMDNS:183.232.231.172 \r\n */
    /* AT+CMDNS="8.8.8.8" return: +CMDNS:8.8.8.8 \r\n  OK */
    if (at_resp_get_data_by_kw(&resp, "+QDNS:", "+QDNS:%s", recvip) <= 0)
    {
        LOG_EXT_E("BC28 domain resolve: resp parse fail, try again, host: %s", domain_name);
        result = OS_ERROR;
        /* If resolve failed, maybe receive an URC CRLF */
        goto __exit;
    }

    if (strlen(recvip) < IPADDR_MIN_STR_LEN)
    {
        LOG_EXT_E("BC28 domain resolve: recvip len < IPADDR_MIN_STR_LEN, len = %d", strlen(recvip));
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("BC28 domain resolve: \"%s\" domain ip is %s, addrlen %d", domain_name, recvip, strlen(recvip));
        inet_aton(recvip, addr);
        /* TODO ipv4 only */
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

static os_err_t bc28_tcp_connect(at_parser_t *parser, os_int32_t connect_id, char *ip_addr, os_uint16_t port)
{
    /* TODO */
	// char buf[16] = {0};
	
    // at_parser_set_resp(parser, 128, 4, os_tick_from_ms(20000));
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+NSOCO=%d,%s,%d", connect_id, ip_addr, port);
//     if (result != OS_EOK)
//     {
//         goto __exit;
//     }

//     if (at_parser_get_data_by_kw(parser, "CONNECT", "CONNECT %s", buf) <= 0)
//     {
//         result = OS_ERROR;
//         goto __exit;
//     }

// __exit:

    return result;
}

os_err_t bc28_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = bc28_tcp_connect(parser, netconn->connect_id, remote_ip, port);
        break;
    case NETCONN_TYPE_UDP:
        result = OS_EOK;    /* UDP does not need to connect */
        break;
    default:
        result = OS_ERROR;
        break;
    }

    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s connect to %s:%d failed!", module->name, remote_ip, port);
        return result;
    }

    ip_addr_copy(netconn->remote_ip, addr);
    netconn->remote_port = port;
    netconn->stat        = NETCONN_STAT_CONNECT;

    LOG_EXT_D("Module %s connect to %s:%d successfully!", module->name, remote_ip, port);

    return OS_EOK;
}

static os_size_t bc28_tcp_send(at_parser_t *parser, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    os_err_t   result       = OS_EOK;
    os_size_t  sent_size    = 0;
    os_size_t  cur_pkt_size = 0;
    os_int32_t connect_id   = BC28_CONN_ID_NULL;
    os_size_t  cnt          = 0;

    char send_cmd[30] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    while (sent_size < size)
    {
        if (size - sent_size < SEND_DATA_MAX_SIZE)
        {
            cur_pkt_size = size - sent_size;
        }
        else
        {
            cur_pkt_size = SEND_DATA_MAX_SIZE;
        }

        snprintf(send_cmd, sizeof(send_cmd), "AT+NSOSD=%d,%d,", netconn->connect_id, (int)cur_pkt_size / 2);

        if (at_parser_send(parser, send_cmd, strlen(send_cmd)) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        if (at_parser_send(parser, data + sent_size, cur_pkt_size) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        result = at_parser_exec_cmd(parser, &resp, "");
        if (result != OS_EOK)
        {
            goto __exit;
        }

        result = OS_ERROR;
        for (int i = 1; i <= resp.line_counts; i++)
        {
            if (at_resp_get_data_by_line(&resp, i, "%d,%d", &connect_id, &cnt) > 0 && cnt == cur_pkt_size / 2)
            {
                result = OS_EOK;
                break;
            }
        }

        if (OS_ERROR == result)
        {
            goto __exit;
        }

        sent_size += cur_pkt_size;
    }

__exit:

    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn %d send %d bytes data failed!",
                  parser->name,
                  netconn->connect_id,
                  cur_pkt_size / 2);
    }

    return sent_size / 2;
}

static os_size_t bc28_udp_send(at_parser_t *parser, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    os_err_t   result       = OS_EOK;
    os_size_t  sent_size    = 0;
    os_size_t  cur_pkt_size = 0;
    os_int32_t connect_id   = BC28_CONN_ID_NULL;
    os_size_t  cnt          = 0;

    char send_cmd[40]                      = {0};
    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF]  = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    strncpy(remote_ip, inet_ntoa(netconn->remote_ip), IPADDR_MAX_STR_LEN);

    while (sent_size < size)
    {
        if (size - sent_size < SEND_DATA_MAX_SIZE)
        {
            cur_pkt_size = size - sent_size;
        }
        else
        {
            cur_pkt_size = SEND_DATA_MAX_SIZE;
        }

        snprintf(send_cmd,
                 sizeof(send_cmd),
                 "AT+NSOST=%d,%s,%d,%d,",
                 netconn->connect_id,
                 remote_ip,
                 netconn->remote_port,
                 (int)cur_pkt_size / 2);

        if (at_parser_send(parser, send_cmd, strlen(send_cmd)) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        if (at_parser_send(parser, data + sent_size, cur_pkt_size) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        result = at_parser_exec_cmd(parser, &resp, "");
        if (result != OS_EOK)
        {
            goto __exit;
        }

                result = OS_ERROR;
        for (int i = 1; i <= resp.line_counts; i++)
        {
            if (at_resp_get_data_by_line(&resp, i, "%d,%d", &connect_id, &cnt) > 0 && cnt == cur_pkt_size / 2)
            {
                result = OS_EOK;
                break;
            }
        }

        if (OS_ERROR == result)
        {
            goto __exit;
        }

        sent_size += cur_pkt_size;
    }

__exit:
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn %d send %d bytes data failed!",
                  parser->name,
                  netconn->connect_id,
                  cur_pkt_size / 2);
    }

    return sent_size;
}

os_size_t bc28_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_size_t    sent_size = 0;

    char *hexstr = calloc(1, size * 2 + 1);
    if (OS_NULL == hexstr)
    {
        LOG_EXT_E("Moudle &s netconn %d calloc %d bytes memory failed!",
                  module->name,
                  netconn->connect_id,
                  size * 2 + 1);
        return 0;
    }

    bytes_to_hexstr(data, hexstr, size);

    mo_bc28_t *bc28 = os_container_of(module, mo_bc28_t, parent);

    bc28_lock(&bc28->netconn_lock);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        sent_size = bc28_tcp_send(parser, netconn, hexstr, strlen(hexstr));
        break;
    case NETCONN_TYPE_UDP:
		sent_size = bc28_udp_send(parser, netconn, hexstr, strlen(hexstr));
        break;
    default:
        break;
    }

    bc28_unlock(&bc28->netconn_lock);

    free(hexstr);

    return sent_size;
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;

    sscanf(data, "+NSOCLI: %d", &connect_id);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = bc28_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        LOG_EXT_E("Module %s receive error close urc data of connect %d", module->name, connect_id);
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

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;

    sscanf(data, "+NSONMI:%d,%*[^,],%*d,%d,", &connect_id, &data_size);

    LOG_EXT_I("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = bc28_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        LOG_EXT_E("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    if (netconn->stat == NETCONN_STAT_CONNECT)
    {
        /*  bufflen >= strsize + 1 */
        char *recv_buff = calloc(1, data_size * 2 + 1);
        if (recv_buff == OS_NULL)
        {
            LOG_EXT_E("Calloc recv buff %d bytes fail, no enough memory", data_size * 2 + 1);
            return;
        }
        /* Get receive data to receive buffer */
        /* Alert! if using sscanf stores strings, be rember allocating enouth memory! */
        sscanf(data, "+NSONMI:%*d,%*[^,],%*d,%*d,%s", recv_buff);

        char *recv_str = calloc(1, data_size + 1);
        if (recv_str == OS_NULL)
        {
            LOG_EXT_E("Calloc recv str %d bytes fail, no enough memory", data_size + 1);
            return;
        }

        hexstr_to_bytes(recv_buff, recv_str, data_size * 2);
  
        mo_netconn_data_recv_notice(netconn, recv_str, data_size);

        free(recv_buff);
    }

    return;
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+NSOCLI:", .suffix = "\r\n", .func = urc_close_func},
    {.prefix = "+NSONMI:", .suffix = "\r\n", .func = urc_recv_func },
};

void bc28_netconn_init(mo_bc28_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < BC28_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = BC28_CONN_ID_NULL;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* BC28_USING_NETCONN_OPS */
