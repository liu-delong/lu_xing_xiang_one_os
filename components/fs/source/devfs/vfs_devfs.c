/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2020,RT-Thread Development Team
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
 * @file        vfs_devfs.c
 *
 * @brief       This file implement the DEV filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2018-02-11   Bernard         Ignore O_CREAT flag in open.
 ***********************************************************************************************************************
 */

#include <os_device.h>
#include <os_assert.h>
#include <os_object.h>
#include <vfs.h>
#include <vfs_fs.h>
#include <vfs_file.h>
#include <devfs/vfs_devfs.h>

/**
 ***********************************************************************************************************************
 * @struct      device_dirent
 *
 * @brief       The device dir entry.
 ***********************************************************************************************************************
 */
struct device_dirent
{
    os_device_t **devices;          /* The address to save the device object adress. */ 
    os_uint16_t read_index;         /* The current read index of dir entry. */
    os_uint16_t device_count;       /* The total device number. */
};

/**
 ***********************************************************************************************************************
 * @brief           Mount device filesystem.
 *
 * @param[in]       fs          The VFS object.
 * @param[in]       rwflag      The read/write flag, not used now.
 * @param[out]      data        The private data, not used now.
 *
 * @return          Mount result.
 * @retval          0           Mount successfully.
 ***********************************************************************************************************************
 */
