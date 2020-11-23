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
 * @file        block_dev.c
 *
 * @brief       This file provides functions for mmcsd read/wrtie/probe.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_device.h>
#include <os_list.h>
#include <vfs_fs.h>
#include <os_sem.h>
#include <string.h>
#include <sdio/mmcsd_core.h>
#include <os_memory.h>

#ifdef OS_SDIO_DEBUG
#define DRV_EXT_LVL DBG_EXT_DEBUG
#else
#define DRV_EXT_LVL DBG_EXT_INFO
#endif /* OS_SDIO_DEBUG */
#define DRV_EXT_TAG "block_dev"
#include <drv_log.h>

static os_list_node_t blk_devices = OS_LIST_INIT(blk_devices);

#define BLK_MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 ***********************************************************************************************************************
 * @struct      mmcsd_blk_device
 *
 * @brief       structure of mmcsd_blk_device
 ***********************************************************************************************************************
 */
struct mmcsd_blk_device
{
    struct os_mmcsd_card         *card;
    os_list_node_t                list;
    struct os_device              dev;
    struct vfs_partition          part;
    struct os_device_blk_geometry geometry;
    os_size_t                     max_req_size;
};

#ifndef OS_MMCSD_MAX_PARTITION
#define OS_MMCSD_MAX_PARTITION 16
#endif

os_int32_t mmcsd_num_wr_blocks(struct os_mmcsd_card *card)
{
    os_int32_t  err;
    os_uint32_t blocks;

    struct os_mmcsd_req  req;
    struct os_mmcsd_cmd  cmd;
    struct os_mmcsd_data data;
    os_uint32_t          timeout_us;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = APP_CMD;
    cmd.arg      = card->rca << 16;
    cmd.flags    = RESP_SPI_R1 | RESP_R1 | CMD_AC;

    err = mmcsd_send_cmd(card->host, &cmd, 0);
    if (err)
        return OS_ERROR;
    if (!controller_is_spi(card->host) && !(cmd.resp[0] & R1_APP_CMD))
        return OS_ERROR;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SD_APP_SEND_NUM_WR_BLKS;
    cmd.arg      = 0;
    cmd.flags    = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    memset(&data, 0, sizeof(struct os_mmcsd_data));

    data.timeout_ns   = card->tacc_ns * 100;
    data.timeout_clks = card->tacc_clks * 100;

    timeout_us = data.timeout_ns / 1000;
    timeout_us += data.timeout_clks * 1000 / (card->host->io_cfg.clock / 1000);

    if (timeout_us > 100000)
    {
        data.timeout_ns   = 100000000;
        data.timeout_clks = 0;
    }

    data.blksize = 4;
    data.blks    = 1;
    data.flags   = DATA_DIR_READ;
    data.buf     = &blocks;

    memset(&req, 0, sizeof(struct os_mmcsd_req));

    req.cmd  = &cmd;
    req.data = &data;

    mmcsd_send_request(card->host, &req);

    if (cmd.err || data.err)
        return OS_ERROR;

    return blocks;
}

