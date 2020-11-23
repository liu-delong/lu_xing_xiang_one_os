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
 * @file        ws_common.h
 *
 * @brief       The websocket common api header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __WS_COMMON_H__
#define __WS_COMMON_H__

#include <sys/socket.h>
#include "core/include/ws_def.h"

#ifdef NET_USING_WEBSOCKET_CLIENT

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int  base64_encode(os_uint8_t *in, os_size_t in_len, os_uint8_t *out, os_size_t out_len);
int  base64_decode(os_uint8_t *in, os_size_t in_len, os_uint8_t *out, os_size_t out_len);
int  ws_url_parsing(const char *url, os_bool_t *secure, struct in_addr *sin_addr, os_uint16_t *sin_port);
int  ws_en_package(os_uint8_t    *in,
                   os_size_t      in_len,
                   os_uint8_t    *out,
                   os_size_t      out_len,
                   os_bool_t      mask,
                   ws_data_type_t type);
int  ws_de_package(os_uint8_t *data, os_size_t len, ws_data_type_t *type);
int  ws_send_ping_packet(int socket, os_uint8_t *buf, os_size_t len);
int  ws_generate_respond_shake_key(os_uint8_t *in, os_size_t in_len, os_uint8_t *out, os_size_t out_len);
void ws_get_random_string(os_uint8_t *buf, os_size_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NET_USING_WEBSOCKET_CLIENT */

#endif /* __WS_COMMON_H__ */
