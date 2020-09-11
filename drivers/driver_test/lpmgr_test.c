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
 * @file        lpmgr_test.c
 *
 * @brief       The test file for low power management
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_clock.h>
#include <stdio.h>
#include <shell.h>
#include <lpmgr/lpmgr.h>

static os_uint8_t demo_mode = SYS_SLEEP_MODE_NONE;

static int demo_suspend(const struct os_device *device, os_uint8_t mode)
{
    if (demo_mode == mode)
        return 0;

    os_kprintf("demo_suspend mode: %d, %d\r\n", demo_mode, mode);

    if (mode >= SYS_SLEEP_MODE_MAX || mode < demo_mode)
    {
        os_kprintf("demo_suspend invalide mode %d ==> %d\r\n", demo_mode, mode);
        return -1;
    }

    os_kprintf("demo_suspend mode change %d ==> %d\r\n", demo_mode, mode);
    demo_mode = mode;

    return 0;
}

static void demo_resume(const struct os_device *device, os_uint8_t mode)
{
    if (demo_mode == mode)
        return;

    os_kprintf("demo_resume mode: %d, %d\r\n", demo_mode, mode);

    if (mode >= SYS_SLEEP_MODE_MAX || mode > demo_mode)
    {
        os_kprintf("demo_resume invalide mode %d ==> %d\r\n", demo_mode, mode);
        return;
    }

    os_kprintf("demo_resume mode change %d ==> %d\r\n", demo_mode, mode);
    demo_mode = mode;
}

static int demo_frequency_change(const struct os_device *device, os_uint8_t mode)
{
    os_kprintf("demo_frequency_change: %d MHz \r\n", mode + 1);

    return 0;
}

static struct os_lpmgr_device_ops demo_lpmgr_ops =
{
    demo_suspend,
    demo_resume,
    demo_frequency_change,
};

static int lpmgr_test(int argc, char *argv[])
{
    struct os_device *device = os_device_find(OS_CONSOLE_DEVICE_NAME);

    OS_ASSERT(device);

    os_lpmgr_device_register(device, &demo_lpmgr_ops);

    return 0;
}
SH_CMD_EXPORT(lpmgr_test, lpmgr_test, "lpmgr_test");
