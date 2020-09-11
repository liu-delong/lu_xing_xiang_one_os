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
 * @file        vfs_fatfs.c
 *
 * @brief       This file is adapter for Vitual filesystem and FAT filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-20   OneOS Team      Modify some code to compliant with the posix syntax and optimize some feature.
 ***********************************************************************************************************************
 */

#include "ffconf.h"
#include "ff.h"
#include "diskio.h"
#include <string.h>
#include <sys/time.h>
#include <os_util.h>
#include <os_assert.h>
#include <os_dbg.h>
#include <vfs_fs.h>
#include <vfs_file.h>

#define FAT_RENAME_NO_ACTION    1

#define ENTER_V_FATFS(v_ffs)        os_mutex_lock(&v_ffs->lock, OS_IPC_WAITING_FOREVER)
#define LEAVE_V_FATFS(v_ffs)        os_mutex_unlock(&v_ffs->lock)

struct v_fatentry
{
    os_list_node_t          list_node;
    char                   *name;
    os_uint16_t             ref_cnt;
};

struct v_fatfs{
    FATFS                   fat;
    os_list_node_t          open_entry_head;
    os_mutex_t              lock;
};

static struct v_fatfs *v_fat_init(void)
{
    char name[OS_NAME_MAX];
    struct v_fatfs *v_fat;

    v_fat = os_malloc(sizeof(struct v_fatfs));
    if (!v_fat)
    {
        return OS_NULL;
    }

    os_list_init(&v_fat->open_entry_head);
    os_snprintf(name, sizeof(name), "V_FAT%p", v_fat);
    os_mutex_init(&v_fat->lock, name, OS_IPC_FLAG_FIFO, OS_FALSE);

    return v_fat;
}

static int v_fat_deinit(struct v_fatfs *v_fat)
{
    os_list_node_t *head;
    struct v_fatentry *v_entry;

    if (os_list_empty(&v_fat->open_entry_head))
    {
        os_mutex_deinit(&v_fat->lock);
        os_free(v_fat);
        return 0;
    }
    else
    {
        head = &v_fat->open_entry_head;
        ENTER_V_FATFS(v_fat);
        os_list_for_each_entry(v_entry, head, struct v_fatentry, list_node)
        {
            LOG_W(FAT_FS_TAG, "This file/dir should be closed:%s", v_entry->name);
        }
        LEAVE_V_FATFS(v_fat);
        return -1;
    }
}

static int v_fat_add_entry(struct v_fatfs *v_fat, char *path)
{
    os_list_node_t *head;
    struct v_fatentry *v_entry;
    struct v_fatentry *v_entry_temp;
    os_bool_t   foundflag = OS_FALSE;
    os_uint32_t len;
    int result = 0; 

    head = &v_fat->open_entry_head;
    len = strlen(path);

    ENTER_V_FATFS(v_fat);
    os_list_for_each_entry_safe(v_entry, v_entry_temp, head, struct v_fatentry, list_node)
    {
        if (strcmp(v_entry->name, path) == 0)
        {
            v_entry->ref_cnt++;
            foundflag = OS_TRUE;
            break;
        }
    }
    if (foundflag == OS_FALSE)
    {
        v_entry = os_malloc(sizeof(struct v_fatentry));
        if (!v_entry)
        {
            result = -1;
            goto end;
        }
        v_entry->name = os_malloc(len + 1);
        if (!v_entry->name)
        {
            result = -1;
            goto end;
        }
        memset(v_entry->name, 0 , len + 1);
        strncpy(v_entry->name, path, strlen(path));
        os_list_add(&v_fat->open_entry_head, &v_entry->list_node);
        v_entry->ref_cnt = 1;
    }

end:
    LEAVE_V_FATFS(v_fat);

    return result;
}

static int v_fat_del_entry(struct v_fatfs *v_fat, char *path)
{
    os_list_node_t *head;
    struct v_fatentry *v_entry;
    struct v_fatentry *v_entry_temp;
    int result = -1; 

    head = &v_fat->open_entry_head;
    ENTER_V_FATFS(v_fat);
    os_list_for_each_entry_safe(v_entry, v_entry_temp, head, struct v_fatentry, list_node)
    {
        if (strcmp(v_entry->name, path) == 0)
        {
            if (--v_entry->ref_cnt == 0)
            {
                os_list_del(&v_entry->list_node);
                os_free(v_entry->name);
                os_free(v_entry);
                result = 0;
            }
            break;
        }
    }
    LEAVE_V_FATFS(v_fat);

    return result;
}

static os_bool_t v_fat_find_entry(struct v_fatfs *v_fat, const char *path)
{
    os_list_node_t *head;
    struct v_fatentry *v_entry;
    os_bool_t result = OS_FALSE; 

    head = &v_fat->open_entry_head;
    ENTER_V_FATFS(v_fat);
    os_list_for_each_entry(v_entry, head, struct v_fatentry, list_node)
    {
        if (strcmp(v_entry->name, path) == 0)
        {
            result = OS_TRUE;
            break;
        }
    }
    LEAVE_V_FATFS(v_fat);

    return result;
}

