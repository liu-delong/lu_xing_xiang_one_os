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
 * @file        vfs_yaffs.c
 *
 * @brief       This file is adapter for vfs and yaffs2.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <vfs_fs.h>
#include <vfs_file.h>
#include <nand.h>
#include "yaffs/yaffs_guts.h"
#include "yaffs/direct/yaffsfs.h"

static int vfs_yaffs_open(struct vfs_fd *file)
{
    struct vfs_filesystem *fs;
    struct yaffs_dev *yfs_dev;
    int fd;
    yaffs_DIR *dir;
    int result;
    struct yaffs_stat *buf;
    int ret = 0;

    if ((!file) || (!file->fs) || (!file->fs->data))
    {
        return -EINVAL;
    }
    fs = (struct vfs_filesystem *)file->fs;
    yfs_dev = (struct yaffs_dev *)fs->data;

    if (file->flags & O_DIRECTORY)
    {
        if (file->flags & O_CREAT)
        {
            result = yaffs_mkdir_reldev(yfs_dev, file->path, 0x777);
            if (result < 0)
            {
                return yaffsfs_GetLastError();
            }
        }

        dir = yaffs_opendir_reldev(yfs_dev, file->path);
        if (!dir)
        {
            return yaffsfs_GetLastError();
        }
        file->data = dir;
    }
    else
    {
        fd = yaffs_open_reldev(yfs_dev, file->path, file->flags, S_IRUSR | S_IWUSR);
        if (fd < 0)
        {
            return yaffsfs_GetLastError();
        }

        buf = os_malloc(sizeof(struct yaffs_stat));
        if (!buf)
        {
            return -ENOMEM;
        }
        ret = yaffs_fstat(fd, buf);
        if (ret < 0)
        {
            os_free(buf);
            return -EINVAL;
        }
        file->data = (void *)fd;
        file->pos = 0;
        file->size = buf->st_size;
        os_free(buf);
        if (file->flags & O_APPEND)
        {
            file->pos = file->size;
            file->size = yaffs_lseek(fd, 0, SEEK_END);
        }
    }
    return 0;
}

static int vfs_yaffs_close(struct vfs_fd *file)
{
    if ((!file) || ((file->type != FT_REGULAR) && (file->type != FT_DIRECTORY)))
    {
        return -EINVAL;
    }

    if (file->flags & O_DIRECTORY)
    {
        if (!file->data)
        {
            return -EINVAL;
        }
        if (yaffs_closedir((yaffs_DIR *)(file->data)) < 0)
        {
            return yaffsfs_GetLastError();
        }
    }
    else
    {
        if (((int)(file->data)) < 0)
        {
            return -EINVAL;
        }
        if (yaffs_close((int)(file->data)) < 0)
        {
            return yaffsfs_GetLastError();
        }
    }

    file->data = NULL;
    file->size = 0;
    file->pos = 0;

    return 0;
}

static int vfs_yaffs_ioctl(struct vfs_fd *file, int cmd, void *args)
{
    return -ENOSYS;
}

static int vfs_yaffs_read(struct vfs_fd *file, void *buf, size_t len)
{
    int fd;
    int read_size;

    if ((!file) || (((int)(file->data)) < 0) || (!buf))
    {
        return -EINVAL;
    }
    fd = (int)(file->data);
    read_size = yaffs_read(fd, buf, len);
    if (read_size < 0)
    {
        return yaffsfs_GetLastError();
    }

    file->pos += read_size;

    return read_size;
}

static int vfs_yaffs_write(struct vfs_fd *file, const void *buf, size_t len)
{
    int fd;
    int write_size;

    if ((!file) || (((int)(file->data)) < 0) || (!buf))
    {
        return -EINVAL;
    }
    fd = (int)(file->data);

    write_size = yaffs_write(fd, buf, len);
    if (write_size < 0)
    {
        return yaffsfs_GetLastError();
    }
    file->pos += write_size;

    return write_size;
}

static int vfs_yaffs_flush(struct vfs_fd *file)
{
    int fd;
    int result;

    if ((!file) || (((int)(file->data)) < 0))
    {
        return -EINVAL;
    }
    fd = (int)(file->data);

    result = yaffs_flush(fd);
    if (result < 0)
    {
        return yaffsfs_GetLastError();
    }

    return 0;
}

