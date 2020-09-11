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
 * @file        fal.c
 *
 * @brief       fal
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <fal.h>
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_assert.h>
#include <string.h>

static fal_flash_t *fal_flash_table = OS_NULL;
static int fal_flash_table_size = 0;

#ifdef OS_FAL_DYNAMIC_FLASH
static os_list_node_t fal_dynamic_list = OS_LIST_INIT(fal_dynamic_list);

int fal_dynamic_flash_register(fal_flash_dynamic_t *flash)
{
    os_base_t level;
    
    level = os_hw_interrupt_disable();
    os_list_add_tail(&fal_dynamic_list, &flash->list);
    os_hw_interrupt_enable(level);
    
    return 0;
}

static fal_flash_t *fal_dynamic_flash_find(const char *name)
{
    fal_flash_dynamic_t *flash;
    
    os_base_t level;
    
    level = os_hw_interrupt_disable();
    
    os_list_for_each_entry(flash, &fal_dynamic_list, fal_flash_dynamic_t, list)
    {
        if (!strcmp(name, flash->flash.name))
        {
            os_hw_interrupt_enable(level);
            return (fal_flash_t *)flash;
        }
    }
    
    os_hw_interrupt_enable(level);

    return OS_NULL;
}

static int fal_dynamic_flash_init(void)
{
    fal_flash_dynamic_t *flash;
    
    os_base_t level;
    
    level = os_hw_interrupt_disable();
    
    os_list_for_each_entry(flash, &fal_dynamic_list, fal_flash_dynamic_t, list)
    {
        if (flash->flash.ops.init)
        {
            flash->flash.ops.init(&flash->flash);
        }
        
        os_kprintf("Flash device | %*.*s | len: 0x%08x | block_size: 0x%08x | page_size: 0x%08x\n",
                FAL_DEV_NAME_MAX, FAL_DEV_NAME_MAX, flash->flash.name, flash->flash.capacity,
                flash->flash.block_size, flash->flash.page_size);
    }
    
    os_hw_interrupt_enable(level);

    return 0;
}

#endif

fal_flash_t *fal_flash_find(const char *name)
{
    OS_ASSERT(name);

    size_t i;

    for (i = 0; i < fal_flash_table_size; i++)
    {
        if (!strncmp(name, fal_flash_table[i].name, FAL_DEV_NAME_MAX)) {
            return &fal_flash_table[i];
        }
    }

#ifdef OS_FAL_DYNAMIC_FLASH
    return fal_dynamic_flash_find(name);
#else
    return OS_NULL;
#endif
}

static int fal_flash_init(void)
{
    int i;

#if defined(__CC_ARM)                           /* ARM MDK Compiler */    
    extern const int fal_flash_table$$Base;
    extern const int fal_flash_table$$Limit;
    
    fal_flash_table = (fal_flash_t *)&fal_flash_table$$Base;
    fal_flash_table_size = (fal_flash_t *)&fal_flash_table$$Limit - fal_flash_table;
#elif defined(__ICCARM__) || defined(__ICCRX__) /* for IAR Compiler */
    fal_flash_table = (fal_flash_t *)__section_begin("fal_flash_table");
    fal_flash_table_size = (fal_flash_t *)__section_end("fal_flash_table") - fal_flash_table;
#elif defined(__GNUC__)                         /* for GCC Compiler */
    extern const int __fal_flash_table_start;
    extern const int __fal_flash_table_end;
    fal_flash_table = (fal_flash_t *)&__fal_flash_table_start;
    fal_flash_table_size = (fal_flash_t *)&__fal_flash_table_end - fal_flash_table;
#endif

    for (i = 0; i < fal_flash_table_size; i++)
    {
        if (fal_flash_table[i].ops.init)
        {
            fal_flash_table[i].ops.init(&fal_flash_table[i]);
        }
        
        os_kprintf("Flash device | %*.*s | len: 0x%08x | block_size: 0x%08x | page_size: 0x%08x\n",
                FAL_DEV_NAME_MAX, FAL_DEV_NAME_MAX, fal_flash_table[i].name, fal_flash_table[i].capacity,
                fal_flash_table[i].block_size, fal_flash_table[i].page_size);
    }

#ifdef OS_FAL_DYNAMIC_FLASH
    return fal_dynamic_flash_init();
#else
    return 0;
#endif
}

int fal_part_init(void);

static int fal_init(void)
{
    int result;

    result = fal_flash_init();

    if (result < 0)
    {
        os_kprintf("fal flash init failed\r\n");
        return result;
    }

    result = fal_part_init();

    if (result < 0)
    {
        os_kprintf("fal part init failed\r\n");
        return result;
    }

    return result;
}

OS_INIT_EXPORT(fal_init, "5");
