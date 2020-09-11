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
 * @file        wdg_test.c
 *
 * @brief       The test file for wdg.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_device.h>
#include <os_errno.h>
#include <os_idle.h>
#include <watchdog/watchdog.h>
#include <string.h>
#include <shell.h>

#define WDT_DEVICE_NAME "wwdg"

static int wwdg_test(int argc, char *argv[])
{
    os_err_t    ret     = OS_EOK;
    char        device_name[OS_NAME_MAX];
    int         count = 0;
    os_device_t *wdg_dev;
    
    if (argc == 2)
    {
        strncpy(device_name, argv[1], OS_NAME_MAX);
    }
    else
    {
        strncpy(device_name, WDT_DEVICE_NAME, OS_NAME_MAX);
    }

    wdg_dev = os_device_find(device_name);
    if (!wdg_dev)
    {
        os_kprintf("find %s failed!\n", device_name);
        return OS_ERROR;
    }

    ret = os_device_init(wdg_dev);
    if (ret != OS_EOK)
    {
        os_kprintf("initialize %s failed!\n", device_name);
        return OS_ERROR;
    }

    ret = os_device_control(wdg_dev, OS_DEVICE_CTRL_WDT_START, OS_NULL);
    if (ret != OS_EOK)
    {
        os_kprintf("start %s failed!\n", device_name);
        return OS_ERROR;
    }

    os_device_control(wdg_dev, OS_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
    os_kprintf("watch dog keep alive for :5s\n");
    while (count < 500)
    {
        os_task_mdelay(10);
        count++;
        os_device_control(wdg_dev, OS_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
        //os_kprintf("watch dog keep alive for :%dms\n", count*20);
    }
    count = 0;
    os_kprintf("\n");
    os_kprintf("watch dog stop feed\n");
    while (1)
    {
        os_task_mdelay(10);
        count++;
    }
}

SH_CMD_EXPORT(wwdg_test, wwdg_test, "test Independent window watchdog!");
