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
 * @file        rt_ulog.c
 *
 * @brief       Implementation of RT-Thread adaper log function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <rtconfig.h>
#include <rtdef.h>

#ifdef OS_USING_DLOG
#include <dlog.h>
#endif  /* OS_USING_DLOG */

#ifdef RT_USING_ULOG
void ulog_output(rt_uint32_t level, const char *tag, rt_bool_t newline, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    dlog_voutput((os_uint16_t)level, tag, (os_bool_t)newline, format, args);
    va_end(args);
    
    return;
}

void ulog_raw(const char *format, ...)
{
    return;
}

void ulog_hexdump(const char *tag, rt_size_t width, rt_uint8_t *buf, rt_size_t size)
{
    return;
}

void ulog_flush(void)
{
    dlog_flush();
    return;
}

#endif /* RT_USING_ULOG */

