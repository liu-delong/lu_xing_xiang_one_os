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
 * @file        main.c
 *
 * @brief       User application entry
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include "os_task.h"
#include "os_device.h"
#include "serial.h"

static void test_task(void *parameter)
{
    uint8_t      buff[64];
    os_device_t *uart_dev = os_device_find("uart0");
    os_device_open(uart_dev, OS_DEVICE_FLAG_INT_RX | OS_DEVICE_FLAG_RDWR);
    while (1)
    {
        snprintf((char *)buff, sizeof(buff) - 1, "test task: uart0\r\n");
        os_device_write(uart_dev, 0, buff, strlen((const char *)buff));
        os_task_mdelay(2000);
    }
}

int main(void)
{
    int        i = 0;
    os_task_t *task;

    task = os_task_create("test", test_task, OS_NULL, 1024, 3, 5);
    os_task_startup(task);

    while (1)
    {
        os_task_mdelay(5000);
    }
}
