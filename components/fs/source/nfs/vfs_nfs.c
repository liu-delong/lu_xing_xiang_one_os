/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2020,RT-Thread Development Team
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        vfs_nfs.c
 *
 * @brief       Adapter file between nfs and vfs.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      Rafactor nfs code
 ***********************************************************************************************************************
 */


#include <stdio.h>
#include <vfs_fs.h>
#include <vfs.h>
#include <vfs_file.h>
#include <rpc/rpc.h>
#include "nfs.h"

#define HOLE_BUF_SIZE       1024

static int vfs_nfs_mount(struct vfs_filesystem *fs, unsigned long rwflag, const void *data)
{
    struct nfs_filesystem *nfs;

    if (!fs)
    {
        return -EIO;
    }

    nfs = nfs_mount((char *)data);
    if (!nfs)
    {
        return -EIO;
    }
    fs->data = nfs;

    return 0;
}

static int vfs_nfs_unmount(struct vfs_filesystem *fs)
{
    struct nfs_filesystem *nfs;

    if ((!fs) || (!fs->data))
    {
        return -EIO;
    }

    nfs = fs->data;
    if (nfs_unmount(nfs) < 0)
    {
        return -EIO;
    }
    fs->data = OS_NULL;

    return 0;
}

int vfs_nfs_ioctl(struct vfs_fd *file, int cmd, void *args)
{
    return -ENOSYS;
}

static int vfs_nfs_read(struct vfs_fd *file, void *buf, size_t size)
{
    struct nfs_fd *n_fd;
    struct vfs_filesystem *fs;
    struct nfs_filesystem *nfs;
    int read_size = 0;
    int file_size = 0;

    if ((!file) || (!file->fs) || (!file->fs->data) || (!file->data) || (!buf))
    {
        return -EINVAL;
    }

    if (file->type == FT_DIRECTORY)
    {
        return -EISDIR;
    }
    if (file->flags & O_WRONLY)
    {
        return -EINVAL;
    }
    if(size <= 0)
    {
        return 0;
    }

    fs = file->fs;
    nfs = fs->data;
    n_fd = (struct nfs_fd *)file->data;

    file_size = nfs_get_filesize(nfs, file->path);
    if (file->pos > file_size)
    {
        return -EINVAL;
    }
    size = (size > (file_size - file->pos)) ? (file_size - file->pos) : size;
    read_size = nfs_read(nfs, n_fd, buf, file->pos, size);
    if (read_size > 0)
    {
        file->pos += read_size;
    }

    return read_size;
}

static int vfs_nfs_write(struct vfs_fd *file, const void *buf, size_t size)
{
    struct nfs_fd *n_fd;
    struct vfs_filesystem *fs;
    struct nfs_filesystem *nfs;
    int write_size = 0;
    char *buf_hole;
    int size_hole;
    int size_temp;
    off_t pos;

    if ((!file) || (!file->fs) || (!file->fs->data) || (!file->data) || (!buf))
    {
        return -EINVAL;
    }
    if (file->type == FT_DIRECTORY)
    {
        return -EISDIR;
    }
    if ((!(file->flags & O_RDWR)) && (!(file->flags & O_WRONLY)))
    {
        return -EINVAL;
    }
    if(size <= 0)
    {
        return 0;
    }

    fs = file->fs;
    nfs = fs->data;
    n_fd = (struct nfs_fd *)file->data;

    /* If hole exist, fill hole to null value */
    if (file->pos > file->size)
    {
        size_hole = file->pos - file->size;
        pos = file->size;
        size_temp = (size_hole > HOLE_BUF_SIZE) ? HOLE_BUF_SIZE : size_hole;
        buf_hole = os_malloc(size_temp);
        if (!buf_hole)
        {
            return -ENOMEM;
        }
        memset(buf_hole, 0, size_temp);
        while(size_hole > 0)
        {
            write_size = (size_hole > size_temp) ? size_temp : size_hole;
            write_size = nfs_write(nfs, n_fd, buf_hole, pos, write_size);
            if (write_size > 0)
            {
                size_hole -= write_size;
                pos += write_size;
            }
            else
            {
                os_free(buf_hole);
                return write_size;
            }
        }
        os_free(buf_hole);
    }

    /*Write actual data */
    write_size = nfs_write(nfs, n_fd, buf, file->pos, size);
    if (write_size > 0)
    {
        file->pos += write_size;
    }

    file->size = (file->pos > file->size) ? file->pos : file->size;

    return write_size;
}


