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
 * @file        types.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-13   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <stdint.h>
#include <os_types.h>

typedef os_int32_t clockid_t;
typedef os_int32_t key_t;       /* Used for interprocess communication. */
typedef os_int32_t pid_t;       /* Used for process IDs and process group IDs. */
typedef signed long ssize_t;    /* Used for a count of bytes or an error indication. */

#endif
