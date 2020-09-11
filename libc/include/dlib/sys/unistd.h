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
 * @file        unistd.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-13   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __SYS_UNISTD_H__
#define __SYS_UNISTD_H__

#include <oneos_config.h>

#ifdef OS_USING_VFS

#define STDIN_FILENO    0       /* Standard input file descriptor. */
#define STDOUT_FILENO   1       /* Standard output file descriptor. */
#define STDERR_FILENO   2       /* Standard error file descriptor. */

#include <vfs_posix.h>
#else
#define _FREAD      0x0001      /* Read enabled. */
#define _FWRITE     0x0002      /* Write enabled. */
#define _FAPPEND    0x0008      /* Append (writes guaranteed at the end). */
#define _FMARK      0x0010      /* Internal; mark during gc(). */
#define _FDEFER     0x0020      /* Internal; defer for next gc pass. */
#define _FASYNC     0x0040      /* Signal pgrp when data ready. */
#define _FSHLOCK    0x0080      /* BSD flock() shared lock present. */
#define _FEXLOCK    0x0100      /* BSD flock() exclusive lock present. */
#define _FCREAT     0x0200      /* Open with file create. */
#define _FTRUNC     0x0400      /* Open with truncation. */
#define _FEXCL      0x0800      /* Error on open if file exists. */
#define _FNBIO      0x1000      /* Non blocking I/O (sys5 style). */
#define _FSYNC      0x2000      /* Do all writes synchronously. */
#define _FNONBLOCK  0x4000      /* Non blocking I/O (POSIX style). */
#define _FNDELAY    _FNONBLOCK  /* Non blocking I/O (4.2 style). */
#define _FNOCTTY    0x8000      /* Don't assign a ctty on this open. */

#define O_RDONLY    0           /* +1 == FREAD. */
#define O_WRONLY    1           /* +1 == FWRITE. */
#define O_RDWR      2           /* +1 == FREAD|FWRITE. */
#define O_APPEND    _FAPPEND
#define O_CREAT     _FCREAT
#define O_TRUNC     _FTRUNC
#define O_EXCL      _FEXCL
#define O_SYNC      _FSYNC
#endif

#endif /* __SYS_UNISTD_H__ */
