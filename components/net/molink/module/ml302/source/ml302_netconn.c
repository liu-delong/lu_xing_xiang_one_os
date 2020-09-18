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
 * @file        ml302_netconn.c
 *
 * @brief       ml302 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ml302_netconn.h"
#include "ml302.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DBG_EXT_TAG "ml302.netconn"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define SEND_DATA_MAX_SIZE (1460)

#ifndef ML302_DATA_QUEUE_SIZE
#define ML302_DATA_QUEUE_SIZE (5)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define ML302_EVENT_CONN_OK   (1L << 0)
#define ML302_EVENT_SEND_OK   (1L << 1)
#define ML302_EVENT_RECV_OK   (1L << 2)
#define ML302_EVNET_CLOSE_OK  (1L << 3)
#define ML302_EVENT_CONN_FAIL (1L << 4)
#define ML302_EVENT_SEND_FAIL (1L << 5)
#define ML302_EVENT_DOMAIN_OK (1L << 6)

#ifdef ML302_USING_NETCONN_OPS

static os_err_t ml302_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_IPC_WAITING_FOREVER);
}

static os_err_t ml302_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static os_bool_t ml302_check_state(mo_object_t *module, os_int32_t connect_id)
{
    at_parser_t *parser       = &module->parser;
    char         mipstate[50] = {0};
    const char   connect[10]  = "CONNECT";
    const char   listen[10]   = "LISTEN";

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+MIPSTATE=%d", connect_id) != OS_EOK)
    {
        LOG_EXT_E("Check connecd id %d state failed!", connect_id);
        return OS_FALSE;
    }

    if (at_resp_get_data_by_kw(&resp, "+MIPSTATE:", "+MIPSTATE:%s", &mipstate) <= 0)
    {
        LOG_EXT_E("Get connect_id :%d  ip state failed", connect_id);
        return OS_ERROR;
    }
    if (OS_NULL == strstr(mipstate, connect) && OS_NULL == strstr(mipstate, listen))
    {
        LOG_EXT_I("Check connecd id %d state free!", connect_id);
        return OS_TRUE;
    }

    LOG_EXT_I("Check connecd id %d state used!", connect_id);
    return OS_FALSE;
}

static mo_netconn_t *ml302_netconn_alloc(mo_object_t *module)
{
    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);

    for (int i = 0; i < ML302_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == ml302->netconn[i].stat)
        {
            if (ml302_check_state(module, i))
            {
                ml302->netconn[i].connect_id = i;

                return &ml302->netconn[i];
            }
        }
    }

    LOG_EXT_E("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *ml302_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);

    return &ml302->netconn[connect_id];
}

os_err_t ml302_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);

    info->netconn_array = ml302->netconn;
    info->netconn_nums  = sizeof(ml302->netconn) / sizeof(ml302->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *ml302_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);
    ml302_lock(&ml302->netconn_lock);
    mo_netconn_t *netconn = ml302_netconn_alloc(module);

    if (OS_NULL == netconn)
    {

        return OS_NULL;
    }
    os_data_queue_init(&netconn->data_queue, ML302_DATA_QUEUE_SIZE, 0, OS_NULL);

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;
    ml302_unlock(&ml302->netconn_lock);
    return netconn;
}

os_err_t ml302_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;

    LOG_EXT_D("Module %s in %d netconnn status", module->name, netconn->stat);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 2,
                      .timeout   = 1 * OS_TICK_PER_SECOND
                     };

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
        break;
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+MIPCLOSE=%d", netconn->connect_id);
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

os_err_t ml302_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;


    mo_ml302_t * ml302  = os_container_of(module, mo_ml302_t, parent);
    ml302_lock(&ml302->netconn_lock);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    os_uint32_t event = SET_EVENT(netconn->connect_id, ML302_EVENT_CONN_OK | ML302_EVENT_CONN_FAIL);

    os_event_recv(&ml302->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_IPC_WAITING_NO, OS_NULL);

    os_err_t result = OS_EOK;

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+MIPOPEN=%d,\"TCP\",\"%s\",%d",
                                    netconn->connect_id,
                                    remote_ip,
                                    port);
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+MIPOPEN=%d,\"UDP\",\"%s\",%d,",
                                    netconn->connect_id,
                                    remote_ip,
                                    port);
        break;
    default:
        result = OS_ERROR;
        goto __exit;
    }

    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&ml302->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn id %d wait conect event timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&ml302->netconn_evt,
                           ML302_EVENT_CONN_OK | ML302_EVENT_CONN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn id %d wait conect result timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    if (event & ML302_EVENT_CONN_FAIL)
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
    ml302_unlock(&ml302->netconn_lock);
    return result;
}

