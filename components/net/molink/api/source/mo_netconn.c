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
 * @file        mo_netconn.c
 *
 * @brief       module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_netconn.h"

#define DBG_EXT_TAG "molink.netconn"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_NETCONN_OPS

static mo_netconn_ops_t *get_netconn_ops(mo_object_t *module)
{
    mo_netconn_ops_t *ops = (mo_netconn_ops_t *)module->ops_table[MODULE_OPS_NETCONN];

    if (OS_NULL == ops)
    {
        LOG_EXT_E("Module %s does not support general operates", module->name);
    }

    return ops;
}

void mo_netconn_data_recv_notice(mo_netconn_t *netconn, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != netconn);
    OS_ASSERT(OS_NULL != data);

    if (netconn->stat == NETCONN_STAT_CONNECT)
    {
        os_data_queue_push(&netconn->data_queue, data, size, OS_IPC_WAITING_FOREVER);
        
#ifdef MOLINK_USING_SOCKETS_OPS  
        if (OS_NULL != netconn->evt_func)
        {
            netconn->evt_func(netconn, MO_NETCONN_EVT_RCVPLUS, size);
        }
#endif
    }
    else
    {
        LOG_EXT_E("Netconn id %d receive state error", netconn->connect_id);
    }

    return;  
}

/**
 * tcp/udp_close urc recv, which means socket closed by peer or something exception happend
 */
void mo_netconn_pasv_close_notice(mo_netconn_t *netconn)
{
    OS_ASSERT(OS_NULL != netconn);
    
    netconn->stat = NETCONN_STAT_CLOSE;
    
#ifdef MOLINK_USING_SOCKETS_OPS
    if (OS_NULL != netconn->evt_func)
    {
        // when socket closed, notice recv & error event
        netconn->evt_func(netconn, MO_NETCONN_EVT_RCVPLUS, 0);
        netconn->evt_func(netconn, MO_NETCONN_EVT_ERROR, 0);
    }
#endif

    os_data_queue_reset(&netconn->data_queue);

    return;
}

os_err_t mo_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != info);

    mo_netconn_ops_t *ops = get_netconn_ops(module);

    if (OS_NULL == ops)
    {
        return OS_EOK;
    }

    if (OS_NULL == ops->get_info)
    {
        LOG_EXT_E("Module %s does not support get netconn infomation operate", module->name);
        return OS_ERROR;
    }

    return ops->get_info(module, info);
}

/**
 ***********************************************************************************************************************
 * @brief           Create an instance of a molink module netconn object
 *
 * @param[in]       module          The descriptor of molink module instance 
 * @param[in]       type            The type of molink module netconn object. @ref mo_netconn_type_t
 * 
 * @return          On success, return a molink module instance netconn descriptor; 
 *                  On error, OS_NULL is returned.
 ***********************************************************************************************************************
 */
mo_netconn_t *mo_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(NETCONN_TYPE_NULL != type);

    mo_netconn_ops_t *ops = get_netconn_ops(module);
    
    if (OS_NULL == ops)
    {
        return OS_NULL;
    }

    if (OS_NULL == ops->create)
    {
        LOG_EXT_E("Module %s does not support create netconn operate", module->name);
        return OS_NULL;
    }

    return ops->create(module, type);
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy an instance of a molink module netconn object
 *
 * @param[in]       module          The descriptor of molink module instance 
 * @param[in]       netconn         The descriptor of molink module netconn instance
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Destroy failed
 * @retval          OS_EOK          Destroy successfully
 ***********************************************************************************************************************
 */
os_err_t mo_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != netconn);

    if (NETCONN_STAT_NULL ==  netconn->stat)
    {
        LOG_EXT_E("Module %s netconn id %d connect state %d error!", module->name, netconn->connect_id, netconn->stat);
        return OS_ERROR;
    }

    mo_netconn_ops_t *ops = get_netconn_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->destroy)
    {
        LOG_EXT_E("Module %s does not support destroy netconn operate", module->name);
        return OS_ERROR;
    }

    return ops->destroy(module, netconn);
}

/**
 ***********************************************************************************************************************
 * @brief           Get IP address by domain name
 *
 * @param[in]       module          The descriptor of molink module instance 
 * @param[in]       domain_name     The domain name
 * @param[out]      addr            The to store ip address
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Get IP address failed
 * @retval          OS_ETIMEOUT     Get IP address timeout
 * @retval          OS_EOK          Get IP address successfully
 ***********************************************************************************************************************
 */
