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
 * @file        libc_syms.c
 *
 * @brief       This file export some symbols for module.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <setjmp.h>
#include <assert.h>
#include <oneos_config.h>
#include <os_module.h>

EXPORT_SYMBOL(strcpy);
EXPORT_SYMBOL(strncpy);
EXPORT_SYMBOL(strlen);
EXPORT_SYMBOL(strcat);
EXPORT_SYMBOL(strstr);
EXPORT_SYMBOL(strchr);
EXPORT_SYMBOL(strcmp);
EXPORT_SYMBOL(strtol);
EXPORT_SYMBOL(strtoul);
EXPORT_SYMBOL(strncmp);

EXPORT_SYMBOL(memcpy);
EXPORT_SYMBOL(memcmp);
EXPORT_SYMBOL(memmove);
EXPORT_SYMBOL(memset);
EXPORT_SYMBOL(memchr);

EXPORT_SYMBOL(putchar);
EXPORT_SYMBOL(puts);
EXPORT_SYMBOL(printf);
EXPORT_SYMBOL(sprintf);
EXPORT_SYMBOL(snprintf);

EXPORT_SYMBOL(fwrite);

#ifdef OS_USING_RTC
EXPORT_SYMBOL(localtime);
EXPORT_SYMBOL(time);
#endif

EXPORT_SYMBOL(longjmp);
EXPORT_SYMBOL(setjmp);

EXPORT_SYMBOL(exit);
EXPORT_SYMBOL(abort);

EXPORT_SYMBOL(rand);

EXPORT_SYMBOL(__assert_func);
