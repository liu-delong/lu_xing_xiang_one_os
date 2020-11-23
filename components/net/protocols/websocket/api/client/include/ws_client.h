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
 * @file        ws_client.h
 *
 * @brief       The websocket client API header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __WS_CLIENT_H__
#define __WS_CLIENT_H__

#ifdef NET_USING_WEBSOCKET_CLIENT

#include "core/include/ws_def.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int        ws_client_create(const char *url);
int        ws_client_event_register(int fd, struct ws_event *event);
int        ws_client_start(int fd, os_uint8_t time_out, char *header);
int        ws_client_send(int fd, os_uint8_t *data, os_uint16_t len, ws_data_type_t type);
int        ws_client_get_by_url(const char *url);
void       ws_client_destroy(int fd);
void       ws_client_stop(int fd, os_uint16_t code);
char      *ws_client_get_default_header(int fd);
ws_state_t ws_client_get_state(int fd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NET_USING_WEBSOCKET_CLIENT */

#endif /* __WS_CLIENT_H__ */
