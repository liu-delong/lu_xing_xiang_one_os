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
 * @file        cmiot_hal.h
 *
 * @brief       The hal header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_HAL_H__
#define __CMIOT_HAL_H__

#include "cmiot_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(CMIOT_BOOTLOADER)
#if defined(CMIOT_ONEOS)
#else
#include "cmiot_at_device.h"
#endif
#include "cmiot_http.h"
#include "cmiot_coap.h"
#include "cmiot_md5.h"
#include "fal_part.h"
#endif

#if defined(CMIOT_AUTO_TEST_FOTA)
#include <stdlib.h>
#endif

#define CMIOT_MAX_DELAY    (cmiot_uint32)0xFFFFFFFFU
#define CMIOT_TOKEN_MAXLEN 16

#if (CMIOT_HAL_PROTOCOL_MAX_LEN < 256)
#define CMIOT_HAL_DATA_MAX_LEN 256
#else
#define CMIOT_HAL_DATA_MAX_LEN CMIOT_HAL_PROTOCOL_MAX_LEN
#endif

#define CMIOT_DOWNLOAD_MD5_LEN 16
#if defined(CMIOT_SLIM_RES)
#define CMIOT_DOWNLOAD_HOST_MAX_LEN 32
#define CMIOT_DOWNLOAD_URI_MAX_LEN  80
#else
#define CMIOT_DOWNLOAD_HOST_MAX_LEN 64
#define CMIOT_DOWNLOAD_URI_MAX_LEN  128
#endif

#define CMIOT_AT_IP_MAXLEN 4

#if (CMIOT_DOWNLOAD_HOST_MAX_LEN < 32)
#define CMIOT_BUF_MAX_LEN 32
#else
#define CMIOT_BUF_MAX_LEN CMIOT_DOWNLOAD_HOST_MAX_LEN
#endif

#define CMIOT_SN_VER_MAXLEN 4

#if defined(CMIOT_SLIM_RES) && ((CMIOT_KEY_VERSION == 2) || (CMIOT_KEY_VERSION == 3))
#define CMIOT_HASH_MAXLEN 4
#else
#define CMIOT_HASH_MAXLEN 32
#endif

typedef enum
{
    CMIOT_FLASH_OPERATION_READ,
    CMIOT_FLASH_OPERATION_WRITE,
    CMIOT_FLASH_OPERATION_ERASE,
    CMIOT_FLASH_OPERATION_END
} cmiot_flash_operation_t;

#if defined(CMIOT_AUTO_TEST_FOTA)
#define CMIOT_AUTO_TEST_FOTA_COUNT   4
#define CMIOT_AUTO_TEST_FOTA_VER_LEN 8

typedef enum
{
    CMIOT_AUTO_TEST_MODULE_ERROR_TYPE_1,
    CMIOT_AUTO_TEST_MODULE_ERROR_TYPE_END
} cmiot_auto_test_module_error_t;

typedef enum
{
    CMIOT_AUTO_TEST_NET_ERROR_TYPE_MD5_ERROR,
    CMIOT_AUTO_TEST_NET_ERROR_TYPE_DELTA_WRITE_ERROR,
    CMIOT_AUTO_TEST_NET_ERROR_TYPE_NO_RESPOND,
    CMIOT_AUTO_TEST_NET_ERROR_TYPE_COMMUNICATION_FAILURE,
    CMIOT_AUTO_TEST_NET_ERROR_TYPE_END = CMIOT_AUTO_TEST_NET_ERROR_TYPE_COMMUNICATION_FAILURE + 16
} cmiot_auto_test_net_error_t;

typedef struct
{
    cmiot_uint32 addr;
    cmiot_uint   size;
    cmiot_int    fail_count;
} CMIOT_ALIGN(1) test_flash_t;