static int vfs_device_fs_mount(struct vfs_filesystem *fs, unsigned long rwflag, const void *data)
{
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Run ioctl cmd for device.
 *
 * @param[in]       file        The file descriptor.
 * @param[in]       cmd         The ioctl cmd.
 * @param[in,out]   args        The arguments, depends on cmd.
 *
 * @return          The cmd result.
 * @retval          0           Run cmd successfully.
 * @retval          -EIO        Run cmd failed.
 ***********************************************************************************************************************
 */
static int vfs_device_fs_ioctl(struct vfs_fd *file, int cmd, void *args)
{
    os_err_t result;
    os_device_t *dev_id;

    OS_ASSERT(file != OS_NULL);

    dev_id = (os_device_t *)file->data;
    OS_ASSERT(dev_id != OS_NULL);

    result = os_device_control(dev_id, cmd, args);
    if (result == OS_EOK)
    {
        return 0;
    }

    return -EIO;
}

/**
 ***********************************************************************************************************************
 * @brief           Read device file.
 *
 * @param[in,out]   file        The file descriptor.
 * @param[out]      buf         The pointer of buf to save read content.
 * @param[in]       count       The expected read size.
 *
 * @return          The actual read size.
 * @retval          int         The actual read size.
 ***********************************************************************************************************************
 */
static int vfs_device_fs_read(struct vfs_fd *file, void *buf, size_t count)
{
    int result;
    os_device_t *dev_id;

    OS_ASSERT(file != OS_NULL);

    dev_id = (os_device_t *)file->data;
    OS_ASSERT(dev_id != OS_NULL);

    result = os_device_read(dev_id, file->pos, buf, count);
    file->pos += result;

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Write device file.
 *
 * @param[in,out]   file        The file descriptor.
 * @param[in]       buf         The pointer of buf to write.
 * @param[in]       count       The expected write size.
 *
 * @return          The actual write size.
 * @retval          int         The actual write size.
 ***********************************************************************************************************************
 */
static int vfs_device_fs_write(struct vfs_fd *file, const void *buf, size_t count)
{
    int result;
    os_device_t *dev_id;

    OS_ASSERT(file != OS_NULL);

    dev_id = (os_device_t *)file->data;
    OS_ASSERT(dev_id != OS_NULL);

    result = os_device_write(dev_id, file->pos, buf, count);
    file->pos += result;

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Close device file.
 *
 * @param[in,out]   file        The file descriptor.
 *
 * @return          Close result.
 * @retval          0           Close successfully.
 * @retval          -EIO        Close failed.
 ***********************************************************************************************************************
 */
static int vfs_device_fs_close(struct vfs_fd *file)
{
    os_err_t result;
    os_device_t *dev_id;

    OS_ASSERT(file != OS_NULL);

    /* If directory, free the root dir entry. */
    if (file->type == FT_DIRECTORY)
    {
        struct device_dirent *root_dirent;

        root_dirent = (struct device_dirent *)file->data;
        OS_ASSERT(root_dirent != OS_NULL);
        os_free(root_dirent);

        return 0;
    }

    /* If device file, close device. */
    dev_id = (os_device_t *)file->data;
    OS_ASSERT(dev_id != OS_NULL);

    result = os_device_close(dev_id);
    if (result == OS_EOK)
    {
        file->data = OS_NULL;

        return 0;
    }

    return -EIO;
}

/**
 ***********************************************************************************************************************
 * @brief           Open device file.
 *
 * @param[in,out]   file        The file descriptor.
 *
 * @return          Open result.
 * @retval          0           Open successfully.
 * @retval          others      Open failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_device_fs_open(struct vfs_fd *file)
{
    os_err_t result;
    os_device_t *device;

    /* If root directory, traverse devcie object and save device info. */
    if ((file->path[0] == '/') && (file->path[1] == '\0') &&
        (file->flags & O_DIRECTORY))
    {
        os_object_t *object;
        os_list_node_t *node;
        os_object_info_t *information;
        struct device_dirent *root_dirent;
        os_uint32_t count = 0;

        os_enter_critical();

        /* Traverse device object to get device number. */
        information = os_object_get_info(OS_OBJECT_DEVICE);
        OS_ASSERT(information != OS_NULL);
        for (node = information->object_list.next; node != &(information->object_list); node = node->next)
        {
            count ++;
        }

        /* Allocate memory to save all the device object address. */
        root_dirent = (struct device_dirent *)os_malloc(sizeof(struct device_dirent) +
                      count * sizeof(os_device_t *));
        if (root_dirent != OS_NULL)
        {
            root_dirent->devices = (os_device_t **)(root_dirent + 1);
            root_dirent->read_index = 0;
            root_dirent->device_count = count;
            count = 0;

            for (node = information->object_list.next; node != &(information->object_list); node = node->next)
            {
                object = os_list_entry(node, struct os_object, list);
                root_dirent->devices[count] = (os_device_t *)object;
                count ++;
            }
        }
        os_exit_critical();

        /* Save dir entry info to file->data. */
        file->data = root_dirent;

        return 0;
    }

    /* If device file, find the device and open it, and save the device info to file->data. */
    device = os_device_find(&file->path[1]);
    if (device == OS_NULL)
    {
        return -ENODEV;
    }

#ifdef OS_USING_POSIX
    if (device->fops)
    {
        file->fops = device->fops;
        file->data = (void *)device;

        if (file->fops->open)
        {
            result = file->fops->open(file);
            if (result == OS_EOK || result == OS_ENOSYS)
            {
                return 0;
            }
        }
    }
    else
#endif
    {
        result = os_device_open(device, OS_DEVICE_OFLAG_RDWR);
        if (result == OS_EOK || result == OS_ENOSYS)
        {
            file->data = device;
            return 0;
        }
    }

    /* Open device failed. */
    file->data = OS_NULL;

    return -EIO;
}


/**
 ***********************************************************************************************************************
 * @brief           Get the device file status.
 *
 * @param[in]       fs          The VFS object.
 * @param[in]       path        The device file path.
 * @param[out]      st          The pointer of buffer to save device file status.
 *
 * @return          The operation result.
 * @retval          0           Get status successfully.
 * @retval          -ENOENT     Not find the entry.
 ***********************************************************************************************************************
 */
static int vfs_device_fs_stat(struct vfs_filesystem *fs, const char *path, struct stat *st)
{
    if ((path[0] == '/') && (path[1] == '\0'))  /* If root directory. */
    {
        st->st_dev = 0;

        st->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                      S_IWUSR | S_IWGRP | S_IWOTH;
        st->st_mode &= ~S_IFREG;
        st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;

        st->st_size  = 0;
        st->st_mtime = 0;

        return 0;
    }
    else                                        /* If device file. */
    {
        os_device_t *dev_id;

        dev_id = os_device_find(&path[1]);
        if (dev_id != OS_NULL)
        {
            st->st_dev = 0;

            st->st_mode = S_IRUSR | S_IRGRP | S_IROTH |
                          S_IWUSR | S_IWGRP | S_IWOTH;

            if (dev_id->type == OS_DEVICE_TYPE_CHAR)
                st->st_mode |= S_IFCHR;
            else if (dev_id->type == OS_DEVICE_TYPE_BLOCK)
                st->st_mode |= S_IFBLK;
            else if (dev_id->type == OS_DEVICE_TYPE_PIPE)
                st->st_mode |= S_IFIFO;
            else
                st->st_mode |= S_IFREG;

            st->st_size  = 0;
            st->st_mtime = 0;

            return 0;
        }
    }

    return -ENOENT;
}

/**
 ***********************************************************************************************************************
 * @brief           Get the device dir entry.
 *
 * @param[in]       file        The file descriptor.
 * @param[out]      dirp        The pointer of buffer to save dir entry.
 * @param[in]       count       The buffer size to save dir entry.
 *
 * @return          The operation result.
 * @retval          int         The actual size of read.
 * @retval          -EINVAL     Invaild parameter.
 ***********************************************************************************************************************
 */
static int vfs_device_fs_getdents(struct vfs_fd *file, struct dirent *dirp, uint32_t count)
{
    os_uint32_t index;
    os_object_t *object;
    struct dirent *d;
    struct device_dirent *root_dirent;

    root_dirent = (struct device_dirent *)file->data;
    OS_ASSERT(root_dirent != OS_NULL);

    /* Convert bytes to dirent count*/
    count = (count / sizeof(struct dirent));
    if (count == 0)
    {
        return -EINVAL;
    }

    for (index = 0; index < count && index + root_dirent->read_index < root_dirent->device_count; index ++)
    {
        object = (os_object_t *)root_dirent->devices[root_dirent->read_index + index];

        d = dirp + index;
        d->d_type = DT_REG;
        d->d_namlen = OS_NAME_MAX;
        d->d_reclen = (os_uint16_t)sizeof(struct dirent);
        strncpy(d->d_name, object->name, OS_NAME_MAX);
    }

    root_dirent->read_index += index;

    return index * sizeof(struct dirent);
}


/**
 ***********************************************************************************************************************
 * @brief           The poll operation of devcie file, not implemented now.
 *
 * @param[in]       file        The file descriptor.
 * @param[in]       req         The poll request.
 *
 * @return          The operation result.
 * @retval          int         The operation result.
 ***********************************************************************************************************************
 */
static int vfs_device_fs_poll(struct vfs_fd *file, struct os_pollreq *req)
{
    int mask = 0;

    return mask;
}

static const struct vfs_file_ops _device_fops =
{
    vfs_device_fs_open,
    vfs_device_fs_close,
    vfs_device_fs_ioctl,
    vfs_device_fs_read,
    vfs_device_fs_write,
    OS_NULL,                    /* Not support flush. */
    OS_NULL,                    /* Not support lseek. */
    vfs_device_fs_getdents,
    vfs_device_fs_poll,
};

static const struct vfs_filesystem_ops _device_fs =
{
    "devfs",
    VFS_FS_FLAG_DEFAULT,
    &_device_fops,

    vfs_device_fs_mount,
    OS_NULL,                    /* Not support unmount. */
    OS_NULL,                    /* Not support mkfs. */
    OS_NULL,                    /* Not support statfs. */

    OS_NULL,                    /* Not support unlink. */
    vfs_device_fs_stat,
    OS_NULL,                    /* Not support rename. */
};

/**
 ***********************************************************************************************************************
 * @brief           Register DEVFS operation structure to VFS.
 *
 * @param[in]       None
 *
 * @return          The register result.
 * @retval          0           Register successfully.
 * @retval          -1          Register failed.
 ***********************************************************************************************************************
 */
int vfs_devfs_init(void)
{
    /* Register device file system. */
    return vfs_register(&_device_fs);
}
