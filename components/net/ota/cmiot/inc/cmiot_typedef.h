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
 * @file        cmiot_typedef.h
 *
 * @brief       The typedef header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_TYPEDEF_H__
#define __CMIOT_TYPEDEF_H__

#include "cmiot_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define cmiot_extern extern

/* get data size by index */
#define CMIOT_DATA_SEQ_MAX_LEN (1 << (CMIOT_DEFAULT_SEGMENT_SIZE_INDEX + 4))
#define CMIOT_HTTP_HEAD_LEN    640
#define CMIOT_HTTP_MAX_LEN     (CMIOT_DATA_SEQ_MAX_LEN + CMIOT_HTTP_HEAD_LEN)
#define CMIOT_COAP_MAX         (CMIOT_DATA_SEQ_MAX_LEN + 32)
#define CMIOT_COAP_MIN         256

#if (CMIOT_COAP_MAX < CMIOT_COAP_MIN)
#define CMIOT_COAP_MAX_LEN CMIOT_COAP_MIN
#else
#define CMIOT_COAP_MAX_LEN CMIOT_COAP_MAX
#endif

#if defined(__GNUC__)
#ifndef cmiot_weak
#define cmiot_weak __attribute__((weak))
#endif                             /* cmiot_weak */
#define CMIOT_ASM           __asm  /* !< asm keyword for GNU Compiler */
#define CMIOT_INLINE        inline /* !< inline keyword for GNU Compiler */
#define CMIOT_STATIC_INLINE static inline
#define CMIOT_ALIGN(x)      __attribute__((aligned(x)))
#elif defined(__ICCARM__) || defined(__ICCRX__) || defined(__ICCSTM8__)
#ifndef cmiot_weak
#define cmiot_weak __weak
#endif                             /* cmiot_weak */
#define CMIOT_ASM           __asm  /* !< asm keyword for IAR Compiler */
#define CMIOT_INLINE        inline /* !< inline keyword for IAR Compiler. Only available in High optimization mode! */
#define CMIOT_STATIC_INLINE static inline
#define CMIOT_ALIGN(x)      __attribute__((aligned(x)))
#elif defined(__CC_ARM)
#ifndef cmiot_weak
#define cmiot_weak __weak
#endif                               /* cmiot_weak */
#define CMIOT_ASM           __asm    /* !< asm keyword for ARM Compiler */
#define CMIOT_INLINE        __inline /* !< inline keyword for ARM Compiler */
#define CMIOT_STATIC_INLINE static __inline
#define CMIOT_ALIGN(x)      __align(x)
#else
#error "Alignment not supported for this compiler."
#endif

typedef enum
{
    CMIOT_FILETYPE_APP,
    CMIOT_FILETYPE_PATCH,
    CMIOT_FILETYPE_BACKUP,
    CMIOT_FILETYPE_PATCH_INFO,
    CMIOT_FILETYPE_END
} cmiot_peer_filetype_t;

#define CMIOTSW32(x)                                                                                                   \
    ((((cmiot_uint32)(x) & (cmiot_uint32)0x000000ff) << 24) | (((cmiot_uint32)(x) & (cmiot_uint32)0x0000ff00) << 8) |  \
     (((cmiot_uint32)(x) & (cmiot_uint32)0x00ff0000) >> 8) | (((cmiot_uint32)(x) & (cmiot_uint32)0xff000000) >> 24))

#define CMIOT_UPDATE_SUCCESS   1
#define CMIOT_UPDATE_AUTH_FAIL 98
#define CMIOT_UPDATE_FAIL      99
#define CMIOT_NO_UPDATE_RESULT 1000

#define CMIOT_MID_MAXLEN           31
#define CMIOT_DEVICEID_MAX_LEN     31
#define CMIOT_DEVICESECRET_MAX_LEN 63
#define CMIOT_CRC_MAXLEN           4
#define CMIOT_VERSION_MAXLEN       4
#define CMIOT_SN_DATETIME_MAXLEN   22

#if defined(CMIOT_WIFI_SSID_PWD)
#define CMIOT_WIFI_SSID_MAXLEN 32
#define CMIOT_WIFI_PWD_MAXLEN  32
#endif

typedef char               cmiot_char;
typedef unsigned short     cmiot_wchar;
typedef unsigned char      cmiot_uint8;
typedef signed char        cmiot_int8;
typedef unsigned short int cmiot_uint16;
typedef signed short int   cmiot_int16;
typedef unsigned int       cmiot_uint;
typedef signed int         cmiot_int;
typedef unsigned long      cmiot_ulong;
typedef signed long        cmiot_long;
typedef unsigned long long cmiot_ull;
typedef signed long long   cmiot_ll;

#ifdef CMIOT_8BIT
typedef unsigned long      cmiot_uint32;
typedef signed long        cmiot_int32;
typedef unsigned long long cmiot_uint64;
typedef signed long long   cmiot_int64;
#else
typedef unsigned int       cmiot_uint32;
typedef signed int         cmiot_int32;
typedef unsigned long long cmiot_uint64;
typedef signed long long   cmiot_int64;
#endif

typedef unsigned int cmiot_size_t;
typedef signed int   cmiot_intptr_t;
typedef unsigned int cmiot_uintptr_t;

typedef enum
{
    STATE_INIT = 1,
    STATE_CV, /* Check Version */
    STATE_DL, /* Download*/
    STATE_RD, /* Report download result*/
    STATE_UG,
    STATE_RU, /* Report upgrade result */
    STATE_KY,
    STATE_RG, /* Register */
    STATE_END
} flow_state_t;

#define E_CMIOT_SUCCESS      0
#define E_CMIOT_FAILURE      -1
#define E_CMIOT_NOMEMORY     -2
#define E_CMIOT_NOT_INITTED  -3
#define E_CMIOT_LAST_VERSION -4
#define E_CMIOT_NO_UPGRADE   -5

typedef enum
{
    CMIOT_DIFF_PATCH,
    CMIOT_FULL_PATCH,
    CMIOT_PATCH_END
} cmiot_patch_method_t;

typedef cmiot_uint8 cmiot_bool;
#define cmiot_true  ((cmiot_bool)1)
#define cmiot_false ((cmiot_bool)0)

cmiot_char *cmiot_get_manufacturer(void);
cmiot_char *cmiot_get_model_number(void);
cmiot_char *cmiot_get_product_id(void);
cmiot_char *cmiot_get_product_sec(void);
cmiot_char *cmiot_get_device_type(void);
cmiot_char *cmiot_get_platform(void);
cmiot_char *cmiot_get_sdk_version(void);
cmiot_char *cmiot_get_apk_version(void);
cmiot_char *cmiot_get_firmware_version(void);
cmiot_char *cmiot_get_hw_version(void);
cmiot_char *cmiot_get_sw_version(void);
cmiot_int32 cmiot_get_down_start_time(void);
cmiot_int32 cmiot_get_down_end_time(void);
cmiot_uint  cmiot_get_default_module(void);
cmiot_bool  cmiot_set_default_module(cmiot_uint moudule);
cmiot_uint8 cmiot_get_default_protocol(void);
cmiot_uint8 cmiot_get_default_segment_size(void);
cmiot_uint  cmiot_get_data_max_len(void);
cmiot_char *cmiot_get_buf(void);
void        cmiot_reset_buf(void);
cmiot_uint  cmiot_get_buf_len(void);

#ifdef __cplusplus
}
#endif

#endif
