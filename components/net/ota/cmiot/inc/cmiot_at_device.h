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
 * @file        cmiot_at_device.h
 *
 * @brief       The at device header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_AT_DEVICE_H__
#define __CMIOT_AT_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmiot_config.h"

#ifndef CMIOT_BOOTLOADER
#ifndef CMIOT_DEFAULT_HTTP_PROTOCOL
#define CMIOT_DEFAULT_HTTP_PROTOCOL CMIOT_PROTOCOL_NONE
#endif
#ifndef CMIOT_DEFAULT_COAP_PROTOCOL
#define CMIOT_DEFAULT_COAP_PROTOCOL CMIOT_PROTOCOL_NONE
#endif
#ifndef CMIOT_DEFAULT_NETWORK_PROTOCOL
#define CMIOT_DEFAULT_NETWORK_PROTOCOL (CMIOT_DEFAULT_HTTP_PROTOCOL + CMIOT_DEFAULT_COAP_PROTOCOL)
#endif

#if (CMIOT_DEFAULT_NETWORK_PROTOCOL == CMIOT_PROTOCOL_HTTP)
#define CMIOT_UARTRX_MAXLEN CMIOT_HTTP_MAX_LEN
#define CMIOT_UARTTX_MAXLEN (448)
#elif (CMIOT_DEFAULT_NETWORK_PROTOCOL == CMIOT_PROTOCOL_COAP_HTTP)
#if ((CMIOT_COAP_MAX_LEN * 2 + 100) > CMIOT_HTTP_MAX_LEN)
#define CMIOT_UARTRX_MAXLEN (CMIOT_COAP_MAX_LEN * 2 + 100)
#else
#define CMIOT_UARTRX_MAXLEN CMIOT_HTTP_MAX_LEN
#endif
#if (CMIOT_UARTRX_MAXLEN > 512)
#define CMIOT_UARTTX_MAXLEN CMIOT_UARTRX_MAXLEN
#else
#define CMIOT_UARTTX_MAXLEN (400)
#endif
#else
#define CMIOT_UARTRX_MAXLEN (CMIOT_COAP_MAX_LEN * 2 + 100)
#define CMIOT_UARTTX_MAXLEN 512
#endif

#if (CMIOT_DEFAULT_NETWORK_PROTOCOL == CMIOT_PROTOCOL_HTTP)
#define CMIOT_HAL_PROTOCOL_MAX_LEN (300 + CMIOT_DATA_SEQ_MAX_LEN)
#elif (CMIOT_DEFAULT_NETWORK_PROTOCOL == CMIOT_PROTOCOL_COAP_HTTP)
#if (CMIOT_COAP_MAX_LEN > (300 + CMIOT_DATA_SEQ_MAX_LEN))
#define CMIOT_HAL_PROTOCOL_MAX_LEN (CMIOT_COAP_MAX_LEN)
#else
#define CMIOT_HAL_PROTOCOL_MAX_LEN (300 + CMIOT_DATA_SEQ_MAX_LEN)
#endif
#else
#define CMIOT_HAL_PROTOCOL_MAX_LEN (CMIOT_COAP_MAX_LEN)
#endif

#else
#if defined(CMIOT_DEBUG_MODE)
#ifndef CMIOT_FOTA_ENABLE_DIFF_DEBUG
#define CMIOT_FOTA_ENABLE_DIFF_DEBUG
#endif
#ifndef CMIOT_FOTA_ENABLE_DIFF_ERROR
#define CMIOT_FOTA_ENABLE_DIFF_ERROR
#endif
#ifndef CMIOT_BL_PRINT_MAXLEN
#define CMIOT_BL_PRINT_MAXLEN 512
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CMIOT_AT_DEVICE_H__ */
