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
 * @file        vfs_file.c
 *
 * @brief       This file implement file related operations of vfs.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <vfs.h>
#include <vfs_file.h>
#include "vfs_private.h"

#define VFS_FILE_TAG         "VFS_FILE"

/**
 ***********************************************************************************************************************
 * @brief           This function will open a file which specified by path with specified flags.
 *
 * @param[out]      fd        The file descriptor pointer to return the corresponding result.
 * @param[in]       path      The specified file path.
 * @param[in]       flags     the flags for open operation.
 *
 * @return          0 on successful or error code on failed.
 ***********************************************************************************************************************
 */ 
int vfs_file_open(struct vfs_fd *fd, const char *path, int flags)
{
    struct vfs_filesystem *fs;
    char *fullpath;
    int   result;

    /* Parameter check */
    if (fd == NULL)
    {
        return -EINVAL;
    }

    /* Make sure we have an absolute path */
    fullpath = vfs_normalize_path(NULL, path);
    if (fullpath == NULL)
    {
        return -ENOMEM;
    }

    LOG_D(VFS_FILE_TAG, "open file:%s", fullpath);

    /* Find filesystem */
    fs = vfs_filesystem_lookup(fullpath);
    if (fs == NULL)
    {
        os_free(fullpath);
        return -ENOENT;
    }

    LOG_D(VFS_FILE_TAG, "open in filesystem:%s", fs->ops->name);
    fd->fs    = fs;
    fd->fops  = fs->ops->fops;

    /* Initialize the fd item */
    fd->type  = FT_REGULAR;
    fd->flags = flags;
    fd->size  = 0;
    fd->pos   = 0;
    fd->data  = fs;

    if (!(fs->ops->flags & VFS_FS_FLAG_FULLPATH))
    {
        if (vfs_subdir(fs->path, fullpath) == NULL)
        {
            fd->path = os_strdup("/");
        }
        else
        {
            fd->path = os_strdup(vfs_subdir(fs->path, fullpath));
        }
        os_free(fullpath);
        LOG_D(VFS_FILE_TAG, "Actual file path: %s", fd->path);
    }
    else
    {
        fd->path = fullpath;
    }

    /* Specific file system open routine */
    if (fd->fops->open == NULL)
    {
        /* Clear fd */
        os_free(fd->path);
        fd->path = NULL;

        return -ENOSYS;
    }

    if ((result = fd->fops->open(fd)) < 0)
    {
        /* Clear fd */
        os_free(fd->path);
        fd->path = NULL;

        LOG_D(VFS_FILE_TAG, "%s open failed", fullpath);

        return result;
    }

    fd->flags |= VFS_F_OPEN;
    if (flags & O_DIRECTORY)
    {
        fd->type = FT_DIRECTORY;
        fd->flags |= VFS_F_DIRECTORY;
    }

    LOG_D(VFS_FILE_TAG, "open successful");
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Close a file descriptor.
 *
 * @param[in]      fd     The file descriptor to be closed.
 *
 * @return          0 on successful or error code on failed.
 ***********************************************************************************************************************
 */ 
int vfs_file_close(struct vfs_fd *fd)
{
    int result = 0;

    if (fd == NULL)
    {
        return -ENXIO;
    }

    if (fd->fops->close != NULL)
    {
        result = fd->fops->close(fd);
    }

    /* Close fd error, return */
    if (result < 0)
    {
        return result;
    }

    os_free(fd->path);
    fd->path = NULL;

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief          This function will perform a io control on a file descriptor.
 *
 * @param[in]      fd     The file descriptor.
 * @param[in]      cmd    The command to send to file descriptor.
 * @param[in/out]  args   the argument to send to file descriptor.
 *
 * @return         0 on successful or error code on failed.
 ***********************************************************************************************************************
 */  
int vfs_file_ioctl(struct vfs_fd *fd, int cmd, void *args)
{
    if (fd == NULL)
    {
        return -EINVAL;
    }

    /* Regular file system fd */
    if (fd->type == FT_REGULAR)
    {
        switch (cmd)
        {
        case F_GETFL:
            return fd->flags;
        case F_SETFL:
            {
                int flags = (int)(os_base_t)args;
                int mask  = O_NONBLOCK | O_APPEND;

                flags &= mask;
                fd->flags &= ~mask;
                fd->flags |= flags;
            }
            return 0;
        }
    }

    if (fd->fops->ioctl != NULL)
    {
        return fd->fops->ioctl(fd, cmd, args);
    }

    return -ENOSYS;
}

/**
 ***********************************************************************************************************************
 * @brief          This function will read specified length data from a file descriptor to a a buffer.
 *
 * @param[in]      fd     The file descriptor.
 * @param[out]     buf    The buffer where read data is stored.
 * @param[in]      len    The length of data to be read.
 *
 * @return         Return the actual read data bytes or 0 on end of file or failed.
 ***********************************************************************************************************************
 */ 
int vfs_file_read(struct vfs_fd *fd, void *buf, size_t len)
{
    int result = 0;

    if (fd == NULL)
    {
        return -EINVAL;
    }

    if (fd->fops->read == NULL)
    {
        return -ENOSYS;
    }

    if ((result = fd->fops->read(fd, buf, len)) < 0)
    {
        fd->flags |= VFS_F_EOF;
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief          This function will fetch directory entries from a directory descriptor.
 *
 * @param[in]      fd     The file descriptor.
 * @param[out]     dirp   The directory entry buffer to save result.
 * @param[in]      nbytes The available room in the buffer.
 *
 * @return         Return the read dirent, or error code on failed.
 ***********************************************************************************************************************
 */ 
int vfs_file_getdents(struct vfs_fd *fd, struct dirent *dirp, size_t nbytes)
{
    if (fd == NULL || fd->type != FT_DIRECTORY)
    {
        return -EINVAL;
    }

    if (fd->fops->getdents != NULL)
    {
        return fd->fops->getdents(fd, dirp, nbytes);
    }

    return -ENOSYS;
}

/**
 ***********************************************************************************************************************
 * @brief          This function will unlink (remove) a specified path file from file system.
 *
 * @param[in]      path   The specified file path.
 *
 * @return         Return 0 on successful, or error code on failed.
 ***********************************************************************************************************************
 */ 
int vfs_file_unlink(const char *path)
{
    int result;
    char *fullpath;
    struct vfs_filesystem *fs;

    /* Make sure we have an absolute path */
    fullpath = vfs_normalize_path(NULL, path);
    if (fullpath == NULL)
    {
        return -EINVAL;
    }

    /* Get filesystem */
    if ((fs = vfs_filesystem_lookup(fullpath)) == NULL)
    {
        result = -ENOENT;
        goto __exit;
    }

    /* Check whether file is already open */
    if (fd_is_open(fullpath) == 0)
    {
        result = -EBUSY;
        goto __exit;
    }

    if (fs->ops->unlink != NULL)
    {
        if (!(fs->ops->flags & VFS_FS_FLAG_FULLPATH))
        {
            if (vfs_subdir(fs->path, fullpath) == NULL)
            {
                result = fs->ops->unlink(fs, "/");
            }
            else
            {
                result = fs->ops->unlink(fs, vfs_subdir(fs->path, fullpath));
            }
        }
        else
        {
            result = fs->ops->unlink(fs, fullpath);
        }
    }
    else 
    {
        result = -ENOSYS;
    }

__exit:
    os_free(fullpath);
    return result;
}

/**
 ***********************************************************************************************************************
 * @brief          This function will write some specified length data to file system.
 *
 * @param[in]      fd     The file descriptor.
 * @param[in]      buf    The buffer of data to be written.
 * @param[in]      len    The length of data to be written.
 *
 * @return         Return the actual written data bytes, or error code if failed.
 ***********************************************************************************************************************
 */ 
int vfs_file_write(struct vfs_fd *fd, const void *buf, size_t len)
{
    if (fd == NULL)
    {
        return -EINVAL;
    }

    if (fd->fops->write == NULL)
    {
        return -ENOSYS;
    }

    return fd->fops->write(fd, buf, len);
}

/**
 ***********************************************************************************************************************
 * @brief          This function will flush buffer on a file descriptor
 *
 * @param[in]      fd   The specified file descriptor.
 *
 * @return         Return 0 on successful, or error code on failed.
 ***********************************************************************************************************************
 */ 
int vfs_file_flush(struct vfs_fd *fd)
{
    if (fd == NULL)
    {
        return -EINVAL;
    }

    if (fd->fops->flush == NULL)
    {
        return -ENOSYS;
    }

    return fd->fops->flush(fd);
}

/**
 ***********************************************************************************************************************
 * @brief          This function will seek the offset for specified file descriptor.
 *
 * @param[in]      fd       The specified file descriptor.
 * @param[in]      offset   The offset to be positioned.
 *
 * @return         Return the position after seek on success, or error code on failure.
 ***********************************************************************************************************************
 */ 
int vfs_file_lseek(struct vfs_fd *fd, off_t offset)
{
    int result;

    if (fd == NULL)
    {
        return -EINVAL;
    }

    if (fd->fops->lseek == NULL)
    {
        return -ENOSYS;
    }

    result = fd->fops->lseek(fd, offset);

    /* Update current position */
    if (result >= 0)
    {
        fd->pos = result;
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief          This function will get file stat information.
 *
 * @param[in]      path  The specified file path.
 * @param[out]      buf   The buffer where stat information is to be saved.
 *
 * @return         Return 0 on success, or error code on failure.
 ***********************************************************************************************************************
 */ 
int vfs_file_stat(const char *path, struct stat *buf)
{
    int result;
    char *fullpath;
    struct vfs_filesystem *fs;

    fullpath = vfs_normalize_path(NULL, path);
    if (fullpath == NULL)
    {
        return -EINVAL;
    }

    if ((fs = vfs_filesystem_lookup(fullpath)) == NULL)
    {
        LOG_E(VFS_FILE_TAG, "can't find mounted filesystem on this path:%s", fullpath);
        os_free(fullpath);

        return -ENOENT;
    }

    if ((fullpath[0] == '/' && fullpath[1] == '\0') ||
        (vfs_subdir(fs->path, fullpath) == NULL))
    {
        /* It's the root directory */
        buf->st_dev   = 0;

        buf->st_mode  = S_IRUSR | S_IRGRP | S_IROTH |
                        S_IWUSR | S_IWGRP | S_IWOTH;
        buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;

        buf->st_size    = 0;
        buf->st_mtime   = 0;

        /* Release full path */
        os_free(fullpath);

        return 0;
    }
    else
    {
        if (fs->ops->stat == NULL)
        {
            os_free(fullpath);
            LOG_E(VFS_FILE_TAG, "the filesystem didn't implement this function");

            return -ENOSYS;
        }

        /* Get the real file path and get file stat */
        if (fs->ops->flags & VFS_FS_FLAG_FULLPATH)
        {
            result = fs->ops->stat(fs, fullpath, buf);
        }
        else
        {
            result = fs->ops->stat(fs, vfs_subdir(fs->path, fullpath), buf);
        }
    }

    os_free(fullpath);

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief          This function will rename an old path name to a new path name.
 *
 * @param[in]      oldpath   The old path name.
 * @param[in]      newpath   The new file path name.
 *
 * @return         Return 0 on success, or error code on failure.
 ***********************************************************************************************************************
 */ 
int vfs_file_rename(const char *oldpath, const char *newpath)
{
    int result;
    struct vfs_filesystem *olvfs;
    struct vfs_filesystem *newfs;
    char *oldfullpath;
    char *newfullpath;

    result = 0;
    newfullpath = NULL;
    oldfullpath = NULL;

    oldfullpath = vfs_normalize_path(NULL, oldpath);
    if (oldfullpath == NULL)
    {
        result = -ENOENT;
        goto __exit;
    }

    newfullpath = vfs_normalize_path(NULL, newpath);
    if (newfullpath == NULL)
    {
        result = -ENOENT;
        goto __exit;
    }

    olvfs = vfs_filesystem_lookup(oldfullpath);
    newfs = vfs_filesystem_lookup(newfullpath);

    if ((olvfs == newfs) && olvfs)
    {
        if (olvfs->ops->rename == NULL)
        {
            result = -ENOSYS;
        }
        else
        {
            if (olvfs->ops->flags & VFS_FS_FLAG_FULLPATH)
            {
                result = olvfs->ops->rename(olvfs, oldfullpath, newfullpath);
            }
            else
            {
                /* Use sub directory to rename in file system */
                result = olvfs->ops->rename(olvfs,
                                            vfs_subdir(olvfs->path, oldfullpath),
                                            vfs_subdir(newfs->path, newfullpath));
            }
        }
    }
    else
    {
        result = -EXDEV;
    }

__exit:
    os_free(oldfullpath);
    os_free(newfullpath);

    /* Not at same file system, return EXDEV */
    return result;
}

#ifdef OS_USING_SHELL
#include <shell.h>

static struct vfs_fd fd;
static struct dirent dirent;

/**
 ***********************************************************************************************************************
 * @brief          This function will list out subdirs and files under a specified dir.
 *
 * @param[in]      pathname   The pathname to be listed.
 *
 * @return         No return value..
 ***********************************************************************************************************************
 */ 
void ls(const char *pathname)
{
    struct stat stat;
    int         length;
    char       *fullpath;
    char       *path;

    fullpath = NULL;
    if (pathname == NULL)
    {
#ifdef VFS_USING_WORKDIR
        /* Open current working directory */
        path = os_strdup(working_directory);
#else
        path = os_strdup("/");
#endif
        if (path == NULL)
        {
            return ; /* Out of memory */
        }
    }
    else
    {
        path = (char *)pathname;
    }

    /* List directory */
    if (vfs_file_open(&fd, path, O_DIRECTORY) == 0)
    {
        os_kprintf("Directory %s:\n", path);
        do
        {
            memset(&dirent, 0, sizeof(struct dirent));
            length = vfs_file_getdents(&fd, &dirent, sizeof(struct dirent));
            if (length > 0)
            {
                memset(&stat, 0, sizeof(struct stat));

                /* Build full path for each file */
                fullpath = vfs_normalize_path(path, dirent.d_name);
                if (fullpath == NULL)
                {
                    break;
                }

                if (vfs_file_stat(fullpath, &stat) == 0)
                {
                    os_kprintf("%-20s", dirent.d_name);
                    if (S_ISDIR(stat.st_mode))
                    {
                        os_kprintf("%-25s\n", "<DIR>");
                    }
                    else
                    {
                        os_kprintf("%-25lu\n", stat.st_size);
                    }
                }
                else
                {
                    os_kprintf("BAD file: %s\n", dirent.d_name);
                }
                os_free(fullpath);
            }
        }
        while (length > 0);

        vfs_file_close(&fd);
    }
    else
    {
        os_kprintf("No such directory\n");
    }
    if (pathname == NULL)
    {
        os_free(path);
    }
}

/**
 ***********************************************************************************************************************
 * @brief          This function will remove the specified file.
 *
 * @param[in]      filename  The file to be removed.
 *
 * @return         No return value..
 ***********************************************************************************************************************
 */ 
void rm(const char *filename)
{
    if (vfs_file_unlink(filename) < 0)
    {
        os_kprintf("Delete %s failed\r\n", filename);
    }
}

/**
 ***********************************************************************************************************************
 * @brief          This function will print out a specified file.
 *
 * @param[in]      filename   The name of file to be printed.
 *
 * @return         No return value..
 ***********************************************************************************************************************
 */ 
void cat(const char *filename)
{
    uint32_t length;
    char     buffer[81];

    if (vfs_file_open(&fd, filename, O_RDONLY) < 0)
    {
        os_kprintf("Open %s failed\r\n", filename);

        return;
    }

    do
    {
        memset(buffer, 0, sizeof(buffer));
        length = vfs_file_read(&fd, buffer, sizeof(buffer) - 1);
        if (length > 0)
        {
            os_kprintf("%s", buffer);
        }
    }
    while (length > 0);
    os_kprintf("\r\n");

    vfs_file_close(&fd);
}

#define BUF_SZ  4096
static void copyfile(const char *src, const char *dst)
{
    struct vfs_fd src_fd;
    os_uint8_t   *block_ptr;
    os_int32_t    read_bytes;

    block_ptr = os_malloc(BUF_SZ);
    if (block_ptr == NULL)
    {
        os_kprintf("out of memory\r\n");

        return;
    }

    if (vfs_file_open(&src_fd, src, O_RDONLY) < 0)
    {
        os_free(block_ptr);
        os_kprintf("Read %s failed\r\n", src);

        return;
    }
    if (vfs_file_open(&fd, dst, O_WRONLY | O_CREAT) < 0)
    {
        os_free(block_ptr);
        vfs_file_close(&src_fd);

        os_kprintf("Write %s failed\r\n", dst);

        return;
    }

    do
    {
        read_bytes = vfs_file_read(&src_fd, block_ptr, BUF_SZ);
        if (read_bytes > 0)
        {
            int length;

            length = vfs_file_write(&fd, block_ptr, read_bytes);
            if (length != read_bytes)
            {
                /* Write failed. */
                os_kprintf("Write file data failed, errno=%d\r\n", length);
                break;
            }
        }
    }
    while (read_bytes > 0);

    vfs_file_close(&src_fd);
    vfs_file_close(&fd);
    os_free(block_ptr);
}

extern int mkdir(const char *path, mode_t mode);
static void copydir(const char *src, const char *dst)
{
    struct dirent dirent;
    struct stat stat;
    int length;
    struct vfs_fd cpfd;
    if (vfs_file_open(&cpfd, src, O_DIRECTORY) < 0)
    {
        os_kprintf("open %s failed\r\n", src);
        return ;
    }

    do
    {
        memset(&dirent, 0, sizeof(struct dirent));

        length = vfs_file_getdents(&cpfd, &dirent, sizeof(struct dirent));
        if (length > 0)
        {
            char *src_entry_full = NULL;
            char *dst_entry_full = NULL;

            if (strcmp(dirent.d_name, "..") == 0 || strcmp(dirent.d_name, ".") == 0)
                continue;

            /* Build full path for each file */
            if ((src_entry_full = vfs_normalize_path(src, dirent.d_name)) == NULL)
            {
                os_kprintf("out of memory!\r\n");
                break;
            }
            if ((dst_entry_full = vfs_normalize_path(dst, dirent.d_name)) == NULL)
            {
                os_kprintf("out of memory!\r\n");
                os_free(src_entry_full);
                break;
            }

            memset(&stat, 0, sizeof(struct stat));
            if (vfs_file_stat(src_entry_full, &stat) != 0)
            {
                os_kprintf("open file: %s failed\r\n", dirent.d_name);
                continue;
            }

            if (S_ISDIR(stat.st_mode))
            {
                mkdir(dst_entry_full, 0);
                copydir(src_entry_full, dst_entry_full);
            }
            else
            {
                copyfile(src_entry_full, dst_entry_full);
            }
            os_free(src_entry_full);
            os_free(dst_entry_full);
        }
    }
    while (length > 0);

    vfs_file_close(&cpfd);
}

static const char *_get_path_lastname(const char *path)
{
    char *ptr;
    if ((ptr = strrchr(path, '/')) == NULL)
        return path;

    /* Skip the '/' then return */
    return ++ptr;
}

/**
 ***********************************************************************************************************************
 * @brief          This function will make a copy of specified pathname, with the specified dst pathname.
 *
 * @param[in]      src   The source pathname to be copied.
 * @param[in]      dst   The destination path name of the copy.
 *
 * @return         No return value..
 ***********************************************************************************************************************
 */ 
void copy(const char *src, const char *dst)
{
#define FLAG_SRC_TYPE      0x03
#define FLAG_SRC_IS_DIR    0x01
#define FLAG_SRC_IS_FILE   0x02
#define FLAG_SRC_NON_EXSIT 0x00

#define FLAG_DST_TYPE      0x0C
#define FLAG_DST_IS_DIR    0x04
#define FLAG_DST_IS_FILE   0x08
#define FLAG_DST_NON_EXSIT 0x00

    struct stat stat;
    uint32_t flag = 0;

    /* Check the staus of src and dst */
    if (vfs_file_stat(src, &stat) < 0)
    {
        os_kprintf("copy failed, bad %s\r\n", src);
        return;
    }
    if (S_ISDIR(stat.st_mode))
    {
        flag |= FLAG_SRC_IS_DIR;
    }
    else
    {
        flag |= FLAG_SRC_IS_FILE;
    }

    if (vfs_file_stat(dst, &stat) < 0)
    {
        flag |= FLAG_DST_NON_EXSIT;
    }
    else
    {
        if (S_ISDIR(stat.st_mode))
        {
            flag |= FLAG_DST_IS_DIR;
        }
        else
        {
            flag |= FLAG_DST_IS_FILE;
        }
    }

    //2. check status
    if ((flag & FLAG_SRC_IS_DIR) && (flag & FLAG_DST_IS_FILE))
    {
        os_kprintf("cp faild, cp dir to file is not permitted!\r\n");
        return ;
    }

    //3. do copy
    if (flag & FLAG_SRC_IS_FILE)
    {
        if (flag & FLAG_DST_IS_DIR)
        {
            char *fdst;
            fdst = vfs_normalize_path(dst, _get_path_lastname(src));
            if (fdst == NULL)
            {
                os_kprintf("out of memory\r\n");
                return;
            }
            copyfile(src, fdst);
            os_free(fdst);
        }
        else
        {
            copyfile(src, dst);
        }
    }
    else
    {
        if (flag & FLAG_DST_IS_DIR)
        {
            char *fdst;
            fdst = vfs_normalize_path(dst, _get_path_lastname(src));
            if (fdst == NULL)
            {
                os_kprintf("out of memory\r\n");
                return;
            }
            mkdir(fdst, 0);
            copydir(src, fdst);
            os_free(fdst);
        }
        else if ((flag & FLAG_DST_TYPE) == FLAG_DST_NON_EXSIT)
        {
            mkdir(dst, 0);
            copydir(src, dst);
        }
        else
        {
            copydir(src, dst);
        }
    }
}

#endif /* OS_USING_SHELL */

