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
 * @file        vfs_posix.c
 *
 * @brief       This file implements POSIX I/O functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-22   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <vfs.h>
#include <vfs_posix.h>
#include "vfs_private.h"

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will open a file and return a file descriptor
 *                  according specified flags.
 *
 * @param[in]       file            The path name of file
 * @param[in]       flags           The file open flags
 *
 * @return          An non-negative file descriptor
 * @retval          >=0             Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int open(const char *file, int flags, ...)
{
    int fd, result;
    struct vfs_fd *d;

    /* Allocate a fd */
    fd = fd_new();
    if (fd < 0)
    {
        os_set_errno(-ENOMEM);

        return -1;
    }
    d = fd_get(fd);

    result = vfs_file_open(d, file, flags);
    if (result < 0)
    {
        /* Release the ref-count of fd */
        fd_put(d);
        fd_put(d);

        os_set_errno(result);

        return -1;
    }

    /* Release the ref-count of fd */
    fd_put(d);

    return fd;
}
EXPORT_SYMBOL(open);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will close the open file descriptor.
 *
 * @param[in]       fd              The file descriptor.
 * @param[in]       flags           The file open flags.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int close(int fd)
{
    int result;
    struct vfs_fd *d;

    d = fd_get(fd);
    if (d == NULL)
    {
        os_set_errno(-EBADF);

        return -1;
    }

    result = vfs_file_close(d);
    fd_put(d);

    if (result < 0)
    {
        os_set_errno(result);

        return -1;
    }

    fd_put(d);

    return 0;
}
EXPORT_SYMBOL(close);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will read specified data buffer length for
 *                  an open file descriptor.
 *
 * @param[in]       fd              The file descriptor.
 * @param[out]      buf             The buffer to save the read data.
 * @param[in]       len             The maximal length of data buffer.
 *
 * @return          The actual read data buffer length.
 * @retval          >0              Success
 * @retval          0               Reach the end of file
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
#if defined(OS_USING_NEWLIB) && defined(_EXFUN)
_READ_WRITE_RETURN_TYPE _EXFUN(read, (int fd, void *buf, size_t len))
#else
int read(int fd, void *buf, size_t len)
#endif
{
    int result;
    struct vfs_fd *d;

    /* Get the fd */
    d = fd_get(fd);
    if (d == NULL)
    {
        os_set_errno(-EBADF);

        return -1;
    }

    result = vfs_file_read(d, buf, len);
    if (result < 0)
    {
        fd_put(d);
        os_set_errno(result);

        return -1;
    }

    /* Release the ref-count of fd */
    fd_put(d);

    return result;
}
EXPORT_SYMBOL(read);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will write specified data buffer length for
 *                  an open file descriptor.
 *
 * @param[in]       fd              The file descriptor.
 * @param[in]       buf             The data buffer to be written.
 * @param[in]       len             The data buffer length.
 *
 * @return          The actual written data buffer length.
 * @retval          >=0             Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
#if defined(OS_USING_NEWLIB) && defined(_EXFUN)
_READ_WRITE_RETURN_TYPE _EXFUN(write, (int fd, const void *buf, size_t len))
#else
int write(int fd, const void *buf, size_t len)
#endif
{
    int result;
    struct vfs_fd *d;

    /* Get the fd */
    d = fd_get(fd);
    if (d == NULL)
    {
        os_set_errno(-EBADF);

        return -1;
    }

    result = vfs_file_write(d, buf, len);
    if (result < 0)
    {
        fd_put(d);
        os_set_errno(result);

        return -1;
    }

    /* Release the ref-count of fd */
    fd_put(d);

    return result;
}
EXPORT_SYMBOL(write);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will seek the offset for an open file descriptor.
 *
 * @param[in]       fd              The file descriptor.
 * @param[in]       offset          The offset to be seeked.
 * @param[in]       whence          The directory of seek.
 *
 * @return          The current read/write position in the file.
 * @retval          >=0             Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