#if _VOLUMES > 1
extern int fat_get_vol(FATFS * fat);
#endif

 /**
 ***********************************************************************************************************************
 * @brief           Convert FATFS error code to VFS error code.
 *
 * @param[in]       result          The FATFS error code.
 *
 * @return          The corresponding VFS error code.
 * @retval          int             The VFS error code.
 ***********************************************************************************************************************
 */
static int fat_result_to_vfs(FRESULT result)
{
    int status = 0;

    switch (result)
    {
    case FR_OK:
        break;

    case FR_NO_FILE:
    case FR_NO_PATH:
    case FR_NO_FILESYSTEM:
        status = -ENOENT;
        break;

    case FR_INVALID_NAME:
        status = -EINVAL;
        break;

    case FR_EXIST:
    case FR_INVALID_OBJECT:
        status = -EEXIST;
        break;

    case FR_DISK_ERR:
    case FR_NOT_READY:
    case FR_INT_ERR:
        status = -EIO;
        break;

    case FR_WRITE_PROTECTED:
    case FR_DENIED:
        status = -EROFS;
        break;

    case FR_MKFS_ABORTED:
        status = -EINVAL;
        break;

    default:
        status = -1;
        break;
    }

    return status;
}

/**
 ***********************************************************************************************************************
 * @brief           According to the device object to find the disk device index.
 *
 * @param[in]       id              The device object. If id is NULL, means try to find an empty position. 
 *
 * @return          The disk index.
 * @retval          int             The disk index.
 * @retval          -1              Not found this disk device.
 ***********************************************************************************************************************
 */
static int get_disk(os_device_t *id)
{
    int index;

    for (index = 0; index < _VOLUMES; index ++)
    {
        if (disk_get_devid(index) == id)
        {
            return index;
        }
    }

    return -1;
}

/**
 ***********************************************************************************************************************
 * @brief           Mount FAT filesystem
 *
 * @param[in,out]   fs              The pointer of VFS object.
 * @param[in]       rwflag          The read/write flag, not used now.
 * @param[out]      data            The private data, not used now.
 *
 * @return          Mount result.
 * @retval          0               Mount successfully.
 * @retval          negative int    Mount failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_mount(struct vfs_filesystem *fs, unsigned long rwflag, const void *data)
{
    struct v_fatfs *v_fat;
    FRESULT result;
    int index;
    struct os_device_blk_geometry geometry;
    char logic_nbr[2] = {'0',':'};

    /* Get an empty position. */
    index = get_disk(OS_NULL);
    if (index == -1)
    {
        return -ENOENT;
    }
    logic_nbr[0] = '0' + index;

    /* Save device object. */
    disk_set_devid(index, fs->dev_id);
    /* Check device sector size. */
    if (os_device_control(fs->dev_id, OS_DEVICE_CTRL_BLK_GETGEOME, &geometry) == OS_EOK)
    {
        if (geometry.bytes_per_sector > _MAX_SS)
        {
            LOG_E(FAT_FS_TAG, "The sector size of device is greater than the sector size of FAT.");
            return -EINVAL;
        }
    }

    v_fat = v_fat_init();
    if (!v_fat)
    {
        disk_set_devid(index, OS_NULL);
        return -ENOMEM;
    }

    /* Mount FATFS. */
    result = f_mount(&v_fat->fat, (const TCHAR*)logic_nbr, 1);
    if (result == FR_OK)
    {
        char drive[8];
        DIR *dir;

        os_snprintf(drive, sizeof(drive), "%d:/", index);
        dir = (DIR *)os_malloc(sizeof(DIR));

        /* If malloc dir failed, unmount FATFS. */
        if (!dir)
        {
            f_mount(OS_NULL, (const TCHAR*)logic_nbr, 1);
            disk_set_devid(index, OS_NULL);
            v_fat_deinit(v_fat);
            return -ENOMEM;
        }

        /* Open the root directory to test whether the FATFS is valid. */
        result = f_opendir(dir, drive);
        if (result != FR_OK)
        {
            goto __err;
        }

        /* Mount succeed, save FATFS object to fs->data. */
        fs->data = v_fat;
        os_free(dir);
        return 0;
    }

__err:
    f_mount(OS_NULL, (const TCHAR*)logic_nbr, 1);   /* Unmount FATFS. */
    disk_set_devid(index, OS_NULL);
    v_fat_deinit(v_fat);
    return fat_result_to_vfs(result);
}

