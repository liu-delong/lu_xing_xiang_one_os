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
 * @file        mo_general.h
 *
 * @brief       module link kit general api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_GENERAL_H__
#define __MO_GENERAL_H__

#include "mo_type.h"
#include "mo_object.h"

#ifdef MOLINK_USING_GENERAL_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 ***********************************************************************************************************************
 * @struct      mo_general_ops_t
 *
 * @brief       molink module general ops table
 ***********************************************************************************************************************
 */
typedef struct mo_general_ops
{
    os_err_t (*at_test)(mo_object_t *self);
    os_err_t (*get_imei)(mo_object_t *self, char *value, os_size_t len);
    os_err_t (*get_imsi)(mo_object_t *self, char *value, os_size_t len);
    os_err_t (*get_iccid)(mo_object_t *self, char *value, os_size_t len);
    os_err_t (*get_cfun)(mo_object_t *self, os_uint8_t *fun_lvl);
    os_err_t (*set_cfun)(mo_object_t *self, os_uint8_t fun_lvl);
    os_err_t (*soft_reset)(mo_object_t *self);
    os_err_t (*clear_stored_earfcn)(mo_object_t *self);
    os_err_t (*get_app_version)(mo_object_t *self, char *value, os_size_t len);
} mo_general_ops_t;

os_err_t mo_at_test(mo_object_t *self);
os_err_t mo_get_imei(mo_object_t *self, char *value, os_size_t len);
os_err_t mo_get_imsi(mo_object_t *self, char *value, os_size_t len);
os_err_t mo_get_iccid(mo_object_t *self, char *value, os_size_t len);
os_err_t mo_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl);
os_err_t mo_set_cfun(mo_object_t *self, os_uint8_t fun_lvl);
os_err_t mo_soft_reset(mo_object_t *self);
os_err_t mo_clear_stored_earfcn(mo_object_t *self);
os_err_t mo_get_app_version(mo_object_t *self, char *value, os_size_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_GENERAL_OPS */

#endif /* __MO_GENERAL_H__ */
