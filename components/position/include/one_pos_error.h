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
 * @file        one_pos_error.h
 * 
 * @brief       Function error state
 * 
 * @details     Defines the location service function run error return value and fault tolerant judgment
 * 
 * @revision
 * Date         Author          Notes
 * 2020-07-17   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __ONE_POS_ERROR_H__
#define __ONE_POS_ERROR_H__

#include "one_pos_platform.h"
#include "one_pos_types.h"

/* Location service component error code definitions */
#define OPS_EOK                         0                          /* There is no error */
#define OPS_ENULL                       1                          /* A pointer is empty */
#define OPS_EZERO                       2                          /* The dividend is zero */
#define OPS_ELOWER                      3                          /* Value below the lower limit */
#define OPS_EUPPER                      4                          /* Value greater than upper limit */
#define OPS_ETIMEOUT                    5                          /* wait timeout */
#define OPS_EADD                        6                          /* Error adding element */
#define OPS_ESUB                        7                          /* Error subtract  element */
#define OPS_EMALLOC                     8                          /* Space application failed */
#define OPS_EMEM_CREATE                 9                          /* Memory manager create failed */     
#define OPS_EMEM_ADD                    10                         /* Memory block add failed */
#define OPS_EFUNC                       11                         /* Funtion run error */
#define OPS_EERROR                      12                         /* Other errors */

#if defined __OPS_WINDOWS__  /* Windows platform */
/* Function return value type */
typedef DWORD ops_err_t;

#elif defined __OPS_ONE_OS__ /* OneOS platform */
/* Function return value type */
typedef ops_int_t ops_err_t;

#else
#error Undefined platform
#endif 

#endif /* __ONE_POS_ERROR_H__ */
