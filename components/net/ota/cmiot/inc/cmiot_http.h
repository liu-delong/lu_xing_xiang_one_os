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
 * @file        cmiot_http.h
 *
 * @brief       The http header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_HTTP_H__
#define __CMIOT_HTTP_H__

#include "cmiot_typedef.h"
#include "cmiot_hal_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    cmiot_uint8  type[32];
    cmiot_uint16 content_len;
    cmiot_uint8 *content;
} CMIOT_ALIGN(1) cmiot_http_parameter_t;
cmiot_extern cmiot_int cmiot_http_content_len;

cmiot_extern void cmiot_fota_make_json_request(cmiot_int8 state, cmiot_char *ptr, cmiot_uint len);
cmiot_extern cmiot_int8 *cmiot_make_http_data(cmiot_uint8 state);
cmiot_extern cmiot_bool  cmiot_parse_http_data(cmiot_char *            data,
                                               cmiot_http_parameter_t *http_parameter,
                                               cmiot_uint16            count);
cmiot_extern cmiot_char *cmiot_is_http_data(cmiot_char *data, cmiot_uint16 len);
cmiot_extern cmiot_uint  cmiot_http_callback(cmiot_uint8 state, cmiot_char *data, cmiot_uint len);
void                     cmiot_http_get_new_version(void);
cmiot_char *             cmiot_get_http_server_host(void);
void                     cmiot_http_report_result(void);
cmiot_extern             cmiot_char *
                         cmiot_get_signptr(cmiot_char *mid, cmiot_char *product_id, cmiot_char *product_secret, cmiot_uint32 utc_time);
cmiot_extern cmiot_uint8 *cmiot_get_common_data(void);
cmiot_extern cmiot_uint   cmiot_get_common_data_len(void);
cmiot_extern cmiot_bool   cmiot_get_domain(cmiot_char *server,
                                           cmiot_char *domain,
                                           cmiot_int16 domain_len,
                                           cmiot_uint *port);

#ifdef __cplusplus
}
#endif

#endif
