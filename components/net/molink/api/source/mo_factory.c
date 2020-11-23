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
#include <inttypes.h>

#define DBG_EXT_TAG "module.factory"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

/***************************************** Start to list of NB-IoT modules ********************************************/
#ifdef MOLINK_USING_M5310A
#include "m5310a.h"
#endif

#ifdef MOLINK_USING_M5311
#include "m5311.h"
#endif

#ifdef MOLINK_USING_BC95
#include "bc95.h"
#endif

#ifdef MOLINK_USING_BC28
#include "bc28.h"
#endif

#ifdef MOLINK_USING_SIM7020
#include "sim7020.h"
#endif
/***************************************** End of NB-IoT modules list *************************************************/


/*************************************** Start to list of 4G cat1 modules *********************************************/
#ifdef MOLINK_USING_EC200X
#include "ec200x.h"
#endif

#ifdef MOLINK_USING_ML302
#include "ml302.h"
#endif

#ifdef MOLINK_USING_GM190
#include "gm190.h"
#endif

#ifdef MOLINK_USING_A7600X
#include "a7600x.h"
#endif
/**************************************** End of 4G cat1 modules list *************************************************/


/*************************************** Start to list of 4G cat4 modules *********************************************/

#ifdef MOLINK_USING_GM510
#include "gm510.h"
#endif

/**************************************** End of 4G cat4 modules list *************************************************/


/*************************************** Start to list of wifi modules ************************************************/
#ifdef MOLINK_USING_ESP8266
#include "esp8266.h"
#endif
#ifdef MOLINK_USING_ESP32
#include "esp32.h"
#endif
/***************************************** End of wifi modules list ***************************************************/


static const mo_create_fn gs_mo_create_t[] = {
/* Type of NB-IoT modules */
#ifdef MOLINK_USING_M5310A
    [MODULE_TYPE_M5310A] = module_m5310a_create,
#endif
#ifdef MOLINK_USING_M5311
    [MODULE_TYPE_M5311] = module_m5311_create,
#endif
#ifdef MOLINK_USING_BC95
    [MODULE_TYPE_BC95] = module_bc95_create,
#endif
#ifdef MOLINK_USING_BC28
    [MODULE_TYPE_BC28] = module_bc28_create,
#endif
#ifdef MOLINK_USING_SIM7020
    [MODULE_TYPE_SIM7020] = module_sim7020_create,
#endif

/* Type of 4G cat1 modules */
#ifdef MOLINK_USING_EC200X
    [MODULE_TYPE_EC200X] = module_ec200x_create,
#endif
#ifdef MOLINK_USING_ML302
    [MODULE_TYPE_ML302] = module_ml302_create,
#endif
#ifdef MOLINK_USING_GM190
    [MODULE_TYPE_GM190] = module_gm190_create,
#endif
#ifdef MOLINK_USING_A7600X
    [MODULE_TYPE_A7600X] = module_a7600x_create,
#endif

/* Type of 4G cat4 modules */
#ifdef MOLINK_USING_GM510
    [MODULE_TYPE_GM510] = module_gm510_create,
#endif

/* Type of wifi modules */
#ifdef MOLINK_USING_ESP8266
    [MODULE_TYPE_ESP8266] = module_esp8266_create,
#endif
#ifdef MOLINK_USING_ESP32
    [MODULE_TYPE_ESP32] = module_esp32_create,
#endif

    [MODULE_TYPE_MAX] = OS_NULL,
};

static const mo_destory_fn gs_mo_destroy_t[] = {
/* Type of NB-IoT modules */
#ifdef MOLINK_USING_M5310A
    [MODULE_TYPE_M5310A] = module_m5310a_destroy,
#endif
#ifdef MOLINK_USING_M5311
    [MODULE_TYPE_M5311] = module_m5311_destroy,
#endif
#ifdef MOLINK_USING_BC95
    [MODULE_TYPE_BC95] = module_bc95_destroy,
#endif
#ifdef MOLINK_USING_BC28
    [MODULE_TYPE_BC28] = module_bc28_destroy,
#endif
#ifdef MOLINK_USING_SIM7020
    [MODULE_TYPE_SIM7020] = module_sim7020_destroy,
#endif

/* Type of 4G cat1 modules */
#ifdef MOLINK_USING_EC200X
    [MODULE_TYPE_EC200X] = module_ec200x_destroy,
#endif
#ifdef MOLINK_USING_ML302
    [MODULE_TYPE_ML302] = module_ml302_destroy,
#endif
#ifdef MOLINK_USING_GM190
    [MODULE_TYPE_GM190] = module_gm190_destroy,
#endif
#ifdef MOLINK_USING_A7600X
    [MODULE_TYPE_A7600X] = module_a7600x_destroy,
#endif

/* Type of 4G cat4 modules */
#ifdef MOLINK_USING_GM510
    [MODULE_TYPE_GM510] = module_gm510_destroy,
#endif

/* Type of wifi modules */
#ifdef MOLINK_USING_ESP8266
    [MODULE_TYPE_ESP8266] = module_esp8266_destroy,
#endif
#ifdef MOLINK_USING_ESP32
    [MODULE_TYPE_ESP32] = module_esp32_destroy,
#endif

    [MODULE_TYPE_MAX] = OS_NULL,
};

/**
 ***********************************************************************************************************************
 * @brief           Create an instance of a molink module object
 *
 * @param[in]       name            The molink module instance name
 * @param[in]       type            The type of molink module object. @ref mo_type_t
 * @param[in]       device          The device used by molink module instance at parser
 * @param[in]       recv_len        The receive buffer length of at parser
 * 
 * @return          On success, return a molink module instance descriptor; 
 *                  On error, OS_NULL is returned.
 ***********************************************************************************************************************
 */
mo_object_t *mo_create(const char *name, mo_type_t type, void *parser_config)
{
    OS_ASSERT(OS_NULL != name);

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

    return gs_mo_create_t[type](name, parser_config);
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy an instance of a molink module object
 *
 * @param[in]       self            The molink module instance descriptor
 * @param[in]       type            The type of molink module object. @ref mo_type_t
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Destroy failed
 * @retval          OS_EOK          Destroy successfully
 ***********************************************************************************************************************
 */
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