os_size_t ml302_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;

    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);

    ml302_lock(&ml302->netconn_lock);

    ml302->curr_connect = netconn->connect_id;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 2,
                      .timeout   = 5 * OS_TICK_PER_SECOND
                     };

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

        result = at_parser_exec_cmd(parser, &resp, "AT+MIPSEND=%d,%d", netconn->connect_id, curr_size);
        if (result != OS_EOK)
        {
            goto __exit;
        }

        if (at_parser_send(parser, data + sent_size, curr_size) <= 0)
        {
            goto __exit;
        }

        result = os_event_recv(&ml302->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               10 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            LOG_EXT_E("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&ml302->netconn_evt,
                               ML302_EVENT_SEND_OK | ML302_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            LOG_EXT_E("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & ML302_EVENT_SEND_FAIL)
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

    ml302_unlock(&ml302->netconn_lock);

    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s connect id %d send %d bytes data failed", module->name, netconn->connect_id, size);
        return 0;
    }

    return sent_size;
}

os_err_t ml302_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != domain_name);
    OS_ASSERT(OS_NULL != addr);

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};

    mo_ml302_t *ml302 = os_container_of(self, mo_ml302_t, parent);

    os_event_recv(&ml302->netconn_evt,
                  ML302_EVENT_DOMAIN_OK,
                  OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                  OS_IPC_WAITING_NO,
                  OS_NULL);

    ml302->netconn_data = addr;

    result = at_parser_exec_cmd(parser, &resp, "AT+MDNSGIP=\"%s\"", domain_name);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&ml302->netconn_evt,
                           ML302_EVENT_DOMAIN_OK,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);

__exit:
    ml302->netconn_data = OS_NULL;

    return result;
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_ml302_t  *ml302  = os_container_of(module, mo_ml302_t, parent);

    os_int32_t connect_id = -1;
    connect_id = data[0] - '0';
    if (strstr(data, "CONNECT OK"))
    {
        os_event_send(&ml302->netconn_evt, SET_EVENT(connect_id, ML302_EVENT_CONN_OK));
    }
    else
    {
        os_event_send(&ml302->netconn_evt, SET_EVENT(connect_id, ML302_EVENT_CONN_FAIL));
    }
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_ml302_t  *ml302  = os_container_of(module, mo_ml302_t, parent);

    os_int32_t curr_connect = ml302->curr_connect;

    if (strstr(data, "SEND OK"))
    {

        os_event_send(&ml302->netconn_evt, SET_EVENT(curr_connect, ML302_EVENT_SEND_OK));
    }
    else
    {
        os_event_send(&ml302->netconn_evt, SET_EVENT(curr_connect, ML302_EVENT_SEND_FAIL));
    }
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;

    sscanf(data, "+MIPURC: \"recv\",%d,%d", &connect_id, &data_size);

    os_int32_t timeout = data_size > 10 ? data_size : 10;

    LOG_EXT_I("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = ml302_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        LOG_EXT_E("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    char *recv_buff = calloc(1, data_size);
    if (recv_buff == OS_NULL)
    {
        /* read and clean the coming data */
        LOG_EXT_E("Calloc recv buff %d bytes fail, no enough memory", data_size * 2);
        os_size_t temp_size    = 0;
        char      temp_buff[8] = {0};
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
        return;
    }

    if (at_parser_recv(parser, recv_buff, data_size, timeout) != data_size)
    {
        LOG_EXT_E("Module %s netconnt id %d recv %d bytes data failed!", module->name, netconn->connect_id, data_size);
        return;
    }
    
    mo_netconn_data_recv_notice(netconn, recv_buff, data_size);

    return;
}

static void urc_state_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id    = 0;
    os_int32_t connect_state = 0;

    sscanf(data, "+MIPURC: \"STATE\",%d,%d", &connect_id, &connect_state);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = ml302_get_netconn_by_id(module, connect_id);

    switch (connect_state)
    {
    case 1:
        LOG_EXT_W("Module %s receive close urc data of connect %d, server closed the connection.",
                  module->name,
                  connect_id);
        break;
    case 2:
        LOG_EXT_W("Module %s receive close urc data of connect %d, connection exception.",
                  module->name,
                  connect_id);
        break;
    default:
        break;
    }

    mo_netconn_pasv_close_notice(netconn);
}

