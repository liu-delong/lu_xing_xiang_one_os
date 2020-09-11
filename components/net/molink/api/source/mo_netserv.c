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

os_err_t mo_set_netstat(mo_object_t *self, os_uint8_t stat)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_netstat)
    {
        LOG_EXT_E("Module %s does not support set netstat operate", self->name);
        return OS_ERROR;
    }

    return ops->set_netstat(self, stat);
}

os_err_t mo_get_netstat(mo_object_t *self, os_uint8_t *stat)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != stat);  

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_netstat)
    {
        LOG_EXT_E("Module %s does not support get netstat operate", self->name);
        return OS_ERROR;
    }

    return ops->get_netstat(self, stat);
}

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