static int vfs_yaffs_lseek(struct vfs_fd *file, os_off_t offset)
{
    int fd;
    yaffs_DIR * dir;
    struct yaffs_dirent * yaffs_d;
    int result = -1;
    int entry_num;

    if (!file)
    {
        return -EINVAL;
    }

    if (file->flags & O_DIRECTORY)
    {
        dir = (yaffs_DIR *)file->data;
        if (!dir)
        {
            return -EINVAL;
        }
        yaffs_rewinddir(dir);
        file->pos = 0;
        entry_num = offset / sizeof(struct dirent);
        while (entry_num > 0)
        {
            yaffs_d = yaffs_readdir(dir);
            if (!yaffs_d)
            {
                return yaffsfs_GetLastError();
            }
            entry_num--;
            file->pos += sizeof(struct dirent);
        }
        result = file->pos;
    }
    else
    {
        fd = (int)(file->data);
        if (fd < 0)
        {
            return -EINVAL;
        }
        result = yaffs_lseek(fd, offset, SEEK_SET);
        if (result < 0)
        {
            return yaffsfs_GetLastError();
        }
        file->pos = result;
    }

    return result;
}

static int vfs_yaffs_getdents(struct vfs_fd *file, struct dirent *dirp, uint32_t count)
{
    os_uint32_t index;
    struct dirent* d;
    yaffs_DIR* dir;
    struct yaffs_dirent * yaffs_d;

    if ((!file) || (!file->data) || (!dirp) || (file->type != FT_DIRECTORY))
    {
        return -EINVAL;
    }
    dir = (yaffs_DIR*)(file->data);

    count = (count / sizeof(struct dirent)) * sizeof(struct dirent);
    if (count == 0)
    {
        return -EINVAL;
    }

    index = 0;
    while (1)
    {
        d = dirp + index;

        yaffs_d = yaffs_readdir(dir);
        if (!yaffs_d)
        {
            break;
        }

        if (yaffs_d->d_type == YAFFS_DT_DIR)
        {
            d->d_type = DT_DIR;
        }
        else if (yaffs_d->d_type == YAFFS_DT_REG)
        {
            d->d_type = DT_REG;
        }
        else
        {
            d->d_type = DT_UNKNOWN;
        }

        d->d_namlen = strlen(yaffs_d->d_name);
        d->d_reclen = (os_uint16_t)sizeof(struct dirent);
        strncpy(d->d_name, yaffs_d->d_name, strlen(yaffs_d->d_name) + 1);

        index++;
        if (index * sizeof(struct dirent) >= count)
        {
            break;
        }
    }

    file->pos += index * sizeof(struct dirent);

    return index * sizeof(struct dirent);
}


static int vfs_yaffs_mount(struct vfs_filesystem *fs, unsigned long rwflag, const void *data)
{
    os_device_t *os_dev;
    struct yaffs_dev *yfs_dev;

    if ((!fs) || (!fs->dev_id))
    {
        return -1;
    }

    os_dev = (os_device_t*)fs->dev_id;
    if (!os_dev)
    {
        return -1;
    }

    yfs_dev = yaffs_getdev(device_name(os_dev));
    if (!yfs_dev)
    {
        return -1;
    }
    if (yaffs_mount(device_name(os_dev)) < 0)
    {
        return yaffsfs_GetLastError();
    }
    fs->data = yfs_dev;

    return 0;
}

static int vfs_yaffs_unmount(struct vfs_filesystem *fs)
{
    os_device_t *os_dev;

    if ((!fs) || (!fs->dev_id))
    {
        return -1;
    }

    os_dev = (os_device_t*)fs->dev_id;
    if (!os_dev)
    {
        return -1;
    }

    if (yaffs_unmount(device_name(os_dev)) < 0)
    {
        return yaffsfs_GetLastError();
    }
    fs->data = OS_NULL;

    return 0;
}

static int vfs_yaffs_mkfs(os_device_t* dev_id)
{
    if ((!dev_id) || (dev_id->type != OS_DEVICE_TYPE_MTD))
    {
        return -1;
    }

    return yaffs_format(device_name(dev_id), 0, 0, 0);
}