typedef struct
{
    cmiot_char   ver[CMIOT_AUTO_TEST_FOTA_VER_LEN];
    cmiot_int    cv_succ_count;
    cmiot_int    update_fail_count;
    cmiot_int    rpt_succ_count;
    cmiot_uint8  module_error[CMIOT_AUTO_TEST_MODULE_ERROR_TYPE_END];
    cmiot_uint8  net_error[CMIOT_AUTO_TEST_NET_ERROR_TYPE_END];
    test_flash_t operation[CMIOT_FLASH_OPERATION_END];
} CMIOT_ALIGN(1) cmiot_test_t;

typedef struct
{
    cmiot_uint8  mid[CMIOT_MID_MAXLEN + 1];
    cmiot_uint8  device_id[CMIOT_DEVICEID_MAX_LEN + 1];
    cmiot_uint8  device_secret[CMIOT_DEVICESECRET_MAX_LEN + 1];
    cmiot_int    auto_test_count;
    cmiot_int    auto_test_error_count;
    cmiot_int    auto_test_cv_succ_count;
    cmiot_int    auto_test_rpt_succ_count;
    cmiot_int    error_code;
    cmiot_char   current_ver[CMIOT_AUTO_TEST_FOTA_VER_LEN];
    cmiot_test_t auto_test[CMIOT_AUTO_TEST_FOTA_COUNT];
} CMIOT_ALIGN(1) cmiot_autotest_t;
#endif

typedef struct
{
    cmiot_uint8  inited : 4;
    cmiot_uint8  server_no : 4;
    cmiot_uint16 update_result;
    cmiot_uint8  mid[CMIOT_MID_MAXLEN + 1];
    cmiot_uint8  code[32];
    cmiot_uint8  device_id[CMIOT_DEVICEID_MAX_LEN + 1];
    cmiot_uint8  device_secret[CMIOT_DEVICESECRET_MAX_LEN + 1];
    cmiot_uint   index;
    cmiot_uint   index_max;
    cmiot_uint32 delta_id;
    cmiot_char   download_md5[CMIOT_DOWNLOAD_MD5_LEN + 1];
    cmiot_uint   default_module;
#if defined(CMIOT_WIFI_SSID_PWD)
    cmiot_char wifi_ssid[CMIOT_WIFI_SSID_MAXLEN + 1];
    cmiot_char wifi_pwd[CMIOT_WIFI_PWD_MAXLEN + 1];
#endif
#if defined(CMIOT_AUTO_TEST_FOTA)
    cmiot_autotest_t autotest;
#endif
} CMIOT_ALIGN(1) cmiot_update_t;

typedef struct
{
    cmiot_char download_host[CMIOT_DOWNLOAD_HOST_MAX_LEN + 1];
#if !defined(CMIOT_SLIM_RES)
    cmiot_char bkup_host[CMIOT_DOWNLOAD_HOST_MAX_LEN + 1];
#endif
    cmiot_uint8  download_host_ip[CMIOT_AT_IP_MAXLEN + 1];
    cmiot_char   download_url[CMIOT_DOWNLOAD_URI_MAX_LEN + 1];
    cmiot_char   download_md5[CMIOT_DOWNLOAD_MD5_LEN + 1];
    cmiot_uint32 download_delta_id;
    cmiot_uint32 download_delta_size;
    cmiot_uint   download_port;
} CMIOT_ALIGN(1) download_uri_t;

typedef enum
{
    CMIOT_RESULT_FAIL = 0,
    CMIOT_RESULT_CV_SUCC,
    CMIOT_RESULT_CV_FAIL,
    CMIOT_RESULT_CV_NO_NEW,
    CMIOT_RESULT_RPT_SUCC,
    CMIOT_RESULT_RPT_FAIL,
    CMIOT_RESULT_PROGRESS,
    CMIOT_RESULT_END,
} cmiot_result_t;

