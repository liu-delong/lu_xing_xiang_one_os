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
 * @file        mo_wifi.c
 *
 * @brief       module link kit wifi api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_wifi.h"

#include <stdio.h>
#include <stdlib.h>

#define DBG_EXT_TAG "molink.wifi"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_WIFI_OPS

static mo_wifi_ops_t *get_wifi_ops(mo_object_t *self)
{
    mo_wifi_ops_t *ops = (mo_wifi_ops_t *)self->ops_table[MODULE_OPS_WIFI];

    if (OS_NULL == ops)
    {
        LOG_EXT_E("Module %s does not support wifi operates", self->name);
    }

    return ops;
}

os_err_t mo_wifi_set_mode(mo_object_t *module, mo_wifi_mode_t mode)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(mode > MO_WIFI_MODE_NULL && mode < MO_WIFI_MODE_MAX);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_mode)
    {
        LOG_EXT_E("Module %s does not support set wifi module mode operate", module->name);
        return OS_ERROR;
    }

    return ops->set_mode(module, mode);
}

mo_wifi_mode_t mo_wifi_get_mode(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return MO_WIFI_MODE_NULL;
    }

    if (OS_NULL == ops->get_mode)
    {
        LOG_EXT_E("Module %s does not support get wifi module mode operate", module->name);
        return MO_WIFI_MODE_NULL;
    }

    return ops->get_mode(module);
}

mo_wifi_stat_t mo_wifi_get_stat(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return MO_WIFI_STAT_NULL;
    }

    if (OS_NULL == ops->get_stat)
    {
        LOG_EXT_E("Module %s does not support get wifi module state operate", module->name);
        return MO_WIFI_STAT_NULL;
    }

    return ops->get_stat(module);
}

os_err_t mo_wifi_scan_info(mo_object_t *module, char *ssid, mo_wifi_scan_result_t *scan_result)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != scan_result);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->scan_info)
    {
        LOG_EXT_E("Module %s does not support scan wifi ap info operate", module->name);
        return OS_ERROR;
    }

    return ops->scan_info(module, ssid, scan_result);
}

void mo_wifi_scan_info_free(mo_wifi_scan_result_t *scan_result)
{
    OS_ASSERT(OS_NULL != scan_result);

    free(scan_result->info_array);
}

os_err_t mo_wifi_connect_ap(mo_object_t *module, const char *ssid, const char *password)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != ssid);
    OS_ASSERT(OS_NULL != password);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->connect_ap)
    {
        LOG_EXT_E("Module %s does not support connect wifi ap operate", module->name);
        return OS_ERROR;
    }

    return ops->connect_ap(module, ssid, password);
}

#endif /* MOLINK_USING_WIFI_OPS */
