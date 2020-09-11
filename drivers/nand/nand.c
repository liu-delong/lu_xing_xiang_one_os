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
 * @file        nand.c
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-07-22    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_memory.h>
#include <os_util.h>
#include <os_assert.h>
#include <os_irq.h>
#include <drv_cfg.h>

#ifdef OS_FAL_DYNAMIC_FLASH

#include <fal/fal.h>

static int fal_nand_read_page(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{    
    os_nand_device_t *nand = flash->priv;

    return os_nand_read_page(nand, page_addr, buff, page_nr);
}

static int fal_nand_write_page(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    os_nand_device_t *nand = flash->priv;

    return os_nand_write_page(nand, page_addr, buff, page_nr);
}

static int fal_nand_erase_block(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int i, ret;
    
    os_nand_device_t *nand = flash->priv;

    for (i = 0; i < page_nr; i += nand->cfg.info.block_size)
    {
        ret = os_nand_erase_block(nand, page_addr + i);
        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}

static int fal_nand_flash_register(os_nand_device_t *nand)
{
    fal_flash_dynamic_t *fal_flash = os_calloc(1, sizeof(fal_flash_dynamic_t));

    if (fal_flash == OS_NULL)
    {
        os_kprintf("fal nand mem leak %s.\r\n", nand->parent.parent.name);
        return -1;
    }

    memcpy(fal_flash->flash.name,
           nand->parent.parent.name,
           min(FAL_DEV_NAME_MAX - 1, strlen(nand->parent.parent.name)));
    fal_flash->flash.name[min(FAL_DEV_NAME_MAX - 1, strlen(nand->parent.parent.name))] = 0;
    
    fal_flash->flash.capacity = nand->cfg.capacity;
    fal_flash->flash.block_size = nand->cfg.info.page_size * nand->cfg.info.block_size;
    fal_flash->flash.page_size  = nand->cfg.info.page_size;
    
    fal_flash->flash.ops.init        = OS_NULL;
    fal_flash->flash.ops.read_page   = fal_nand_read_page,
    fal_flash->flash.ops.write_page  = fal_nand_write_page,
    fal_flash->flash.ops.erase_block = fal_nand_erase_block,

    fal_flash->flash.priv = nand;

    return fal_dynamic_flash_register(fal_flash);
}

#endif

static os_err_t os_nand_cfg_prepare(struct os_nand_config *cfg)
{
    const struct nand_device_info *info;

    info = get_nand_info_by_id(cfg->info.id);

    if (info == OS_NULL)
    {
        return OS_ENOSYS;
    }

    cfg->info = *info;

    cfg->page_shift  = os_fls(cfg->info.page_size);
    cfg->block_shift = cfg->page_shift + os_fls(cfg->info.block_size);
    cfg->plane_shift = cfg->block_shift + os_fls(cfg->info.plane_size);;
    
    cfg->page_mask   = (cfg->info.block_size - 1) << cfg->page_shift;
    cfg->block_mask  = (cfg->info.plane_size - 1) << cfg->block_shift;    
    cfg->plane_mask  = (cfg->info.plane_nr - 1) << cfg->plane_shift;

    cfg->capacity = info->page_size * info->block_size * info->plane_size * info->plane_nr;

    return OS_EOK;
}

os_err_t os_nand_device_register(os_nand_device_t *nand, const char *name)
{
    os_err_t ret;

    OS_ASSERT(nand != OS_NULL);

    ret = os_nand_cfg_prepare(&nand->cfg);
    if (ret != OS_EOK)
    {
        os_kprintf("nand device config failed %s, %d\r\n", name, ret);
        return ret;
    }

    os_device_default(&nand->parent, OS_DEVICE_TYPE_MTD);
    os_device_register(&nand->parent, name, OS_DEVICE_FLAG_RDWR);

    struct nand_device_info *info = &nand->cfg.info;

    os_kprintf("nand device register success %s(%s):\r\n"
               "    nand  id   : %08X\r\n"
               "    page  size : %d\r\n"
               "    spare size : %d\r\n"
               "    block size : %d\r\n"
               "    plane size : %d\r\n"
               "    plane count: %d\r\n"
               "    total size : %d MB\r\n",
               name, info->name,
               info->id,
               info->page_size,
               info->spare_size,
               info->block_size,
               info->plane_size,
               info->plane_nr,
               nand->cfg.capacity / 0x100000);

#ifdef OS_FAL_DYNAMIC_FLASH
    return fal_nand_flash_register(nand);
#else
    return OS_EOK;
#endif
}

os_err_t os_nand_read_page(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    return nand->ops->read_page(nand, page_addr, buff, page_nr);
}

os_err_t os_nand_write_page(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    return nand->ops->write_page(nand, page_addr, buff, page_nr);
}

os_err_t os_nand_read_spare(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t spare_nr)
{
    return nand->ops->read_spare(nand, page_addr, buff, spare_nr);
}

os_err_t os_nand_write_spare(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t spare_nr)
{
    return nand->ops->write_spare(nand, page_addr, buff, spare_nr);
}

os_err_t os_nand_erase_block(os_nand_device_t *nand, os_uint32_t page_addr)
{
    return nand->ops->erase_block(nand, page_addr);
}

os_err_t os_nand_enable_ecc(os_nand_device_t *nand)
{
    return nand->ops->enable_ecc(nand);
}

os_err_t os_nand_disable_ecc(os_nand_device_t *nand)
{
    return nand->ops->disable_ecc(nand);
}

os_err_t os_nand_get_ecc(os_nand_device_t *nand, os_uint32_t *ecc_value)
{
    return nand->ops->get_ecc(nand, ecc_value);
}

os_uint32_t os_nand_get_status(os_nand_device_t *nand)
{
    return nand->ops->get_status(nand);
}

void nand_page2addr(os_nand_device_t *nand, os_uint32_t page_addr, os_nand_addr_t *nand_addr)
{
    os_uint32_t addr = page_addr << nand->cfg.page_shift;

    nand_addr->plane = (addr & nand->cfg.plane_mask) >> nand->cfg.plane_shift;
    nand_addr->block = (addr & nand->cfg.block_mask) >> nand->cfg.block_shift;
    nand_addr->page  = (addr & nand->cfg.page_mask) >> nand->cfg.page_shift;
}

os_uint32_t nand_addr2page(os_nand_device_t *nand, os_nand_addr_t *nand_addr)
{
    os_uint32_t page_addr;

    page_addr  = nand_addr->plane << nand->cfg.plane_shift;
    page_addr |= nand_addr->block << nand->cfg.block_shift;
    page_addr |= nand_addr->page << nand->cfg.page_shift;
        
    return page_addr;
}

