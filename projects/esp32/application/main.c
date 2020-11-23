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

#include <stdio.h>
#include "os_kernel.h"
#include "os_task.h"

static void user_task(void *parameter)
{
    while (1)
    {
        //os_kprintf("user_task\n");
        os_task_delay(1000);
    }
}

int main(void)
{
    os_task_t *task;

    os_kprintf("main entry\n");
    //task = os_task_create("user", user_task, NULL, 512, 3, 5);
    //OS_ASSERT(task);
    //os_task_startup(task);

    while(1)
    {
        os_task_delay(1000);
    }

    return 0;
}


