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
 * @file        gui_test.c
 *
 * @brief       The test file for gui.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <string.h>
#include <stdio.h>
#include <shell.h>
#include <lvgl/lvgl.h>

static void btn_event_cb(struct _lv_obj_t *obj, lv_event_t event)
{
    static int pos = 0;
    pos++;

    lv_obj_set_pos(obj, 10 + (pos & 1) * 100, 10);
    os_kprintf("----------------------button clicked\r\n");
}

static void gui_test(void *parameter)
{
    int  i;
    char buff[64];

    /* Gui */
    /* Button */
    lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL); /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                       /*Set its position*/
    lv_obj_set_size(btn, 100, 50);                     /*Set its size*/
    lv_obj_set_event_cb(btn, btn_event_cb);            /*Assign a callback to the button*/

    lv_obj_t *label = lv_label_create(btn, NULL); /*Add a label to the button*/
    lv_label_set_text(label, "Button");           /*Set the labels text*/

#ifdef OS_USING_GUI_LVGL_CONSOLE
    /* Gui console */
    os_device_t *gui_console = os_device_find("gui_console");
    OS_ASSERT(gui_console);
    os_device_open(gui_console, OS_DEVICE_FLAG_RDWR);

    for (i = 0; i < 10; i++)
    {
        snprintf(buff, sizeof(buff) - 1, "%d\n", i);
        os_device_write(gui_console, 0, buff, strlen((const char *)buff));
        os_task_mdelay(1000);
    }

    os_device_close(gui_console);
#endif
}
SH_CMD_EXPORT(gui_test, gui_test, "test gui_test");
