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
 * @brief       ec200x module link kit netconn api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ec200x_netconn.h"
#include "ec200x.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DBG_EXT_TAG "ec200x.netconn"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define SEND_DATA_MAX_SIZE (1460)

#ifndef EC200X_DATA_QUEUE_SIZE
#define EC200X_DATA_QUEUE_SIZE (5)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define EC200X_EVENT_CONN_OK   (1L << 0)
#define EC200X_EVENT_SEND_OK   (1L << 1)
#define EC200X_EVENT_RECV_OK   (1L << 2)
#define EC200X_EVNET_CLOSE_OK  (1L << 3)
#define EC200X_EVENT_CONN_FAIL (1L << 4)
#define EC200X_EVENT_SEND_FAIL (1L << 5)
#define EC200X_EVENT_DOMAIN_OK (1L << 6)

#ifdef EC200X_USING_NETCONN_OPS

static os_err_t ec200x_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_IPC_WAITING_FOREVER);
}

static os_err_t ec200x_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static os_bool_t ec200x_check_state(mo_object_t *module, os_int32_t connect_id)
{
    at_parser_t *parser = &module->parser;

    if (at_parser_exec_cmd(parser, "AT+QISTATE=1,%d", connect_id) != OS_EOK)
    {
        LOG_EXT_E("Check connecd id %d state failed!", connect_id);
        return OS_FALSE;
    }

    if (at_parser_get_line_by_kw(parser, "+QISTATE:") != OS_NULL)
    {
        /* connect id already in use */
        return OS_FALSE;
    }

    return OS_TRUE;
}

static mo_netconn_t *ec200x_netconn_alloc(mo_object_t *module)
{
    mo_ec200x_t *ec200x = os_container_of(module, mo_ec200x_t, parent);

    for (int i = 0; i < EC200X_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == ec200x->netconn[i].stat)
        {
            if (ec200x_check_state(module, i))
            {
                ec200x->netconn[i].connect_id = i;

                return &ec200x->netconn[i];
            }            
        }
    }

    LOG_EXT_E("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *ec200x_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    mo_ec200x_t *ec200x = os_container_of(module, mo_ec200x_t, parent);

    return &ec200x->netconn[connect_id];
}

mo_netconn_t *ec200x_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{

    mo_netconn_t *netconn = ec200x_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        return OS_NULL;
    }

    os_data_queue_init(&netconn->data_queue, EC200X_DATA_QUEUE_SIZE, 0, OS_NULL);

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    return netconn;
}

os_err_t ec200x_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;

    LOG_EXT_D("Module %s in %d netconnn status", module->name, netconn->stat);

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, "AT+QICLOSE=%d", netconn->connect_id);
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

    LOG_EXT_I("Module %s netconnn id %d destroyed", module->name, netconn->connect_id);

    netconn->connect_id = -1;
    netconn->stat       = NETCONN_STAT_NULL;
    netconn->type       = NETCONN_TYPE_NULL;

    return OS_EOK;
}

os_err_t ec200x_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;
    mo_ec200x_t *ec200x = os_container_of(module, mo_ec200x_t, parent);

    at_parser_set_resp(parser, 64, 0, os_tick_from_ms(150 * 1000));

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    os_uint32_t event = SET_EVENT(netconn->connect_id, EC200X_EVENT_CONN_OK | EC200X_EVENT_CONN_FAIL);

    os_event_recv(&ec200x->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_IPC_WAITING_NO, OS_NULL);

    os_err_t result = OS_EOK;

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser,
                                    "AT+QIOPEN=1,%d,\"TCP\",\"%s\",%d,0,1", 
                                    netconn->connect_id, 
                                    remote_ip, 
                                    port);
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser,
                                    "AT+QIOPEN=1,%d,\"UDP\",\"%s\",%d,0,1", 
                                    netconn->connect_id, 
                                    remote_ip, 
                                    port);
        break;
    default:
        LOG_EXT_E("Module %s connect to %s:%d failed!", module->name, remote_ip, port);
        result = OS_ERROR;
        goto __exit;
    }

    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&ec200x->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn id %d wait conect event timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&ec200x->netconn_evt,
                           EC200X_EVENT_CONN_OK | EC200X_EVENT_CONN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn id %d wait conect result timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    if (event & EC200X_EVENT_CONN_FAIL)
    {
        result = OS_ERROR;
        LOG_EXT_E("Module %s netconn id %d conect failed!", module->name, netconn->connect_id);
    }

__exit:
    if (OS_EOK == result)
    {
        ip_addr_copy(netconn->remote_ip, addr);
        netconn->remote_port = port;
        netconn->stat        = NETCONN_STAT_CONNECT;

        LOG_EXT_D("Module %s connect to %s:%d successfully!", module->name, remote_ip, port);
    }
    else
    {
        LOG_EXT_E("Module %s connect to %s:%d failed!", module->name, remote_ip, port);
    }

    at_parser_reset_resp(parser);

    return result;
}

