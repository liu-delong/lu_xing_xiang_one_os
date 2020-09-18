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
 * @file        mo_netserv.c
 *
 * @brief       module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_netserv.h"

#include <stdio.h>
#include <string.h>

#define DBG_EXT_TAG "molink.nerserv"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_NETSERV_OPS

static mo_netserv_ops_t *get_netserv_ops(mo_object_t *self)
{
    mo_netserv_ops_t *ops = (mo_netserv_ops_t *)self->ops_table[MODULE_OPS_NETSERV];

    if (OS_NULL == ops)
    {
        LOG_EXT_E("Module %s does not support network service operates", self->name);
    }

    return ops;
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to set the state of PS attachment
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       attach_stat     Indicates the state of PS attachment
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set the state of PS attachment successfully
 * @retval          OS_ERROR        Set the state of PS attachment error
 * @retval          OS_ETIMEOUT     Set the state of PS attachment timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_attach)
    {
        LOG_EXT_E("Module %s does not support set attach state operate", self->name);
        return OS_ERROR;
    }

    return ops->set_attach(self, attach_stat);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get the state of PS attachment
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      attach_stat     The buffer to store the state of PS attachment
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get the state of PS attachment successfully
 * @retval          OS_ERROR        Get the state of PS attachment error
 * @retval          OS_ETIMEOUT     Get the state of PS attachment timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(attach_stat != OS_NULL);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_attach)
    {
        LOG_EXT_E("Module %s does not support get attach state operate", self->name);
        return OS_ERROR;
    }

    return ops->get_attach(self, attach_stat);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to set the presentation of an network registration urc data
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       reg_n           The presentation of an network registration urc data
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set the presentation of an network registration urc data successfully
 * @retval          OS_ERROR        Set the presentation of an network registration urc data error
 * @retval          OS_ETIMEOUT     Set the presentation of an network registration urc data timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_reg)
    {
    LOG_EXT_E("Module %s does not support set register state operate", self->name);
    return OS_ERROR;
	}

	return ops->set_reg(self, reg_n);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get the presentation of an network registration urc data and 
 *                  the network registration status
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      reg_n           The buffer to store the presentation of an network registration urc data
 * @param[out]      reg_stat        The buffer to store the network registration status
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get successfully
 * @retval          OS_ERROR        Get error
 * @retval          OS_ETIMEOUT     Get timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_reg(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != reg_n);
    OS_ASSERT(OS_NULL != reg_stat);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_reg)
    {
        LOG_EXT_E("Module %s does not support get register state operate", self->name);
        return OS_ERROR;
    }

    return ops->get_reg(self, reg_n, reg_stat);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to activate or deactivate PDP Context
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       cid             A numeric parameter which specifies a particular PDP context
 * @param[in]       act_stat        Indicates the state of PDP context activation
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Activate or deactivate PDP Context successfully
 * @retval          OS_ERROR        Activate or deactivate PDP Context error
 * @retval          OS_ETIMEOUT     Activate or deactivate PDP Context timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_reg)
    {
        LOG_EXT_E("Module %s does not support set cgact operate", self->name);
        return OS_ERROR;
	}

	return ops->set_cgact(self, cid, act_stat);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get the state of PDP context activation 
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      cid             The buffer to store the cid
 * @param[out]      act_stat        The buffer to store the state of PDP context activation
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get the state of PDP context activation successfully
 * @retval          OS_ERROR        Get the state of PDP context activation error
 * @retval          OS_ETIMEOUT     Get the state of PDP context activation timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != cid);
    OS_ASSERT(OS_NULL != act_stat);
    
    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_cgact)
    {
        LOG_EXT_E("Module %s does not support get cgact state operate", self->name);
        return OS_ERROR;
    }

    return ops->get_cgact(self, cid, act_stat);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get the signal strength indication <rssi> and 
 *                  channel bit error rate <ber> from the ME
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      rssi            The buffer to store the signal strength
 * @param[out]      ber             The buffer to store the channel bit error rate
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get the signal quality successfully
 * @retval          OS_ERROR        Get the signal quality error
 * @retval          OS_ETIMEOUT     Get the signal quality timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != rssi);
    OS_ASSERT(OS_NULL != ber);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_csq)
    {
        LOG_EXT_E("Module %s does not support get signal quality operate", self->name);
        return OS_ERROR;
    }

    return ops->get_csq(self, rssi, ber);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to query UE statistics
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      radio_info      The buffer to store the UE statistics
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get the signal quality successfully
 * @retval          OS_ERROR        Get the signal quality error
 * @retval          OS_ETIMEOUT     Get the signal quality timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_radio(mo_object_t *self, radio_info_t *radio_info)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != radio_info);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_radio)
    {
        LOG_EXT_E("Module %s does not support get radio infomation operate", self->name);
        return OS_ERROR;
    }

    return ops->get_radio(self, radio_info);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get ip address
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      ip              The buffer to store the ip address
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get the ip address successfully
 * @retval          OS_ERROR        Get the ip address error
 * @retval          OS_ETIMEOUT     Get the ip address timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_ipaddr(mo_object_t *self, char ip[])
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != ip);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_ipaddr)
    {
        LOG_EXT_E("Module %s does not support get ipaddr operate", self->name);
        return OS_ERROR;
    }

    return ops->get_ipaddr(self, ip);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to set molink module dns server address
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       dns             The dns server address. @see dns_server_t
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set dns address successfully
 * @retval          OS_ETIMEOUT     Set dns address timeout
 * @retval          OS_ERROR        Set dns address error
 ***********************************************************************************************************************
 */
