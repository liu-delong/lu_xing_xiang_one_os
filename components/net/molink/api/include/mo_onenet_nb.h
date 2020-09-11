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
 * @file        mo_onenet_nb.h
 *
 * @brief       module link kit onenet nb api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_ONENET_H__
#define __MO_ONENET_H__

#include "mo_object.h"
#include <oneos_config.h>

#ifdef MOLINK_USING_ONENET_NB_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_IPV6
#define NBCONFIG_IPADDR_MAX_STR_LEN     (63)
#else
#define NBCONFIG_IPADDR_MAX_STR_LEN     (15)
#endif

#define NBCONFIG_AUTHCODE_MAX_STR_LEN   (16)
#define NBCONFIG_PSK_MAX_STR_LEN        (16)

typedef struct mo_config_resp
{
    os_bool_t   guide_mode_enable;
    os_int8_t   ip[NBCONFIG_IPADDR_MAX_STR_LEN + 1];
    os_uint16_t port;
    os_uint8_t  rsp_timeout;
    os_bool_t   obs_autoack_enable;
    os_bool_t   auth_enable;
    os_int8_t   auth_code[NBCONFIG_AUTHCODE_MAX_STR_LEN + 1];
    os_bool_t   dtls_enable;
    os_int8_t   psk[NBCONFIG_PSK_MAX_STR_LEN + 1];
    os_uint8_t  write_format;
    os_uint8_t  buf_cfg;
    os_uint8_t  buf_urc_mode;
} mo_config_resp_t;

typedef struct
{
    os_uint32_t ref;
    os_uint32_t mid;
    os_uint32_t objid;
    os_uint32_t insid;
    os_uint32_t resid;
    os_uint32_t type;
    os_uint32_t len;
    os_int8_t   *value;
} module_mgr_resp_t;

#define ONENET_NB_FUNC_ARGS (mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)

typedef struct mo_onenet_ops
{
#define DEFINE_ONENET_OPT_FUNC(NAME, ARGS) os_err_t (*NAME) ARGS
    DEFINE_ONENET_OPT_FUNC(onenetnb_get_config, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_set_config, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_create, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_createex, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_addobj, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_discoverrsp, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_nmi, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_open, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_notify, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_update, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_get_write, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_writersp, ONENET_NB_FUNC_ARGS);
#ifdef OS_USING_SHELL
    DEFINE_ONENET_OPT_FUNC(onenetnb_all, ONENET_NB_FUNC_ARGS);
#endif
} mo_onenet_ops_t;

/* return ref */
os_err_t mo_onenetnb_get_config(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_set_config(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_create(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_createex(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_addobj(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_discoverrsp(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_nmi(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_open(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_notify(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_update(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_get_write(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_writersp(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
#ifdef OS_USING_SHELL
os_err_t mo_onenetnb_all(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...);
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#endif /* MOLINK_USING_ONENET_NB_OPS */

#endif /* __MO_ONENET_H__ */