off_t lseek(int fd, off_t offset, int whence)
{
    int result;
    struct vfs_fd *d;

    d = fd_get(fd);
    if (d == NULL)
    {
        os_set_errno(-EBADF);

        return -1;
    }

    switch (whence)
    {
    case SEEK_SET:
        break;

    case SEEK_CUR:
        offset += d->pos;
        break;

    case SEEK_END:
        offset += d->size;
        break;

    default:
        fd_put(d);
        os_set_errno(-EINVAL);

        return -1;
    }

    if (offset < 0)
    {
        fd_put(d);
        os_set_errno(-EINVAL);

        return -1;
    }
    result = vfs_file_lseek(d, offset);
    if (result < 0)
    {
        fd_put(d);
        os_set_errno(result);

        return -1;
    }

    /* Release the ref-count of fd */
    fd_put(d);

    return offset;
}
EXPORT_SYMBOL(lseek);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will rename old file name to new file name.
 *
 * @attention       The old and new file name must be belong to a same file system.
 *
 * @param[in]       old             The old file name.
 * @param[in]       new             The new file name.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int rename(const char *old, const char *new)
{
    int result;

    result = vfs_file_rename(old, new);
    if (result < 0)
    {
        os_set_errno(result);

        return -1;
    }

    return 0;
}
EXPORT_SYMBOL(rename);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will unlink (remove) a specified path file from
 *                  file system.
 *
 * @param[in]       pathname        The specified path name to be unlinked.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int unlink(const char *pathname)
{
    int result;

    result = vfs_file_unlink(pathname);
    if (result < 0)
    {
        os_set_errno(result);

        return -1;
    }

    return 0;
}
EXPORT_SYMBOL(unlink);

#ifndef _WIN32
/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will get file information.
 *
 * @param[in]       path            The file name.
 * @param[out]      buf             The data buffer to save stat description.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int stat(const char *path, struct stat *buf)
{
    int result;

    result = vfs_file_stat(path, buf);
    if (result < 0)
    {
        os_set_errno(result);

        return -1;
    }

    return result;
}
EXPORT_SYMBOL(stat);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will get file status.
 *
 * @param[in]       fd              The file descriptor.
 * @param[out]      buf             The data buffer to save stat description.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int fstat(int fd, struct stat *buf)
{
    struct vfs_fd *d;

    /* Get the fd */
    d = fd_get(fd);
    if (d == NULL)
    {
        os_set_errno(-EBADF);

        return -1;
    }

    /* It's the root directory */
    buf->st_dev = 0;

    buf->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                             S_IWUSR | S_IWGRP | S_IWOTH;
    if (d->type == FT_DIRECTORY)
    {
        buf->st_mode &= ~S_IFREG;
        buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    }

    buf->st_size    = d->size;
    buf->st_mtime   = 0;

    fd_put(d);

    return OS_EOK;
}
EXPORT_SYMBOL(fstat);
#endif

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which shall request that all data for the open file
 *                  descriptor named by fd is to be transferred to the storage device associated with the file described by fd.
 *
 * @param[in]       fd              The file descriptor.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int fsync(int fd)
{
    int ret;
    struct vfs_fd *d;

    /* Get the fd */
    d = fd_get(fd);
    if (d == NULL)
    {
        os_set_errno(-EBADF);
        return -1;
    }

    ret = vfs_file_flush(d);

    fd_put(d);
    return ret;
}
EXPORT_SYMBOL(fsync);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which shall perform a variety of control functions on
 *                  devices.
 *
 * @param[in]       fd              The file descriptor.
 * @param[in]       cmd             The specified command.
 * @param[in]       data            The data represents the additional information that is needed by this specific device
 *                                  to perform the requested function.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int fcntl(int fd, int cmd, ...)
{
    int ret = -1;
    struct vfs_fd *d;

    /* Get the fd */
    d = fd_get(fd);
    if (d)
    {
        void *arg;
        va_list ap;

        va_start(ap, cmd);
        arg = va_arg(ap, void *);
        va_end(ap);

        ret = vfs_file_ioctl(d, cmd, arg);
        fd_put(d);
    }
    else ret = -EBADF;

    if (ret < 0)
    {
        os_set_errno(ret);
        ret = -1;
    }

    return ret;
}
EXPORT_SYMBOL(fcntl);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which shall perform a variety of control functions on
 *                  devices.
 *
 * @param[in]       fd              The file descriptor.
 * @param[in]       cmd             The specified command.
 * @param[in]       data            The data represents the additional information that is needed by this specific device
 *                                  to perform the requested function.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int ioctl(int fd, int cmd, ...)
{
    void   *arg;
    va_list ap;

    va_start(ap, cmd);
    arg = va_arg(ap, void *);
    va_end(ap);

    /* We use fcntl for this API. */
    return fcntl(fd, cmd, arg);
}
EXPORT_SYMBOL(ioctl);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will return the information about a mounted file
 *                  system.
 *
 * @param[in]       path            The path which mounted file system.
 * @param[out]      buf             The buffer to save the returned information.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int statfs(const char *path, struct statfs *buf)
{
    int result;

    result = vfs_statfs(path, buf);
    if (result < 0)
    {
        os_set_errno(result);

        return -1;
    }

    return result;
}
EXPORT_SYMBOL(statfs);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will make a directory,
 *
 * @param[in]       path            The directory path to be made.
 * @param[in]       mode            Not supported.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int mkdir(const char *path, mode_t mode)
{
    int fd;
    struct vfs_fd *d;
    int result;

    fd = fd_new();
    if (fd == -1)
    {
        os_set_errno(-ENOMEM);

        return -1;
    }

    d = fd_get(fd);

    result = vfs_file_open(d, path, O_DIRECTORY | O_CREAT);

    if (result < 0)
    {
        fd_put(d);
        fd_put(d);
        os_set_errno(result);

        return -1;
    }

    vfs_file_close(d);
    fd_put(d);
    fd_put(d);

    return 0;
}
EXPORT_SYMBOL(mkdir);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will remove a directory,
 *
 * @param[in]       pathname        The path name to be removed.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int rmdir(const char *pathname)
{
    int result;

    result = vfs_file_unlink(pathname);
    if (result < 0)
    {
        os_set_errno(result);

        return -1;
    }

    return 0;
}
EXPORT_SYMBOL(rmdir);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will open a directory.
 *
 * @param[in]       name            The path name to be removed.
 *
 * @return          A pointer to the directory stream.
 * @retval          pointer         Success
 * @retval          NULL            Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