os_size_t ec200x_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;

    mo_ec200x_t *ec200x = os_container_of(module, mo_ec200x_t, parent);

    ec200x_lock(&ec200x->netconn_lock);

    ec200x->curr_connect = netconn->connect_id;

    at_parser_set_resp(parser, 128, 2, 5 * OS_TICK_PER_SECOND);

    at_parser_set_end_mark(parser, ">", 1);

    while (sent_size < size)
    {
        if (size - sent_size < SEND_DATA_MAX_SIZE)
        {
            curr_size = size - sent_size;
        }
        else
        {
            curr_size = SEND_DATA_MAX_SIZE;
        }

        result = at_parser_exec_cmd(parser, "AT+QISEND=%d,%d", netconn->connect_id, curr_size);
        if (result != OS_EOK)
        {
            goto __exit;
        }

        if (at_parser_send(parser, data + sent_size, curr_size) <= 0)
        {
            goto __exit;
        }

        result = os_event_recv(&ec200x->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               10 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            LOG_EXT_E("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&ec200x->netconn_evt,
                               EC200X_EVENT_SEND_OK | EC200X_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            LOG_EXT_E("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & EC200X_EVENT_SEND_FAIL)
        {
            LOG_EXT_E("Module %s connect id %d send failed!", module->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        sent_size += curr_size;

        os_task_mdelay(10);
    }

__exit:

    at_parser_set_end_mark(parser, OS_NULL, 0);

    at_parser_reset_resp(parser);

    ec200x_unlock(&ec200x->netconn_lock);
    
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s connect id %d send %d bytes data failed", module->name, netconn->connect_id, size);
        return 0;
    }

    return sent_size;
}

os_err_t ec200x_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != domain_name);
    OS_ASSERT(OS_NULL != addr);

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;

    at_parser_set_resp(parser, 128, 0, os_tick_from_ms(300));

    mo_ec200x_t *ec200x = os_container_of(self, mo_ec200x_t, parent);

    os_event_recv(&ec200x->netconn_evt,
                  EC200X_EVENT_DOMAIN_OK,
                  OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                  OS_IPC_WAITING_NO,
                  OS_NULL);

    ec200x->netconn_data = addr;

    result = at_parser_exec_cmd(parser, "AT+QIDNSGIP=1,\"%s\"", domain_name);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&ec200x->netconn_evt,
                           EC200X_EVENT_DOMAIN_OK,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);

__exit:
    ec200x->netconn_data = OS_NULL;

    at_parser_reset_resp(parser);

    return result;
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_ec200x_t *ec200x = os_container_of(module, mo_ec200x_t, parent);

    os_int32_t connect_id = 0;
    os_int32_t result     = 0;

    sscanf(data, "+QIOPEN: %d,%d", &connect_id , &result);

    if (0 == result)
    {
        os_event_send(&ec200x->netconn_evt, SET_EVENT(connect_id, EC200X_EVENT_CONN_OK));
    }
    else
    {
        os_event_send(&ec200x->netconn_evt, SET_EVENT(connect_id, EC200X_EVENT_CONN_FAIL));
    }
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_ec200x_t *ec200x = os_container_of(module, mo_ec200x_t, parent);

    os_int32_t curr_connect = ec200x->curr_connect;

    if (strstr(data, "SEND OK"))
    {
        os_event_send(&ec200x->netconn_evt, SET_EVENT(curr_connect, EC200X_EVENT_SEND_OK));
    }
    else if (strstr(data, "SEND FAIL"))
    {
        os_event_send(&ec200x->netconn_evt, SET_EVENT(curr_connect, EC200X_EVENT_SEND_FAIL));
    }
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;

    sscanf(data, "+QIURC: \"closed\",%d", &connect_id);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = ec200x_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        LOG_EXT_E("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    LOG_EXT_W("Module %s receive close urc data of connect %d", module->name, connect_id);

    netconn->stat = NETCONN_STAT_CLOSE;

    os_data_queue_reset(&netconn->data_queue);
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;

    sscanf(data, "+QIURC: \"recv\",%d,%d", &connect_id, &data_size);

    os_int32_t timeout = data_size > 10 ? data_size : 10;

    LOG_EXT_I("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = ec200x_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        LOG_EXT_E("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    char *recv_buff    = calloc(1, data_size);
    char  temp_buff[8] = {0};
    if (recv_buff == OS_NULL)
    {
        /* read and clean the coming data */
        LOG_EXT_E("Calloc recv buff %d bytes fail, no enough memory", data_size);
        os_size_t temp_size    = 0;
        while (temp_size < data_size)
        {
            if (data_size - temp_size > sizeof(temp_buff))
            {
                at_parser_recv(parser, temp_buff, sizeof(temp_buff), timeout);
            }
            else
            {
                at_parser_recv(parser, temp_buff, data_size - temp_size, timeout);
            }
            temp_size += sizeof(temp_buff);
        }

        /* handle "\r\n" */
        at_parser_recv(parser, temp_buff, 2, timeout);

        return;
    }

    if (at_parser_recv(parser, recv_buff, data_size, timeout) != data_size)
    {
        LOG_EXT_E("Module %s netconnt id %d recv %d bytes data failed!", module->name, netconn->connect_id, data_size);
        return;
    }

    /* handle "\r\n" */
    at_parser_recv(parser, temp_buff, 2, timeout);

    if (netconn->stat == NETCONN_STAT_CONNECT)
    {
        os_data_queue_push(&netconn->data_queue, recv_buff, data_size, OS_IPC_WAITING_FOREVER);
    }
    else
    {
        LOG_EXT_E("Module %s netconn id %d receive state error", module->name, connect_id);
    }
}

static void urc_pdpdeact_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    int context_id = 0;

    sscanf(data, "+QIURC: \"pdpdeact\",%d", &context_id);

    LOG_EXT_E("context (%d) is deactivated.", context_id);
}

static void urc_dnsqip_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_ec200x_t *ec200x = os_container_of(module, mo_ec200x_t, parent);

    int j = 0;

    for (int i = 0; i < size; i++)
    {
        if (*(data + i) == '.')
            j++;
    }
    /* There would be several dns result, we just pickup one */
    if (3 == j)
    {
        char recvip[IPADDR_MAX_STR_LEN + 1] = {0};

        sscanf(data, "+QIURC: \"dnsgip\",\"%[^\"]", recvip);
        recvip[IPADDR_MAX_STR_LEN] = '\0';

        inet_aton(recvip, (ip_addr_t *)ec200x->netconn_data);

        os_event_send(&ec200x->netconn_evt, EC200X_EVENT_DOMAIN_OK);
    }
    else
    {
        LOG_EXT_D("Not required dns URC data, not processed");
    }
}

