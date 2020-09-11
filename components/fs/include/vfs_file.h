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
 * @file        vfs_file.h
 *
 * @brief       This file declare file operation functions of vfs.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_FILE_H__
#define __VFS_FILE_H__

#include <vfs.h>
#include <vfs_fs.h>

#ifdef __cplusplus
extern "C" {
#endif

struct os_pollreq;

/**
 ***********************************************************************************************************************
 * @struct      vfs_file_ops
 *
 * @brief       Define the vfs file operations
 ***********************************************************************************************************************
 */
struct vfs_file_ops
{
    int (*open)     (struct vfs_fd *fd);
    int (*close)    (struct vfs_fd *fd);
    int (*ioctl)    (struct vfs_fd *fd, int cmd, void *args);
    int (*read)     (struct vfs_fd *fd, void *buf, size_t count);
    int (*write)    (struct vfs_fd *fd, const void *buf, size_t count);
    int (*flush)    (struct vfs_fd *fd);
    int (*lseek)    (struct vfs_fd *fd, off_t offset);
    int (*getdents) (struct vfs_fd *fd, struct dirent *dirp, uint32_t count);

    int (*poll)     (struct vfs_fd *fd, struct os_pollreq *req);
};

/**
 ***********************************************************************************************************************
 * @struct      vfs_fd
 *
 * @brief       Define the vfs file descriptor structure
 ***********************************************************************************************************************
 */
#define VFS_FD_MAGIC     0xfdfd
struct vfs_fd
{
    uint16_t magic;              /* File descriptor magic number */
    uint16_t type;               /* Type (regular or socket) */

    char    *path;               /* Name (below mount point) */
    int      ref_count;          /* Descriptor reference count */

    struct vfs_filesystem      *fs;
    const struct vfs_file_ops  *fops;

    uint32_t flags;              /* Descriptor flags */
    size_t   size;               /* Size in bytes */
    off_t    pos;                /* Current file position */

    void    *data;               /* Specific file system data */
};

int vfs_file_open(struct vfs_fd *fd, const char *path, int flags);
int vfs_file_close(struct vfs_fd *fd);
int vfs_file_ioctl(struct vfs_fd *fd, int cmd, void *args);
int vfs_file_read(struct vfs_fd *fd, void *buf, size_t len);
int vfs_file_getdents(struct vfs_fd *fd, struct dirent *dirp, size_t nbytes);
int vfs_file_unlink(const char *path);
int vfs_file_write(struct vfs_fd *fd, const void *buf, size_t len);
int vfs_file_flush(struct vfs_fd *fd);
int vfs_file_lseek(struct vfs_fd *fd, off_t offset);

int vfs_file_stat(const char *path, struct stat *buf);
int vfs_file_rename(const char *oldpath, const char *newpath);

#ifdef __cplusplus
}
#endif

#endif
