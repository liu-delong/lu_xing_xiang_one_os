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
 * @file        vfs.c
 *
 * @brief       This file implements the vfs system.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-09   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <vfs.h>
#include <vfs_fs.h>
#include <vfs_file.h>
#include "vfs_private.h"
#ifdef OS_USING_LWP
#include <lwp.h>
#endif

#if defined(OS_USING_VFS_DEVFS) && defined(OS_USING_POSIX)
#include <libc.h>
#endif

#define VFS_TAG           "VFS"

/* Used to manage registered file systems */
const struct vfs_filesystem_ops *filesystem_operation_table[VFS_FILESYSTEM_TYPES_MAX];

/* Used to manage mounted filesystem devices */
struct vfs_filesystem filesystem_table[VFS_FILESYSTEMS_MAX];

/* Virtual filesystem lock */
static struct os_mutex fslock;

#ifdef VFS_USING_WORKDIR
char working_directory[VFS_PATH_MAX] = {"/"};
#endif

static struct vfs_fdtable _fdtab;
static int  fd_alloc(struct vfs_fdtable *fdt, int startfd);

/**
 ***********************************************************************************************************************
 * @brief           This function initializes virtual file system.
 *
 * @param           None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
int vfs_init(void)
{
    static os_bool_t init_ok = OS_FALSE;

    if (init_ok)
    {
        LOG_W(VFS_TAG, "vfs already init.");
        return 0;
    }

    /* Clear filesystem operations table */
    memset((void *)filesystem_operation_table, 0, sizeof(filesystem_operation_table));
    /* Clear filesystem table */
    memset(filesystem_table, 0, sizeof(filesystem_table));
    /* Clean fd table */
    memset(&_fdtab, 0, sizeof(_fdtab));

    /* Create vfs filesystem lock */
    os_mutex_init(&fslock, "fslock", OS_IPC_FLAG_FIFO, OS_TRUE);

#ifdef VFS_USING_WORKDIR
    /* Set current working directory */
    memset(working_directory, 0, sizeof(working_directory));
    working_directory[0] = '/';
#endif

#ifdef OS_USING_VFS_DEVFS
    {
        extern int vfs_devfs_init(void);

        /* If enable vfs, initialize and mount it as soon as possible */
        vfs_devfs_init();

        vfs_mount(NULL, "/dev", "devfs", 0, 0);
    }
#endif

    init_ok = OS_TRUE;

    return 0;
}
OS_PREV_INIT(vfs_init);

