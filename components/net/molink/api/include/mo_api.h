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
 * @file        mo_api.h
 *
 * @brief       module link kit api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_API_H__
#define __MO_API_H__

#include "mo_common.h"

#include "mo_factory.h"

#ifdef MOLINK_USING_GENERAL_OPS
#include "mo_general.h"
#endif

#ifdef MOLINK_USING_NETSERV_OPS
#include "mo_netserv.h"
#endif

#ifdef MOLINK_USING_NETCONN_OPS
#include "mo_netconn.h"
#endif

#ifdef MOLINK_USING_SOCKETS_OPS
#include "mo_socket.h"
#endif

#ifdef MOLINK_USING_ONENET_NB_OPS
#include "mo_onenet_nb.h"
#endif

#ifdef MOLINK_USING_WIFI_OPS
#include "mo_wifi.h"
#endif

#endif /* __MO_API_H__ */
