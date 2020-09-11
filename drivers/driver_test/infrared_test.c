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
 * @file        infrared_test.c
 *
 * @brief       The test file for infrared.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_clock.h>
#include <shell.h>
#include <string.h>
#include <infrared/infrared.h>

#define DBG_EXT_TAG "infrared_test"
#define DBG_EXT_LVL DBG_EXT_DEBUG
#include <os_dbg_ext.h>

static void infrared_recv_task(void *parameter)
{
    os_device_t *infrared;
    struct os_infrared_info info;
    int infrared_rx_count = 0;

    infrared = os_device_find("atk_rmt");
    OS_ASSERT(infrared);

    os_device_open(infrared, OS_DEVICE_OFLAG_RDWR);

    while (1)
    {
        os_device_read(infrared, 0, &info, sizeof(info));
        LOG_EXT_I("infrared_rx_done(%d) addr: %02x, data: %02x", ++infrared_rx_count, info.addr, info.data);
    }
}

static int infrared_recv_test(void)
{
    os_task_t *task;

    task = os_task_create("ir_recv", infrared_recv_task, NULL, 2048, 3, 5);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}

static int infrared_send_test(int argc, char **argv)
{
    os_uint8_t addr = 0x5a;
    os_uint8_t data = 0x3c;
    int i, repeat, loops = 1;

    if (argc == 1 || (argc == 2 && !strcmp(argv[1], "help")))
    {
        LOG_EXT_I("usage:");
        LOG_EXT_I("infrared_send_test [addr] [data] [repeat] [loops]");
        return 0;
    }

    if (argc > 1)
    {
        addr = strtol(argv[1], NULL, 0);
    }

    if (argc > 2)
    {
        data = strtol(argv[2], NULL, 0);
    }

    if (argc > 3)
    {
        repeat = strtol(argv[3], NULL, 0);
    }

    if (argc > 4)
    {
        loops = strtol(argv[4], NULL, 0);
    }

    os_device_t *infrared;
    struct os_infrared_info info;

    infrared = os_device_find("atk_rmt");
    OS_ASSERT(infrared);

    os_device_open(infrared, OS_DEVICE_OFLAG_RDWR);

    os_task_t *self = os_task_self();
    os_uint8_t task_prio = self->current_priority;
    os_uint8_t high_prio = 1;
    os_task_control(self, OS_TASK_CTRL_CHANGE_PRIORITY, &high_prio);

    for (i = 0; i < loops; i++)
    {
        os_task_msleep(1000);

        info.addr  = addr;
        info.data  = data + i;
        info.times = repeat;
        os_device_write(infrared, 0, &info, sizeof(info));
    }

    os_device_close(infrared);

    os_task_control(self, OS_TASK_CTRL_CHANGE_PRIORITY, &task_prio);

    return 0;
}

SH_CMD_EXPORT(infrared_recv_test, infrared_recv_test, "infrared_recv_test");
SH_CMD_EXPORT(infrared_send_test, infrared_send_test, "infrared_send_test");
