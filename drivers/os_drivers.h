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
 * @file        drivers.h
 *
 * @brief       this file implements timer related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_hw.h>
#include <os_util.h>
#include <os_device.h>
#include <os_errno.h>
#include <os_assert.h>

#define OS_DEVICE(device)               ((os_device_t *)device)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(ar)                  (sizeof(ar) / sizeof(ar[0]))
#endif

#ifndef container_of
#define container_of(ptr, type, member) ((type *)(((unsigned long)ptr) - (unsigned long)(&(((type *)0)->member))))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN min
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX max
#endif

#ifndef OS_DEBUG
#undef OS_ASSERT
#define OS_ASSERT(EX) while (!(EX))
#endif

void calc_mult_shift(os_uint32_t *mult, os_uint32_t *shift, os_uint32_t from, os_uint32_t to, os_uint32_t max_from);

unsigned short crc16(unsigned short init_crc, void *buff, int len);

void hex_dump(unsigned char *buff, int count);

#define OS_HAL_DEVICE_DEFINE(drv_name, dev_name, instance)  \
OS_DEVICE_INFO instance##_info = {                          \
    .name   = dev_name,                                     \
    .driver = drv_name,                                     \
    .info   = &instance,                                    \
}

int os_ffs(os_uint32_t value);
int os_fls(os_uint32_t value);