static os_err_t
os_mmcsd_req_blk(struct os_mmcsd_card *card, os_uint32_t sector, void *buf, os_size_t blks, os_uint8_t dir)
{
    struct os_mmcsd_cmd   cmd, stop;
    struct os_mmcsd_data  data;
    struct os_mmcsd_req   req;
    struct os_mmcsd_host *host = card->host;
    os_uint32_t           r_cmd, w_cmd;

    mmcsd_host_lock(host);
    memset(&req, 0, sizeof(struct os_mmcsd_req));
    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));
    memset(&stop, 0, sizeof(struct os_mmcsd_cmd));
    memset(&data, 0, sizeof(struct os_mmcsd_data));
    req.cmd  = &cmd;
    req.data = &data;

    cmd.arg = sector;
    if (!(card->flags & CARD_FLAG_SDHC))
    {
        cmd.arg <<= 9;
    }
    cmd.flags = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    data.blksize = SECTOR_SIZE;
    data.blks    = blks;

    if (blks > 1)
    {
        if (!controller_is_spi(card->host) || !dir)
        {
            req.stop      = &stop;
            stop.cmd_code = STOP_TRANSMISSION;
            stop.arg      = 0;
            stop.flags    = RESP_SPI_R1B | RESP_R1B | CMD_AC;
        }
        r_cmd = READ_MULTIPLE_BLOCK;
        w_cmd = WRITE_MULTIPLE_BLOCK;
    }
    else
    {
        req.stop = OS_NULL;
        r_cmd    = READ_SINGLE_BLOCK;
        w_cmd    = WRITE_BLOCK;
    }

    if (!dir)
    {
        cmd.cmd_code = r_cmd;
        data.flags |= DATA_DIR_READ;
    }
    else
    {
        cmd.cmd_code = w_cmd;
        data.flags |= DATA_DIR_WRITE;
    }

    mmcsd_set_data_timeout(&data, card);
    data.buf = buf;
    mmcsd_send_request(host, &req);

    if (!controller_is_spi(card->host) && dir != 0)
    {
        do
        {
            os_int32_t err;

            cmd.cmd_code = SEND_STATUS;
            cmd.arg      = card->rca << 16;
            cmd.flags    = RESP_R1 | CMD_AC;
            err          = mmcsd_send_cmd(card->host, &cmd, 5);
            if (err)
            {
                LOG_EXT_E("error %d requesting status", err);
                break;
            }
            /*
             * Some cards mishandle the status bits,
             * so make sure to check both the busy
             * indication and the card state.
             */
        } while (!(cmd.resp[0] & R1_READY_FOR_DATA) || (R1_CURRENT_STATE(cmd.resp[0]) == 7));
    }

    mmcsd_host_unlock(host);

    if (cmd.err || data.err || stop.err)
    {
        LOG_EXT_E("mmcsd request blocks error");
        LOG_EXT_E("%d,%d,%d, 0x%08x,0x%08x", cmd.err, data.err, stop.err, data.flags, sector);

        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t os_mmcsd_init(os_device_t *dev)
{
    return OS_EOK;
}

static os_err_t os_mmcsd_open(os_device_t *dev, os_uint16_t oflag)
{
    return OS_EOK;
}

static os_err_t os_mmcsd_close(os_device_t *dev)
{
    return OS_EOK;
}

static os_err_t os_mmcsd_control(os_device_t *dev, int cmd, void *args)
{
    struct mmcsd_blk_device *blk_dev = (struct mmcsd_blk_device *)dev->user_data;
    switch (cmd)
    {
    case OS_DEVICE_CTRL_BLK_GETGEOME:
        memcpy(args, &blk_dev->geometry, sizeof(struct os_device_blk_geometry));
        break;
    default:
        break;
    }
    return OS_EOK;
}

static os_size_t os_mmcsd_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_err_t    err         = 0;
    os_size_t   offset      = 0;
    os_size_t   req_size    = 0;
    os_size_t   remain_size = size;
    void       *rd_ptr      = (void *)buffer;
	
    struct mmcsd_blk_device *blk_dev  = (struct mmcsd_blk_device *)dev->user_data;
    struct vfs_partition    *part     = &blk_dev->part;

    if (dev == OS_NULL)
    {
        os_set_errno(-EINVAL);
        return 0;
    }

    os_sem_wait(part->lock, OS_IPC_WAITING_FOREVER);
    while (remain_size)
    {
        req_size = (remain_size > blk_dev->max_req_size) ? blk_dev->max_req_size : remain_size;
        err      = os_mmcsd_req_blk(blk_dev->card, part->offset + pos + offset, rd_ptr, req_size, 0);
        if (err)
            break;
        offset += req_size;
        rd_ptr = (void *)((os_uint8_t *)rd_ptr + (req_size << 9));
        remain_size -= req_size;
    }
    os_sem_post(part->lock);

    /* the length of reading must align to SECTOR SIZE */
    if (err)
    {
        os_set_errno(-EIO);
        return 0;
    }
    return size - remain_size;
}

