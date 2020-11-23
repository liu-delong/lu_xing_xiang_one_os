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
 * @file        ramdisk.c
 *
 * @brief       This file implement ramdisk device.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-06   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <string.h>
#include <os_dbg.h>
#include <os_errno.h>
#include <os_memory.h>
#include <os_device.h>
#include "ramdisk.h"

#define RAMDISK_TAG         "RAMDISK"

static os_size_t ramdisk_dev_read(os_device_t *dev, os_off_t pos, void* buffer, os_size_t count)
{
    ramdisk_dev_t *ram_dev;    

    if ((!dev) || (!buffer))
    {        
        LOG_E(RAMDISK_TAG, "invalid paramater");
        return 0;
    }

    ram_dev = os_container_of(dev, ramdisk_dev_t, device);
    if (ram_dev->init_flag == OS_FALSE)
    {
        return 0;
    }
    if ((pos > ram_dev->block_cnt) || (pos < 0))
    {
        LOG_E(RAMDISK_TAG, "invalid paramater");
        return 0;
    }

    if (pos + count > ram_dev->block_cnt)
    {
        count = ram_dev->block_cnt - pos;
    }

    memcpy(buffer, ram_dev->ram_addr + ram_dev->block_size*pos, ram_dev->block_size*count);

    return count;
};

static os_size_t ramdisk_dev_write(os_device_t *dev, os_off_t pos, const void* buffer, os_size_t count)
{
    ramdisk_dev_t *ram_dev;

    if ((!dev) || (!buffer))
    {        
        LOG_E(RAMDISK_TAG, "invalid paramater");
        return 0;
    }

    ram_dev = os_container_of(dev, ramdisk_dev_t, device);
    if (ram_dev->init_flag == OS_FALSE)
    {
        return 0;
    }
    if ((pos > ram_dev->block_cnt) || (pos < 0))
    {
        LOG_E(RAMDISK_TAG, "invalid paramater");
        return 0;
    }

    if (pos + count > ram_dev->block_cnt)
    {
        count = ram_dev->block_cnt - pos;
    }
    memcpy(ram_dev->ram_addr + ram_dev->block_size*pos, buffer, ram_dev->block_size*count);

    return count;
};

static os_err_t ramdisk_dev_control(os_device_t *dev, int cmd, void *args)
{    
    ramdisk_dev_t *ram_dev;    
    struct os_device_blk_geometry *geometry;
    os_size_t pos;

    if (!dev)
    {
        return OS_EINVAL;
    }

    ram_dev = os_container_of(dev, ramdisk_dev_t, device);
    if (ram_dev->init_flag == OS_FALSE)
    {
        return OS_EINVAL;
    }

    switch (cmd)
    {
    case OS_DEVICE_CTRL_BLK_GETGEOME:
        if (!args)
        {
            LOG_E(RAMDISK_TAG, "invalid paramater");
            return OS_EINVAL;
        }
        geometry = (struct os_device_blk_geometry *) args;
        geometry->block_size = ram_dev->block_size;
        geometry->bytes_per_sector = ram_dev->block_size;
        geometry->sector_count = ram_dev->block_cnt;
        break;

    case OS_DEVICE_CTRL_BLK_ERASE:
        if (!args)
        {
            LOG_E(RAMDISK_TAG, "invalid paramater");
            return OS_EINVAL;
        }
        pos = *(os_size_t *)args;
        memset(ram_dev->ram_addr + ram_dev->block_size*pos, 0, ram_dev->block_size);
        break;

    default:
        break;
    }

    return OS_EOK;
}

const static struct os_device_ops ramdisk_ops =
{
    OS_NULL,
    OS_NULL,
    OS_NULL,
    ramdisk_dev_read,
    ramdisk_dev_write,
    ramdisk_dev_control
};

static void ramdisk_set_ops(os_device_t *device)
{
    device->type = OS_DEVICE_TYPE_BLOCK;
    device->ops = &ramdisk_ops;
}

/**
 ***********************************************************************************************************************
 * @brief           Init ramdisk device.
 *
 * @param[in,out]   ram_dev         The pointer of ramdisk device.
 * @param[in]       addr            The start address of ramdisk.
 * @param[in]       name            Ramdisk name.
 * @param[in]       size            Ramdisk size.
 * @param[in]       block_size      Size of every block.
 *
 * @return          The initial result.
 * @retval          OS_EOK          Init successfully.
 * @retval          Others          Init failed, return the error code.
 ***********************************************************************************************************************
 */
