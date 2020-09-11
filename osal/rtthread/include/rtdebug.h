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
* @file        rtdebug.h
*
* @brief       RT-Thread adaper API about debug header file.
*
* @revision
* Date         Author          Notes
* 2020-06-12   OneOS Team      First version.
***********************************************************************************************************************
*/
#ifndef __RTDEBUG_H__
#define __RTDEBUG_H__

#include <os_assert.h>

#define RT_ASSERT(EX)                                           \
    do                                                          \
    {                                                           \
        if (!(EX))                                              \
        {                                                       \
            rt_assert_handler(#EX, __FUNCTION__, __LINE__);     \
        }                                                       \
    } while (0)

extern void rt_assert_handler(const char *ex_string, const char *func, rt_size_t line);

#endif /* __RTDEBUG_H__ */