os_err_t mo_set_dnsserver(mo_object_t *self, dns_server_t dns)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_dnsserver)
    {
        LOG_EXT_E("Module %s does not support set dnsserver operate", self->name);
        return OS_ERROR;
    }

    return ops->set_dnsserver(self, dns);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get molink module dns server address
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       dns             The buffer to store dns server address. @see dns_server_t
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get dns address successfully
 * @retval          OS_ETIMEOUT     Get dns address timeout
 * @retval          OS_ERROR        Get dns address error
 ***********************************************************************************************************************
 */
os_err_t mo_get_dnsserver(mo_object_t *self, dns_server_t *dns)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_dnsserver)
    {
        LOG_EXT_E("Module %s does not support set dnsserver operate", self->name);
        return OS_ERROR;
    }

    return ops->get_dnsserver(self, dns);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to ping remote host
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       host            The remote host name
 * @param[in]       len             The ping bytes  
 * @param[in]       timeout         The ping timeout
 * @param[out]      resp            The buffer to store ping response infomation
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Ping successfully
 * @retval          OS_ETIMEOUT     Ping timeout
 * @retval          OS_ERROR        Ping error
 ***********************************************************************************************************************
 */
os_err_t mo_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != host);
    OS_ASSERT(OS_NULL != resp);  

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->ping)
    {
        LOG_EXT_E("Module %s does not support ping operate", self->name);
        return OS_ERROR;
    }

    return ops->ping(self, host, len, timeout, resp);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get cell infomation
 *
 * @param[in]       self              The descriptor of molink module instance
 * @param[out]      onepos_cell_info  The buffer to store cell infomation
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK             Get cell infomation successfully
 * @retval          OS_ETIMEOUT        Get cell infomation timeout
 * @retval          OS_ERROR           Get cell infomation error
 ***********************************************************************************************************************
 */
os_err_t mo_get_cell_info(mo_object_t *self, onepos_cell_info_t* onepos_cell_info)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != onepos_cell_info);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->ping)
    {
        LOG_EXT_E("Module %s does not support ping operate", self->name);
        return OS_ERROR;
    }

    return ops->get_cell_info(self, onepos_cell_info);	
}

#ifdef OS_USING_SHELL

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <mo_common.h>

os_err_t module_get_reg_sh(int argc, char* argv[])
{    
    os_uint8_t reg_n = 0, reg_status = 0;
    if (mo_get_reg(mo_get_default(), &reg_n, &reg_status) == OS_EOK)
    {
        printf("Get reg reg_n:%d, reg_status:%d!\n", reg_n, reg_status);
    }
    
    return OS_EOK;
}
SH_CMD_EXPORT(mo_get_reg, module_get_reg_sh, "Get reg");

#endif /* OS_USING_SHELL */

#endif /* MOLINK_USING_NETSERV_OPS */
