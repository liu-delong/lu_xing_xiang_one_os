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
 * @file        vfs_cutefs.c
 *
 * @brief       This file implement the RAM filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-10   OneOS Team      First version, now only support run on ramdisk.
 ***********************************************************************************************************************
 */
#include <os_dbg.h>
#include <vfs.h>
#include <vfs_fs.h>
#include <vfs_file.h>
#include <cutefs/vfs_cutefs.h>
#include "cutefs.h"
#include "cutefs_block.h"

/**
 ***********************************************************************************************************************
 * @brief           Mount cute filesystem.
 *
 * @param[in,out]   fs              The pointer of VFS object.
 * @param[in]       rwflag          The read/write flag, not used now.
 * @param[in]       data            private data, not used now.
 *
 * @return          The mount result.
 * @retval          0               Mount successfully.
 * @retval          -EIO            Mount failed, return the error code.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_mount(struct vfs_filesystem *fs, unsigned long rwflag, const void *data)
{
    struct cutefs *cfs;

    if (!fs)
    {
        return -EIO;
    }

    cfs = cutefs_mount(fs->dev_id);
    if (!cfs)
    {
        return -EIO;
    }
    fs->data = cfs;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Unmount cute filesystem.
 *
 * @param[in,out]   fs              The pointer of VFS object.
 *
 * @return          The unmount result.
 * @retval          0               Unmount successfully.
 * @retval          -EIO            Unmount failed.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_unmount(struct vfs_filesystem *fs)
{
    struct cutefs *cfs;

    if ((!fs) || (!fs->data))
    {
        return -EIO;
    }

    cfs = fs->data;
    if (cutefs_unmount(cfs) < 0)
    {
        return -EIO;
    }
    fs->data = OS_NULL;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Get the cute filesystem stat.
 *
 * @param[in]       fs              The pointer of VFS object.
 * @param[out]      buf             The pointer of buf to save the cute filesystem stat.
 *
 * @return          The operation result.
 * @retval          0               Get stat successfully.
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_statfs(struct vfs_filesystem *fs, struct statfs *buf)
{
    struct cutefs *cfs;

    if ((!fs) || (!fs->data) || (!buf))
    {
        return -EINVAL;
    }
    cfs = (struct cutefs *)fs->data;
    cutefs_statfs(cfs, buf);

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Open file in cute filesystem.
 *
 * @param[in,out]   file            The file descriptor.
 *
 * @return          The open result.
 * @retval          0               Open file successfully.
 * @retval          Other           Open failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_open(struct vfs_fd *file)
{
    struct cutefs *cfs;
    struct cutefs_fd *cfs_fd;
    struct vfs_filesystem *fs;
    int result;

    if ((!file) || (!file->fs) || (!file->fs->data))
    {
        return -EINVAL;
    }

    fs = (struct vfs_filesystem *)file->fs;
    cfs = (struct cutefs *)fs->data;
    cfs_fd = os_malloc(sizeof(struct cutefs_fd));
    if (!cfs_fd)
    {
        return -ENOMEM;
    }
    result = cutefs_open(cfs, cfs_fd, file->path, file->flags);
    if (result < 0)
    {
        os_free(cfs_fd);
        return result;
    }

    file->data = cfs_fd;
    file->size = cfs_fd->size;
    if (file->flags & O_APPEND)
    {
        file->pos = file->size;
    }
    else
    {
        file->pos = 0;
    }

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Close file in cute filesystem.
 *
 * @param[in,out]   file            The file descriptor.
 *
 * @return          The close result.
 * @retval          0               File close successfully.
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_close(struct vfs_fd *file)
{
    struct cutefs *cfs;
    struct cutefs_fd *cfs_fd;
    struct vfs_filesystem *fs;

    if ((!file) || (!file->fs) || (!file->fs->data) || (!file->data))
    {
        return -EINVAL;
    }

    fs = (struct vfs_filesystem *)file->fs;
    cfs = (struct cutefs *)fs->data;
    cfs_fd = file->data;
    if (cutefs_close(cfs, cfs_fd) < 0)
    {
        return -EINVAL;
    }
    os_free(cfs_fd);
    file->data = NULL;
    file->size = 0;
    file->pos = 0;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Read file.
 *
 * @param[in,out]   file            The file descriptor.
 * @param[out]      buf             The pointer of buffer to save read content.
 * @param[in]       len             The expected read size.
 *
 * @return          The actual read size.
 * @retval          positive int    The actual read size.
 * @retval          negative int    Read failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_read(struct vfs_fd *file, void *buf, size_t size)
{
    struct cutefs_fd *cfs_fd;
    struct vfs_filesystem *fs;
    struct cutefs *cfs;
    int read_size = 0;

    if ((!file) || (!file->fs) || (!file->fs->data) || (!file->data) || (!buf))
    {
        return -EINVAL;
    }

    fs = file->fs;
    cfs = fs->data;
    cfs_fd = (struct cutefs_fd *)file->data;
    if (size > 0)
    {
        read_size = cutefs_read(cfs, cfs_fd, buf, file->pos, size);
        if (read_size > 0)
        {
            file->pos += read_size;
        }
    }

    return read_size;
}

/**
 ***********************************************************************************************************************
 * @brief           Write file.
 *
 * @param[in,out]   file            The file descriptor.
 * @param[in]       buf             The pointer of data to write.
 * @param[in]       len             The expected write size.
 *
 * @return          The actual write size.
 * @retval          positive int    The actual write size.
 * @retval          negative int    Read failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_write(struct vfs_fd *file, const void *buf, size_t size)
{
    struct cutefs_fd *cfs_fd;
    struct vfs_filesystem *fs;
    struct cutefs *cfs;
    int write_size = 0;

    if ((!file) || (!file->fs) || (!file->fs->data) || (!file->data) || (!buf))
    {
        return -EINVAL;
    }

    fs = file->fs;
    cfs = fs->data;
    cfs_fd = (struct cutefs_fd *)file->data;
    if (size > 0)
    {
        write_size = cutefs_write(cfs, cfs_fd, buf, file->pos, size);
    }
    if (write_size > 0)
    {
        file->pos += write_size;
    }
    file->size =  (file->pos > file->size) ? file->pos : file->size;

    return write_size;
}

/**
 ***********************************************************************************************************************
 * @brief           Reposition read/write file offset.
 *
 * @param[in,out]   file            The file descriptor.
 * @param[in]       offset          The new offset in file.
 *
 * @return          The lseek result.
 * @retval          int             The new position in file.  
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_lseek(struct vfs_fd *file, off_t offset)
{
    if ((!file) || ((file->type != FT_REGULAR) && (file->type != FT_DIRECTORY)))
    {
        return -EINVAL;
    }

    file->pos = offset;
    return file->pos;
}

/**
 ***********************************************************************************************************************
 * @brief           Get dir entry.
 *
 * @param[in,out]   file            The file descriptor of dir path.
 * @param[out]      dirp            The pointer to save dir entry.
 * @param[out]      count           The buffer size to save dir entry.
 *
 * @return          The read size.
 * @retval          int             The actual read size of dir entry.
 * @retval          -EINVAL         The parameter error.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_getdents(struct vfs_fd *file, struct dirent *dirp, uint32_t count)
{
    struct cutefs_fd *cfs_fd;
    struct vfs_filesystem *fs;
    struct cutefs *cfs;
    int size;

    if ((!file) || (!file->data) || (!file->fs) || (!file->fs->data) || (!dirp))
    {
        return -EINVAL;
    }

    fs = file->fs;
    cfs = fs->data;
    cfs_fd = (struct cutefs_fd *)file->data;
    size = cutefs_getdents(cfs, cfs_fd, dirp, file->pos,count);
    if (size > 0)
    {
        file->pos += size;
    }

    return size;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete dir entry from cute filesystem.
 *
 * @param[in]       fs              The pointer of VFS object.
 * @param[in]       path            The path (the file name) to be delete.
 *
 * @return          The delete result.
 * @retval          0               Delete successfully.
 * @retval          others          Delete fail, return err code.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_unlink(struct vfs_filesystem *fs, const char *path)
{
    struct cutefs *cfs;

    if ((!fs) || (!fs->data) || (!path))
    {
        return -EINVAL;
    }

    cfs = (struct cutefs *)fs->data;
    return cutefs_unlink(cfs, path);
}

/**
 ***********************************************************************************************************************
 * @brief           Get the file status.
 *
 * @param[in]       fs              The pointer of VFS object.
 * @param[in]       path            The file path.
 * @param[out]      st              The pointer to save file status.
 *
 * @return          The operation result.
 * @retval          0               Get status successfully.
 * @retval          -ENOENT         Not found the file entry. 
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_stat(struct vfs_filesystem *fs, const char *path, struct stat *st)
{
    struct cutefs *cfs;
    struct cutefs_fileinfo info;
    int result;

    if ((!fs) || (!fs->data) || (!path) || (!st))
    {
        return -EINVAL;
    }
    cfs = (struct cutefs *)fs->data;
    result = cutefs_stat(cfs, path, &info);
    if (result < 0)
    {
        return -ENOENT;
    }

    st->st_dev = 0;
    st->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                  S_IWUSR | S_IWGRP | S_IWOTH;
    if (info.type == FT_DIRECTORY)
    {
        st->st_mode &= ~S_IFREG;
        st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    }
    st->st_size = info.size;
    st->st_mtime = 0;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Rename file/directory in cute filesystem.
 *
 * @param[in]       fs              The pointer of VFS object.
 * @param[in]       oldpath         The old file/directory name.
 * @param[in]       newpath         The new file/directory name.
 *
 * @return          The rename result.
 * @retval          0               Rename successfully.
 * @retval          -EEXIST         The new file/directory name already exist.
 * @retval          -ENOENT         The old file/directory entry not found.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_rename(struct vfs_filesystem *fs, const char *oldpath, const char *newpath)
{
    struct cutefs *cfs;

    if ((!fs) || (!fs->data) || (!oldpath) || (!newpath))
    {
        return -EINVAL;
    }
    cfs = (struct cutefs *)fs->data;

    return cutefs_rename(cfs, oldpath, newpath);
}

static const struct vfs_file_ops _cute_fops =
{
    vfs_cutefs_open,
    vfs_cutefs_close,
    NULL,                   /* Not support ioctl*/
    vfs_cutefs_read,
    vfs_cutefs_write,
    NULL,                   /* Not support flush. */
    vfs_cutefs_lseek,
    vfs_cutefs_getdents,
};

static const struct vfs_filesystem_ops _cutefs =
{
    "cute",
    VFS_FS_FLAG_DEFAULT,
    &_cute_fops,

    vfs_cutefs_mount,
    vfs_cutefs_unmount,
    NULL,                   /* Not support mkfs. */
    vfs_cutefs_statfs,

    vfs_cutefs_unlink,
    vfs_cutefs_stat,
    vfs_cutefs_rename,
};

/**
 ***********************************************************************************************************************
 * @brief           Register cutefs operation structure to VFS.
 *
 * @param[in,out]   None.
 *
 * @return          The register result. 
 * @retval          0               Register successfully.
 * @retval          -1              Register failed.
 ***********************************************************************************************************************
 */
int vfs_cutefs_init(void)
{
    /* Register cute file system. */
    return vfs_register(&_cutefs);
}
OS_CMPOENT_INIT(vfs_cutefs_init);