static int vfs_nfs_lseek(struct vfs_fd *file, off_t offset)
{
    if ((!file) || (!file->data) || ((file->type != FT_REGULAR) && (file->type != FT_DIRECTORY)))
    {
        return -EINVAL;
    }

    if (file->type == FT_REGULAR)
    {
        nfs_seekfile((struct nfs_fd *)file->data, offset);
    }
    if (file->type == FT_DIRECTORY)
    {
        nfs_seekdir((struct nfs_dir *)file->data, offset);
    }
    file->pos = offset;

    return file->pos;
}

static int vfs_nfs_close(struct vfs_fd *file)
{
    if ((!file) || (!file->fs) || (!file->fs->data) || (!file->data))
    {
        return -EINVAL;
    }

    if (file->type == FT_REGULAR)
    {
        nfs_closefile((struct nfs_fd *)file->data);
    }
    else if (file->type == FT_DIRECTORY)
    {
        nfs_closedir((struct nfs_dir *)file->data);
    }

    file->data = NULL;
    file->size = 0;
    file->pos = 0;

    return 0;
}

static int vfs_nfs_open(struct vfs_fd *file)
{
    struct nfs_filesystem *nfs;
    struct vfs_filesystem *fs;
    struct nfs_fd *n_fd;
    struct nfs_dir *dir;
    struct fattr3 *info;
    int result;

    if ((!file) || (!file->fs) || (!file->fs->data))
    {
        return -EINVAL;
    }

    fs = (struct vfs_filesystem *)file->fs;
    nfs = (struct nfs_filesystem *)fs->data;

    if (file->flags & O_DIRECTORY)
    {
        if (file->flags & O_CREAT)
        {
            if (nfs_mkdir(nfs, file->path, 0755) < 0)
            {
                return -ENOENT;
            }
        }

        dir = nfs_opendir(nfs, file->path);
        if (!dir)
        {
            return -ENOENT;
        }
        file->data = dir;
        file->size = 0;
        file->pos = 0;
    }
    else
    {
        if (file->flags & O_CREAT)
        {
            /* If O_EXCL specified in conjuncition with O_CREAT, and pathname already exists, then fails with EEXIST*/
            if (file->flags & O_EXCL)
            {
                info = os_malloc(sizeof(struct fattr3));
                if (!info)
                {
                    return -ENOMEM;
                }
                result = nfs_stat(nfs, file->path, info);
                if ((result == 0) && (info->type == NFS3REG) || (info->type == NFS3DIR))
                {
                    os_free(info);
                    return -EEXIST;
                }
                os_free(info);
            }
            if (nfs_mkfile(nfs, file->path, 0664) < 0)
            {
                return -ENOENT;
            }
        }

        /*If the file already exists and the access mode allows writing, it will be truncated to length 0. */
        if ((file->flags & O_TRUNC) && ((file->flags & O_RDWR) || (file->flags & O_WRONLY)))
        {
            info = os_malloc(sizeof(struct fattr3));
            if (!info)
            {
                return -ENOMEM;
            }
            result = nfs_stat(nfs, file->path, info);
            if ((result == 0) && (info->size > 0))
            {
                if (nfs_unlinkfile(nfs, file->path) < 0)
                {
                    os_free(info);
                    return -EINVAL;
                }
                if (nfs_mkfile(nfs, file->path, 0664) < 0)
                {
                    os_free(info);
                    return -ENOENT;
                }
            }
            os_free(info);
        }

        n_fd = nfs_openfile(nfs, file->path, file->flags);
        if (!n_fd)
        {
            return -ENOENT;
        }

        file->data = n_fd;
        file->size = n_fd->size;
        if (file->flags & O_APPEND)
        {
            file->pos = file->size;
        }
        else
        {
            file->pos = 0;
        }
    }
    return 0;
}


