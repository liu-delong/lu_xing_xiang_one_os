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
 * @file        vfs.h
 *
 * @brief       Header file for virtual file system interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-09   OneOS team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_H__
#define __VFS_H__

#include <oneos_config.h>
#include <os_libc.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <os_device.h>
#include <os_sem.h>
#include <os_mutex.h>
#include <os_module.h>
#include <os_memory.h>
#include <os_errno.h>
#include <os_assert.h>

#ifndef VFS_FILESYSTEMS_MAX
#define VFS_FILESYSTEMS_MAX         2
#endif

#ifndef VFS_FD_MAX
#define VFS_FD_MAX                  4
#endif

/*
 * Skip stdin/stdout/stderr normally
 */
#ifndef VFS_FD_OFFSET
#define VFS_FD_OFFSET               3
#endif

#ifndef VFS_PATH_MAX
#define VFS_PATH_MAX                256
#endif

#ifndef SECTOR_SIZE
#define SECTOR_SIZE                 512
#endif

#ifndef VFS_FILESYSTEM_TYPES_MAX
#define VFS_FILESYSTEM_TYPES_MAX    2
#endif

#define VFS_FS_FLAG_DEFAULT         0x00        /* Default flag */
#define VFS_FS_FLAG_FULLPATH        0x01        /* Set full path to underlaying file system */

/* File types */
#define FT_REGULAR                  0           /* Regular file */
#define FT_SOCKET                   1           /* Socket file  */
#define FT_DIRECTORY                2           /* Directory    */
#define FT_USER                     3           /* User defined */

/* File flags */
#define VFS_F_OPEN                  0x01000000
#define VFS_F_DIRECTORY             0x02000000
#define VFS_F_EOF                   0x04000000
#define VFS_F_ERR                   0x08000000

#ifdef __cplusplus
extern "C" {
#endif


char *os_strdup(const char *s);


/**
 ***********************************************************************************************************************
 * @struct      statfs
 *
 * @brief       Define infomation of a specific file system.
 ***********************************************************************************************************************
 */
struct statfs
{
    size_t f_bsize;   /* block size */
    size_t f_blocks;  /* total data blocks in file system */
    size_t f_bfree;   /* free blocks in file system */
};

/**
 ***********************************************************************************************************************
 * @struct      dirent
 *
 * @brief       Define the directory entry.
 ***********************************************************************************************************************
 */
struct dirent
{
    uint8_t  d_type;           /* The type of the file */
    uint8_t  d_namlen;         /* The length of the not including the terminating null file name */
    uint16_t d_reclen;         /* length of this record */
    char d_name[VFS_PATH_MAX]; /* The null-terminated file name */
};

/**
 ***********************************************************************************************************************
 * @struct      vfs_fdtable
 *
 * @brief       Define file descriptor table of virtual file system.
 ***********************************************************************************************************************
 */
struct vfs_fdtable
{
    uint32_t        maxfd;
    struct vfs_fd **fds;
};

/* Initialization of vfs */
int   vfs_init(void);

char *vfs_normalize_path(const char *directory, const char *filename);
void  vfs_lock(void);
void  vfs_unlock(void);
const char *vfs_subdir(const char *directory, const char *filename);

/* FD APIs */
int   fd_new(void);
void  fd_put(struct vfs_fd *fd);
int   fd_is_open(const char *pathname);
struct vfs_fd *fd_get(int fd);

struct vfs_fdtable *vfs_fdtable_get(void);

#ifdef __cplusplus
}
#endif

#endif