static int vfs_yaffs_statfs(struct vfs_filesystem *fs, struct statfs *buf)
{
    struct yaffs_dev *yfs_dev;
    struct yaffs_param *yfs_param;
    Y_LOFF_T freespace;

    if ((!fs) || (!fs->data) || (!buf))
    {
        return -EINVAL;
    }
    yfs_dev = (struct yaffs_dev *)fs->data;
    freespace = yaffs_freespace_reldev(yfs_dev);
    if (freespace < 0)
    {
        return -EINVAL;
    }
    yfs_param = &yfs_dev->param;
    buf->f_bsize = yfs_param->total_bytes_per_chunk * yfs_param->chunks_per_block;
    buf->f_blocks = yfs_param->end_block + 1;
    buf->f_bfree = freespace /buf->f_bsize;

    return 0;
}

static int vfs_yaffs_unlink(struct vfs_filesystem *fs, const char *path)
{
    int result;
    struct yaffs_stat s;
    struct yaffs_dev *yfs_dev;

    if ((!fs) || (!fs->data) || (!path))
    {
        return -EINVAL;
    }
    yfs_dev = (struct yaffs_dev *)fs->data;

    if (yaffs_stat_reldev(yfs_dev, path, &s) < 0)
    {
        return yaffsfs_GetLastError();
    }

    switch (s.st_mode & S_IFMT)
    {
    case S_IFREG:
        result = yaffs_unlink_reldev(yfs_dev, path);
        break;
    case S_IFDIR:
        result = yaffs_rmdir_reldev(yfs_dev, path);
        break;
    default:
        return -1;
    }
    if (result < 0)
    {
        return yaffsfs_GetLastError();
    }

    return 0;
}

static int vfs_yaffs_stat(struct vfs_filesystem *fs, const char *path, struct stat *st)
{
    int result;
    struct yaffs_stat s;
    struct yaffs_dev *yfs_dev;

    if ((!fs) || (!path) || (!st))
    {
        return -EINVAL;
    }
    yfs_dev = (struct yaffs_dev *)fs->data;

    result = yaffs_stat_reldev(yfs_dev, path, &s);
    if (result < 0)
    {
        return yaffsfs_GetLastError();
    }

    st->st_dev = 0;
    st->st_mode = s.st_mode;
    st->st_size = s.st_size;
    st->st_mtime = s.yst_mtime;

    return 0;
}

static int vfs_yaffs_rename(struct vfs_filesystem *fs, const char *oldpath, const char *newpath)
{
    int result;
    struct yaffs_dev *yfs_dev;
    struct yaffs_stat s;

    if ((!fs) || (!oldpath) || (!newpath))
    {
        return -EINVAL;
    }
    yfs_dev = (struct yaffs_dev *)fs->data;

    if (strcmp(oldpath, newpath) == 0)
    {
        /* If oldpath same as newpath, and path exist, return success. */
        if (yaffs_stat_reldev(yfs_dev, oldpath, &s) == 0)
        {
            return 0;
        }
    }

    result = yaffs_rename_reldev(yfs_dev, oldpath, newpath);
    if (result < 0)
    {
        return yaffsfs_GetLastError();
    }

    return 0;
}

static const struct vfs_file_ops _fops =
{
    vfs_yaffs_open,
    vfs_yaffs_close,
    vfs_yaffs_ioctl,
    vfs_yaffs_read,
    vfs_yaffs_write,
    vfs_yaffs_flush,
    vfs_yaffs_lseek,
    vfs_yaffs_getdents,
    OS_NULL,
};

static const struct vfs_filesystem_ops _fsops =
{
    "yaffs",
    VFS_FS_FLAG_DEFAULT,
    &_fops,

    vfs_yaffs_mount,
    vfs_yaffs_unmount,
    vfs_yaffs_mkfs,
    vfs_yaffs_statfs,

    vfs_yaffs_unlink,
    vfs_yaffs_stat,
    vfs_yaffs_rename,
};

/**
 ***********************************************************************************************************************
 * @brief           Register YAFFS operation structure to VFS.
 *
 * @param[in,out]   None.
 *
 * @return          The register result. 
 * @retval          0               Register successfully.
 * @retval          -1              Register failed.
 ***********************************************************************************************************************
 */
int vfs_yaffs_init(void)
{
    return vfs_register(&_fsops);
}
OS_CMPOENT_INIT(vfs_yaffs_init);

