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
 * @file        vfs_fs.h
 *
 * @brief       Declare the filesystem related APIs of vfs.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-10   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_FS_H__
#define __VFS_FS_H__

#include <vfs.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Pre-declaration */
struct vfs_filesystem;
struct vfs_fd;

/**
 ***********************************************************************************************************************
 * @struct      vfs_filesystem_ops
 *
 * @brief       File system operations of a file system instance.
 ***********************************************************************************************************************
 */
struct vfs_filesystem_ops
{
    char     *name;
    uint32_t  flags;      /* Flags for file system operations */

    /* Operations for file */
    const struct vfs_file_ops *fops;

    /* Mount and unmount file system */
    int (*mount)    (struct vfs_filesystem *fs, unsigned long rwflag, const void *data);
    int (*unmount)  (struct vfs_filesystem *fs);

    /* Make a file system */
    int (*mkfs)     (os_device_t *devid);
    int (*statfs)   (struct vfs_filesystem *fs, struct statfs *buf);

    int (*unlink)   (struct vfs_filesystem *fs, const char *pathname);
    int (*stat)     (struct vfs_filesystem *fs, const char *filename, struct stat *buf);
    int (*rename)   (struct vfs_filesystem *fs, const char *oldpath, const char *newpath);
};

/**
 ***********************************************************************************************************************
 * @struct      vfs_filesystem
 *
 * @brief       Mounted file system infomation.
 ***********************************************************************************************************************
 */
struct vfs_filesystem
{
    os_device_t                         *dev_id;      /* Attached device */

    char                                *path;        /* File system mount point */
    const struct vfs_filesystem_ops     *ops;         /* Operations for file system type */

    void                                *data;        /* Specific file system data */
};

/* File system partition table */
struct vfs_partition
{
    uint8_t      type;        /* File system type */
    off_t        offset;      /* Partition start offset */
    size_t       size;        /* Partition size */
    os_sem_t    *lock;
};

/* Mount table */
struct vfs_mount_tbl
{
    const char   *device_name;
    const char   *path;
    const char   *filesystemtype;
    unsigned long rwflag;
    const void   *data;
};

int vfs_register(const struct vfs_filesystem_ops *ops);
struct vfs_filesystem *vfs_filesystem_lookup(const char *path);
const char *vfs_filesystem_get_mounted_path(struct os_device *device);

int vfs_filesystem_get_partition(struct vfs_partition *part, uint8_t *buf, uint32_t pindex);

int vfs_mount(const char    *device_name,
              const char    *path,
              const char    *filesystemtype,
              unsigned long  rwflag,
              const void    *data);

int vfs_unmount(const char *specialfile);

int vfs_mkfs(const char *fs_name, const char *device_name);
int vfs_statfs(const char *path, struct statfs *buffer);

#ifdef __cplusplus
}
#endif

#endif
