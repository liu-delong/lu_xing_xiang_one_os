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

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT test command
 *
 * @param[in]       self            The descriptor of molink module instance
 *
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          AT test successfully
 * @retval          OS_ERROR        AT test error
 * @retval          OS_ETIMEOUT     AT test timeout
 ***********************************************************************************************************************
 */
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

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get module imei
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      value           The buffer to store imei
 * @param[in]       len             The buffer length
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get imei successfully
 * @retval          OS_ERROR        Get imei error
 * @retval          OS_ETIMEOUT     Get imei timeout
 ***********************************************************************************************************************
 */
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

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get module imsi
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      value           The buffer to store imsi
 * @param[in]       len             The buffer length
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get imsi successfully
 * @retval          OS_ERROR        Get imsi error
 * @retval          OS_ETIMEOUT     Get imsi timeout
 ***********************************************************************************************************************
 */
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

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get module iccid
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      value           The buffer to store iccid
 * @param[in]       len             The buffer length
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get iccid successfully
 * @retval          OS_ERROR        Get iccid error
 * @retval          OS_ETIMEOUT     Get iccid timeout
 ***********************************************************************************************************************
 */
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

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get module functionality level
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      fun_lvl         The buffer to store module functionality level
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module functionality level successfully
 * @retval          OS_ERROR        Get module functionality level error
 * @retval          OS_ETIMEOUT     Get module functionality level timeout
 ***********************************************************************************************************************
 */
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

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to set module functionality level
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       fun_lvl         The functionality level
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set module functionality level successfully
 * @retval          OS_ERROR        Set module functionality level error
 * @retval          OS_ETIMEOUT     Set module functionality level timeout
 ***********************************************************************************************************************
 */
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

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to soft reset module
 *
 * @param[in]       self            The descriptor of molink module instance
 *
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Soft reset successfully
 * @retval          OS_ERROR        Soft reset error
 * @retval          OS_ETIMEOUT     Soft reset timeout
 ***********************************************************************************************************************
 */
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

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to clear stored earfcn
 *
 * @param[in]       self            The descriptor of molink module instance
 *
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          clear stored earfcn successfully
 * @retval          OS_ERROR        clear stored earfcn error
 * @retval          OS_ETIMEOUT     clear stored earfcn timeout
 ***********************************************************************************************************************
 */
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

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to get module firmware app version
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       value           The buffer to store app version
 * @param[in]       len             The buffer length
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module firmware app version successfully
 * @retval          OS_ERROR        Get module firmware app version error
 * @retval          OS_ETIMEOUT     Get module firmware app version timeout
 ***********************************************************************************************************************
 */
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
