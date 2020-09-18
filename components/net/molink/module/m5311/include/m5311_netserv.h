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
 * @file        m5311_netserv.h
 *
 * @brief       m5311 module link kit netservice api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __M5311_NETSERV_H__
#define __M5311_NETSERV_H__

#include "mo_netserv.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef M5311_USING_NETSERV_OPS

os_err_t m5311_set_attach(mo_object_t *self, os_uint8_t attach_stat);
os_err_t m5311_get_attach(mo_object_t *self, os_uint8_t *attach_stat);
os_err_t m5311_set_reg(mo_object_t *self, os_uint8_t reg_n);
os_err_t m5311_get_reg(mo_object_t *self, os_uint8_t *reg_n, os_uint8_t *reg_stat);
os_err_t m5311_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat);
os_err_t m5311_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat);
os_err_t m5311_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber);
os_err_t m5311_get_radio(mo_object_t *self, radio_info_t *radio_info);
os_err_t m5311_get_ipaddr(mo_object_t *self, char ip[]);
os_err_t m5311_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp);

#endif /* M5311_USING_NETSERV_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __M5311_NETSERV_H__ */
