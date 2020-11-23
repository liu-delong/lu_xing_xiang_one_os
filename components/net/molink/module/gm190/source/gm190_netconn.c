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
 * @file        gm190_netconn.c
 *
 * @brief       gm190 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "gm190_netconn.h"
#include "gm190.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DBG_EXT_TAG "gm190.netconn"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define SEND_DATA_MAX_SIZE (1400)

#ifndef GM190_DATA_QUEUE_SIZE
#define GM190_DATA_QUEUE_SIZE (5)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define GM190_EVENT_CONN_OK    (1L << 0)
#define GM190_EVENT_SEND_OK    (1L << 1)
#define GM190_EVENT_RECV_OK    (1L << 2)
#define GM190_EVENT_CLOSE_OK   (1L << 3)
#define GM190_EVENT_CONN_FAIL  (1L << 4)
#define GM190_EVENT_SEND_FAIL  (1L << 5)
#define GM190_EVENT_DOMAIN_OK  (1L << 6)
#define GM190_EVENT_STAT_OK    (1L << 7)
#define GM190_EVENT_STAT_FAIL  (1L << 8)


enum STAT_URC
{
    INIT = 0, CHECK_STAT, CONNECT_STAT, CLOSE_STAT, UNKNOWN_STAT		
};
static enum STAT_URC  g_stat_urc = INIT;

#ifdef GM190_USING_NETCONN_OPS

static os_bool_t  gm190_pdp_set(mo_object_t *module)
{
    at_parser_t *parser       = &module->parser;
    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * AT_RESP_TIMEOUT_DEF};

    if (at_parser_exec_cmd(parser, &resp, "AT+CGDCONT=1,\"IPV4V6\",\"cmnet\"") != OS_EOK)
    {
        return OS_FALSE;
    }

    if (at_parser_exec_cmd(parser, &resp, "AT+CFUN=0") != OS_EOK)
    {
        return OS_FALSE;
    }	

	if (at_parser_exec_cmd(parser, &resp, "AT+CFUN=1") != OS_EOK)
    {
        return OS_FALSE;
    }

	return OS_TRUE;

}

static os_bool_t gm190_check_zipcall(mo_object_t *module, os_int32_t connect_id)
{
    at_parser_t *parser           = &module->parser;
    char         zipcall[30]      = {0};
    char         resp_buff[256]   = {0};
    char         zpas[30]         = {0};
	at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * AT_RESP_TIMEOUT_DEF};

    if (at_parser_exec_cmd(parser, &resp, "AT+ZPAS?") != OS_EOK)
    {
        return OS_FALSE;
    }

    if (at_resp_get_data_by_kw(&resp, "+ZPAS:", "+ZPAS: %s", &zpas) <= 0)
    {
        return OS_FALSE;
    }

    if (OS_NULL != strstr(zpas, "No"))
    {
        LOG_EXT_E("There is no net service!");
        return OS_FALSE;
    }

	
    if (at_parser_exec_cmd(parser, &resp, "AT+ZIPCALL?") != OS_EOK)
    {
        LOG_EXT_E("Get ip call failed");
        return OS_FALSE;
    }

    if (at_resp_get_data_by_kw(&resp, "+ZIPCALL:", "+ZIPCALL: %s", &zipcall) <= 0)
    {
        LOG_EXT_E("Get ip call failed");
        return OS_FALSE;
    }
		
    if ('1' != zipcall[0] )
    {
	    if (at_parser_exec_cmd(parser, &resp, "AT+ZIPCALL=1") != OS_EOK)
        {
            LOG_EXT_E("Get ip call not ready");
            return OS_FALSE;
        }
    }
	
	LOG_EXT_I("Check call ip ready!");
	return OS_TRUE;

}

static os_err_t gm190_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_IPC_WAITING_FOREVER);
}

