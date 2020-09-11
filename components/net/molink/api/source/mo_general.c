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
 * @file        mo_general.c
 *
 * @brief       module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_general.h"

#define DBG_EXT_TAG "molink.general"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_GENERAL_OPS

static mo_general_ops_t *get_general_ops(mo_object_t *self)
{
    mo_general_ops_t *ops = (mo_general_ops_t *)self->ops_table[MODULE_OPS_GENERAL];

    if (OS_NULL == ops)
    {
        LOG_EXT_E("Module %s does not support general operates", self->name);
    }

    return ops;
}

os_err_t mo_at_test(mo_object_t *self)
{
    OS_ASSERT(OS_NULL != self);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->at_test)
    {
        LOG_EXT_E("Module %s does not support at test operate", self->name);
        return OS_ERROR;
    }

    return ops->at_test(self);
}

os_err_t mo_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != value);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_imei)
    {
        LOG_EXT_E("Module %s does not support get imei operate", self->name);
        return OS_ERROR;
    }

    return ops->get_imei(self, value, len);
}

os_err_t mo_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != value);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_imsi)
    {
        LOG_EXT_E("Module %s does not support get imsi operate", self->name);
        return OS_ERROR;
    }

    return ops->get_imsi(self, value, len);
}

os_err_t mo_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != value);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_iccid)
    {
        LOG_EXT_E("Module %s does not support get iccid operate", self->name);
        return OS_ERROR;
    }

    return ops->get_iccid(self, value, len);
}

os_err_t mo_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != fun_lvl);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_cfun)
    {
        LOG_EXT_E("Module %s does not support get level of functionality operate", self->name);
        return OS_ERROR;
    }

    return ops->get_cfun(self, fun_lvl);
}

os_err_t mo_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    OS_ASSERT(OS_NULL != self);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_cfun)
    {
        LOG_EXT_E("Module %s does not support set level of functionality operate", self->name);
        return OS_ERROR;
    }

    return ops->set_cfun(self, fun_lvl);
}

os_err_t mo_set_echo(mo_object_t *self, os_bool_t is_echo)
{
    OS_ASSERT(OS_NULL != self);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_echo)
    {
        LOG_EXT_E("Module %s does not support set echo mode operate", self->name);
        return OS_ERROR;
    }

    return ops->set_echo(self, is_echo);
}

os_err_t mo_soft_reset(mo_object_t *self)
{
    OS_ASSERT(OS_NULL != self);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->soft_reset)
    {
        LOG_EXT_E("Module %s does not support set echo mode operate", self->name);
        return OS_ERROR;
    }

    return ops->soft_reset(self);
}

os_err_t mo_clear_stored_earfcn(mo_object_t *self)
{
    OS_ASSERT(OS_NULL != self);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->clear_stored_earfcn)
    {
        LOG_EXT_E("Module %s does not support clear stored earfcn operate", self->name);
        return OS_ERROR;
    }

    return ops->soft_reset(self);
}

os_err_t mo_get_app_version(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(OS_NULL != self);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_app_version)
    {
        LOG_EXT_E("Module %s does not support get application version operate", self->name);
        return OS_ERROR;
    }

    return ops->get_app_version(self, value, len);
}

#endif /* MOLINK_USING_GENERAL_OPS */
