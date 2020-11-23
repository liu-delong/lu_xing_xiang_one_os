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
 * @file        cmiot_hmd5.h
 *
 * @brief       The hmd5 header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_HMD5_H__
#define __CMIOT_HMD5_H__

#define CMIOT_HMD5_LEN 34

#include "cmiot_typedef.h"
#include "cmiot_md5.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MD5_DIGESTSIZE
#define MD5_DIGESTSIZE 16
#endif

#ifndef MD5_BLOCKSIZE
#define MD5_BLOCKSIZE 64
#endif

typedef struct
{
    cmiot_md5_ctx_t ictx;
    cmiot_md5_ctx_t octx;
    cmiot_char      imd[MD5_DIGESTSIZE];
    cmiot_char      omd[MD5_DIGESTSIZE];
    cmiot_char      buf[MD5_BLOCKSIZE];
} cmiot_hmd5_ctx_t;

cmiot_extern cmiot_char *cmiot_hmd5_pid_psec_mid(cmiot_char *device_id, cmiot_char *device_secret, cmiot_char *mid);

#ifdef __cplusplus
}
#endif

#endif
