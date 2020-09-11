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
 * @file        mo_factory.c
 *
 * @brief       module link kit factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_factory.h"

#define DBG_EXT_TAG "module.factory"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_M5310A
#include "m5310a.h"
#endif

#ifdef MOLINK_USING_M5311
#include "m5311.h"
#endif

#ifdef MOLINK_USING_EC200X
#include "ec200x.h"
#endif

#ifdef MOLINK_USING_ESP8266
#include "esp8266.h"
#endif

#ifdef MOLINK_USING_ML302
#include "ml302.h"
#endif

#ifdef MOLINK_USING_BC95
#include "bc95.h"
#endif

#ifdef MOLINK_USING_BC28
#include "bc28.h"
#endif

static const mo_create_fn gs_mo_create_t[] = {
#ifdef MOLINK_USING_M5310A
    [MODULE_TYPE_M5310A] = module_m5310a_create,
#endif
#ifdef MOLINK_USING_M5311
    [MODULE_TYPE_M5311] = module_m5311_create,
#endif
#ifdef MOLINK_USING_EC200X
    [MODULE_TYPE_EC200X] = module_ec200x_create,
#endif
#ifdef MOLINK_USING_ESP8266
    [MODULE_TYPE_ESP8266] = module_esp8266_create,
#endif
#ifdef MOLINK_USING_ML302
	[MODULE_TYPE_ML302] = module_ml302_create,
#endif
#ifdef MOLINK_USING_BC95
    [MODULE_TYPE_BC95] = module_bc95_create,
#endif
#ifdef MOLINK_USING_BC28
    [MODULE_TYPE_BC28] = module_bc28_create,
#endif
    [MODULE_TYPE_MAX] = OS_NULL,
};

static const mo_destory_fn gs_mo_destroy_t[] = {
#ifdef MOLINK_USING_M5310A
    [MODULE_TYPE_M5310A] = module_m5310a_destroy,
#endif
#ifdef MOLINK_USING_M5311
    [MODULE_TYPE_M5311] = module_m5311_destroy,
#endif
#ifdef MOLINK_USING_EC200X
    [MODULE_TYPE_EC200X] = module_ec200x_destroy,
#endif
#ifdef MOLINK_USING_ESP8266
    [MODULE_TYPE_ESP8266] = module_esp8266_destroy,
#endif
#ifdef MOLINK_USING_ML302
	[MODULE_TYPE_ML302] = module_ml302_destroy,
#endif
#ifdef MOLINK_USING_BC95
    [MODULE_TYPE_BC95] = module_bc95_destroy,
#endif
#ifdef MOLINK_USING_BC28
    [MODULE_TYPE_BC28] = module_bc28_destroy,
#endif
    [MODULE_TYPE_MAX] = OS_NULL,
};

mo_object_t *mo_create(const char *name, mo_type_t type, os_device_t *device, os_size_t recv_len)
{
    OS_ASSERT(OS_NULL != name);
    OS_ASSERT(OS_NULL != device);

    if (type <= MODULE_TYPE_NULL || type >= MODULE_TYPE_MAX)
    {
        LOG_EXT_E("Failed to create module object, module type error");
        return OS_NULL;
    }

    if (OS_NULL == gs_mo_create_t[type])
    {
        LOG_EXT_E("The system did not find the create function for the module %s", name);
        return OS_NULL;
    }

    return gs_mo_create_t[type](name, device, recv_len);
}

os_err_t mo_destroy(mo_object_t *self, mo_type_t type)
{
    OS_ASSERT(OS_NULL != self);

    if (type <= MODULE_TYPE_NULL || type >= MODULE_TYPE_MAX)
    {
        LOG_EXT_E("Failed to destroy module object, module type error");
        return OS_ERROR;
    }

    if (OS_NULL == gs_mo_destroy_t[type])
    {
        LOG_EXT_E("The system did not find the destroy function for the module %s", self->name);
        return OS_ERROR;
    }

    return gs_mo_destroy_t[type](self);
}
