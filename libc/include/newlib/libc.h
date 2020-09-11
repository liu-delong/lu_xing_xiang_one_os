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
 * @file        libc.h
 *
 * @brief       Header file for libc interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <oneos_config.h>

#ifndef _EXFUN
#define _EXFUN(name, proto) name proto
#endif

#define MILLISECOND_PER_SECOND  1000UL
#define MICROSECOND_PER_SECOND  1000000UL
#define NANOSECOND_PER_SECOND   1000000000UL

#define MILLISECOND_PER_TICK    (MILLISECOND_PER_SECOND / OS_TICK_PER_SECOND)
#define MICROSECOND_PER_TICK    (MICROSECOND_PER_SECOND / OS_TICK_PER_SECOND)
#define NANOSECOND_PER_TICK     (NANOSECOND_PER_SECOND  / OS_TICK_PER_SECOND)

int libc_system_init(void);
int libc_stdio_set_console(const char* device_name, int mode);
int libc_stdio_get_console(void);

/* Some time related function. */
int libc_set_time(const struct timespec *time);
int libc_get_time(struct timespec *time);
int libc_time_to_tick(const struct timespec *time);

#endif
