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
 * @file        fal_os.c
 *
 * @brief       The fal test function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-20   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <fal/fal.h>
#include <os_memory.h>
#include <string.h>
#include <stdlib.h>
#include <os_clock.h>
#include <os_dbg_ext.h>

#include <os_task.h>
#include <drv_cfg.h>

struct fal_blk_device
{
    struct os_device                parent;
    struct os_device_blk_geometry   geometry;
    fal_part_t     *fal_part;
};

/* device interface */
static os_err_t blk_dev_control(os_device_t *dev, int cmd, void *args)
{
    struct fal_blk_device *part = (struct fal_blk_device*) dev;

    OS_ASSERT(part != OS_NULL);

    if (cmd == OS_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct os_device_blk_geometry *geometry;

        geometry = (struct os_device_blk_geometry *) args;
        if (geometry == OS_NULL)
        {
            return OS_ERROR;
        }

        memcpy(geometry, &part->geometry, sizeof(struct os_device_blk_geometry));
    }
    else if (cmd == OS_DEVICE_CTRL_BLK_ERASE)
    {
        os_uint32_t *addrs = (os_uint32_t *) args, start_addr = addrs[0], end_addr = addrs[1], phy_start_addr;
        os_size_t phy_size;

        if (addrs == OS_NULL || start_addr > end_addr)
        {
            return OS_ERROR;
        }

        if (end_addr == start_addr)
        {
            end_addr++;
        }

        phy_start_addr = start_addr * part->geometry.bytes_per_sector;
        phy_size = (end_addr - start_addr) * part->geometry.bytes_per_sector;

        if (fal_part_erase(part->fal_part, phy_start_addr, phy_size) < 0)
        {
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

static os_size_t blk_dev_read(os_device_t *dev, os_off_t pos, void* buffer, os_size_t size)
{
    int ret = 0;
    struct fal_blk_device *part = (struct fal_blk_device*) dev;

    OS_ASSERT(part != OS_NULL);

    ret = fal_part_read(part->fal_part, pos * part->geometry.bytes_per_sector, buffer, size * part->geometry.bytes_per_sector);

    if (ret != (int)(size * part->geometry.block_size))
    {
        ret = 0;
    }
    else
    {
        ret = size;
    }

    return ret;
}

static os_size_t blk_dev_write(os_device_t *dev, os_off_t pos, const void* buffer, os_size_t size)
{
    int ret = 0;
    struct fal_blk_device *part;
    os_off_t phy_pos;
    os_size_t phy_size;

    part = (struct fal_blk_device*) dev;
    OS_ASSERT(part != OS_NULL);

    /* change the block device's logic address to physical address */
    phy_pos = pos * part->geometry.bytes_per_sector;
    phy_size = size * part->geometry.bytes_per_sector;

    ret = fal_part_erase(part->fal_part, phy_pos, phy_size);

    if (ret == (int) phy_size)
    {
        ret = fal_part_write(part->fal_part, phy_pos, buffer, phy_size);
    }

    if (ret != (int) phy_size)
    {
        ret = 0;
    }
    else
    {
        ret = size;
    }

    return ret;
}

/**
 * create block device by specified partition
 *
 * @param parition_name partition name
 *
 * @return != NULL: created block device
 *            NULL: created failed
 */
struct os_device *fal_blk_device_create(const char *parition_name)
{
    struct fal_blk_device *blk_dev;
    struct fal_part *fal_part = fal_part_find(parition_name);
    const struct fal_flash *fal_flash = fal_part->flash;

    if (!fal_part)
    {
        LOG_EXT_E("Error: the partition name (%s) is not found.", parition_name);
        return NULL;
    }

    blk_dev = (struct fal_blk_device*) os_malloc(sizeof(struct fal_blk_device));
    if (blk_dev)
    {
        blk_dev->fal_part = fal_part;
        blk_dev->geometry.bytes_per_sector = fal_flash->page_size;
        blk_dev->geometry.block_size = fal_flash->block_size;
        blk_dev->geometry.sector_count = fal_part->info->size / fal_flash->page_size;

        /* register device */
        blk_dev->parent.type = OS_DEVICE_TYPE_BLOCK;
        blk_dev->parent.init = NULL;
        blk_dev->parent.open = NULL;
        blk_dev->parent.close = NULL;
        blk_dev->parent.read = blk_dev_read;
        blk_dev->parent.write = blk_dev_write;
        blk_dev->parent.control = blk_dev_control;
        /* no private */
        blk_dev->parent.user_data = OS_NULL;

        LOG_EXT_I("The FAL block device (%s) created successfully", parition_name);
        os_device_register(OS_DEVICE(blk_dev), parition_name, OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_STANDALONE);
    }
    else
    {
        LOG_EXT_E("Error: no memory for create FAL block device");
    }

    return OS_DEVICE(blk_dev);
}