static os_size_t os_mmcsd_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_err_t                 err         = 0;
    os_size_t                offset      = 0;
    os_size_t                req_size    = 0;
    os_size_t                remain_size = size;
    void                    *wr_ptr      = (void *)buffer;
    struct mmcsd_blk_device *blk_dev     = (struct mmcsd_blk_device *)dev->user_data;
    struct vfs_partition    *part        = &blk_dev->part;

    if (dev == OS_NULL)
    {
        os_set_errno(-EINVAL);
        return 0;
    }

    os_sem_wait(part->lock, OS_IPC_WAITING_FOREVER);
    while (remain_size)
    {
        req_size = (remain_size > blk_dev->max_req_size) ? blk_dev->max_req_size : remain_size;
        err      = os_mmcsd_req_blk(blk_dev->card, part->offset + pos + offset, wr_ptr, req_size, 1);
        if (err)
            break;
        offset += req_size;
        wr_ptr = (void *)((os_uint8_t *)wr_ptr + (req_size << 9));
        remain_size -= req_size;
    }
    os_sem_post(part->lock);

    /* the length of reading must align to SECTOR SIZE */
    if (err)
    {
        os_set_errno(-EIO);

        return 0;
    }
    return size - remain_size;
}

static os_int32_t mmcsd_set_blksize(struct os_mmcsd_card *card)
{
    struct os_mmcsd_cmd cmd;
    int                 err;

    /* Block-addressed cards ignore MMC_SET_BLOCKLEN. */
    if (card->flags & CARD_FLAG_SDHC)
        return 0;

    mmcsd_host_lock(card->host);
    cmd.cmd_code = SET_BLOCKLEN;
    cmd.arg      = 512;
    cmd.flags    = RESP_SPI_R1 | RESP_R1 | CMD_AC;
    err          = mmcsd_send_cmd(card->host, &cmd, 5);
    mmcsd_host_unlock(card->host);

    if (err)
    {
        LOG_EXT_E("MMCSD: unable to set block size to %d: %d", cmd.arg, err);

        return OS_ERROR;
    }

    return 0;
}

const static struct os_device_ops mmcsd_blk_ops =
{
    os_mmcsd_init, 
    os_mmcsd_open, 
    os_mmcsd_close, 
    os_mmcsd_read, 
    os_mmcsd_write, 
    os_mmcsd_control,
};

