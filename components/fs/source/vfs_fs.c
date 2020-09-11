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
 * @file        vfs_fs.c
 *
 * @brief       This file implement the filesystem related APIs of vfs.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-10   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <vfs_fs.h>
#include <vfs_file.h>
#include "vfs_private.h"

/**
 ***********************************************************************************************************************
 * @def         VFS_FS_TAG
 *
 * @brief       Define the log tag for vfs.
 ***********************************************************************************************************************
 */
#define VFS_FS_TAG        "VFS_FS"

/**
 ***********************************************************************************************************************
 * @brief           This function register a new file system instance to vfs. 
 *
 * @param[in]       ops       Filesystem ops of the new file system instance.
 *
 * @return          Return 0 on successful, -1 on failed.
 ***********************************************************************************************************************
 */
int vfs_register(const struct vfs_filesystem_ops *ops)
{
    int ret = 0;
    const struct vfs_filesystem_ops **empty = NULL;
    const struct vfs_filesystem_ops **iter;

    /* Lock filesystem */
    vfs_lock();
    /* Check if this filesystem was already registered */
    for (iter = &filesystem_operation_table[0]; 
         iter < &filesystem_operation_table[VFS_FILESYSTEM_TYPES_MAX];
         iter ++)
    {
        /* Find out an empty filesystem type entry */
        if (*iter == NULL)
        {
            (empty == NULL) ? (empty = iter) : 0;
        }
        else if (strcmp((*iter)->name, ops->name) == 0)
        {
            os_set_errno(-EEXIST);
            ret = -1;
            LOG_E(VFS_FS_TAG, "Filesystem %s was already registered.", ops->name);
            break;
        }
    }

    /* Save the filesystem's operations */
    if (empty == NULL)
    {
        os_set_errno(-ENOSPC);
        LOG_E(VFS_FS_TAG, "There is no space to register this file system (%s).", ops->name);
        ret = -1;
    }
    else if (ret == 0)
    {
        *empty = ops;
        LOG_I(VFS_FS_TAG, "Register filesystem %s", ops->name);
    }

    vfs_unlock();

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function return the file system mounted on the specified path. 
 *
 * @param[in]       path       The specified path.
 *
 * @return          Return the found file system, or NULL if no match was found.
 ***********************************************************************************************************************
 */
struct vfs_filesystem *vfs_filesystem_lookup(const char *path)
{
    struct vfs_filesystem *iter;
    struct vfs_filesystem *fs = NULL;
    uint32_t     fspath;
    uint32_t     prefixlen;

    prefixlen = 0;

    OS_ASSERT(path);

    /* Lock filesystem */
    vfs_lock();

    /* Lookup it in the filesystem table */
    for (iter = &filesystem_table[0];
         iter < &filesystem_table[VFS_FILESYSTEMS_MAX];
         iter++)
    {
        if ((iter->path == NULL) || (iter->ops == NULL))
        {
            continue;
        }

        fspath = strlen(iter->path);
        if ((fspath < prefixlen) || (strncmp(iter->path, path, fspath) != 0))
        {
            continue;
        }

        /* Check next path separator */
        if (fspath > 1 && (strlen(path) > fspath) && (path[fspath] != '/'))
        {
            continue;
        }

        fs = iter;
        prefixlen = fspath;
    }

    vfs_unlock();

    return fs;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will return the mounted path for specified device. 
 *
 * @param[in]       device       The device object which is mounted.
 *
 * @return          Return the mounted path or NULL if none device mounted.
 ***********************************************************************************************************************
 */
const char *vfs_filesystem_get_mounted_path(struct os_device *device)
{
    const char *path = NULL;
    struct vfs_filesystem *iter;

    /* If no device is specified, return NULL. */
    if(!device)
    {
        return NULL;
    }

    vfs_lock();
    for (iter = &filesystem_table[0];
         iter < &filesystem_table[VFS_FILESYSTEMS_MAX]; iter++)
    {
        if (iter->ops == NULL) 
        {
            continue;
        }
        else if (iter->dev_id == device)
        {
            path = iter->path;
            break;
        }
    }

    /* Release filesystem_table lock */
    vfs_unlock();

    return path;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will fetch the partition table on specified buffer.
 *
 * @param[out]      part            The returned partition structure.
 * @param[in]       buf             The buffer contains partition table.
 * @param[in]       pindex          The index of partition table to fetch.
 *
 * @return          OS_EOK on successful or -OS_ERROR on failed.
 ***********************************************************************************************************************
 */ 
int vfs_filesystem_get_partition(struct vfs_partition *part,
                                 uint8_t              *buf,
                                 uint32_t              pindex)
{
#define DPT_ADDRESS     0x1be       /* device partition offset in Boot Sector */
#define DPT_ITEM_SIZE   16          /* partition item size */

    uint8_t *dpt;
    uint8_t  type;

    OS_ASSERT(part != NULL);
    OS_ASSERT(buf != NULL);

    dpt = buf + DPT_ADDRESS + pindex * DPT_ITEM_SIZE;

    /* Check if it is a valid partition table */
    if ((*dpt != 0x80) && (*dpt != 0x00))
    {
        return OS_EIO;
    }

    /* Get partition type */
    type = *(dpt + 4);
    if (type == 0)
    {
        return OS_EIO;
    }

    /* Set partition information, size is the number of 512-Byte */
    part->type = type;
    part->offset = *(dpt + 8) | *(dpt + 9) << 8 | *(dpt + 10) << 16 | *(dpt + 11) << 24;
    part->size = *(dpt + 12) | *(dpt + 13) << 8 | *(dpt + 14) << 16 | *(dpt + 15) << 24;

    os_kprintf("found part[%d], begin: %d, size: ", pindex, part->offset * 512);
    if ((part->size >> 11) == 0)
    {
        os_kprintf("%d%s", part->size >> 1, "KB\n"); /* KB */
    }
    else
    {
        unsigned int part_size;
        part_size = part->size >> 11;                /* MB */
        if ((part_size >> 10) == 0)
        {
            os_kprintf("%d.%d%s", part_size, (part->size >> 1) & 0x3FF, "MB\n");
        }
        else
        {
            os_kprintf("%d.%d%s", part_size >> 10, part_size & 0x3FF, "GB\n");
        }
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will mount a file system on a specified path.
 *
 * @param[in]       device_name       The name of device to be mounted, which includes a file system
 * @param[in]       path              The path of mount point.
 * @param[in]       filesystemtype    The file system type to use.
 * @param[in]       rwflag            The supported access mode, read/write etc. flag.
 * @param[in]       data              The private data(parameter) for this file system.
 *
 * @return          Return 0 on successful, -1 on failed.
 ***********************************************************************************************************************
 */
int vfs_mount(const char    *device_name,
              const char    *path,
              const char    *filesystemtype,
              unsigned long  rwflag,
              const void    *data)
{
    const struct vfs_filesystem_ops **ops;
    struct vfs_filesystem *iter;
    struct vfs_filesystem *fs = NULL;
    char *fullpath = NULL;
    os_device_t *dev_id;

    /* Open specific device */
    if (device_name == NULL)
    {
        /* Which is a non-device filesystem mount */
        dev_id = NULL;
    }
    else if ((dev_id = os_device_find(device_name)) == NULL)
    {
        os_set_errno(-ENODEV);
        return -1;
    }

    /* Find out the specific filesystem */
    vfs_lock();

    for (ops = &filesystem_operation_table[0];
         ops < &filesystem_operation_table[VFS_FILESYSTEM_TYPES_MAX]; 
         ops++)
    {
        if ((*ops != NULL) && (strcmp((*ops)->name, filesystemtype) == 0))
        {
            break;
        }
    }

    vfs_unlock();

    if (ops == &filesystem_operation_table[VFS_FILESYSTEM_TYPES_MAX])
    {
        os_set_errno(-ENODEV);
        return -1;
    }

    /* Check if there is mount implementation */
    if ((*ops == NULL) || ((*ops)->mount == NULL))
    {
        os_set_errno(-ENOSYS);
        return -1;
    }

    /* Make full path for special file */
    fullpath = vfs_normalize_path(NULL, path);
    if (fullpath == NULL) /* not an abstract path */
    {
        os_set_errno(-ENOTDIR);
        return -1;
    }

    /* Check if the path exists or not, raw APIs call, fixme */
    if ((strcmp(fullpath, "/") != 0) && (strcmp(fullpath, "/dev") != 0))
    {
        struct vfs_fd fd;

        if (vfs_file_open(&fd, fullpath, O_RDONLY | O_DIRECTORY) < 0)
        {
            os_free(fullpath);
            os_set_errno(-ENOTDIR);

            return -1;
        }
        vfs_file_close(&fd);
    }

    /* Check whether the file system mounted or not  in the filesystem table
    * if it is unmounted yet, find out an empty entry */
    vfs_lock();

    for (iter = &filesystem_table[0];
         iter < &filesystem_table[VFS_FILESYSTEMS_MAX];
         iter++)
    {
        /* Check if it is an empty filesystem table entry? if it is, save fs */
        if (iter->ops == NULL)
        {
            (fs == NULL) ? (fs = iter) : 0;
        }
        /* Check if the PATH is mounted */
        else if (strcmp(iter->path, path) == 0)
        {
            os_set_errno(-EINVAL);
            goto err1;
        }
    }

    if ((fs == NULL) && (iter == &filesystem_table[VFS_FILESYSTEMS_MAX]))
    {
        os_set_errno(-ENOSPC);
        LOG_E(VFS_FS_TAG, "There is no space to mount this file system (%s).", filesystemtype);
        goto err1;
    }

    /* Register file system */
    fs->path   = fullpath;
    fs->ops    = *ops;
    fs->dev_id = dev_id;
    vfs_unlock();

    /* Open device, but do not check the status of device */
    if (dev_id != NULL)
    {
        if (os_device_open(fs->dev_id, OS_DEVICE_OFLAG_RDWR) != OS_EOK)
        {
            /* The underlaying device has error, clear the entry. */
            vfs_lock();
            memset(fs, 0, sizeof(struct vfs_filesystem));

            goto err1;
        }
    }

    /* Call mount of this filesystem */
    if ((*ops)->mount(fs, rwflag, data) < 0)
    {
        if (dev_id != NULL)
        {
            os_device_close(fs->dev_id);
        }

        /* Mount failed */
        vfs_lock();
        /* Clear filesystem table entry */
        memset(fs, 0, sizeof(struct vfs_filesystem));

        goto err1;
    }
    LOG_I(VFS_FS_TAG, "Mount %s to %s", filesystemtype, path);

    return 0;

err1:
    vfs_unlock();
    os_free(fullpath);

    return -1;
}

/**
***********************************************************************************************************************
* @brief           This function will unmount the file system on a specified path.
*
* @param[in]       specialfile     The specified path which mounted a file system.
*
* @return          Return 0 on successful or -1 on failed.
***********************************************************************************************************************
*/ 
int vfs_unmount(const char *specialfile)
{
    char *fullpath;
    struct vfs_filesystem *iter;
    struct vfs_filesystem *fs = NULL;

    fullpath = vfs_normalize_path(NULL, specialfile);
    if (fullpath == NULL)
    {
        os_set_errno(-ENOTDIR);

        return -1;
    }

    /* Lock filesystem */
    vfs_lock();

    for (iter = &filesystem_table[0];
         iter < &filesystem_table[VFS_FILESYSTEMS_MAX];
         iter++)
    {
        /* Check if the PATH is mounted */
        if ((iter->path != NULL) && (strcmp(iter->path, fullpath) == 0))
        {
            fs = iter;
            break;
        }
    }

    if (fs == NULL ||
        fs->ops->unmount == NULL ||
        fs->ops->unmount(fs) < 0)
    {
        goto err1;
    }

    /* Close device, but do not check the status of device */
    if (fs->dev_id != NULL)
        os_device_close(fs->dev_id);

    if (fs->path != NULL)
        os_free(fs->path);

    /* Clear this filesystem table entry */
    memset(fs, 0, sizeof(struct vfs_filesystem));

    vfs_unlock();
    os_free(fullpath);

    return 0;

err1:
    vfs_unlock();
    os_free(fullpath);

    return -1;
}

/**
***********************************************************************************************************************
* @brief           This function will format specified device into specified file system.
*
* @param[in]       fs_name       The file system name.
* @param[in]       device_name   The device name.
*
* @return          Return 0 on successful or -1 on failed.
***********************************************************************************************************************
*/  
int vfs_mkfs(const char *fs_name, const char *device_name)
{
    int index;
    os_device_t *dev_id = NULL;

    /* Check device name, and it should not be NULL */
    if (device_name != NULL)
    {
        dev_id = os_device_find(device_name);
    }

    if (dev_id == NULL)
    {
        os_set_errno(-ENODEV);
        LOG_E(VFS_FS_TAG, "Device (%s) was not found", device_name);
        return -1;
    }

    vfs_lock();
    /* Find the file system operations */
    for (index = 0; index < VFS_FILESYSTEM_TYPES_MAX; index ++)
    {
        if (filesystem_operation_table[index] != NULL &&
            strcmp(filesystem_operation_table[index]->name, fs_name) == 0)
        {
            break;
        }
    }
    vfs_unlock();

    if (index < VFS_FILESYSTEM_TYPES_MAX)
    {
        /* Find file system operation */
        const struct vfs_filesystem_ops *ops = filesystem_operation_table[index];
        if (ops->mkfs == NULL)
        {
            LOG_E(VFS_FS_TAG, "The file system (%s) mkfs function was not implement", fs_name);
            os_set_errno(-ENOSYS);
            return -1;
        }

        return ops->mkfs(dev_id);
    }

    LOG_E(VFS_FS_TAG, "File system (%s) was not found.", fs_name);

    return -1;
}

/**
***********************************************************************************************************************
* @brief           This function will return the information about the mounted file system on specified path.
*
* @param[in]       path     The path on which the file system is mounted.
* @param[out]      buffer   The buffer to save the returned information.
*
* @return          Return 0 on successful or -1 on failed.
***********************************************************************************************************************
*/  
int vfs_statfs(const char *path, struct statfs *buffer)
{
    struct vfs_filesystem *fs;
    char *fullpath;

    fullpath = vfs_normalize_path(NULL, path);
    if (fullpath == NULL)
    {
        return -1;
    }

    fs = vfs_filesystem_lookup(fullpath);
    if (fs != NULL)
    {
        struct vfs_fd fd;
        int result;

        /* Check if path is valid */
        result = vfs_file_open( &fd, fullpath,  O_RDONLY);
        if(result < 0)
        {
            result = vfs_file_open( &fd, fullpath,  O_RDONLY | O_DIRECTORY);
        }
        os_free(fullpath);

        if(result < 0)
        {
            return -1;
        }

        if (result == 0)
        {
            vfs_file_close(&fd);
        }
        
        if (fs->ops->statfs != NULL)
        {
            return fs->ops->statfs(fs, buffer);
        }
    }
    else
    {
        os_free(fullpath);
    }

    return -1;
}


#ifdef OS_USING_SHELL
#include <shell.h>
void mkfs(const char *fs_name, const char *device_name)
{
    vfs_mkfs(fs_name, device_name);
}

/**
***********************************************************************************************************************
* @brief           This function print out free space infomation of the file system mounted on specified path.
*
* @param[in]       path     The path on which the file system is mounted.
*
* @return          Return 0 on successful or -1 on failed.
***********************************************************************************************************************
*/
int df(const char *path)
{
    int   result;
    int   minor = 0;
    long long     cap;
    struct statfs buffer;
    

    int unit_index = 0;
    char *unit_str[] = {"KB", "MB", "GB"};

    if(!path)
    {
        return -1;
    }

    result = vfs_statfs(path, &buffer);
    if (result != 0)
    {
        os_kprintf("vfs_statfs failed.\r\n");
        return -1;
    }

    cap = ((long long)buffer.f_bsize) * ((long long)buffer.f_bfree) / 1024LL;
    for (unit_index = 0; unit_index < 2; unit_index ++)
    {
        if (cap < 1024) 
        {
            break;
        }
        
        minor = (cap % 1024) * 10 / 1024; /* Only one decimal point */
        cap = cap / 1024;
    }

    os_kprintf("disk free: %d.%d %s [ %d block, %d bytes per block ]\r\n",
               (unsigned long)cap, 
               minor, 
               unit_str[unit_index], 
               buffer.f_bfree, 
               buffer.f_bsize);
    return 0;
}
#endif /* OS_USING_SHELL */

