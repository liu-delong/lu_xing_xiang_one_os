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
 * @file        cmiot_stdlib.h
 *
 * @brief       The stdlib header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_STDLIB_H__
#define __CMIOT_STDLIB_H__

#include "cmiot_typedef.h"
#include "stdarg.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef NULL
#define NULL        0 /* see <stddef.h> */
#define __ISMASK(x) (_ctype[(cmiot_int)(cmiot_uint8)(x)])

#define ISALNUM(c)  ((__ISMASK(c) & (CMIOT_U | CMIOT_L | CMIOT_D)) != 0)
#define ISALPHA(c)  ((__ISMASK(c) & (CMIOT_U | CMIOT_L)) != 0)
#define ISCNTRL(c)  ((__ISMASK(c) & (CMIOT_C)) != 0)
#define ISGRAPH(c)  ((__ISMASK(c) & (CMIOT_P | CMIOT_U | CMIOT_L | CMIOT_D)) != 0)
#define ISLOWER(c)  ((__ISMASK(c) & (CMIOT_L)) != 0)
#define ISPRINT(c)  ((__ISMASK(c) & (CMIOT_P | CMIOT_U | CMIOT_L | CMIOT_D | CMIOT_SP)) != 0)
#define ISPUNCT(c)  ((__ISMASK(c) & (CMIOT_P)) != 0)
#define ISSPACE(c)  ((__ISMASK(c) & (CMIOT_S)) != 0)
#define ISUPPER(c)  ((__ISMASK(c) & (CMIOT_U)) != 0)
#define ISXDIGIT(c) ((__ISMASK(c) & (CMIOT_D | CMIOT_X)) != 0)
#define ISDIGIT(c)  ((unsigned)((c) - '0') < 10)
#define ISASCII(c)  (((cmiot_uint8)(c)) <= 0x7f)
#define TOASCII(c)  (((cmiot_uint8)(c)) & 0x7f)

#define ZEROPAD (1 << 0) /* pad with zero */
#define SIGN    (1 << 1) /* unsigned/signed long */
#define PLUS    (1 << 2) /* show plus */
#define SPACE   (1 << 3) /* space if plus */
#define LEFT    (1 << 4) /* left justified */
#define SPECIAL (1 << 5) /* 0x */
#define LARGE   (1 << 6) /* use 'ABCDEF' instead of 'abcdef' */

#define INT_MAX   ((cmiot_int)(~0U >> 1))
#define INT_MIN   (-INT_MAX - 1)
#define UINT_MAX  (~0U)
#define LONG_MAX  ((cmiot_int)(~0UL >> 1))
#define LONG_MIN  (-LONG_MAX - 1)
#define ULONG_MAX (~0UL)

#define CMIOT_U  0x01 /* upper */
#define CMIOT_L  0x02 /* lower */
#define CMIOT_D  0x04 /* digit */
#define CMIOT_C  0x08 /* cntrl */
#define CMIOT_P  0x10 /* punct */
#define CMIOT_S  0x20 /* white space (space/lf/tab) */
#define CMIOT_X  0x40 /* hex digit */
#define CMIOT_SP 0x80 /* hard space (0x20) */

typedef cmiot_int (*cmiot_rand)(void);
typedef void (*cmiot_srand)(cmiot_uint seed);

cmiot_extern cmiot_int cmiot_vsnprintf(cmiot_char *buf, cmiot_int size, const cmiot_char *fmt, va_list args);
cmiot_extern cmiot_int cmiot_snprintf(cmiot_char *s, cmiot_int n, const cmiot_char *content, ...);

cmiot_extern void *    cmiot_memmove(void *dest, const void *src, cmiot_int n);
cmiot_extern void *    cmiot_memcpy(void *dest, const void *src, cmiot_int count);
cmiot_extern void *    cmiot_memset(void *s, cmiot_int c, cmiot_int count);
cmiot_extern cmiot_int cmiot_memcmp(const void *cs, const void *ct, cmiot_int count);
cmiot_extern cmiot_char *cmiot_strcat(cmiot_char *dest, const cmiot_char *src);

cmiot_extern cmiot_int cmiot_strlen(const cmiot_char *s);
cmiot_extern cmiot_int cmiot_sscanf(const cmiot_char *buf, const cmiot_char *fmt, ...);
cmiot_extern cmiot_int cmiot_strncmp(const cmiot_char *cs, const cmiot_char *ct, cmiot_int count);
cmiot_extern cmiot_int cmiot_strcmp(const cmiot_char *string1, const cmiot_char *string2);
cmiot_extern cmiot_char *cmiot_strcpy(cmiot_char *strDestination, const cmiot_char *strSource);
cmiot_extern cmiot_char *cmiot_strncpy(cmiot_char *dst, const cmiot_char *src, cmiot_int n);
cmiot_extern cmiot_char *cmiot_strstr(const cmiot_char *str, const cmiot_char *sub);
cmiot_extern             cmiot_uint8 *
                         cmiot_datadata(const cmiot_uint8 *data, cmiot_int data_len, const cmiot_uint8 *sub, cmiot_int sub_len);
cmiot_extern             cmiot_uint8 *
                         cmiot_datardata(const cmiot_uint8 *data, cmiot_int data_len, const cmiot_uint8 *sub, cmiot_int sub_len);
cmiot_extern cmiot_char *cmiot_strchr(const cmiot_char *s1, cmiot_int i);

cmiot_extern cmiot_int cmiot_atoi(const cmiot_char *str);
cmiot_extern           cmiot_char *
                       cmiot_itoa(cmiot_int32 num, cmiot_char *str, cmiot_int strlen, cmiot_int base, cmiot_bool unsignedint);
cmiot_extern cmiot_char * cmiot_get_32bit_dec_tmp(cmiot_int32 number, cmiot_bool unsignedint);
cmiot_extern void         cmiot_memfree(void *p);
cmiot_extern cmiot_uint   cmiot_str2uint32(const cmiot_char *str);
cmiot_extern void         cmiot_byte2hexstr(const cmiot_uint8 *source, cmiot_char *dest, cmiot_int source_len);
cmiot_extern void         cmiot_hexstr2byte(cmiot_char *dest, const cmiot_uint8 *source, cmiot_int source_len);
cmiot_extern cmiot_int    cmiot_c2i(cmiot_char ch);
cmiot_extern cmiot_int    cmiot_hex2dec(cmiot_char *hex);
cmiot_extern cmiot_bool   cmiot_str_del_all(cmiot_char *str, cmiot_char *sub, cmiot_int *len, cmiot_int sublen);
cmiot_extern cmiot_bool   cmiot_str_is_same_char(cmiot_char *str, cmiot_int len, cmiot_char ch);
cmiot_extern cmiot_bool   cmiot_is_bigendian(void);
cmiot_extern cmiot_uint32 cmiot_sw32(cmiot_uint32 x);
cmiot_extern cmiot_uint16 cmiot_sw16(cmiot_uint16 x);
cmiot_extern void         cmiot_get_randstr(cmiot_char  s[],
                                            cmiot_int   num,
                                            cmiot_char *str,
                                            cmiot_int   str_len,
                                            cmiot_srand srand,
                                            cmiot_rand  rand,
                                            cmiot_uint  seed);

#ifdef __cplusplus
}
#endif

#endif