os_err_t ramdisk_dev_init(ramdisk_dev_t *ram_dev, void *addr, const char *name, os_uint32_t size, os_uint32_t block_size)
{
    os_uint32_t block_cnt;

    block_size = OS_ALIGN_UP(block_size, 4);
    if ((!ram_dev) || (!addr) || (!name) || (block_size < BLOCK_MINSIZE) || (size < block_size))
    {
        LOG_E(RAMDISK_TAG, "invalid device_name or ramdisk_size");
        return OS_EINVAL;
    }

    ram_dev->ram_addr = (char *)OS_ALIGN_UP((os_uint32_t)addr, 4);
    size -= (ram_dev->ram_addr - (char *)addr);
    block_cnt = size/block_size;
    ram_dev->ram_size = block_cnt*block_size;
    ram_dev->block_size = block_size;
    ram_dev->block_cnt = block_cnt;
    ram_dev->init_flag = OS_TRUE;

    memset(&ram_dev->device, 0, sizeof(os_device_t));
    ramdisk_set_ops(&ram_dev->device);
    if(OS_EOK != os_device_register(&ram_dev->device, name, OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_STANDALONE))
    {
        LOG_W(RAMDISK_TAG, "create ramdisk device failed");
        return OS_ERROR;
    }

    LOG_W(RAMDISK_TAG, "create ramdisk device:%s successfully", name);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Deinit ramdisk device.
 *
 * @param[in,out]   ram_dev         The pointer of ramdisk device.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void ramdisk_dev_deinit(ramdisk_dev_t* ram_dev)
{
    if (!ram_dev)
    {
        LOG_E(RAMDISK_TAG, "invalid device");
        return;
    }

    os_device_unregister(&ram_dev->device);
    ram_dev->ram_addr = OS_NULL;
    ram_dev->ram_size = 0;
    ram_dev->block_cnt = 0;
    ram_dev->block_size = 0;    
    ram_dev->init_flag = OS_FALSE;
}

#ifdef OS_USING_HEAP
/**
 ***********************************************************************************************************************
 * @brief           Create ramdisk device, the ramdisk's memory will be dynamci allocted.
 *
 * @param[in]       name            Ramdisk name.
 * @param[in]       size            Ramdisk size.
 * @param[in]       block_size      Size of every block.
 *
 * @return          The initial result.
 * @retval          ramdisk_dev_t*  The pointer of ramdisk device..
 * @retval          OS_NULL         Create ramdisk failed.
 ***********************************************************************************************************************
 */
ramdisk_dev_t *ramdisk_dev_create(const char *name, os_uint32_t size, os_uint32_t block_size)
{
    ramdisk_dev_t *ram_dev;
    os_uint32_t block_cnt;

    block_size = OS_ALIGN_UP(block_size, 4);
    if ((!name) || (block_size < BLOCK_MINSIZE) || (size < block_size))
    {
        LOG_E(RAMDISK_TAG, "invalid device_name or ramdisk_size");
        return OS_NULL;
    }

    ram_dev = os_malloc(sizeof(ramdisk_dev_t));
    if (!ram_dev)
    {
        LOG_E(RAMDISK_TAG, "no memory for create ramdisk device");
        return OS_NULL;
    }

    block_cnt = size/block_size;
    ram_dev->ram_addr = os_malloc(block_cnt*block_size);
    if (!ram_dev->ram_addr)
    {
        os_free(ram_dev);
        LOG_E(RAMDISK_TAG, "no memory for create ramdisk");
        return OS_NULL;
    }
    ram_dev->ram_size = block_cnt*block_size;
    ram_dev->block_size = block_size;
    ram_dev->block_cnt = block_cnt;
    ram_dev->init_flag = OS_TRUE;

    memset(&ram_dev->device, 0, sizeof(os_device_t));
    ramdisk_set_ops(&ram_dev->device);
    if(OS_EOK != os_device_register(&ram_dev->device, name, OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_STANDALONE))
    {
        os_free(ram_dev->ram_addr);
        os_free(ram_dev);
        LOG_W(RAMDISK_TAG, "create ramdisk device failed");
        return OS_NULL;
    }

    LOG_W(RAMDISK_TAG, "create ramdisk device:%s successfully", name);

    return ram_dev;
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy ramdisk device, the ramdisk's memory will be free.
 *
 * @param[in,out]   ram_dev         The pointer of ramdisk device.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void ramdisk_dev_destroy(ramdisk_dev_t *ram_dev)
{
    if (!ram_dev)
    {
        LOG_E(RAMDISK_TAG, "invalid device");
        return;
    }

    os_device_unregister(&ram_dev->device);
    os_free(ram_dev->ram_addr);
    os_free(ram_dev);
}
#endif