os_err_t mo_netconn_gethostbyname(mo_object_t *module, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != addr);
    OS_ASSERT(OS_NULL != domain_name);

    mo_netconn_ops_t *ops = get_netconn_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (inet_addr(domain_name) != IPADDR_NONE)
    {
        LOG_EXT_D("Resolve input is valid IP address, no need to resolve.\n");
        inet_aton(domain_name, addr);
        return OS_EOK;
    }

    if (OS_NULL == ops->gethostbyname)
    {
        LOG_EXT_E("Module %s does not support gethostbyname operate", module->name);
        return OS_ERROR;
    }

    return ops->gethostbyname(module, domain_name, addr);
}

/**
 ***********************************************************************************************************************
 * @brief           Connect a molink module netconn to a specific remote IP address and port
 *
 * @param[in]       module          The descriptor of molink module instance
 * @param[in]       netconn         The descriptor of molink module netconn instance 
 * @param[in]       addr            The remote IP address to connect to
 * @param[in]       port            The remote port to connect to
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Connect failed
 * @retval          OS_ETIMEOUT     Connect timeout
 * @retval          OS_EOK          Connect successfully
 ***********************************************************************************************************************
 */
os_err_t mo_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != netconn);

    if (netconn->stat != NETCONN_STAT_INIT)
    {
        LOG_EXT_E("Module %s netconn id %d connect state %d error!", module->name, netconn->connect_id, netconn->stat);
        return OS_ERROR;
    }

    mo_netconn_ops_t *ops = get_netconn_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->connect)
    {
        LOG_EXT_E("Module %s does not support connect operate", module->name);
        return OS_ERROR;
    }

    return ops->connect(module, netconn, addr, port);
}

/**
 ***********************************************************************************************************************
 * @brief           Send data over a TCP or UDP molink module netconn (that is already connected)
 *
 * @param[in]       module          The descriptor of molink module instance
 * @param[in]       netconn         The descriptor of molink module netconn instance 
 * @param[in]       data            The buffer containing the data to send
 * @param[in]       size            The size of the data to send
 * 
 * @return          The length of data successfully sent
 ***********************************************************************************************************************
 */
os_size_t mo_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != netconn);
    OS_ASSERT(OS_NULL != data);

    if (netconn->stat != NETCONN_STAT_CONNECT)
    {
        LOG_EXT_E("Module %s netconn %d does not connect to server, send failed!", module->name, netconn->connect_id);
        return 0;
    }

    mo_netconn_ops_t *ops = get_netconn_ops(module);

    if (OS_NULL == ops)
    {
        return 0;
    }

    if (OS_NULL == ops->send)
    {
        LOG_EXT_E("Module %s does not support send operate", module->name);
        return 0;
    }

    return ops->send(module, netconn, (char *)data, size);
}

/**
 ***********************************************************************************************************************
 * @brief           Receive data from a molink module netconn
 *
 * @param[in]       module          The descriptor of molink module instance
 * @param[in]       netconn         The descriptor of molink module netconn instance 
 * @param[out]      data            The secondary pointer to data item to store data
 * @param[out]      size            The size of recved data
 * @param[in]       timeout         The timeout to receive data
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Receive data failed
 * @retval          OS_ETIMEOUT     Receive data timeout
 * @retval          OS_EOK          Receive data successfully
 ***********************************************************************************************************************
 */
os_err_t mo_netconn_recv(mo_object_t *module, mo_netconn_t *netconn, void **data, os_size_t *size, os_tick_t timeout)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != netconn);
    OS_ASSERT(OS_NULL != data);
    OS_ASSERT(OS_NULL != size);

    if (netconn->stat != NETCONN_STAT_CONNECT)
    {
        LOG_EXT_E("Module %s netconn %d does not connect to server, recv failed!", module->name, netconn->connect_id);
        return OS_ERROR;
    }

    os_err_t result = os_data_queue_pop(&netconn->data_queue, (const void **)data, size, timeout);

    if (OS_ETIMEOUT == result)
    {
        LOG_EXT_E("Module %s netconn %d receive timeout", module->name, netconn->connect_id);
    }

    if (OS_ERROR == result)
    {
        LOG_EXT_E("Module %s netconn %d receive error", module->name, netconn->connect_id);
    }
    
#ifdef MOLINK_USING_SOCKETS_OPS  
    if (OS_EOK == result)
    {    
        if (OS_NULL!= netconn->evt_func)
        {
            netconn->evt_func(netconn, MO_NETCONN_EVT_RCVMINUS, *size);
        }
    }
#endif

    return result;
}

#endif /* MOLINK_USING_NETCONN_OPS */