static os_err_t gm190_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static os_err_t gm190_check_state(mo_object_t *module,os_int32_t connect_id)
{
    at_parser_t *parser         = &module->parser;
    char         resp_buff[256] = {0};
    os_err_t     result         = OS_ERROR;

    mo_gm190_t * gm190  = os_container_of(module, mo_gm190_t, parent);
		
    at_parser_exec_lock(parser);
	  gm190_lock(&gm190->netconn_lock);;
	  g_stat_urc = CHECK_STAT;
	  os_uint32_t event = SET_EVENT(connect_id, GM190_EVENT_STAT_OK | GM190_EVENT_STAT_FAIL);
	  os_event_recv(&gm190->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_IPC_WAITING_NO, OS_NULL);

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+ZIPSTAT=%d",connect_id) != OS_EOK)
    {
        LOG_EXT_E("Check connecd id %d state failed!",connect_id);
        result = OS_ERROR;
        goto __exit;
    }

    result = os_event_recv(&gm190->netconn_evt,
                           GM190_EVENT_STAT_OK | GM190_EVENT_STAT_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn id %d wait check result timeout!", module->name, connect_id);
        goto __exit;
    }

    if (event & GM190_EVENT_STAT_FAIL)
    {
        result = OS_ERROR;
		goto __exit;
    }
__exit:
	if (OS_EOK == result)
	{	
			LOG_EXT_I("Module %s check id %d :ready!", module->name, connect_id);
	}
	else
	{
			LOG_EXT_E("Module %s check id %d :used!", module->name, connect_id);
	}	
	gm190_unlock(&gm190->netconn_lock);
	at_parser_exec_unlock(parser);
	return result;
}

static mo_netconn_t *gm190_netconn_alloc(mo_object_t *module)
{
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);

    for (int i = 1; i < GM190_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == gm190->netconn[i].stat)
        {
            if (OS_EOK == gm190_check_state(module, i))
            {
                gm190->netconn[i].connect_id = i;

                return &gm190->netconn[i];
            }
        }
    }

    LOG_EXT_E("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *gm190_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);

    return &gm190->netconn[connect_id];
}

os_err_t gm190_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);

    info->netconn_array = gm190->netconn;
    info->netconn_nums  = sizeof(gm190->netconn) / sizeof(gm190->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *gm190_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);
    gm190_lock(&gm190->netconn_lock);
    mo_netconn_t *netconn = gm190_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        gm190_unlock(&gm190->netconn_lock);
        return OS_NULL;
    }
    os_data_queue_init(&netconn->data_queue, GM190_DATA_QUEUE_SIZE, 0, OS_NULL);

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;
    gm190_unlock(&gm190->netconn_lock);
    return netconn;
}

os_err_t gm190_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);


	at_parser_exec_lock(parser);
	gm190_lock(&gm190->netconn_lock);
	
	os_uint32_t event = SET_EVENT(netconn->connect_id, GM190_EVENT_CLOSE_OK);
    os_event_recv(&gm190->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_IPC_WAITING_NO, OS_NULL);

 	g_stat_urc = CLOSE_STAT;

    LOG_EXT_D("Module %s in %d netconnn status", module->name, netconn->stat);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 2,
                      .timeout   = 10 * OS_TICK_PER_SECOND
                     };

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
        break;
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+ZIPCLOSE=%d", netconn->connect_id);
        if (result != OS_EOK)
        {
            LOG_EXT_E("Module %s destroy %s netconn failed",
                      module->name,
                      (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
					  gm190_unlock(&gm190->netconn_lock);
	          at_parser_exec_unlock(parser);
            return result;
        }

	    result = os_event_recv(&gm190->netconn_evt,
                           GM190_EVENT_CLOSE_OK,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           2 * OS_TICK_PER_SECOND,
                           &event);
        if (result != OS_EOK)
        {
            LOG_EXT_E("Module %s netconn id %d wait close result timeout!", module->name, netconn->connect_id);
					  gm190_unlock(&gm190->netconn_lock);
	          at_parser_exec_unlock(parser);
            return result;;
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

    LOG_EXT_I("Module %s netconnn id %d destroyed", module->name, netconn->connect_id);

    netconn->connect_id  = -1;
    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);
		
    gm190_unlock(&gm190->netconn_lock);
	  at_parser_exec_unlock(parser);
    return OS_EOK;
}



os_err_t gm190_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
#define ZIP_CALL_TIMES (18)
    at_parser_t *parser = &module->parser;
    os_err_t    result  = OS_EOK;  
    mo_gm190_t * gm190  = os_container_of(module, mo_gm190_t, parent);
    at_parser_exec_lock(parser);   
	  gm190_lock(&gm190->netconn_lock);
    
	  g_stat_urc = CONNECT_STAT;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);
		
    os_uint32_t event = SET_EVENT(netconn->connect_id, GM190_EVENT_CONN_OK | GM190_EVENT_CONN_FAIL);
    os_event_recv(&gm190->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_IPC_WAITING_NO, OS_NULL);
		
	  if (OS_TRUE != gm190_check_zipcall(module, netconn->connect_id))
	  {
        if (OS_FALSE == gm190_pdp_set(module))
	      {
			      LOG_EXT_E("Module %s set !", module->name);
			      result = OS_ERROR;
			      goto __exit;
		    }

			int i = 0;
		    while (OS_TRUE != gm190_check_zipcall(module, netconn->connect_id))
		    {
			      LOG_EXT_E("Wait module %s call ip !", module->name);
				  i++;
				  if(i > ZIP_CALL_TIMES)
				  {
                      LOG_EXT_E("Wait module %s call ip failed !", module->name);
					  break;
				  }
				  os_task_mdelay(5000);
		    }		
  	}

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+ZIPOPEN=%d,0,%s,%d",
                                    netconn->connect_id,
                                    remote_ip,
                                    port);
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+ZIPOPEN=%d,1,%s,%d",
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

    result = os_event_recv(&gm190->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn id %d wait conect event timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&gm190->netconn_evt,
                           GM190_EVENT_CONN_OK | GM190_EVENT_CONN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn id %d wait conect result timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    if (event & GM190_EVENT_CONN_FAIL)
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
    gm190_unlock(&gm190->netconn_lock);
		at_parser_exec_unlock(parser);
    return result;
}

