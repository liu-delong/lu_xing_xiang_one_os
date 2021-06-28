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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <board.h> //kai fa ban di ceng xiang guan
#include <oneos_config.h>// xi tong hong
#include <os_clock.h>// shi zhong jie pai
#include <os_kernel.h> //cao zuo xi tong nei he
#include <os_dbg_ext.h>//tiao shi xin xi
#include <location.h>
#include <shell.h>

#include <sensors/sensor.h> //chuan gan qi
#include <st7789vw.h>//lcd xian shi
#include "onenet_mqtts.h"
#include "liu_de_long.h"
#include "liu_mqtts_func.h"
/*
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
            os_task_msleep(500);

            os_pin_write(led_table[i].pin, !led_table[i].active_level);
            os_task_msleep(500);
        }
    }
}
*/
void signal_task(void *para)
{
		int i = 0;

		for (i = 0; i < led_table_size; i++)
		{
				os_pin_mode(led_table[i].pin, PIN_MODE_OUTPUT);
		}
		int wifi;
		int onenet;
		os_pin_write(led_table[0].pin,led_table[0].active_level);
		int last_i=0;
		while (1)
		{
				wifi=wifi_is_connet();
				onenet=onenet_mqtts_device_is_connected();
				if((!wifi)&&(!onenet))
				{
						if(last_i!=0) 
						{
								os_pin_write(led_table[last_i].pin, !led_table[last_i].active_level);
								os_pin_write(led_table[0].pin,led_table[0].active_level);
								last_i=0;
						}
				}
				else if((wifi)&&(!onenet))
				{
						if(last_i!=2)
						{
								os_pin_write(led_table[last_i].pin, !led_table[last_i].active_level);
								os_pin_write(led_table[2].pin,led_table[2].active_level);
								last_i=2;
						}
				}
				else
				{
						if(last_i!=1)
						{
								os_pin_write(led_table[last_i].pin, !led_table[last_i].active_level);
								os_pin_write(led_table[1].pin,led_table[1].active_level);
								last_i=1;
						}
				}
				os_task_msleep(2000);
		}
}

void test()
{
		int id=0;
		char* key[3]={"00000000000000000000000000000000000000000",
									"00000000000000000000000000000000000000000",
									"00000000000000000000000000000000000000000"};
		int value[3];
		int num;
		while (1)
		{
			num=getdata(0,key,value);
			/*
			lcd_show_string(0, 0, 16, "%s:",key[0]);
			lcd_show_string(0, 16, 16, "%d",value[0]);
			lcd_show_string(0, 32, 16, "%s:",key[1]);
			lcd_show_string(0, 48, 16, "%d",value[1]);
			*/
			mysend(key,value,num,id);
			num=getdata(1,key,value);
			/*
			lcd_show_string(0, 64, 16, "%s:",key[0]);
			lcd_show_string(0, 80, 16, "%d",value[0]);
			lcd_show_string(0, 96, 16, "%s:",key[1]);
			lcd_show_string(0, 112, 16, "%d",value[1]);
			*/
			mysend(key,value,num,id);
			num=getdata(2,key,value);
			/*
			lcd_show_string(0, 128, 16, "%s:",key[0]);
			lcd_show_string(0, 144, 16, "%d",value[0]);
			lcd_show_string(0, 160, 16, "%s:",key[1]);
			lcd_show_string(0, 178, 16, "%d",value[1]);
			*/
			mysend(key,value,num,id);	
			id++;
		}
}

int main(void)
{
    onenet_mqtts_device_start();
		os_task_t *task1;// status_led
		os_task_t *task2;// data_up_to_cloud
		
		task1 = os_task_create("status_led", signal_task, NULL, 512, 6, 5);
    OS_ASSERT(task1);
    os_task_startup(task1);
		
		task2=os_task_create("data_up",test,NULL,8096,7,10);
		OS_ASSERT(task2);
		os_task_startup(task2);
	
    return 0;
}