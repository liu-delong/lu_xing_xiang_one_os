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
 * @file        mo_netserv.h
 *
 * @brief       module link kit netservice api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_NETSERV_H__
#define __MO_NETSERV_H__

#include "mo_type.h"
#include "mo_object.h"

#ifdef MOLINK_USING_IP
#include <mo_ipaddr.h>
#endif /* MOLINK_USING_IP */

#ifdef MOLINK_USING_NETSERV_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CELL_ID_MAX_LEN (28)
#define IP_SIZE         (16)
#define MIN_IP_SIZE     (7)

typedef enum network_reg_state
{
    NOT_REG = 0,
    REG_HOME_NETWORK,
    SEARCHING_FOR_OPERATOR,
    REG_DENIED,
    UNKNOWN_NETWORK_STATE,
    REG_ROAMING,
    REG_SMS_ONLY_HOME_NETWORK,
    REG_SMS_ONLY_ROAMING,
    UNDEF_NET_REG_STATE
} network_reg_state_t;

typedef enum mo_net_state
{
    MO_NET_DETACH = 0,         /* module detach from PS */
    MO_NET_ATTACH,             /* module attach to PS */
    MO_NET_EPS_REG_FAIL,       /* module reg EPS fail */
    MO_NET_EPS_REG_OK,         /* module reg EPS OK */
    MO_NET_DEACTIVATED,        /* module deactivate PDP context */
    MO_NET_ACTIVATED,          /* module activate PDP context */
    MO_NET_NETWORK_REG_OK,     /* module get IP address successfully indicates network registration OK */
    MO_NET_NETWORK_REG_FAIL,   /* module fail to get IP address indicates network registration fail */
    MO_NET_UNDEFINE_STATE
} mo_net_state_t;

typedef enum ps_attach_state
{
    DETACHED = 0,
    ATTACHED,
    UNDEF_ATTACH_STATE
} ps_attach_state_t;

typedef enum pdp_context_state
{
    DEACTIVATED = 0,
    ACTIVATED,
    UNDEF_CGACT_STATE
} pdp_context_state_t;

typedef struct radio_info
{
    char cell_id[CELL_ID_MAX_LEN + 1];

    os_int32_t ecl;
    os_int32_t snr;
    os_int32_t earfcn;
    os_int32_t signal_power;
    os_int32_t rsrq;
} radio_info_t;

typedef struct dns_server
{
    char primary_dns[IP_SIZE];
    char secondary_dns[IP_SIZE];
} dns_server_t;

typedef struct ping_resp
{
    ip_addr_t   ip_addr;   /* response IP address */
    os_uint16_t data_len;  /* response data length */
    os_uint16_t ttl;       /* time to live */
    os_uint32_t time;      /* response time, unit ms */
    void       *user_data; /* user-specific data */
} ping_resp_t;


typedef struct{
    os_uint32_t mnc;
    os_uint32_t mcc;
    os_uint32_t lac;
    os_uint32_t cid;
    os_int32_t  ss;
    // os_uint32_t ta;
}cell_info_t;

typedef struct{
    os_uint32_t  cell_num;
    os_uint8_t   net_type;
    cell_info_t *cell_info;
}onepos_cell_info_t;

typedef struct mo_netserv_ops
{
    os_err_t (*set_attach)(mo_object_t *self, os_uint8_t attach_stat);
    os_err_t (*get_attach)(mo_object_t *self, os_uint8_t *attach_stat);
    os_err_t (*set_reg)(mo_object_t *self, os_uint8_t reg_n);
    os_err_t (*get_reg)(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat);
    os_err_t (*set_cgact)(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat);
    os_err_t (*get_cgact)(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat);
    os_err_t (*get_csq)(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber);
    os_err_t (*get_radio)(mo_object_t *self, radio_info_t *radio_info);
    os_err_t (*get_ipaddr)(mo_object_t *self, char ip[]);
    os_err_t (*set_netstat)(mo_object_t *self, os_uint8_t stat);
    os_err_t (*get_netstat)(mo_object_t *self, os_uint8_t *stat);
    os_err_t (*set_dnsserver)(mo_object_t *self, dns_server_t dns);
    os_err_t (*get_dnsserver)(mo_object_t *self, dns_server_t *dns);
    os_err_t (*ping)(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp);
	  os_err_t (*get_cell_info)(mo_object_t *self, onepos_cell_info_t* onepos_cell_info);
} mo_netserv_ops_t;

os_err_t mo_set_attach(mo_object_t *self, os_uint8_t attach_stat);
os_err_t mo_get_attach(mo_object_t *self, os_uint8_t *attach_stat);
os_err_t mo_set_reg(mo_object_t *self, os_uint8_t reg_n);
os_err_t mo_get_reg(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat);
os_err_t mo_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat);
os_err_t mo_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat);
os_err_t mo_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber);
os_err_t mo_get_radio(mo_object_t *self, radio_info_t *radio_info);
os_err_t mo_get_ipaddr(mo_object_t *self, char ip[]);
os_err_t mo_set_netstat(mo_object_t *self, os_uint8_t stat);
os_err_t mo_get_netstat(mo_object_t *self, os_uint8_t *stat);
os_err_t mo_set_dnsserver(mo_object_t *self, dns_server_t dns);
os_err_t mo_get_dnsserver(mo_object_t *self, dns_server_t *dns);
os_err_t mo_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp);
os_err_t mo_get_cell_info(mo_object_t *self, onepos_cell_info_t* onepos_cell_info);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_NETSERV_OPS */

#endif /* __MO_NETSERV_H__ */