os_size_t gm190_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;
    char         connect_ready[10] = {0}; 
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);

    gm190_lock(&gm190->netconn_lock);

    gm190->curr_connect = netconn->connect_id;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 2,
                      .timeout   = 5 * OS_TICK_PER_SECOND
                     };

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

        result = at_parser_exec_cmd(parser, &resp, "AT+ZIPSENDRAW=%d,%d", netconn->connect_id, curr_size);
        if (result != OS_EOK)
        {
            goto __exit;
        }
				
        if (at_resp_get_data_by_line(&resp, 2,"%s", connect_ready) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }
				if (OS_NULL == strstr(connect_ready,"CONNECT"))
        {
            result = OS_ERROR;
            goto __exit;
        }				
        if (at_parser_send(parser, data + sent_size, curr_size) <= 0)
        {
            goto __exit;
        }

        result = os_event_recv(&gm190->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               10 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            LOG_EXT_E("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&gm190->netconn_evt,
                               GM190_EVENT_SEND_OK | GM190_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            LOG_EXT_E("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & GM190_EVENT_SEND_FAIL)
        {
            LOG_EXT_E("Module %s connect id %d send failed!", module->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        sent_size += curr_size;

        os_task_mdelay(10);
    }

__exit:

    gm190_unlock(&gm190->netconn_lock);

    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s connect id %d send %d bytes data failed", module->name, netconn->connect_id, size);
        return 0;
    }

    return sent_size;
}



static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;
	  mo_object_t *module   = os_container_of(parser, mo_object_t, parser);
	  mo_gm190_t *gm190     = os_container_of(module, mo_gm190_t, parent);

    sscanf(data, "+ZIPSENDRAW: %d,%d", &connect_id, &data_size);

    os_int32_t curr_connect = gm190->curr_connect;

    if (data_size > 0)
    {
        os_event_send(&gm190->netconn_evt, SET_EVENT(curr_connect, GM190_EVENT_SEND_OK));
    }
    else
    {
        os_event_send(&gm190->netconn_evt, SET_EVENT(curr_connect, GM190_EVENT_SEND_FAIL));
    }
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t   connect_id                        = 0;
    os_int32_t   data_size                         = 0;

    sscanf(data, "+ZIPRECV: %d,%*[^,],%*d,%d,",&connect_id, &data_size);

    LOG_EXT_I("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = gm190_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        LOG_EXT_E("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    if (netconn->stat == NETCONN_STAT_CONNECT)
    {
        /*  bufflen >= strsize + 1 */
        char *recv_buff = calloc(1, data_size  + 1);
        if (recv_buff == OS_NULL)
        {
            LOG_EXT_E("Calloc recv buff %d bytes fail, no enough memory", data_size  + 1);
            return;
        }

        /* Get receive data to receive buffer */
        /* Alert! if using sscanf stores strings, be rember allocating enouth memory! */
        sscanf(data, "+ZIPRECV:%*d,%*[^,],%*d,%*d,%[^,]", recv_buff);

        char *recv_str = calloc(1, data_size + 1);
        if (recv_str == OS_NULL)
        {
            LOG_EXT_E("Calloc recv str %d bytes fail, no enough memory", data_size + 1);
            return;
        }
        mo_netconn_data_recv_notice(netconn, recv_str, data_size);

        free(recv_buff);
    }
    return;
}
static void urc_state_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id    = 0;
    os_int32_t connect_state = 0;

    sscanf(data, "+ZIPSTAT: %d,%d", &connect_id, &connect_state);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
	
    mo_gm190_t  *gm190  = os_container_of(module, mo_gm190_t, parent);

    mo_netconn_t *netconn = gm190_get_netconn_by_id(module, connect_id);

	switch (g_stat_urc)
	{
    case CHECK_STAT:
		if(connect_state == 0)
		{
			os_event_send(&gm190->netconn_evt, GM190_EVENT_STAT_OK);
		}
		else
		{
		
			os_event_send(&gm190->netconn_evt, GM190_EVENT_STAT_FAIL);
		}
		break;

	case CONNECT_STAT:
		if(connect_state == 1)
		{
			os_event_send(&gm190->netconn_evt, SET_EVENT(connect_id, GM190_EVENT_CONN_OK));
		}
		else
		{
            os_event_send(&gm190->netconn_evt, SET_EVENT(connect_id, GM190_EVENT_CONN_FAIL));
		}
		break;

	case CLOSE_STAT:
		if (NETCONN_STAT_CONNECT == netconn->stat)
        {
            LOG_EXT_W("Module %s receive close urc data of connect %d", module->name, connect_id);

            mo_netconn_pasv_close_notice(netconn);
        }
		if(connect_state == 0)
		{
			os_event_send(&gm190->netconn_evt, SET_EVENT(connect_id, GM190_EVENT_CLOSE_OK));
		}
		break;


    default:
		mo_netconn_pasv_close_notice(netconn);
        break;			
	}

	g_stat_urc = INIT;
}


static at_urc_t gs_urc_table[] = {
    {.prefix = "+ZIPSTAT:",    .suffix = "\r\n",           .func = urc_state_func},
    {.prefix = "+ZIPSENDRAW:", .suffix = "\r\n",           .func = urc_send_func},
    {.prefix = "+ZIPRECV:",    .suffix = "\r\n",           .func = urc_recv_func},
};

static void gm190_network_init(mo_object_t *module)
{
    at_parser_t *parser        = &module->parser;
    os_int32_t   enable_num    = 0;
    os_int32_t   reg_state     = 0;
    os_err_t     result        = OS_ERROR;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};
		
	result = at_parser_exec_cmd(parser, &resp, "AT+CREG?");
    if (result != OS_EOK)
    {
        goto __exit;
    }
    if (at_resp_get_data_by_kw(&resp, "+CREG:", "+CREG: %d,%d", &enable_num , &reg_state) < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }
    if (1 == reg_state || 5 == reg_state)
    {
        result = OS_EOK;
        goto __exit;
    }

    resp.timeout = 10 * OS_TICK_PER_SECOND;

    if (at_parser_exec_cmd(parser, &resp, "AT+CGDCONT=1,\"IPV4V6\",\"cmnet\"") != OS_EOK)
    {
        result = OS_ERROR;
        goto __exit;

    }

    if (at_parser_exec_cmd(parser, &resp, "AT+CFUN=0") != OS_EOK)
    {
        result = OS_ERROR;
        goto __exit;

    }	

	if (at_parser_exec_cmd(parser, &resp, "AT+CFUN=1") != OS_EOK)
    {
        result = OS_ERROR;
        goto __exit;
    }    

	result = at_parser_exec_cmd(parser, &resp, "AT+CREG?");
    if (result != OS_EOK)
    {
        goto __exit;
    }
    if (at_resp_get_data_by_kw(&resp, "+CREG:", "+CREG: %d,%d", &enable_num , &reg_state) < 0)
    {
        goto __exit;
    }
    if (1 == reg_state || 5 == reg_state)
    {
        result = OS_EOK;
        goto __exit;
    }
	else
	{
        result = OS_ERROR;
        goto __exit;
	}


__exit:
    if(result != OS_EOK)
    {
        LOG_EXT_W("GM190 network init failed");
    }
    else
    {
        LOG_EXT_W("GM190 network init sucess");
    }
}

void gm190_netconn_init(mo_gm190_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 1; i < GM190_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    gm190_network_init(&module->parent);

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* GM190_USING_NETCONN_OPS */


