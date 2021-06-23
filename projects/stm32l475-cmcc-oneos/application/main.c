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
#include "onenet_mqtts.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <os_kernel.h>
#include <oneos_config.h>
#include "onenet_mqtts.h"

#include <drv_cfg.h>
#include <os_clock.h>
#include <shell.h>
#include <sensors/sensor.h>
#include <os_dbg_ext.h>
#include <st7789vw.h>
static int pack(char* result,char * key[],int key_num,int r_size)
{
	int i;
	memset(result,0,r_size);
	strcat(result,"{"
         "\"id\": %d,"
         "\"dp\": {");
	for (i=0;i<key_num;i++)
	{
			strcat(result,"\"");
			strcat(result,key[i]);
			strcat(result,"\": [{\"v\": %d}]");
			if(i!=key_num-1)
			{
				strcat(result,",");
			}
	}
	strcat(result,"}}");
	return strlen(result);		
}
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
extern char *base_dp_upload_str;
extern struct os_mq mqtts_mq;

static void  mysend(char *key[],int value[],int num,int id)
{
    os_err_t rc;
    char     pub_buf[128]      = {0};
    char    *pub_msg           = NULL;
    int      pub_msg_len       = 0;
		char gs[200];
    mq_msg_t mq_msg;
		pack(gs,key,num,200);
		if(num==2) snprintf(pub_buf, sizeof(pub_buf), gs, id,value[0],value[1]);
		if(num==1) snprintf(pub_buf, sizeof(pub_buf), gs, id,value[0]);
		if(num==3) snprintf(pub_buf, sizeof(pub_buf), gs, id,value[0],value[1],value[2]);
    pub_msg     = pub_buf;
		os_kprintf("%s\r\n",pub_msg);
    pub_msg_len = strlen(pub_msg);

    memset(&mq_msg, 0x00, sizeof(mq_msg));
    mq_msg.topic_type = DATA_POINT_TOPIC;
    memcpy(mq_msg.data_buf, pub_msg, pub_msg_len);
    mq_msg.data_len = pub_msg_len;

    rc = os_mq_send(&mqtts_mq, (void *)&mq_msg, sizeof(mq_msg_t), 0);
    if (rc != OS_EOK)
    {
        LOG_EXT_E("mqtts_device_messagequeue_send ERR");
    }
    os_task_mdelay(1 * 1000);
}
int getdata(int kind,char* key[],int data[])
/*
0. temparature
1. shi_du
2. guan_gan
4. 
*/
{
		char sensor_name[24];
		struct os_sensor_data sensor_data;
		switch(kind)
		{
			case 0:
				snprintf(sensor_name, sizeof(sensor_name) - 1, "temp_%s", "aht10");
				break;
			case 1:
				snprintf(sensor_name, sizeof(sensor_name) - 1, "humi_%s", "aht10");
				break;
			case 2:
				snprintf(sensor_name, sizeof(sensor_name) - 1, "li_%s", "ap3216c");
				break;
		}	
		os_device_t *sensor = os_device_find(sensor_name);
		OS_ASSERT(sensor != NULL);
		os_device_open(sensor, OS_DEVICE_FLAG_RDWR);

		struct os_sensor_info sensor_info;
		os_device_control(sensor, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
		os_device_read(sensor, 0, &sensor_data, sizeof(struct os_sensor_data));
		os_task_mdelay(500);
		if(kind==0)
		{
				memset(key[0],0,41);
				memset(key[1],0,41);
				key[0]="tempatature_zheng";
				key[1]="tempatature_xiao";
				if (sensor_info.unit == OS_SENSOR_UNIT_MDCELSIUS)
        {
						os_kprintf("sensor temp (%d.%03d)\r\n", sensor_data.data.light / 1000, sensor_data.data.light % 1000);	
						data[0]=sensor_data.data.temp / 1000;
						data[1]=sensor_data.data.temp % 1000;
        }
        else if (sensor_info.unit == OS_SENSOR_UNIT_DCELSIUS)
        {
            data[0]=sensor_data.data.temp;
						data[1]=0;
        }
				return 2;
		}
		if(kind==1)
		{
				memset(key[0],0,41);
				memset(key[1],0,41);
				key[0]="shi_du_zheng";
				key[1]="shi_di_xiao";
				if (sensor_info.unit == OS_SENSOR_UNIT_MPERMILLAGE)
        {
						os_kprintf("sensor humi (%d.%03d)\r\n", sensor_data.data.light / 1000, sensor_data.data.light % 1000);	
						data[0]=sensor_data.data.temp / 1000;
						data[1]=sensor_data.data.temp % 1000;
        }
        else if (sensor_info.unit == OS_SENSOR_UNIT_PERMILLAGE)
        {
            data[0]=sensor_data.data.temp;
						data[1]=0;
        }
				return 2;
		}
		if(kind==2)
		{
				memset(key[0],0,41);
				memset(key[1],0,41);
				key[0]="guang_zheng";
				key[1]="guang_xiao";
				if (sensor_info.unit == OS_SENSOR_UNIT_MLUX)
        {
						os_kprintf("sensor light (%d.%03d)\r\n", sensor_data.data.light / 1000, sensor_data.data.light % 1000);	
						data[0]=sensor_data.data.temp / 1000;
						data[1]=sensor_data.data.temp % 1000;
        }
        else if (sensor_info.unit == OS_SENSOR_UNIT_LUX)
        {
            os_kprintf("sensor light (%d.%03d)\r\n", sensor_data.data.light / 1000, sensor_data.data.light % 1000);
						data[0]=sensor_data.data.temp;
						data[1]=0;
        }
				return 2;
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
			lcd_show_string(0, 0, 16, "%s:",key[0]);
			lcd_show_string(0, 16, 16, "%d:",value[0]);
			lcd_show_string(0, 32, 16, "%s:",key[1]);
			lcd_show_string(0, 48, 16, "%d:",value[1]);
			mysend(key,value,num,id);
			num=getdata(1,key,value);
			lcd_show_string(0, 64, 16, "%s:",key[0]);
			lcd_show_string(0, 80, 16, "%d:",value[0]);
			lcd_show_string(0, 96, 16, "%s:",key[1]);
			lcd_show_string(0, 112, 16, "%d:",value[1]);
			mysend(key,value,num,id);
			num=getdata(2,key,value);
			lcd_show_string(0, 128, 16, "%s:",key[0]);
			lcd_show_string(0, 144, 16, "%d:",value[0]);
			lcd_show_string(0, 160, 16, "%s:",key[1]);
			lcd_show_string(0, 178, 16, "%d:",value[1]);
			mysend(key,value,num,id);
			
			id++;
		}
}
static void         generate_onenet_publish_data_cycle_thread_func(void *arg)
{
    os_err_t rc;
    char     pub_buf[128]      = {0};
    char    *pub_msg           = NULL;
    int      pub_msg_len       = 0;
    mq_msg_t mq_msg;
    int      id                = 0;
    int      temperature_value = 0;
    int      power_value       = 0;

    while (1)
    {
        if (id != 2147483647)
        {
            id++;
        }
        else
        {
            id = 1;
        }
				char sensor_name[24];
				struct os_sensor_data sensor_data;

				

				snprintf(sensor_name, sizeof(sensor_name) - 1, "temp_%s", "aht10");

				os_device_t *sensor = os_device_find(sensor_name);
				OS_ASSERT(sensor != NULL);
				os_device_open(sensor, OS_DEVICE_FLAG_RDWR);

				struct os_sensor_info sensor_info;
				os_device_control(sensor, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
				os_device_read(sensor, 0, &sensor_data, sizeof(struct os_sensor_data));
				os_task_mdelay(1000);
        temperature_value = sensor_data.data.temp/1000;
        power_value       = sensor_data.data.temp%1000;
        snprintf(pub_buf, sizeof(pub_buf), base_dp_upload_str, id, temperature_value, power_value);

        pub_msg     = pub_buf;
        pub_msg_len = strlen(pub_msg);

        memset(&mq_msg, 0x00, sizeof(mq_msg));
        mq_msg.topic_type = DATA_POINT_TOPIC;
        memcpy(mq_msg.data_buf, pub_msg, pub_msg_len);
        mq_msg.data_len = pub_msg_len;

        rc = os_mq_send(&mqtts_mq, (void *)&mq_msg, sizeof(mq_msg_t), 0);
        if (rc != OS_EOK)
        {
            LOG_EXT_E("mqtts_device_messagequeue_send ERR");
        }

        os_task_mdelay(1 * 1000);
    }
}
int main(void)
{
    os_task_t *task;
		os_task_t *task2;
    task = os_task_create("user", user_task, NULL, 512, 3, 5);
    OS_ASSERT(task);
    os_task_startup(task);
		onenet_mqtts_device_start();
		os_task_msleep(2000);
		task2=os_task_create("up",test,NULL,8192,5,10);
		OS_ASSERT(task2);
		os_task_startup(task2);
		

    return 0;
}
