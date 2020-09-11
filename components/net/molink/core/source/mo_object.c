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
 * @file        mo_object.c
 *
 * @brief       module link kit object api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DBG_EXT_TAG "molink.core"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef NET_USING_MOLINK

static os_slist_node_t gs_mo_object_list    = {0};
static mo_object_t    *gs_mo_object_default = OS_NULL;

static void mo_object_list_add(mo_object_t *self)
{
    os_base_t level = os_hw_interrupt_disable();

    os_slist_init(&(self->list));

    if (OS_NULL == gs_mo_object_default)
    {
        gs_mo_object_default = self;
    }

    /* tail insertion */
    os_slist_add_tail(&(gs_mo_object_list), &(self->list));

    os_hw_interrupt_enable(level);
}

static void mo_object_list_del(mo_object_t *self)
{
    OS_ASSERT(self != OS_NULL);

    os_slist_node_t *node  = OS_NULL;
    mo_object_t     *entry = OS_NULL;

    os_base_t level = os_hw_interrupt_disable();

    for (node = &gs_mo_object_list; node; node = os_slist_next(node))
    {
        entry = os_slist_entry(node, mo_object_t, list);
        if (entry == self)
        {
            os_slist_del(&(gs_mo_object_list), &(self->list));

            if (gs_mo_object_default == self)
            {
                gs_mo_object_default = OS_NULL;
            }
            break;
        }
    }

    os_hw_interrupt_enable(level);
}

static mo_object_t *module_object_get_by_device(os_device_t *device)
{
    OS_ASSERT(device != OS_NULL);

    os_slist_node_t *node  = OS_NULL;
    mo_object_t     *entry = OS_NULL;

    if (OS_NULL == gs_mo_object_list.next)
    {
        return OS_NULL;
    }

    os_base_t level = os_hw_interrupt_disable();

    for (node = &gs_mo_object_list; node; node = os_slist_next(node))
    {
        entry = os_slist_entry(node, mo_object_t, list);
        if (entry && entry->parser.device == device)
        {
            os_hw_interrupt_enable(level);
            return entry;
        }
    }

    os_hw_interrupt_enable(level);

    return OS_NULL;
}

mo_object_t *mo_object_get_by_name(const char *name)
{
    OS_ASSERT(name != OS_NULL);

    os_slist_node_t *node  = OS_NULL;
    mo_object_t     *entry = OS_NULL;

    if (OS_NULL == gs_mo_object_list.next)
    {
        return OS_NULL;
    }

    os_base_t level = os_hw_interrupt_disable();

    for (node = &gs_mo_object_list; node; node = os_slist_next(node))
    {
        entry = os_slist_entry(node, mo_object_t, list);
        if (entry && (strncmp(entry->name, name, OS_NAME_MAX) == 0))
        {
            os_hw_interrupt_enable(level);
            return entry;
        }
    }

    os_hw_interrupt_enable(level);

    return OS_NULL;
}

mo_object_t *mo_object_get_default(void)
{
    if (OS_NULL == gs_mo_object_default)
    {
        LOG_EXT_E("There are no default module in the system now");
    }

    return gs_mo_object_default;
}

void mo_object_set_default(mo_object_t *self)
{
    OS_ASSERT(self != OS_NULL);

    gs_mo_object_default = self;
}

os_err_t mo_object_init(mo_object_t *self, const char *name, os_device_t *device, os_size_t recv_len)
{
    OS_ASSERT(name != OS_NULL);
    OS_ASSERT(device != OS_NULL);

    os_err_t     result = OS_EOK;
	mo_object_t *temp   = OS_NULL;

    if (strlen(name) == 0 || mo_object_get_by_name(name) != OS_NULL)
    {
        LOG_EXT_E("Failed init module object, module name error");
        result = OS_ERROR;
        goto __exit;
    }

    temp = module_object_get_by_device(device);
    if (temp != OS_NULL)
    {
        LOG_EXT_E("Failed init module object, device %s has occupied by the module %s",
                  device->parent.name,
                  temp->name);
        result = OS_ERROR;
        goto __exit;
    }

    memset(self, 0, sizeof(mo_object_t));

    result = at_parser_init(&self->parser, name, device, recv_len);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module object create parser failed!");
        goto __exit;
    }

    strncpy(self->name, name, OS_NAME_MAX);

    mo_object_list_add(self);

__exit:
    if (result != OS_EOK)
    {
        return result;
    }
    else
    {
        at_parser_startup(&self->parser);

        return OS_EOK;
    }
}

os_err_t mo_object_deinit(mo_object_t *self)
{
    OS_ASSERT(self != OS_NULL);

    at_parser_deinit(&self->parser);

    mo_object_list_del(self);

    return OS_EOK;
}

#endif /* NET_USING_MOLINK */
