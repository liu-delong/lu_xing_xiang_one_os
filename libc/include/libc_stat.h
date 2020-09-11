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
 * @file        libc_stat.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __LIBC_STAT_H__
#define __LIBC_STAT_H__

#include <oneos_config.h>
#include <os_device.h>

#if defined(OS_USING_NEWLIB)
/* Use header file of newlib. */
#include <sys/stat.h>

#elif defined(_WIN32)
#include <sys/stat.h>

#define S_IRWXU     00700
#define S_IRUSR     00400
#define S_IWUSR     00200
#define S_IXUSR     00100

#define S_IRWXG     00070
#define S_IRGRP     00040
#define S_IWGRP     00020
#define S_IXGRP     00010

#define S_IRWXO     00007
#define S_IROTH     00004
#define S_IWOTH     00002
#define S_IXOTH     00001

#define S_IFSOCK    0140000
#define S_IFLNK     0120000
#define S_IFBLK     0060000
#define S_IFIFO     0010000
#define S_ISUID     0004000
#define S_ISGID     0002000
#define S_ISVTX     0001000

#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)

#else
#define S_IFMT      00170000
#define S_IFSOCK    0140000
#define S_IFLNK     0120000
#define S_IFREG     0100000
#define S_IFBLK     0060000
#define S_IFDIR     0040000
#define S_IFCHR     0020000
#define S_IFIFO     0010000
#define S_ISUID     0004000
#define S_ISGID     0002000
#define S_ISVTX     0001000

#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

#define S_IRWXU     00700
#define S_IRUSR     00400
#define S_IWUSR     00200
#define S_IXUSR     00100

#define S_IRWXG     00070
#define S_IRGRP     00040
#define S_IWGRP     00020
#define S_IXGRP     00010

#define S_IRWXO     00007
#define S_IROTH     00004
#define S_IWOTH     00002
#define S_IXOTH     00001

#include <stdint.h>
#include <time.h>

struct stat
{
    struct os_device *st_dev;
    uint16_t  st_ino;
    uint16_t  st_mode;
    uint16_t  st_nlink;
    uint16_t  st_uid;
    uint16_t  st_gid;
    struct os_device *st_rdev;
    uint32_t  st_size;
    time_t    st_atime;
    long      st_spare1;
    time_t    st_mtime;
    long      st_spare2;
    time_t    st_ctime;
    long      st_spare3;
    uint32_t  st_blksize;
    uint32_t  st_blocks;
    long      st_spare4[2];
};

#endif

#endif