/**
 ***********************************************************************************************************************
 * @brief           Unmount FAT filesystem
 *
 * @param[in,out]   fs              The pointer of VFS object.
 *
 * @return          Unmount result.
 * @retval          0               Unmount successfully.
 * @retval          negative int    Unmount failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_unmount(struct vfs_filesystem *fs)
{
    struct v_fatfs *v_fat;
    FRESULT result;
    int  index;
    char logic_nbr[2] = {'0',':'};

    v_fat = (struct v_fatfs *)fs->data;
    if (!v_fat)
    {
        return -EINVAL;
    }
    if (v_fat_deinit(v_fat) != 0)
    {
        return -EINVAL;
    }
    fs->data = OS_NULL;

    /* Find the device index and then umount it. */
    index = get_disk(fs->dev_id);
    if (index == -1)                                /* Not found the device. */
    {
        return -ENOENT;
    }

    logic_nbr[0] = '0' + index;
    result = f_mount(OS_NULL, logic_nbr, (BYTE)1); /* Unmount. */
    if (result != FR_OK)
    {
        return fat_result_to_vfs(result);
    }
    disk_set_devid(index, OS_NULL);

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Make FAT filesystem in disk device (format disk).
 *
 * @param[in,out]   dev_id          The pointer of device object.
 *
 * @return          Make filesystem result.
 * @retval          0               Make filesystem successfully.
 * @retval          negative int    Make filesystem failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_mkfs(os_device_t *dev_id)
{
#define FSM_STATUS_INIT            0
#define FSM_STATUS_USE_TEMP_DRIVER 1
    FATFS *fat = OS_NULL;
    BYTE *work;
    int flag;
    FRESULT result;
    int index;
    char logic_nbr[2] = {'0',':'};

    if (!dev_id)
    {
        return -EINVAL;
    }

    flag = FSM_STATUS_INIT;
    index = get_disk(dev_id);
    if (index == -1)            /* if the device not mounted, find an empty drive to do mkfs. */
    {
        index = get_disk(OS_NULL);
        if (index == -1)        /* No space to store an temp driver. */
        {
            LOG_E(FAT_FS_TAG, "sorry, there is no space to do mkfs!");
            return -ENOSPC;
        }
        else
        {
            fat = os_malloc(sizeof(FATFS));
            if (!fat)
            {
                return -ENOMEM;
            }

            flag = FSM_STATUS_USE_TEMP_DRIVER;

            /* Associate disk index and device object. */
            disk_set_devid(index, dev_id);
            os_device_open(dev_id, OS_DEVICE_OFLAG_RDWR);

            /* Just fill the FatFs[vol] in ff.c, or mkfs will failded!
             * consider this condition: you just umount the elm fat,
             * then the space in FatFs[index] is released, and now do mkfs
             * on the disk, you will get a failure. So we need f_mount here,
             * just fill the FatFS[index] in elm fatfs to make mkfs work.
             */
            logic_nbr[0] = '0' + index;
            f_mount(fat, logic_nbr, (BYTE)index);
        }
    }
    else                        /*If the device is already mounted, just do mkfs. */
    {
        LOG_E(FAT_FS_TAG, "The device has been mounted, you should unmount it before mkfs");
        return -EINVAL;
    }

    work = os_malloc(_MAX_SS);
    if(!work)
    {
        return -ENOMEM;
    }
    /* Mkfs on the device, FM_ANY means it maybe FAT(12/16)/FAT32/EXFAT etc. */
    result = f_mkfs(logic_nbr, FM_ANY, 0, work, _MAX_SS);
    os_free(work);

    if (flag == FSM_STATUS_USE_TEMP_DRIVER) /* If temp driver, clear it. */
    {
        os_free(fat);
        f_mount(OS_NULL, logic_nbr,(BYTE)index);
        disk_set_devid(index, OS_NULL);
        os_device_close(dev_id);
    }

    if (result != FR_OK)
    {
        LOG_E(FAT_FS_TAG, "Format error");
        return fat_result_to_vfs(result);
    }

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Get FAT filesystem status.
 *
 * @param[in]       fs              The pointer of VFS object.
 * @param[out]      buf             The pointer of buffer to save status.
 *
 * @return          The operation result.
 * @retval          0               Get status successfully.
 * @retval          negative int    Get stauts failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_statfs(struct vfs_filesystem *fs, struct statfs *buf)
{
    struct v_fatfs *v_fat;
    FATFS *f;
    FRESULT res;
    char driver[4];
    DWORD fre_clust;
    DWORD fre_sect;
    DWORD tot_sect;

    if ((!fs) || (!buf))
    {
        return -EINVAL;
    }

    v_fat = (struct v_fatfs *)fs->data;
    if (!v_fat)
    {
        return -EINVAL;
    }
    f = &v_fat->fat;
    os_snprintf(driver, sizeof(driver), "%d:", f->drv);
    res = f_getfree(driver, &fre_clust, &f);
    if (res)
    {
        return fat_result_to_vfs(res);
    }

    tot_sect = (f->n_fatent - 2) * f->csize;
    fre_sect = fre_clust * f->csize;

    buf->f_bfree = fre_sect;
    buf->f_blocks = tot_sect;
#if _MAX_SS != 512
    buf->f_bsize = f->ssize;
#else
    buf->f_bsize = 512;
#endif

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Open file in FAT filesystem.
 *
 * @param[in,out]   file            The file descriptor.
 *
 * @return          Open result.
 * @retval          0               Open file successfully.
 * @retval          negative int    Open file failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_open(struct vfs_fd *file)
{
    FIL *fd;
    BYTE mode;
    FRESULT result;
    FILINFO *file_info;
    char *drivers_fn;
    struct vfs_filesystem *fs;
    struct v_fatfs *v_fat;
#if (_VOLUMES > 1)
    int vol;
    int size;
#endif

    if ((!file) || (!file->fs))
    {
        return -EINVAL;
    }
    fs = (struct vfs_filesystem *)file->fs;
    v_fat = (struct v_fatfs *)fs->data;
    if (!v_fat)
    {
        return -EINVAL;
    }
#if (_VOLUMES > 1)
    /* If multi vol, the path should contains vol number. */
    vol = fat_get_vol(&v_fat->fat);
    if (vol < 0)
    {
        return -ENOENT;
    }
    size = strlen(file->path) + 2 + 1;
    drivers_fn = os_malloc(size);
    memset(drivers_fn, 0, size);
    if (!drivers_fn)
    {
        return -ENOMEM;
    }
    os_snprintf(drivers_fn, size, "%d:%s", vol, file->path);
#else
    drivers_fn = file->path;
#endif

    if (file->flags & O_DIRECTORY)      /* If it's a directory. */
    {
        DIR *dir;

        if (file->flags & O_CREAT)
        {
            result = f_mkdir(drivers_fn);
            if (result != FR_OK)
            {
#if _VOLUMES > 1
                os_free(drivers_fn);
#endif
                return fat_result_to_vfs(result);
            }
        }

        dir = (DIR *)os_malloc(sizeof(DIR));
        if (!dir)
        {
#if _VOLUMES > 1
            os_free(drivers_fn);
#endif
            return -ENOMEM;
        }

        result = f_opendir(dir, drivers_fn);
#if _VOLUMES > 1
        os_free(drivers_fn);
#endif
        if (result != FR_OK)
        {
            os_free(dir);
            return fat_result_to_vfs(result);
        }
        v_fat_add_entry(v_fat, file->path);
        file->data = dir;
        return 0;
    }
    else                                /* If it's a file. */
    {
        mode = FA_READ;

        if (file->flags & O_WRONLY)
        {
            mode &= (~FA_READ);
            mode |= FA_WRITE;
        }
        if (file->flags & O_RDWR)
        {
            mode |= FA_WRITE;
        }
        if (file->flags & O_CREAT)
        {
            mode |= FA_OPEN_ALWAYS;
            /* If O_EXCL conjunction with O_CREAT, only new file be created; if file already exists, fails with EEXIST*/
            if (file->flags & O_EXCL)
            {
                mode |= FA_CREATE_NEW;
            }
        }
        if (file->flags & O_TRUNC)
        {
            file_info = os_malloc(sizeof(FILINFO));
            if (!file_info)
            {
#if _VOLUMES > 1
                os_free(drivers_fn);
#endif
                return -ENOMEM;
            }
            result = f_stat(drivers_fn, file_info);
            os_free(file_info);
            /* If file already exist, recreate it; If not exist, not create it*/
            if (result == FR_OK)
            {
                mode |= FA_CREATE_ALWAYS;
            }
        }

        /* Allocate a fd */
        fd = (FIL *)os_malloc(sizeof(FIL));
        if (!fd)
        {
#if _VOLUMES > 1
            os_free(drivers_fn);
#endif
            return -ENOMEM;
        }

        result = f_open(fd, drivers_fn, mode);
#if _VOLUMES > 1
        os_free(drivers_fn);
#endif
        if (result == FR_OK)
        {
            file->pos  = fd->fptr;
            file->size = f_size(fd);
            v_fat_add_entry(v_fat, file->path);
            file->data = fd;

            if (file->flags & O_APPEND) /* if need Append, seek to the end of file. */
            {
                f_lseek(fd, f_size(fd));
                file->pos = fd->fptr;
            }
        }
        else
        {
            os_free(fd);
            return fat_result_to_vfs(result);
        }
    }

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Close file in FAT filesystem.
 *
 * @param[in,out]   file            The file descriptor.
 *
 * @return          Close result.
 * @retval          0               Close file successfully.
 * @retval          negative int    Close file failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_close(struct vfs_fd *file)
{
    FRESULT result;
    struct vfs_filesystem *fs;
    struct v_fatfs *v_fat;

    if ((!file) || (!file->fs))
    {
        return -EINVAL;
    }
    fs = (struct vfs_filesystem *)file->fs;
    v_fat = (struct v_fatfs *)fs->data;
    if (!v_fat)
    {
        return -EINVAL;
    }

    result = FR_OK;
    if (file->type == FT_DIRECTORY)
    {
        DIR *dir;

        dir = (DIR *)(file->data);
        if (!dir)
        {
            return -EINVAL;
        }
        v_fat_del_entry(v_fat, file->path);
        os_free(dir);
    }
    else if (file->type == FT_REGULAR)
    {
        FIL *fd;
        fd = (FIL *)(file->data);
        if (!fd)
        {
            return -EINVAL;
        }
        result = f_close(fd);
        if (result == FR_OK)
        {
            v_fat_del_entry(v_fat, file->path);
            os_free(fd);
        }
    }

    return fat_result_to_vfs(result);
}

/**
 ***********************************************************************************************************************
 * @brief           Run ioctl cmd, not implement now.
 *
 * @param[in]       file            The file descriptor.
 * @param[in]       cmd             The ioctl cmd.
 * @param[in]       args            The argument, related to cmd.
 *
 * @return          The operation result.
 * @retval          -ENOSYS     No cmd support.
 ***********************************************************************************************************************
 */
static int vfs_fat_ioctl(struct vfs_fd *file, int cmd, void *args)
{
    return -ENOSYS;
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
static int vfs_fat_read(struct vfs_fd *file, void *buf, size_t len)
{
    FIL *fd;
    FRESULT result;
    UINT byte_read;

    if ((!file) || (!buf))
    {
        return -EINVAL;
    }
    if (file->type == FT_DIRECTORY)
    {
        return -EISDIR;
    }

    fd = (FIL *)(file->data);
    if (!fd)
    {
        return -EINVAL;
    }
    result = f_read(fd, buf, len, &byte_read);
    /* Update the file position. */
    file->pos  = fd->fptr;
    if (result == FR_OK)
    {
        return byte_read;
    }

    return fat_result_to_vfs(result);
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
static int vfs_fat_write(struct vfs_fd *file, const void *buf, size_t len)
{
    FIL *fd;
    FRESULT result;
    UINT byte_write;

    if ((!file) || (!buf))
    {
        return -EINVAL;
    }
    if (file->type == FT_DIRECTORY)
    {
        return -EISDIR;
    }

    fd = (FIL *)(file->data);
    if (!fd)
    {
        return -EINVAL;
    }
    result = f_write(fd, buf, len, &byte_write);
    /* Update the file position and file size. */
    file->pos  = fd->fptr;
    file->size = f_size(fd);
    if (result == FR_OK)
    {
        return byte_write;
    }

    return fat_result_to_vfs(result);
}

/**
 ***********************************************************************************************************************
 * @brief           Sync the file data to disk.
 *
 * @param[in]       file            The file descriptor.
 *
 * @return          Sync result.
 * @retval          0               Sync data successfully.
 * @retval          negative int    Sync data failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_flush(struct vfs_fd *file)
{
    FIL *fd;
    FRESULT result;

    if (!file)
    {
        return -EINVAL;
    }

    fd = (FIL *)(file->data);
    if (!fd)
    {
        return -EINVAL;
    }
    result = f_sync(fd);
    return fat_result_to_vfs(result);
}

/**
 ***********************************************************************************************************************
 * @brief           Reposition read/write file offset.
 *
 * @param[in,out]   file            The file descriptor.
 * @param[in]       offset          The new position in file.
 *
 * @return          Reposition result.
 * @retval          0               Reposition successfully.
 * @retval          negative int    Reposition failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_lseek(struct vfs_fd *file, os_off_t offset)
{
    FIL *fd;
    DIR *dir;
    FRESULT result = FR_OK;

    if (!file)
    {
        return -EINVAL;
    }

    if (file->type == FT_REGULAR)           /* If file, use f_lseek to seek. */
    {
        fd = (FIL *)(file->data);
        if (!fd)
        {
            return -EINVAL;
        }
        result = f_lseek(fd, offset);
        if (result == FR_OK)
        {
            file->pos = fd->fptr;           /* Update the current position. */
            return fd->fptr;
        }
    }
    else if (file->type == FT_DIRECTORY)    /* If directory, use f_seekdir to seek. */
    {
        dir = (DIR *)(file->data);
        if (!dir)
        {
            return -EINVAL;
        }
        result = f_seekdir(dir, offset / sizeof(struct dirent));
        if (result == FR_OK)
        {
            file->pos = offset;             /* Update the current position. */
            return file->pos;
        }
    }

    return fat_result_to_vfs(result);
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
 * @retval          positive int    The actual read size of dir entry.
 * @retval          negative int    Get dir entry failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_getdents(struct vfs_fd *file, struct dirent *dirp, uint32_t count)
{
    DIR *dir;
    FILINFO *fno;
    FRESULT result;
    os_uint32_t index;
    struct dirent *d;

    if ((!file) || (!dirp))
    {
        return -EINVAL;
    }
    dir = (DIR *)(file->data);
    if (!dir)
    {
        return -EINVAL;
    }

    /* Convert bytes to dir entry count. */
    count = (count / sizeof(struct dirent)) * sizeof(struct dirent);
    if (count == 0)
    {
        return -EINVAL;
    }

    fno = os_malloc(sizeof(FILINFO));
    if (!fno)
    {
        return -ENOMEM;
    }
    index = 0;
    while (1)
    {
        char *fn;

        d = dirp + index;

        result = f_readdir(dir, fno);
        if (result != FR_OK || fno->fname[0] == 0)
        {
            break;
        }

#if _USE_LFN
        fn = *fno->fname ? fno->fname : fno->altname;
#else
        fn = fno->fname;
#endif

        d->d_type = DT_UNKNOWN;
        if (fno->fattrib & AM_DIR)
        {
            d->d_type = DT_DIR;
        }
        else
        {
            d->d_type = DT_REG;
        }

        d->d_namlen = (os_uint8_t)strlen(fn);
        d->d_reclen = (os_uint16_t)sizeof(struct dirent);
        strncpy(d->d_name, fn, strlen(fn) + 1);

        index ++;
        if (index * sizeof(struct dirent) >= count)
        {
            break;
        }
    }

    os_free(fno);
    if (index == 0)
    {
        return fat_result_to_vfs(result);
    }

    file->pos += index * sizeof(struct dirent);

    return index * sizeof(struct dirent);
}

/**
 ***********************************************************************************************************************
 * @brief           Delete file/directory.
 *
 * @param[in]       fs              The pointer of VFS object.
 * @param[in]       path            The file/directory path.
 *
 * @return          Delete result.
 * @retval          0               Delete successfully.
 * @retval          negative int    Delete failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_unlink(struct vfs_filesystem *fs, const char *path)
{
    FRESULT result;
    struct v_fatfs *v_fat;
#if _VOLUMES > 1
    int vol;
    int size;
    char *drivers_fn;
#else
    const char *drivers_fn = path;
#endif

    if ((!fs) || (!path))
    {
        return -EINVAL;
    }

    v_fat = (struct v_fatfs *)fs->data;
    if (!v_fat)
    {
        return -EINVAL;
    }
    if (v_fat_find_entry(v_fat, path) == OS_TRUE)
    {
        LOG_W(FAT_FS_TAG, "This file/dir should be closed before delete:%s", path);
        return -EINVAL;
    }
#if _VOLUMES > 1
    /* If multi vol, the path should contains vol number. */
    vol = fat_get_vol(&v_fat->fat);
    if (vol < 0)
    {
        return -ENOENT;
    }
    size = strlen(path) + 2 + 1;
    drivers_fn = os_malloc(size);
    if (!drivers_fn)
    {
        return -ENOMEM;
    }
    memset(drivers_fn, 0, size);
    os_snprintf(drivers_fn, size, "%d:%s", vol, path);
#endif

    result = f_unlink(drivers_fn);
#if _VOLUMES > 1
    os_free(drivers_fn);
#endif
    return fat_result_to_vfs(result);
}


static int vfs_fat_rename_check(const char *oldpath, const char *newpath)
{
    FILINFO *info_old;
    FILINFO *info_new;
    FILINFO *info_dir_new;
    DIR *dir;
    int result_new;
    int result_old;
    int result = 0;
    int entry_exist = OS_FALSE;
    char *lastname = OS_NULL;
    char *fatherdirname = OS_NULL;
    int len;

    /* The new pathname shall not contain a path prefix that names old. */
    if (strlen(newpath) >= strlen(oldpath))
    {
        if (strncmp(newpath, oldpath, strlen(oldpath)) == 0)
        {
            if ('/' == newpath[strlen(oldpath)])
            {
                return -EINVAL;
            }
        }
    }

    info_old = os_malloc(sizeof(FILINFO));
    if (!info_old)
    {
        return -ENOMEM;
    }
    info_new = os_malloc(sizeof(FILINFO));
    if (!info_new)
    {
        os_free(info_old);
        return -ENOMEM;
    }

    /* Check if the old pathname exist, if not exist, return fail. */
    result_old = f_stat(oldpath, info_old);
    if ((result_old != FR_OK))
    {
        result = -EINVAL;
        goto end;
    }

    /* If newpath and oldpath are the same existing file, shall return successfully and perform no other action.*/
    if (strcmp(newpath, oldpath) == 0)
    {
        result = FAT_RENAME_NO_ACTION;
        goto end;
    }

    /* Check if the new pathname exist */
    result_new = f_stat(newpath, info_new);
    if (result_new == FR_OK)
    {
        /* If new pathname's type not same as old pathname's type, return fail.*/
        if ((info_new->fattrib & AM_DIR) != (info_old->fattrib & AM_DIR))
        {
            result = ((info_new->fattrib & AM_DIR) ? (-EISDIR) : (-ENOTDIR));
            goto end;
        }

        /* If new pathname is a file, delete it first */
        if (!(info_new->fattrib & AM_DIR))
        {
            if (f_unlink(newpath) != FR_OK)
            {
                result = -EINVAL;
                goto end;
            }
        }

        /* If new pathname is a directory, check whether is empty */
        if (info_new->fattrib & AM_DIR)
        {
            dir = os_malloc(sizeof(DIR));
            if (!dir)
            {
                result = -ENOMEM;
                goto end;
            }
            info_dir_new = os_malloc(sizeof(FILINFO));
            if (!info_dir_new)
            {
                os_free(dir);
                result = -ENOMEM;
                goto end;
            }
            if (f_opendir(dir, newpath) != FR_OK)
            {
                os_free(info_dir_new);
                os_free(dir);
                result = -ENOENT;
                goto end;
            }
            if (f_readdir(dir, info_dir_new) == FR_OK)
            {
                if (info_dir_new->fname[0] != 0)
                {
                    entry_exist = OS_TRUE;
                }
            }
            os_free(info_dir_new);
            f_closedir(dir);
            os_free(dir);
            if (entry_exist == OS_FALSE)    /* Dest directory is empty, delete it first */
            {
                if(f_unlink(newpath) != FR_OK)
                {
                    result = -EINVAL;
                    goto end;
                }
            }
            else                            /* Dest directory not empty, not allowed to rename*/
            {
                result = -EINVAL;
                goto end;
            }
                
        }
    }
    else
    {
        /* If the new pathname not exist, check whether the father directory exist */
        lastname = strrchr(newpath, '/');
        fatherdirname = os_malloc(strlen(newpath) + 1);
        if (!fatherdirname)
        {
            result = -ENOMEM;
            goto end;
        }
        memset(fatherdirname, 0, strlen(newpath) + 1);
        strncpy(fatherdirname, newpath, strlen(newpath) - strlen(lastname) + 1);
        dir = os_malloc(sizeof(DIR));
        if (!dir)
        {
            os_free(fatherdirname);
            result = -ENOMEM;
            goto end;
        }
        len = strlen(fatherdirname);
        /* If not root directory, remove the last sperator / */
#if _VOLUMES > 1
        if ((len > 2) && (!((fatherdirname[len - 2] == ':') && (fatherdirname[len - 1] == '/'))))
        {
            fatherdirname[len - 1] = '\0';
        }
#else
        if ((len > 1) && (fatherdirname[len - 1] == '/'))
        {
            fatherdirname[len - 1] = '\0';
        }
#endif
        result_new = f_opendir(dir, fatherdirname);
        os_free(fatherdirname);
        f_closedir(dir);
        os_free(dir);
        /* If dest father directory not exist, fail.*/
        if (result_new != FR_OK)
        {
            result = -EINVAL;
            goto end;
        }
    }

end:
    os_free(info_old);
    os_free(info_new);
    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Rename file/directory.
 *
 * @param[in]       fs              The pointer of VFS object.
 * @param[in]       oldpath         The old file/directory name.
 * @param[in]       newpath         The new file/directory name.
 *
 * @return          The rename result.
 * @retval          0               Rename successfully.
 * @retval          negative int    Rename failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_rename(struct vfs_filesystem *fs, const char *oldpath, const char *newpath)
{
    FRESULT result;
    int ret;
    struct v_fatfs *v_fat;
#if _VOLUMES > 1
    int vol;
    int size;
    char *drivers_oldfn;
    char *drivers_newfn;
#else
    const char *drivers_oldfn = (const char *)oldpath;
    const char *drivers_newfn = (const char *)newpath;
#endif

    if ((!fs) || (!oldpath) || (!newpath))
    {
        return -EINVAL;
    }
    v_fat = (struct v_fatfs *)fs->data;
    if (!v_fat)
    {
        return -EINVAL;
    }

    if (v_fat_find_entry(v_fat, oldpath) == OS_TRUE)
    {
        LOG_W(FAT_FS_TAG, "This file/dir should be closed:%s", oldpath);
        return -EINVAL;
    }
    if (v_fat_find_entry(v_fat, newpath) == OS_TRUE)
    {
        LOG_W(FAT_FS_TAG, "This file/dir should be closed:%s", newpath);
        return -EINVAL;
    }
#if _VOLUMES > 1

    /* If multi vol, the path should contains vol number. */
    vol = fat_get_vol(&v_fat->fat);
    if (vol < 0)
    {
        return -ENOENT;
    }
    size = strlen(oldpath) + 2 + 1;
    drivers_oldfn = os_malloc(size);
    if (!drivers_oldfn)
    {
        return -ENOMEM;
    }   
    memset(drivers_oldfn, 0, size);
    os_snprintf(drivers_oldfn, size, "%d:%s", vol, oldpath);

    size = strlen(newpath) + 2 + 1;
    drivers_newfn = os_malloc(size);
    if (!drivers_newfn)
    {
        os_free(drivers_oldfn);
        return -ENOMEM;
    }    
    memset(drivers_newfn, 0, size);
    os_snprintf(drivers_newfn, size, "%d:%s", vol, newpath);
#endif

    ret = vfs_fat_rename_check(drivers_oldfn, drivers_newfn);
    if (ret == 0)
    {
        result = f_rename(drivers_oldfn, drivers_newfn);
        ret = fat_result_to_vfs(result);
    }
    else if (ret == FAT_RENAME_NO_ACTION)
    {
        ret = 0;
    }
#if _VOLUMES > 1
    os_free(drivers_oldfn);
    os_free(drivers_newfn);
#endif

    return ret;
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
 * @retval          negative int    Get status failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_fat_stat(struct vfs_filesystem *fs, const char *path, struct stat *st)
{
    FILINFO *file_info;
    FRESULT result;
#if _VOLUMES > 1
    struct v_fatfs *v_fat;
    int vol;
    int size;
    char *drivers_fn;
#else
    const char *drivers_fn = path;
#endif

    if ((!fs) || (!path))
    {
        return -EINVAL;
    }

#if _VOLUMES > 1
    v_fat = (struct v_fatfs *)fs->data;
    if (!v_fat)
    {
        return -EINVAL;
    }

    /* If multi vol, the path should contains vol number. */
    vol = fat_get_vol(&v_fat->fat);
    if (vol < 0)
    {
        return -ENOENT;
    }
    size = strlen(path) + 2 + 1;
    drivers_fn = os_malloc(size);
    if (!drivers_fn)
    {
        return -ENOMEM;
    }
    memset(drivers_fn, 0, size);
    os_snprintf(drivers_fn, size, "%d:%s", vol, path);
#endif

    file_info = os_malloc(sizeof(FILINFO));
    if (!file_info)
    {
#if _VOLUMES > 1
        os_free(drivers_fn);
#endif
        return -ENOMEM;
    }
    result = f_stat(drivers_fn, file_info);
#if _VOLUMES > 1
    os_free(drivers_fn);
#endif
    if (result == FR_OK)
    {
        st->st_dev = 0;

        st->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                      S_IWUSR | S_IWGRP | S_IWOTH;
        if (file_info->fattrib & AM_DIR)
        {
            st->st_mode &= ~S_IFREG;
            st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        }
        if (file_info->fattrib & AM_RDO)
        {
            st->st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
        }

        st->st_size  = file_info->fsize;

        /* get st_mtime. */
        {
            struct tm tm_file;
            int year, mon, day, hour, min, sec;
            WORD tmp;

            tmp = file_info->fdate;
            day = tmp & 0x1F;               /* bit[4:0] Day(1..31) */
            tmp >>= 5;
            mon = tmp & 0x0F;               /* bit[8:5] Month(1..12) */
            tmp >>= 4;
            year = (tmp & 0x7F) + 1980;     /* bit[15:9] Year origin from 1980(0..127) */

            tmp = file_info->ftime;
            sec = (tmp & 0x1F) * 2;         /* bit[4:0] Second/2(0..29) */
            tmp >>= 5;
            min = tmp & 0x3F;               /* bit[10:5] Minute(0..59) */
            tmp >>= 6;
            hour = tmp & 0x1F;              /* bit[15:11] Hour(0..23) */

            memset(&tm_file, 0, sizeof(tm_file));
            tm_file.tm_year = year - 1900;  /* Years since 1900 */
            tm_file.tm_mon  = mon - 1;      /* Months *since* january: 0-11 */
            tm_file.tm_mday = day;          /* Day of the month: 1-31 */
            tm_file.tm_hour = hour;         /* Hours since midnight: 0-23 */
            tm_file.tm_min  = min;          /* Minutes: 0-59 */
            tm_file.tm_sec  = sec;          /* Seconds: 0-59 */

            st->st_mtime = mktime(&tm_file);
        }
    }

    os_free(file_info);
    return fat_result_to_vfs(result);
}

static const struct vfs_file_ops vfs_fat_fops = 
{
    vfs_fat_open,
    vfs_fat_close,
    vfs_fat_ioctl,
    vfs_fat_read,
    vfs_fat_write,
    vfs_fat_flush,
    vfs_fat_lseek,
    vfs_fat_getdents,
    OS_NULL,                /* Not support poll. */
};

static const struct vfs_filesystem_ops vfs_fat =
{
    "fat",
    VFS_FS_FLAG_DEFAULT,
    &vfs_fat_fops,

    vfs_fat_mount,
    vfs_fat_unmount,
    vfs_fat_mkfs,
    vfs_fat_statfs,

    vfs_fat_unlink,
    vfs_fat_stat,
    vfs_fat_rename,
};

/**
 ***********************************************************************************************************************
 * @brief           Register FATFS operation structure to VFS.
 *
 * @param[in,out]   None.
 *
 * @return          The register result. 
 * @retval          0               Register successfully.
 * @retval          -1              Register failed.
 ***********************************************************************************************************************
 */
int fat_init(void)
{
    /* Register fatfs file system */
    return vfs_register(&vfs_fat);
}
OS_CMPOENT_INIT(fat_init);

