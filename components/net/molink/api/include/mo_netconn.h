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
 * @file        mo_netconn.h
 *
 * @brief       module link kit netconnect api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_NETCONN_H__
#define __MO_NETCONN_H__

#include "mo_type.h"
#include "mo_ipaddr.h"
#include "mo_object.h"

#include <extend/os_dataqueue.h>

#ifdef MOLINK_USING_NETCONN_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum mo_netconn_stat
{
    NETCONN_STAT_NULL = 0, /* netconn has not been created */
    NETCONN_STAT_INIT,     /* netconn was created but not connected */
    NETCONN_STAT_CONNECT,  /* netconn connect OK*/
    NETCONN_STAT_CLOSE,    /* netconn was closed but all connect info were not deleted */
    NETCONN_STAT_UNDEFINED /* netconn undefined status */
} mo_netconn_stat_t;

typedef enum mo_netconn_type
{
    NETCONN_TYPE_NULL = 0,
    NETCONN_TYPE_TCP,
    NETCONN_TYPE_UDP,
} mo_netconn_type_t;

typedef struct mo_netconn
{
#ifdef MOLINK_USING_SOCKETS_OPS
    os_int32_t socket_id;
#endif
    os_int32_t connect_id;

    mo_netconn_stat_t stat;
    mo_netconn_type_t type;
    
    ip_addr_t   remote_ip;
    os_uint16_t remote_port;

    os_data_queue_t data_queue;
} mo_netconn_t;

typedef struct mo_netconn_ops
{
    mo_netconn_t *(*create)(mo_object_t *module, mo_netconn_type_t type);
    os_err_t      (*destroy)(mo_object_t *module, mo_netconn_t *netconn);
    os_err_t      (*connect)(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port);
    os_size_t     (*send)(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size);
    os_err_t      (*gethostbyname)(mo_object_t *self, const char *domain_name, ip_addr_t *addr);
} mo_netconn_ops_t;

mo_netconn_t *mo_netconn_create(mo_object_t *module, mo_netconn_type_t type);
os_err_t      mo_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn);
os_err_t      mo_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port);
os_size_t     mo_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size);
os_err_t      mo_netconn_recv(mo_object_t *module,
                              mo_netconn_t *netconn,
                              void **data,
                              os_size_t *size,
                              os_tick_t timeout);
os_err_t      mo_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_NETCONN_OPS */

#endif /* __MO_NETCONN_H__ */
