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

#ifndef bool
#define bool unsigned int
#endif
#ifndef true
#define true 1u
#endif
#ifndef false
#define false 0u
#endif

typedef struct
{
    long long int value;
    int           dec_len;
} loca_float_t;

#define tras_loca_float(loca_float) ((double)loca_float.value / pow(10, loca_float.dec_len))

typedef struct
{
    int day;
    int month;
    int year;
} loca_date_t;

typedef struct
{
    int hours;
    int minutes;
    int seconds;
    int microseconds;
} loca_time_t;

#endif /* __POS_COMMON_H__ */