typedef enum
{
    CMIOT_HAL_FLAG_START,
    CMIOT_HAL_FLAG_1   = 1,
    CMIOT_HAL_FLAG_2   = 2,
    CMIOT_HAL_FLAG_4   = 4,
    CMIOT_HAL_FLAG_8   = 8,
    CMIOT_HAL_FLAG_ALL = CMIOT_HAL_FLAG_1 | CMIOT_HAL_FLAG_2 | CMIOT_HAL_FLAG_4 | CMIOT_HAL_FLAG_8,
    /* do not add message id at here */
} cmiot_hal_flag_t;

typedef struct
{
    cmiot_uint8 result;
    cmiot_uint  state;
} CMIOT_ALIGN(1) cmiot_action_result_t;

typedef void (*cmiot_msg_cb)(void *ptr);
typedef void (*cmiot_state_result)(cmiot_uint8 state);
cmiot_extern cmiot_action_result_t cmiot_action_result;
cmiot_extern volatile cmiot_bool   cmiot_update_enable;
cmiot_extern volatile cmiot_uint32 cmiot_tick;

cmiot_extern cmiot_msg_cb cmiot_app_msg_cb;
cmiot_extern cmiot_msg_cb cmiot_atp_msg_cb;

cmiot_extern void cmiot_delay(cmiot_uint32 delay);

cmiot_extern void cmiot_inc_tick(void);

#ifndef CMIOT_BOOTLOADER
cmiot_extern void cmiot_at_uart_recv(cmiot_uint8 ch);
#ifdef CMIOT_USART_DEBUG_AT
cmiot_extern void cmiot_debug_uart_recv(cmiot_uint8 ch);
#endif
#endif

cmiot_extern cmiot_uint32 cmiot_get_tick(void);
cmiot_extern void         cmiot_hal_parse_dns(cmiot_char *host, cmiot_uint8 *ip);

cmiot_extern download_uri_t *cmiot_get_download_atp_uri(void);
cmiot_extern void            cmiot_hal_update_device(cmiot_uint8 *mid,
                                                     cmiot_int16  mid_len,
                                                     cmiot_uint8 *device_id,
                                                     cmiot_int16  device_id_len,
                                                     cmiot_uint8 *device_secret,
                                                     cmiot_int16  device_secret_len);
cmiot_extern cmiot_int cmiot_hal_flash_read(cmiot_uint8 type, cmiot_uint32 addr, cmiot_uint8 *buf, cmiot_uint size);
cmiot_extern cmiot_int cmiot_hal_flash_write(cmiot_uint8 type, cmiot_uint32 addr, cmiot_uint8 *buf, cmiot_uint size);
cmiot_extern cmiot_int cmiot_hal_flash_erase(cmiot_uint8 type, cmiot_uint32 addr, cmiot_uint size);
cmiot_uint32           cmiot_hal_get_addr(cmiot_uint8 type, cmiot_uint32 addr);
cmiot_uint             cmiot_hal_get_blocksize(void);
cmiot_uint             cmiot_hal_get_true_blocksize(cmiot_uint8 type);
cmiot_uint32           cmiot_hal_get_info_addr(void);
cmiot_uint32           cmiot_hal_get_backup_addr(void);
cmiot_uint32           cmiot_hal_get_app_addr(void);
cmiot_uint32           cmiot_hal_get_delta_addr(void);
cmiot_uint32           cmiot_hal_get_delta_size(void);
cmiot_uint32           cmiot_download_size(void);
cmiot_extern cmiot_uint8 *cmiot_get_hal_data(void);
cmiot_extern void         cmiot_reset_hal_data(void);
cmiot_extern cmiot_uint   cmiot_get_hal_data_len(void);
cmiot_extern cmiot_bool   cmiot_hal_write_delta(cmiot_uint16 index, cmiot_uint8 *data, cmiot_uint16 len);
cmiot_extern cmiot_bool   cmiot_hal_erase_sector(cmiot_uint8 type, cmiot_uint32 addr);
cmiot_extern cmiot_bool   cmiot_hal_get_inited(void);
cmiot_extern void         cmiot_hal_delta_write(cmiot_bool inited, cmiot_bool succ);

