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
 * @file        rt_util.c
 *
 * @brief       Implementation of RT-Thread adaper utility function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <rtthread.h>
#include <rtconfig.h>
#include <os_util.h>
#include <os_irq.h>
#include <string.h>
#include <rtdef.h>

extern void        *os_memset(void *src, os_uint8_t val, os_size_t count);
extern void        *os_memcpy(void *dst, const void *src, os_size_t count);
extern void        *os_memmove(void *dst, const void *src, os_size_t count);
extern os_int32_t   os_memcmp(const void *str1, const void *str2, os_size_t count);
extern char        *os_strcpy(char *dst, const char *src);
extern char        *os_strncpy(char *dst, const char *src, os_size_t count);
extern os_int32_t   os_strcmp(const char *str1, const char *str2);
extern os_int32_t   os_strncmp(const char *str1, const char *str2, os_size_t count);
extern os_size_t    os_strlen(const char *str);
extern os_size_t    os_strnlen(const char *str, os_size_t max_len);
extern char        *os_strchr(const char *str, char ch);
extern char        *os_strstr(const char *str1, const char *str2);
extern char        *os_strdup(const char *s);
extern int          sscanf(const char * buf, const char * fmt, ...);

extern void         os_log_output(const char *log_buff);

void rt_kprintf(const char *fmt, ...)
{
    va_list args;
    os_size_t length;
    static char log_buff[OS_LOG_BUFF_SIZE];

    va_start(args, fmt);

    /* 
     * The return value of os_vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the cm_log_buf, we have to adjust the output
     * length.
     */
    length = os_vsnprintf(log_buff, sizeof(log_buff), fmt, args);
    if (length > sizeof(log_buff) - 1)
    {
        length = sizeof(log_buff) - 1;
        log_buff[length] = '\0';
    }
    
    va_end(args);

    os_log_output(log_buff);

    return;
}

void rt_kputs(const char *str)
{
    rt_kprintf(str);
}

rt_int32_t rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args)
{
    return (rt_int32_t)os_vsnprintf(buf, size, fmt, args);
}

rt_int32_t rt_vsprintf(char *dest, const char *format, va_list args)
{
    return rt_vsnprintf(dest, (rt_size_t)(-1), format, args);
}

rt_int32_t rt_sprintf(char *buf, const char *format, ...)
{
    rt_int32_t n;
    va_list args;

    va_start(args, format);
    n = rt_vsprintf(buf, format, args);
    va_end(args);

    return n;
}

rt_int32_t rt_snprintf(char *buf, rt_size_t size, const char *format, ...)
{
    rt_int32_t n;
    va_list args;

    va_start(args, format);
    n = (rt_int32_t)os_snprintf(buf, size, format, args);
    va_end(args);

    return n;
}

rt_err_t rt_get_errno(void)
{
    return (rt_err_t)os_get_errno();
}

void rt_set_errno(rt_err_t no)
{
    os_set_errno((os_err_t)no);
}

int *_rt_errno(void)
{
    return os_errno();
}

rt_int32_t rt_strncmp(const char *cs, const char *ct, rt_ubase_t count)
{
    return (rt_int32_t)os_strncmp(cs, ct, (os_size_t)count);
}

rt_int32_t rt_strcmp(const char *cs, const char *ct)
{
    return (rt_int32_t)os_strcmp(cs, ct);
}

rt_size_t rt_strlen(const char *src)
{
    return (rt_size_t)os_strlen(src);
}

rt_size_t rt_strnlen(const char *s, rt_ubase_t maxlen)
{
    return (rt_size_t)os_strnlen(s, (os_size_t)maxlen);
}

#ifdef RT_USING_HEAP
char *rt_strdup(const char *s)
{
    return os_strdup(s);
}
#endif  /* RT_USING_HEAP */

char *rt_strstr(const char *str1, const char *str2)
{
    return os_strstr(str1, str2);
}

char *rt_strncpy(char *dest, const char *src, rt_ubase_t n)
{
    return os_strncpy(dest, src, (os_size_t)n);
}

rt_uint32_t rt_strcasecmp(const char *a, const char *b)
{
    int ca;
    int cb;

    do
    {
        ca = *a++ & 0xff;
        cb = *b++ & 0xff;

        if ((ca >= 'A') && (ca <= 'Z'))
        {
            ca += 'a' - 'A';
        }
        
        if ((cb >= 'A') && (cb <= 'Z'))
        {
            cb += 'a' - 'A';
        }
    } while ((ca == cb) && (ca != '\0'));

    return ca - cb;
}

void *rt_memset(void *src, int c, rt_ubase_t n)
{
    return os_memset(src, (os_uint8_t)c, (os_size_t)n);
}

void *rt_memcpy(void *dest, const void *src, rt_ubase_t n)
{
    return os_memcpy(dest, src, (os_size_t)n);
}

void *rt_memmove(void *dest, const void *src, rt_ubase_t n)
{
    return os_memmove(dest, src, (os_size_t)n);
}

rt_int32_t rt_memcmp(const void *cs, const void *ct, rt_ubase_t count)
{
    return (rt_int32_t)os_memcmp(cs, ct, (os_size_t)count);
}

void rt_assert_handler(const char *ex_string, const char *func, rt_size_t line)
{
    volatile char dummy = 0;

    rt_kprintf("(%s) assertion failed at function:%s, line number:%d \r\n", ex_string, func, line);

    while (!dummy)
    {
        ;
    }
}


