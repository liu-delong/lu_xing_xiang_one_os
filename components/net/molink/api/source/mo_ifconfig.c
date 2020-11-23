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
 * @file        mo_ifconfig.c
 *
 * @brief       module link kit ifconfig api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mo_ifconfig.h"

#define DBG_EXT_TAG "molink.ifconfig"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_IFCONFIG_OPS

static mo_ifconfig_ops_t *get_ifconfig_ops(mo_object_t *self)
{
    mo_ifconfig_ops_t *ops = (mo_ifconfig_ops_t *)self->ops_table[MODULE_OPS_IFCONFIG];

    if (OS_NULL == ops)
    {
        LOG_EXT_E("Module %s does not support ifconfig operates", self->name);
    }

    return ops;
}

os_err_t mo_ifconfig(mo_object_t *self)
{
    OS_ASSERT(OS_NULL != self); 

    mo_ifconfig_ops_t *ops = get_ifconfig_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->ifconfig)
    {
        LOG_EXT_E("Module %s does not support ifconfig operate", self->name);
        return OS_ERROR;
    }

    return ops->ifconfig(self);
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

    mo_ifconfig_ops_t *ops = get_ifconfig_ops(self);

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

    mo_ifconfig_ops_t *ops = get_ifconfig_ops(self);

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
    OS_ASSERT(OS_NULL != dns);

    mo_ifconfig_ops_t *ops = get_ifconfig_ops(self);

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

#endif /* MOLINK_USING_IFCONFIG_OPS */