DIR *opendir(const char *name)
{
    struct vfs_fd *d;
    int fd;
    int result;
    DIR *t;

    t = NULL;

    /* Allocate a fd */
    fd = fd_new();
    if (fd == -1)
    {
        os_set_errno(-ENOMEM);

        return NULL;
    }
    d = fd_get(fd);

    result = vfs_file_open(d, name, O_RDONLY | O_DIRECTORY);
    if (result >= 0)
    {
        /* Open successfully */
        t = (DIR *) os_malloc(sizeof(DIR));
        if (t == NULL)
        {
            vfs_file_close(d);
            fd_put(d);
        }
        else
        {
            memset(t, 0, sizeof(DIR));
            t->fd = fd;
        }
        fd_put(d);

        return t;
    }

    /* Open failed */
    fd_put(d);
    fd_put(d);
    os_set_errno(result);

    return NULL;
}
EXPORT_SYMBOL(opendir);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will return a pointer to a dirent structure representing
 *                  the next directory entry in the directory stream.
 *
 * @param[in]       d               The pinter to directory stream.
 *
 * @return          The next directory entry.
 * @retval          pointer         Success
 * @retval          NULL            Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
struct dirent *readdir(DIR *d)
{
    int result;
    struct vfs_fd *fd;

    fd = fd_get(d->fd);
    if (fd == NULL)
    {
        os_set_errno(-EBADF);
        return NULL;
    }

    if (d->num)
    {
        struct dirent *dirent_ptr;
        dirent_ptr = (struct dirent *)&d->buf[d->cur];
        d->cur += dirent_ptr->d_reclen;
    }

    if (!d->num || d->cur >= d->num)
    {
        /* Get a new entry */
        result = vfs_file_getdents(fd,
                                   (struct dirent *)d->buf,
                                   sizeof(d->buf) - 1);
        if (result <= 0)
        {
            fd_put(fd);
            os_set_errno(result);

            return NULL;
        }

        d->num = result;
        d->cur = 0; /* Current entry index */
    }

    fd_put(fd);

    return (struct dirent *)(d->buf + d->cur);
}
EXPORT_SYMBOL(readdir);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will return current location in directory stream.
 *
 * @param[in]       d               The pinter to directory stream.
 *
 * @return          The current location in directory stream.
 * @retval          >=0             Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