static int vfs_nfs_stat(struct vfs_filesystem *fs, const char *path, struct stat *st)
{
    struct nfs_filesystem *nfs;
    struct fattr3 *info;
    int result;

    if ((!fs) || (!fs->data) || (!path) || (!st))
    {
        return -EINVAL;
    }
    nfs = (struct nfs_filesystem *)fs->data;

    info = os_malloc(sizeof(struct fattr3));
    if (!info)
    {
        return -ENOMEM;
    }
    result = nfs_stat(nfs, path, info);
    if (result < 0)
    {
        os_free(info);
        return -ENOENT;
    }

    st->st_dev = 0;
    st->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                  S_IWUSR | S_IWGRP | S_IWOTH;
    if (info->type == NFS3DIR)
    {
        st->st_mode &= ~S_IFREG;
        st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    }
    st->st_size = info->size;
    st->st_mtime = info->mtime.seconds;
    os_free(info);

    return 0;
}


static int vfs_nfs_statfs(struct vfs_filesystem *fs, struct statfs *buf)
{
    struct nfs_filesystem *nfs;

    if ((!fs) || (!fs->data) || (!buf))
    {
        return -EINVAL;
    }
    nfs = (struct nfs_filesystem *)fs->data;

    return nfs_statfs(nfs, buf);
}


static int vfs_nfs_unlink(struct vfs_filesystem *fs, const char *path)
{
    struct nfs_filesystem *nfs;
    int result = 0;

    if ((!fs) || (!fs->data) || (!path))
    {
        return -EINVAL;
    }

    nfs = (struct nfs_filesystem *)fs->data;

    if (nfs_get_type(nfs, path) == FT_REGULAR)
    {
        result = nfs_unlinkfile(nfs, path);
    }
    else
    {
        result = nfs_unlinkdir(nfs, path);
    }

    return result;
}

static int vfs_nfs_rename(struct vfs_filesystem *fs, const char *oldpath, const char *newpath)
{
    struct nfs_filesystem *nfs;

    if ((!fs) || (!fs->data) || (!oldpath) || (!newpath))
    {
        return -EINVAL;
    }
    nfs = (struct nfs_filesystem *)fs->data;

    return nfs_rename(nfs, oldpath, newpath);
}

int vfs_nfs_getdents(struct vfs_fd *file, struct dirent *dirp, uint32_t count)
{
    struct nfs_dir *dir;
    os_uint32_t index;
    struct dirent *d;
    struct vfs_filesystem *fs;
    struct nfs_filesystem *nfs;

    if ((!file) || (!file->data) || (!file->fs) || (!file->fs->data) || (!dirp))
    {
        return -EINVAL;
    }

    fs  = ((struct vfs_filesystem *)(file->fs));
    nfs = (struct nfs_filesystem *)(fs->data);
    dir = (struct nfs_dir *)(file->data);

    count = (count / sizeof(struct dirent)) * sizeof(struct dirent);
    if (count == 0)
    {
        return -EINVAL;
    }

    index = 0;
    while (1)
    {
        d = dirp + index;
        if (nfs_readdir(nfs, dir, d) < 0)
        {
            break;
        }
        if ((strcmp(d->d_name, ".") == 0) || (strcmp(d->d_name, "..") == 0))
        {
            continue;
        }

        index ++;
        if (index * sizeof(struct dirent) >= count)
        {
            break;
        }
    }

    file->pos += index * sizeof(struct dirent);
    return index * sizeof(struct dirent);
}

static const struct vfs_file_ops nfs_fops =
{
    vfs_nfs_open,
    vfs_nfs_close,
    vfs_nfs_ioctl,
    vfs_nfs_read,
    vfs_nfs_write,
    NULL, /* flush */
    vfs_nfs_lseek,
    vfs_nfs_getdents,
    NULL, /* poll */
};

static const struct vfs_filesystem_ops _nfs =
{
    "nfs",
    VFS_FS_FLAG_DEFAULT,
    &nfs_fops,
    vfs_nfs_mount,
    vfs_nfs_unmount,
    NULL, /* mkfs */
    vfs_nfs_statfs, /* statfs */
    vfs_nfs_unlink,
    vfs_nfs_stat,
    vfs_nfs_rename,
};


/**
 ***********************************************************************************************************************
 * @brief           Register nfs operation structure to VFS.
 *
 * @param[in,out]   None.
 *
 * @return          The register result. 
 * @retval          0               Register successfully.
 * @retval          -1              Register failed.
 ***********************************************************************************************************************
 */
int nfs_init(void)
{
    /* register nfs file system */
    return vfs_register(&_nfs);
}
OS_CMPOENT_INIT(nfs_init);