os_int32_t os_mmcsd_blk_probe(struct os_mmcsd_card *card)
{
    os_int32_t               err = 0;
    os_uint8_t               i, status;
    os_uint8_t              *sector;
    char                     dname[4];
    char                     sname[8];
    struct mmcsd_blk_device *blk_dev = OS_NULL;

    err = mmcsd_set_blksize(card);
    if (err)
    {
        return err;
    }

    LOG_EXT_D("probe mmcsd block device!");

    /* get the first sector to read partition table */
    sector = (os_uint8_t *)os_malloc(SECTOR_SIZE);
    if (sector == OS_NULL)
    {
        LOG_EXT_E("allocate partition sector buffer failed!");

        return OS_ENOMEM;
    }

    status = os_mmcsd_req_blk(card, 0, sector, 1, 0);
    if (status == OS_EOK)
    {
        for (i = 0; i < OS_MMCSD_MAX_PARTITION; i++)
        {
            blk_dev = os_calloc(1, sizeof(struct mmcsd_blk_device));
            if (!blk_dev)
            {
                LOG_EXT_E("mmcsd:malloc memory failed!");
                break;
            }

            blk_dev->max_req_size = BLK_MIN((card->host->max_dma_segs * card->host->max_seg_size) >> 9,
                                            (card->host->max_blk_count * card->host->max_blk_size) >> 9);

            /* get the first partition */
            status = vfs_filesystem_get_partition(&blk_dev->part, sector, i);
            if (status == OS_EOK)
            {
                os_snprintf(dname, 4, "sd%d", i);
                os_snprintf(sname, 8, "sem_sd%d", i);
                blk_dev->part.lock = os_sem_create(sname, 1, OS_IPC_FLAG_FIFO);

                /* register mmcsd device */
                blk_dev->dev.type = OS_DEVICE_TYPE_BLOCK;
                blk_dev->dev.ops = &mmcsd_blk_ops;
                blk_dev->dev.user_data = blk_dev;
                blk_dev->card = card;

                blk_dev->geometry.bytes_per_sector = 1 << 9;
                blk_dev->geometry.block_size       = card->card_blksize;
                blk_dev->geometry.sector_count     = blk_dev->part.size;

                os_device_register(&blk_dev->dev,
                                   dname,
                                   OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_REMOVABLE | OS_DEVICE_FLAG_STANDALONE);
                os_list_add_tail(&blk_devices, &blk_dev->list);
            }
            else
            {
                if (i == 0)
                {
                    /* there is no partition table */
                    blk_dev->part.offset = 0;
                    blk_dev->part.size   = 0;
                    blk_dev->part.lock   = os_sem_create("sem_sd0", 1, OS_IPC_FLAG_FIFO);

                    /* register mmcsd device */
                    blk_dev->dev.type = OS_DEVICE_TYPE_BLOCK;
                    blk_dev->dev.ops = &mmcsd_blk_ops;
                    blk_dev->dev.user_data = blk_dev;

                    blk_dev->card = card;

                    blk_dev->geometry.bytes_per_sector = 1 << 9;
                    blk_dev->geometry.block_size       = card->card_blksize;
                    blk_dev->geometry.sector_count     = card->card_capacity * (1024 / 512);

                    os_device_register(&blk_dev->dev,
                                       "sd0",
                                       OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_REMOVABLE | OS_DEVICE_FLAG_STANDALONE);
                    os_list_add_tail(&blk_devices, &blk_dev->list);
                }
                else
                {
                    os_free(blk_dev);
                    blk_dev = OS_NULL;
                    break;
                }
            }

#ifdef OS_USING_DFS_MNTTABLE
            if (blk_dev)
            {
                LOG_EXT_I("try to mount file system!");
                /* try to mount file system on this block device */
                vfs_mount_device(&(blk_dev->dev));
            }
#endif
        }
    }
    else
    {
        LOG_EXT_E("read mmcsd first sector failed");
        err = OS_ERROR;
    }

    /* release sector buffer */
    os_free(sector);

    return err;
}

void os_mmcsd_blk_remove(struct os_mmcsd_card *card)
{
    os_list_node_t *         l, *n;
    struct mmcsd_blk_device *blk_dev;

    for (l = (&blk_devices)->next, n = l->next; l != &blk_devices; l = n)
    {
        blk_dev = (struct mmcsd_blk_device *)os_list_entry(l, struct mmcsd_blk_device, list);
        if (blk_dev->card == card)
        {
            /* unmount file system */
            const char *mounted_path = vfs_filesystem_get_mounted_path(&(blk_dev->dev));
            if (mounted_path)
            {
                vfs_unmount(mounted_path);
                LOG_EXT_D("unmount file system %s for device %s.\r\n", mounted_path, device_name(&blk_dev->dev));
            }
            os_sem_destroy(blk_dev->part.lock);
            os_device_unregister(&blk_dev->dev);
            os_list_del(&blk_dev->list);
            os_free(blk_dev);
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function will initialize block device on the mmc/sd.
 *
 * @details         since 2.1.0, this function does not need to be invoked in the system initialization.
 *
 * @param[in]       none
 *
 * @return          always return 0
 ***********************************************************************************************************************
 */

int os_mmcsd_blk_init(void)
{
    /* nothing */
    return 0;
}