static void urc_mdnsgip_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
#define HOST_NAME_MAX_LEN 50
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_ml302_t  *ml302  = os_container_of(module, mo_ml302_t, parent);

    int j = 0;

    for (int i = 0; i < size; i++)
    {
        if (*(data + i) == '.')
            j++;
    }
    /* There would be several dns result, we just pickup one */
    if ((j > 2) && (j % 3 == 2))
    {
        char recvip[IPADDR_MAX_STR_LEN + 1] = {0};
        char host_name[HOST_NAME_MAX_LEN + 1] = {0};
        int  error_code =-1;
        sscanf(data, "+MDNSGIP: %d,\"%[^\"]\",\"%[^\"]", &error_code,host_name,recvip);
        recvip[IPADDR_MAX_STR_LEN] = '\0';
        inet_aton(recvip, (ip_addr_t *)ml302->netconn_data);
        os_event_send(&ml302->netconn_evt, ML302_EVENT_DOMAIN_OK);
    }
    else
    {
        LOG_EXT_E("Module %s gethostbyname failed!", module->name);
    }
}

static void urc_mipurc_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    switch (*(data + 10))
    {
    case 'r':
        urc_recv_func(parser, data, size);
        break;
    case 'S':
        urc_state_func(parser, data, size);
        break;
    default:
        break;
    }
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;

    sscanf(data, "%d,CLOSED", &connect_id);

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_netconn_t *netconn = ml302_get_netconn_by_id(module, connect_id);

    if (NETCONN_STAT_CONNECT == netconn->stat)
    {
        LOG_EXT_W("Module %s receive close urc data of connect %d", module->name, connect_id);

        mo_netconn_pasv_close_notice(netconn);
    }
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "",          .suffix = "CONNECT OK\r\n", .func = urc_connect_func},
    {.prefix = "",          .suffix = "SEND OK\r\n",    .func = urc_send_func},
    {.prefix = "+MIPURC:",  .suffix = "\r\n",           .func = urc_mipurc_func},
    {.prefix = "+MDNSGIP:", .suffix = "\r\n",           .func = urc_mdnsgip_func},
    {.prefix = "",          .suffix = ",CLOSED\r\n",    .func = urc_close_func},
};

static os_err_t ml302_network_init(mo_object_t *module)
{
    int          verctrl_data1 = -1;
    int          verctrl_data2 = -1;
    char         CPIN[10]      = {0};
    int          cfun          = -1;
    char         reg_state[10] = {0};
    at_parser_t *parser        = &module->parser;
    /* Use AT+COPS? to query current Network Operator */

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+VERCTRL?");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "+VERCTRL:", "+VERCTRL: %d, %d", &verctrl_data1, &verctrl_data2) < 0)
    {
        goto __exit;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+CPIN?");
    if (result != OS_EOK)
    {
        goto __exit;
    }
    if (at_resp_get_data_by_kw(&resp, "+CPIN:", "+CPIN: %s", &CPIN) < 0)
    {
        goto __exit;
    }
    if (strcmp(CPIN, "READY") != 0)
    {
        goto __exit;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+CFUN?");
    if (result != OS_EOK)
    {
        goto __exit;
    }
    if (at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN: %d", &cfun) < 0)
    {
        goto __exit;
    }
    if (cfun == 0)
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CFUN=1");
        if (result != OS_EOK)
        {
            goto __exit;
        }
        os_task_mdelay(20);
    }

    if (0 == verctrl_data2)
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CGATT=1");
        if (result != OS_EOK)
        {
            goto __exit;
        }

        result = at_parser_exec_cmd(parser, &resp, "AT+CGACT=1");
        if (result != OS_EOK)
        {
            goto __exit;
        }
    }
    else if (1 == verctrl_data2)
    {
        result = OS_EOK;
    }
    else
    {
        result = OS_ERROR;
        goto __exit;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        goto __exit;
    }
    if (at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG: %s", &reg_state) < 0)
    {
        goto __exit;
    }
    if ('1' != reg_state[2] && '5' != reg_state[2])
    {
        result = OS_ERROR;
        goto __exit;
    }

__exit:
    if (result != OS_EOK)
    {
        LOG_EXT_E("ML302 network init failed");
    }

    return result;
}

os_err_t ml302_netconn_init(mo_ml302_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < ML302_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    os_err_t result = ml302_network_init(&module->parent);
    if(result != OS_EOK)
    {
        return result;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    return result;
}

#endif /* ML302_USING_NETCONN_OPS */
