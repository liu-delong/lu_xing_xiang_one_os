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

#include <board.h>

#ifdef OS_USING_ONENET_MQTTS
#include "cmcc_sensor_process.h"
#include "cmcc_lcd_process.h"
#include "cmcc_key_process.h"
#endif
#ifdef PKG_USING_MICROPYTHON
#include <shell.h>
#include <vfs_fs.h>
#ifdef MICROPY_USING_AMS
#include "ams.h"
#endif
#endif

static void user_task(void *parameter)
{
    int i = 0;

    for (i = 0; i < led_table_size; i++)
    {
        os_pin_mode(led_table[i].pin, PIN_MODE_OUTPUT);
    }

    while (1)
    {
        for (i = 0; i < led_table_size; i++)
        {
            os_pin_write(led_table[i].pin, led_table[i].active_level);
            os_task_msleep(200);

            os_pin_write(led_table[i].pin, !led_table[i].active_level);
            os_task_msleep(200);
        }
    }
}

#ifdef PKG_USING_MICROPYTHON
static int sdmmc_init(void)
{
	
	char *file_sys_device = "sd0"; //"W25Q64"; //;
    /* Mount the file system from tf card */
    if (vfs_mount(file_sys_device, "/", "fat", 0, 0) == 0)
    {
        os_kprintf("Filesystem initialized!\n");
		os_task_mdelay(500);
    }
    else
    {
        os_kprintf("Failed to initialize filesystem!\n");
		
		os_task_delay(1);
		if(vfs_mkfs("fat" ,file_sys_device) < 0)
		{
			os_kprintf("dfs_mkfs failed");
		}
		vfs_mount(file_sys_device, "/", "fat", 0, 0);
		return 1;
    }

    return 0;
}
SH_CMD_EXPORT(sdmmc_test, sdmmc_init, "sdmmc_test");
#endif

int main(void)
{
    os_task_t *task;

    task = os_task_create("user", user_task, NULL, 512, 3, 5);
    OS_ASSERT(task);
    os_task_startup(task);

#ifdef OS_USING_ONENET_MQTTS
    cmcc_lcd_show_startup_page(); 
    os_task_mdelay(500);    
    cmcc_sensor_init();
    cmcc_key_init();
    os_task_mdelay(500);
    cmcc_lcd_start();
#endif

#ifdef PKG_USING_MICROPYTHON
	sdmmc_init();
	#ifdef MICROPY_USING_AMS
	setup_ams_thread();
	#endif
#endif
    return 0;
}
