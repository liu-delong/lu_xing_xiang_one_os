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
 * @file        mo_object.h
 *
 * @brief       module link kit object definition and api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_OBJECT_H__
#define __MO_OBJECT_H__

#include "at_parser.h"
#include "mo_type.h"

#include <os_kernel.h>

#ifdef NET_USING_MOLINK

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 ***********************************************************************************************************************
 * @struct      mo_object
 *
 * @brief       molink module object
 ***********************************************************************************************************************
 */
typedef struct mo_object
{
    os_slist_node_t list;                      /* module object manage list  */
    char            name[OS_NAME_MAX + 1];     /* module object name */
    at_parser_t     parser;                    /* module object at parser */
    const void     *ops_table[MODULE_OPS_MAX]; /* module object operates table */
} mo_object_t;

os_err_t mo_object_init(mo_object_t *self, const char *name, os_device_t *device, os_size_t recv_len);
os_err_t mo_object_deinit(mo_object_t *self);

mo_object_t *mo_object_get_by_name(const char *name);
mo_object_t *mo_object_get_default(void);
void         mo_object_set_default(mo_object_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NET_USING_MOLINK */

#endif /* __MO_OBJECT_H__ */