long telldir(DIR *d)
{
    struct vfs_fd *fd;
    long result;

    fd = fd_get(d->fd);
    if (fd == NULL)
    {
        os_set_errno(-EBADF);

        return -1;
    }

    result = fd->pos - d->num + d->cur;
    fd_put(fd);

    return result;
}
EXPORT_SYMBOL(telldir);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will return current location in directory stream.
 *
 * @param[in]       d               The pinter to directory stream.
 * @param[in]       offset          The offset in directory stream.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void seekdir(DIR *d, off_t offset)
{
    struct vfs_fd *fd;

    fd = fd_get(d->fd);
    if (fd == NULL)
    {
        os_set_errno(-EBADF);

        return ;
    }

    /* Seek to the offset position of directory */
    if (vfs_file_lseek(fd, offset) >= 0)
    {
        d->num = d->cur = 0;
    }
    fd_put(fd);
}
EXPORT_SYMBOL(seekdir);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will reset directory stream.
 *
 * @param[in]       d               The pinter to directory stream.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void rewinddir(DIR *d)
{
    struct vfs_fd *fd;

    fd = fd_get(d->fd);
    if (fd == NULL)
    {
        os_set_errno(-EBADF);

        return;
    }

    /* Seek to the beginning of directory */
    if (vfs_file_lseek(fd, 0) >= 0)
    {
        d->num = d->cur = 0;
    }
    fd_put(fd);
}
EXPORT_SYMBOL(rewinddir);

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will close a directory stream.
 *
 * @param[in]       d               The pinter to directory stream.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int closedir(DIR *d)
{
    int result;
    struct vfs_fd *fd;

    fd = fd_get(d->fd);
    if (fd == NULL)
    {
        os_set_errno(-EBADF);

        return -1;
    }

    result = vfs_file_close(fd);
    fd_put(fd);

    fd_put(fd);
    os_free(d);

    if (result < 0)
    {
        os_set_errno(result);

        return -1;
    }
    else
    {
        return 0;
    }
}
EXPORT_SYMBOL(closedir);

#ifdef VFS_USING_WORKDIR

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will change working directory.
 *
 * @param[in]       path            The path name to be changed to.
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int chdir(const char *path)
{
    char *fullpath;
    DIR *d;

    if (path == NULL)
    {
        vfs_lock();
        os_kprintf("%s\n", working_directory);
        vfs_unlock();

        return 0;
    }

    if (strlen(path) > VFS_PATH_MAX)
    {
        os_set_errno(-ENOTDIR);

        return -1;
    }

    fullpath = vfs_normalize_path(NULL, path);
    if (fullpath == NULL)
    {
        os_set_errno(-ENOTDIR);
        
        return -1;
    }

    vfs_lock();
    d = opendir(fullpath);
    if (d == NULL)
    {
        os_free(fullpath);
        /* This is a not exist directory */
        vfs_unlock();

        return -1;
    }

    /* Close directory stream */
    closedir(d);

    /* Copy full path to working directory */
    strncpy(working_directory, fullpath, VFS_PATH_MAX);
    /* Release normalize directory path name */
    os_free(fullpath);

    vfs_unlock();

    return 0;
}
EXPORT_SYMBOL(chdir);

#endif /* VFS_USING_WORKDIR */

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which shall check the file named by the pathname pointed
 *                  to by the path argument for accessibility according to the bit pattern contained in amode.
 *
 * @param[in]       path            The specified file/dir path.
 * @param[in]       amode           The value is either the bitwise-inclusive OR of the access permissions to be
 *                                  checked (R_OK, W_OK, X_OK) or the existence test (F_OK).
 *
 * @return          The result of the operation.
 * @retval          0               Success
 * @retval          -1              Failure and errno set to indicate the error
 ***********************************************************************************************************************
 */
int access(const char *path, int amode)
{
    struct stat sb;
    if (stat(path, &sb) < 0)
    {
        return -1; /* Already sets errno */
    }

    /* Ignore R_OK,W_OK,X_OK condition */
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which will return current working directory.
 *
 * @param[out]      buf             The returned current directory.
 * @param[in]       size            The buffer size.
 *
 * @return          The returned current directory.
 ***********************************************************************************************************************
 */
char *getcwd(char *buf, size_t size)
{
#ifdef VFS_USING_WORKDIR
    vfs_lock();
    strncpy(buf, working_directory, size);
    vfs_unlock();
#else
    os_kprintf(NO_WORKING_DIR);
#endif

    return buf;
}
EXPORT_SYMBOL(getcwd);

