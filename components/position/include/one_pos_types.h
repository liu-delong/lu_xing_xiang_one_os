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
 * @file        one_pos_types.h
 * 
 * @brief       Type definition 
 * 
 * @details     The data type definition for the location service
 * 
 * @revision
 * Date         Author          Notes
 * 2020-07-17   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __ONE_POS_TYPES_H__
#define __ONE_POS_TYPES_H__

#include <stdbool.h>
#include "one_pos_platform.h"

#if defined __OPS_WINDOWS__  /* Windows platform */
#include <Windows.h>

/* primary data type */
typedef unsigned short ops_ushort_t;
typedef short          ops_short_t;
typedef unsigned int   ops_uint_t;
typedef int            ops_int_t;
typedef double         ops_float_t;
typedef void           ops_void_t;
typedef char           ops_char_t;
#define OPS_NULL       NULL

#elif defined __OPS_ONE_OS__ /* OneOS platform */
#include <os_types.h>

/* primary data type */
typedef os_uint16_t    ops_ushort_t;
typedef os_int16_t     ops_short_t;
typedef os_uint32_t    ops_uint_t;
typedef os_int32_t     ops_int_t;
typedef double         ops_float_t;
typedef void           ops_void_t;
typedef char           ops_char_t;
#define OPS_NULL       OS_NULL

#else
#error Undefined platform
#endif 

typedef bool           ops_bool_t;
#define ops_false      false;
#define ops_true       true;

#endif /* __ONE_POS_TYPES_H__ */
