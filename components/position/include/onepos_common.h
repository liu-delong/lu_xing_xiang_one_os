/**
 ***********************************************************************************************************************
 * Copyright (c) China Mobile Communications Group Co.,Ltd.
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
 * @file        pos_common.h
 *
 * @brief
 *
 * @details
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12  OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __POS_COMMON_H__
#define __POS_COMMON_H__

#include <os_types.h>

typedef struct
{
    os_int64_t value;
    os_uint32_t dec_len;
} loca_float_t;

#define tras_loca_float(loca_float) ((double)loca_float.value / pow(10, loca_float.dec_len))

typedef struct
{
    os_uint32_t day;
    os_uint32_t month;
    os_uint32_t year;
} loca_date_t;

typedef struct
{
    os_uint32_t hours;
    os_uint32_t minutes;
    os_uint32_t seconds;
    os_uint32_t microseconds;
} loca_time_t;

#define _isdigit(argv) ((((*argv) - '0') < 10u) && (((*argv) - '0') >= 0u))

#endif /* __POS_COMMON_H__ */