static void urc_func(struct at_parser *parsert, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != data);

    LOG_EXT_I("URC data : %.*s", size, data);
}

static void urc_qiurc_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    switch (*(data + 9))
    {
    case 'c':
        urc_close_func(parser, data, size);
        break;   
    case 'r':
        urc_recv_func(parser, data, size);
        break;  
    case 'p':
        urc_pdpdeact_func(parser, data, size);
        break;
    case 'd':
        urc_dnsqip_func(parser, data, size);
        break;
    default:
        urc_func(parser, data, size);
        break;
    }
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "SEND",     .suffix = "\r\n", .func = urc_send_func},
    {.prefix = "+QIOPEN:", .suffix = "\r\n", .func = urc_connect_func},
    {.prefix = "+QIURC:",  .suffix = "\r\n", .func = urc_qiurc_func},
};

static os_err_t ec200x_network_init(mo_object_t *module)
{
    char tmp_data[20] = {0};
	char APN[10]      = {0};

    at_parser_t *parser = &module->parser;
    /* Use AT+COPS? to query current Network Operator */
    os_err_t result = at_parser_exec_cmd(parser, "AT+COPS?");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser, "+COPS:", "+COPS: %*[^\"]\"%[^\"]", &tmp_data) < 0)
    {
        goto __exit;
    }
	
    if (strcmp(tmp_data, "CHINA MOBILE") == 0)
    {
        strncpy(APN, "CMNET", strlen("CMNET"));
    }
    else if (strcmp(tmp_data, "CHN-UNICOM") == 0)
    {
        strncpy(APN, "UNINET", strlen("UNINET"));
    }
    else if (strcmp(tmp_data, "CHN-CT") == 0)
    {
        strncpy(APN, "CTNET", strlen("CTNET"));
    }

    result = at_parser_exec_cmd(parser, "AT+QICSGP=1,1,\"%s\",\"\",\"\",0", APN);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    at_parser_set_resp(parser, 64, 0, os_tick_from_ms(40 * 1000));

    result = at_parser_exec_cmd(parser, "AT+QIDEACT=1");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    at_parser_set_resp(parser, 64, 0, os_tick_from_ms(150 * 1000));

    result = at_parser_exec_cmd(parser, "AT+QIACT=1");
    if (result != OS_EOK)
    {
        goto __exit;
    }

__exit:
    if (result != OS_EOK)
    {
        LOG_EXT_E("EC200X network init failed");
    }

    at_parser_reset_resp(parser);

    return result;
}

void ec200x_netconn_init(mo_ec200x_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < EC200X_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

	ec200x_network_init(&module->parent);
	
    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* EC200X_USING_NETCONN_OPS */
