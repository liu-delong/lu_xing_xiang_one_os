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
 * @file        drv_lcd_mipi.c
 *
 * @brief       This file implements lcd mipi driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_cfg.h>
#include <string.h>
#include <os_sem.h>
#include <os_memory.h>
#include <os_irq.h>
#include <graphic/graphic.h>

static os_err_t os_graphic_control(os_device_t *device, int cmd, void *args)
{
    os_device_graphic_t *graphic;

    OS_ASSERT(device);

    graphic = (os_device_graphic_t *)device;

    switch (cmd)
    {
    case OS_GRAPHIC_CTRL_RECT_UPDATE:
        if (graphic->ops->update)
        {
            graphic->ops->update((struct os_device_rect_info *)args);
        }
        break;

    case OS_GRAPHIC_CTRL_POWERON:
        if (graphic->ops->display_on)
        {
            graphic->ops->display_on(OS_TRUE);
        }
        break;

    case OS_GRAPHIC_CTRL_POWEROFF:
        if (graphic->ops->display_on)
        {
            graphic->ops->display_on(OS_FALSE);
        }
        break;

    case OS_GRAPHIC_CTRL_GET_INFO:
        memcpy(args, &graphic->info, sizeof(struct os_device_graphic_info));
        break;

    case OS_GRAPHIC_CTRL_SET_MODE:
        break;

    case OS_GRAPHIC_CTRL_GET_EXT:
        break;
    }

    return OS_EOK;
}

void os_graphic_register(const char *name, os_device_graphic_t *graphic)
{
    OS_ASSERT(graphic != OS_NULL);
    OS_ASSERT(graphic->ops != OS_NULL);

    graphic->parent.type  = OS_DEVICE_TYPE_GRAPHIC;
    graphic->parent.control = os_graphic_control;
    os_device_register(&graphic->parent, name, OS_DEVICE_FLAG_STANDALONE);
}

#ifdef OS_USING_SHELL

#include <shell.h>

static int graphic_test_888(os_device_graphic_t *graphic)
{
    /* red */
    os_kprintf("red\r\n");
    for (int i = 0; i < graphic->info.framebuffer_size; i += 4)
    {
        *(volatile uint32_t *)(graphic->info.framebuffer + i) = 0xffff0000;
    }
    graphic->parent.control(&graphic->parent, OS_GRAPHIC_CTRL_RECT_UPDATE, OS_NULL);
    os_task_delay(OS_TICK_PER_SECOND);

    /* green */
    os_kprintf("green\r\n");
    for (int i = 0; i < graphic->info.framebuffer_size; i += 4)
    {
        *(volatile uint32_t *)(graphic->info.framebuffer + i) = 0xff00ff00;
    }
    graphic->parent.control(&graphic->parent, OS_GRAPHIC_CTRL_RECT_UPDATE, OS_NULL);
    os_task_delay(OS_TICK_PER_SECOND);

    /* blue */
    os_kprintf("blue\r\n");
    for (int i = 0; i < graphic->info.framebuffer_size; i += 4)
    {
        *(volatile uint32_t *)(graphic->info.framebuffer + i) = 0xff0000ff;
    }
    graphic->parent.control(&graphic->parent, OS_GRAPHIC_CTRL_RECT_UPDATE, OS_NULL);
    os_task_delay(OS_TICK_PER_SECOND);

    /* black */
    os_kprintf("black\r\n");
    for (int i = 0; i < graphic->info.framebuffer_size; i += 4)
    {
        *(volatile uint32_t *)(graphic->info.framebuffer + i) = 0;
    }
    graphic->parent.control(&graphic->parent, OS_GRAPHIC_CTRL_RECT_UPDATE, OS_NULL);
    os_task_delay(OS_TICK_PER_SECOND);

    /* white */
    os_kprintf("white\r\n");
    for (int i = 0; i < graphic->info.framebuffer_size; i += 4)
    {
        *(volatile uint32_t *)(graphic->info.framebuffer + i) = 0xffffffff;
    }
    graphic->parent.control(&graphic->parent, OS_GRAPHIC_CTRL_RECT_UPDATE, OS_NULL);
    os_task_delay(OS_TICK_PER_SECOND);

    return 0;
}

static int graphic_test_565(os_device_graphic_t *graphic)
{
    /* red */
    os_kprintf("red\r\n");
    for (int i = 0; i < graphic->info.framebuffer_size / 2; i++)
    {
        graphic->info.framebuffer[2 * i]     = 0x00;
        graphic->info.framebuffer[2 * i + 1] = 0xF8;
    }
    graphic->parent.control(&graphic->parent, OS_GRAPHIC_CTRL_RECT_UPDATE, OS_NULL);
    os_task_delay(OS_TICK_PER_SECOND);

    /* green */
    os_kprintf("green\r\n");
    for (int i = 0; i < graphic->info.framebuffer_size / 2; i++)
    {
        graphic->info.framebuffer[2 * i]     = 0xE0;
        graphic->info.framebuffer[2 * i + 1] = 0x07;
    }
    graphic->parent.control(&graphic->parent, OS_GRAPHIC_CTRL_RECT_UPDATE, OS_NULL);
    os_task_delay(OS_TICK_PER_SECOND);

    /* blue */
    os_kprintf("blue\r\n");
    for (int i = 0; i < graphic->info.framebuffer_size / 2; i++)
    {
        graphic->info.framebuffer[2 * i]     = 0x1F;
        graphic->info.framebuffer[2 * i + 1] = 0x00;
    }
    graphic->parent.control(&graphic->parent, OS_GRAPHIC_CTRL_RECT_UPDATE, OS_NULL);
    os_task_delay(OS_TICK_PER_SECOND);

    /* black */
    os_kprintf("black\r\n");
    for (int i = 0; i < graphic->info.framebuffer_size / 2; i++)
    {
        graphic->info.framebuffer[2 * i]     = 0;
        graphic->info.framebuffer[2 * i + 1] = 0;
    }
    graphic->parent.control(&graphic->parent, OS_GRAPHIC_CTRL_RECT_UPDATE, OS_NULL);
    os_task_delay(OS_TICK_PER_SECOND);

    /* white */
    os_kprintf("white\r\n");
    for (int i = 0; i < graphic->info.framebuffer_size / 2; i++)
    {
        graphic->info.framebuffer[2 * i]     = 0xff;
        graphic->info.framebuffer[2 * i + 1] = 0xff;
    }
    graphic->parent.control(&graphic->parent, OS_GRAPHIC_CTRL_RECT_UPDATE, OS_NULL);
    os_task_delay(OS_TICK_PER_SECOND);

    return 0;
}

static os_err_t graphic_test(int argc, char **argv)
{
    if (argc != 2)
    {
        os_kprintf("usage: graphic_test <dev> \r\n");
        os_kprintf("       graphic_test lcd \r\n");
        return -1;
    }

    os_device_graphic_t *graphic;
    graphic = (os_device_graphic_t *)os_device_find(argv[1]);

    if (graphic == NULL || graphic->parent.type != OS_DEVICE_TYPE_GRAPHIC)
    {
        os_kprintf("invalide graphic device [%s].\r\n", argv[1]);
        return OS_EINVAL;
    }

    if (graphic->info.pixel_format == OS_GRAPHIC_PIXEL_FORMAT_ARGB888)
    {
        return graphic_test_888(graphic);
    }

    if (graphic->info.pixel_format == OS_GRAPHIC_PIXEL_FORMAT_RGB565)
    {
        return graphic_test_565(graphic);
    }
    
    return OS_EINVAL;
}

SH_CMD_EXPORT(graphic_test, graphic_test, "graphic_test");

#endif

