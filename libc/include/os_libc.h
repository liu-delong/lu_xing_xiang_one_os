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
 * @file        os_libc.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __OS_LIBC_H__
#define __OS_LIBC_H__

/* Definitions for libc if toolchain has no these definitions. */
#include "libc_stat.h"
#include "libc_errno.h"
#include "libc_fcntl.h"
#include "libc_ioctl.h"
#include "libc_dirent.h"
#include "libc_signal.h"
#include "libc_fdset.h"

#if defined(__CC_ARM) || defined(__CLANG_ARM) || defined(__IAR_SYSTEMS_ICC__)
typedef signed long off_t;
typedef int mode_t;
#endif

#if defined(__MINGW32__) || defined(_WIN32)
typedef signed long off_t;
typedef int mode_t;
#endif

#endif