/**
***********************************************************************************************************************
* @brief           This function locks virtual file system.
*
* @param           None.
*
* @return          None.
***********************************************************************************************************************
*/
void vfs_lock(void)
{
    os_err_t result = OS_EBUSY;

    while (result == OS_EBUSY)
    {
        result = os_mutex_recursive_lock(&fslock, OS_IPC_WAITING_FOREVER);
    }

    if (result != OS_EOK)
    {
        OS_ASSERT(0);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function unlocks virtual file system.
 *
 * @param           None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void vfs_unlock(void)
{
    os_mutex_recursive_unlock(&fslock);
}

/**
 ***********************************************************************************************************************
 * @brief           This function allocates a non-negative integer representing the lowest numbered unused file descriptor.
 *
 * @param[in]       fdt             The fd table.
 * @param[in]       startfd         The start index to search.
 *
 * @return          A non-negative integer.
 ***********************************************************************************************************************
 */
static int fd_alloc(struct vfs_fdtable *fdt, int startfd)
{
    int idx;

    /* Find an empty fd entry */
    for (idx = startfd; idx < (int)fdt->maxfd; idx++)
    {
        if (fdt->fds[idx] == OS_NULL)
        {
            break;
        }
        if (fdt->fds[idx]->ref_count == 0)
        {
            break;
        }
    }

    /* Allocate a larger FD container */
    if (idx == fdt->maxfd && fdt->maxfd < VFS_FD_MAX)
    {
        int cnt, index;
        struct vfs_fd **fds;

        /* Increase the number of FD with 4 step length */
        cnt = fdt->maxfd + 4;
        cnt = cnt > VFS_FD_MAX ? VFS_FD_MAX : cnt;

        fds = os_realloc(fdt->fds, cnt * sizeof(struct vfs_fd *));
        if (fds == NULL) 
        {
            goto __exit; /* return fdt->maxfd */
        }
        
        /* Clean the new allocated fds */
        for (index = fdt->maxfd; index < cnt; index ++)
        {
            fds[index] = NULL;
        }

        fdt->fds   = fds;
        fdt->maxfd = cnt;
    }

    /* Allocate  'struct vfs_fd' */
    if (idx < (int)fdt->maxfd && fdt->fds[idx] == OS_NULL)
    {
        fdt->fds[idx] = os_calloc(1, sizeof(struct vfs_fd));
        if (fdt->fds[idx] == OS_NULL)
        {
            idx = fdt->maxfd;
        }
    }

__exit:
    return idx;
}

/**
 ***********************************************************************************************************************
 * @brief           This function allocats a file descriptor.
 *
 * @param           None.
 *
 * @return          A file descriptor.
 * @retval          non-negative integer  success.
 * @retval          -1                    failure.
 ***********************************************************************************************************************
 */
int fd_new(void)
{
    struct vfs_fd *d;
    int idx;
    struct vfs_fdtable *fdt;

    fdt = vfs_fdtable_get();

    vfs_lock();

    /* Find an empty fd entry */
    idx = fd_alloc(fdt, 0);

    /* Can't find an empty fd entry */
    if (idx == fdt->maxfd)
    {
        idx = -(1 + VFS_FD_OFFSET);
        LOG_E(VFS_TAG, "VFS fd new is failed! Could not found an empty fd entry.");
        goto __result;
    }

    d = fdt->fds[idx];
    d->ref_count = 1;
    d->magic = VFS_FD_MAGIC;

__result:
    vfs_unlock();
    return idx + VFS_FD_OFFSET;
}

/**
 ***********************************************************************************************************************
 * @brief           This function returns a file descriptor structure according to file.
 *
 * @param[in]       fd              A non-negative integer.
 *
 * @return          File decriptor entry.
 * @retval          OS_NULL         failure
 * @retval          otherwise       success
 ***********************************************************************************************************************
 */
struct vfs_fd *fd_get(int fd)
{
    struct vfs_fd *d;
    struct vfs_fdtable *fdt;

#if defined(OS_USING_VFS_DEVFS) && defined(OS_USING_POSIX)
    if ((0 <= fd) && (fd <= 2))
    {
        fd = libc_stdio_get_console();
    }
#endif

    fdt = vfs_fdtable_get();
    fd = fd - VFS_FD_OFFSET;
    if (fd < 0 || fd >= (int)fdt->maxfd)
    {
        return NULL;
    }

    vfs_lock();
    d = fdt->fds[fd];

    /* Check vfs_fd valid or not */
    if ((d == NULL) || (d->magic != VFS_FD_MAGIC))
    {
        vfs_unlock();
        return NULL;
    }

    /* Increase the reference count */
    d->ref_count ++;
    vfs_unlock();

    return d;
}

/**
 ***********************************************************************************************************************
 * @brief           This function puts the file descriptor.
 *
 * @param[in]       fd              The file descriptor entry.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void fd_put(struct vfs_fd *fd)
{
    OS_ASSERT(fd != NULL);

    vfs_lock();

    fd->ref_count --;

    /* Clear this fd entry */
    if (fd->ref_count == 0)
    {
        int index;
        struct vfs_fdtable *fdt;

        fdt = vfs_fdtable_get();
        for (index = 0; index < (int)fdt->maxfd; index ++)
        {
            if (fdt->fds[index] == fd)
            {
                os_free(fd);
                fdt->fds[index] = 0;
                break;
            }
        }
    }
    vfs_unlock();
}

/**
 ***********************************************************************************************************************
 * @brief           This function returns whether this file has been opend.
 *
 * @param[in]       pathname        The file path name.
 *
 * @return          an integer
 * @retval          0               The file has been opened
 * @retval          -1              The file has not been opened
 ***********************************************************************************************************************
 */
int fd_is_open(const char *pathname)
{
    char *fullpath;
    unsigned int index;
    struct vfs_filesystem *fs;
    struct vfs_fd *fd;
    struct vfs_fdtable *fdt;

    fdt = vfs_fdtable_get();
    fullpath = vfs_normalize_path(NULL, pathname);
    if (fullpath != NULL)
    {
        char *mountpath;
        fs = vfs_filesystem_lookup(fullpath);
        if (fs == NULL)
        {
            /* Can't find mounted file system */
            os_free(fullpath);
            return -1;
        }

        /* Get file path name under mounted file system */
        if (fs->path[0] == '/' && fs->path[1] == '\0')
        {
            mountpath = fullpath;
        }
        else
        {
            mountpath = fullpath + strlen(fs->path);
        }

        vfs_lock();

        for (index = 0; index < fdt->maxfd; index++)
        {
            fd = fdt->fds[index];
            if (fd == NULL || fd->fops == NULL || fd->path == NULL)
            {
                continue;
            }

            if (fd->fs == fs && strcmp(fd->path, mountpath) == 0)
            {
                /* Found file in file descriptor table */
                os_free(fullpath);
                vfs_unlock();

                return 0;
            }
        }
        vfs_unlock();

        os_free(fullpath);
    }

    return -1;
}

/**
 ***********************************************************************************************************************
 * @brief           This function returns a sub-path name under directory.
 *
 * @param[in]       pathname        The parent directory.
 *
 * @param[in]       filename        The filename.
 *
 * @return          The sub-path name
 * @retval          char *          success
 * @retval          NULL            failure
 ***********************************************************************************************************************
 */
const char *vfs_subdir(const char *directory, const char *filename)
{
    const char *dir;

    if (strlen(directory) == strlen(filename))
    {
        return NULL;
    }

    dir = filename + strlen(directory);
    if ((*dir != '/') && (dir != filename))
    {
        dir --;
    }

    return dir;
}
EXPORT_SYMBOL(vfs_subdir);

/**
 ***********************************************************************************************************************
 * @brief           This function normalizes a path according to specified parent directory.
 *
 * @param[in]       directory       The parent directory.
 *
 * @param[in]       filename        The filename.
 *
 * @return          char*           The built full file path (absolute path)
 * @retval          >0              success
 * @retval          0               failure
 ***********************************************************************************************************************
 */
char *vfs_normalize_path(const char *directory, const char *filename)
{
    char *fullpath;
    char *fulldir = NULL;
    char *dst0, *dst, *src;

    OS_ASSERT(filename != NULL);

#ifdef VFS_USING_WORKDIR
    /* Shall use working directory */
    if (directory == NULL)
    {
        directory = &working_directory[0];
    }
    else if (directory[0] != '/')
    {
        /* If base directory is not a absolute path, convert it into full path first */
        fulldir = os_malloc(strlen(directory) + strlen(working_directory) + 2);
        if(fulldir == NULL)
        {
            return NULL;
        }
        os_snprintf(fulldir, strlen(directory) + strlen(working_directory) + 2,
                    "%s/%s", working_directory, directory);
    }
#else
    if (((directory == NULL) && (filename[0] != '/')) || (directory && (directory[0] != '/')))
    {
        LOG_E(VFS_TAG, NO_WORKING_DIR);

        return NULL;
    }
#endif

    if (filename[0] != '/')
    {
        if(fulldir) /* Directory is not a absolute path. use fulldir */
        {
            fullpath = os_malloc(strlen(fulldir) + strlen(filename) + 2);
            if (fullpath == NULL)
            {
                os_free(fulldir);
                return NULL;
            }
            
            /* Join path and file name */
            os_snprintf(fullpath, strlen(fulldir) + strlen(filename) + 2,
                        "%s/%s", fulldir, filename);
            os_free(fulldir);
        }
        else
        {
            fullpath = os_malloc(strlen(directory) + strlen(filename) + 2);
            if (fullpath == NULL)
            {
                return NULL;
            }

            /* Join path and file name */
            os_snprintf(fullpath, strlen(directory) + strlen(filename) + 2,
                        "%s/%s", directory, filename);
        }
    }
    else    /* It's an absolute path, use it directly */
    {
        fullpath = os_strdup(filename); /* Copy string */

        if (fullpath == NULL)
        {
            if(fulldir)
            {
                os_free(fulldir);
            }
            return NULL;
        }
    }

    src = fullpath;
    dst = fullpath;

    dst0 = dst;
    while (1)
    {
        char c = *src;

        if (c == '.')
        {
            if (!src[1])
            {
                    src ++; /* '.' and ends */
            }
            else if (src[1] == '/')
            {
                /* './' case */
                src += 2;

                while ((*src == '/') && (*(src+1) != '\0'))
                {
                    src ++;
                }
                continue;
            }
            else if (src[1] == '.')
            {
                if (!src[2])
                {
                    /* '..' and ends case */
                    src += 2;
                    goto up_one;
                }
                else if (src[2] == '/')
                {
                    /* '../' case */
                    src += 3;

                    while ((*src == '/') && (*(src+1) != '\0'))
                    {
                        src ++;
                    }
                    goto up_one;
                }
            }
        }

        /* Copy up the next '/' and erase all '/' */
        while ((c = *src++) != '\0' && c != '/')
        {
            *dst ++ = c;
        }

        if (c == '/')
        {
            *dst ++ = '/';
            while (c == '/')
            {
                c = *src++;
            }

            src --;
        }
        else if (!c)
        {
            break;
        }

        continue;

up_one:
        dst --;
        if (dst < dst0)
        {
            os_free(fullpath);
            return NULL;
        }
        while (dst0 < dst && dst[-1] != '/')
        {
            dst --;
        }
    }

    *dst = '\0';

    /* Remove '/' in the end of path if exist */
    dst --;
    if ((dst != fullpath) && (*dst == '/'))
    {
        *dst = '\0';
    }

    /* Final check fullpath is not empty, for the special path of lwext "/.." */
    if ('\0' == fullpath[0])
    {
        fullpath[0] = '/';
        fullpath[1] = '\0';
    }

    return fullpath;
}
EXPORT_SYMBOL(vfs_normalize_path);

/**
 ***********************************************************************************************************************
 * @brief           This function gets the file descriptor table of current process.
 *
 * @param           None.
 *
 * @return          vfs_fdtable     The file descriptor table
 ***********************************************************************************************************************
 */
struct vfs_fdtable *vfs_fdtable_get(void)
{
    struct vfs_fdtable *fdt;
#ifdef OS_USING_LWP
    struct os_lwp *lwp;

    lwp = (struct os_lwp *)os_thread_self()->lwp;
    if (lwp)
    {
        fdt = &lwp->fdt;
    }
    else
    {
        fdt = &_fdtab;
    }
#else
    fdt = &_fdtab;
#endif

    return fdt;
}

#ifdef OS_USING_SHELL
#include <shell.h>
static os_err_t sh_list_fd(os_int32_t argc, char **argv)
{
    int index;
    struct vfs_fdtable *fd_table;

    fd_table = vfs_fdtable_get();
    if (!fd_table)
    {
        return -1;
    }

    os_enter_critical();

    os_kprintf("fd type    ref magic  path\r\n");
    os_kprintf("-- ------  --- ----- ------\r\n");
    for (index = 0; index < (int)fd_table->maxfd; index ++)
    {
        struct vfs_fd *fd = fd_table->fds[index];

        if (fd && fd->fops)
        {
            os_kprintf("%2d ", index);
            if (fd->type == FT_DIRECTORY) 
            {
                os_kprintf("%-7.7s ", "dir");
            }
            else if (fd->type == FT_REGULAR) 
            {
                os_kprintf("%-7.7s ", "file");
            }
            else if (fd->type == FT_SOCKET) 
            {
                os_kprintf("%-7.7s ", "socket");
            }
            else if (fd->type == FT_USER) 
            {   
                os_kprintf("%-7.7s ", "user");
            }
            else 
            {
                os_kprintf("%-8.8s ", "unknown");
            }
            os_kprintf("%3d ", fd->ref_count);
            os_kprintf("%04x  ", fd->magic);
            if (fd->path)
            {
                os_kprintf("%s\r\n", fd->path);
            }
            else
            {
                os_kprintf("\r\n");
            }
        }
    }
    os_exit_critical();

    return OS_EOK;
}
SH_CMD_EXPORT(list_fd, sh_list_fd, "list file descriptor");
#endif