cmiot_extern void cmiot_uart_recved(void);
cmiot_extern void cmiot_hal_app_msg(void *ptr);
cmiot_extern void cmiot_hal_start(cmiot_msg_cb app_msg_cb, cmiot_msg_cb atp_msg_cb, cmiot_state_result state_result);
cmiot_extern cmiot_bool  cmiot_hal_started(void);
cmiot_extern void        cmiot_hal_stop(void);
void                     cmiot_display_update(cmiot_bool enable);
cmiot_extern cmiot_uint  cmiot_get_download_index(void);
cmiot_extern cmiot_uint  cmiot_get_download_index_max(void);
cmiot_extern cmiot_uint  cmiot_md5_calc_result;
cmiot_extern cmiot_uint  download_index;
cmiot_extern cmiot_uint  download_index_max;
cmiot_extern cmiot_uint8 cmiot_current_token[CMIOT_TOKEN_MAXLEN];
cmiot_extern cmiot_uint8 cmiot_current_token_len;
cmiot_extern cmiot_int8  cmiot_get_conn_try_count(void);
cmiot_extern void        cmiot_set_conn_try_count(cmiot_int8);
cmiot_extern cmiot_uint8 *cmiot_get_str_ip(cmiot_uint8 *ip);
cmiot_extern cmiot_update_t *cmiot_hal_init_update(void);
cmiot_extern cmiot_update_t *cmiot_hal_rst_fota(void);
cmiot_extern cmiot_update_t *cmiot_hal_get_update(void);
cmiot_extern cmiot_bool      cmiot_hal_set_update(cmiot_update_t *cmiot_update);

#if defined(CMIOT_BOOTLOADER)
#else
#if defined(CMIOT_WIFI_SSID_PWD)
cmiot_bool   cmiot_hal_check_ssid_pwd(cmiot_char *ssid, cmiot_uint16 ssid_len, cmiot_char *pwd, cmiot_uint16 pwd_len);
#endif
cmiot_bool   cmiot_hal_get_state(void);
cmiot_char * cmiot_get_server_host(void);
cmiot_bool   cmiot_hal_para_url(cmiot_char *url, cmiot_uint16 url_len);
cmiot_uint * cmiot_get_host_port(void);
cmiot_bool   cmiot_hal_para_http(cmiot_uint8 *data, cmiot_http_parameter_t *tmp, cmiot_uint8 tmplen);
cmiot_uint   cmiot_md5_calc(void);
cmiot_int    cmiot_md5_calc_internal(cmiot_uint buflen, cmiot_char *md5out);
cmiot_int8   cmiot_hal_get_current_state(void);
cmiot_uint8 *cmiot_hal_get_download_host_ip(void);
cmiot_uint32 cmiot_hal_get_download_delta_id(void);
cmiot_uint32 cmiot_hal_get_download_delta_size(void);
cmiot_char * cmiot_hal_get_download_url(void);
cmiot_char * cmiot_hal_get_download_host(void);
cmiot_uint32 cmiot_hal_get_delta_id(void);
cmiot_uint8 *cmiot_hal_get_device_id(void);
cmiot_uint8 *cmiot_hal_get_device_secret(void);
#endif

cmiot_int    cmiot_hal_get_key_version(void);
cmiot_bool   cmiot_hal_check_key_version(void);
cmiot_bool   cmiot_hal_server_support_key(void);
cmiot_uint8  cmiot_hal_get_server_no(void);
cmiot_uint8 *cmiot_hal_get_mid(void);
cmiot_uint16 cmiot_hal_get_update_result(void);
void         cmiot_hal_set_update_result(cmiot_uint16 result);
void         cmiot_hal_upgrade_success(void);
void         cmiot_hal_upgrade_fail(void);
#if defined(CMIOT_ONEOS)
fal_part_t *cmiot_hal_get_device(cmiot_uint8 type);
#endif

#ifdef __cplusplus
}
#endif

#endif
