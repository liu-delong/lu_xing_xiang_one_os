
#ifndef A_T
#define A_T
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
#include <temperature_analysis.h>
#include <math.h>

int get_temperature()
{
	int temperature[2];
	char sensor_name[24];
	struct os_sensor_data sensor_data;
	snprintf(sensor_name, sizeof(sensor_name) - 1, "temp_%s", "aht10");
	os_device_t *sensor = os_device_find(sensor_name);
	OS_ASSERT(sensor != NULL);
	os_device_open(sensor, OS_DEVICE_FLAG_RDWR);
	struct os_sensor_info sensor_info;
	os_device_control(sensor, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
	os_device_read(sensor, 0, &sensor_data, sizeof(struct os_sensor_data));
	os_task_mdelay(500);
	return sensor_data.data.temp;
} 
static int average_temperature(int data[])
{
	int i=0;
	int sum=0;
	while(i<10)
	{
		sum=sum+data[i];
		i=i+1;
	}
	sum=sum/10;
	return sum;
}
int temperature_analysis()
{
	/*
	judge1
	1:????????
	2:????????
	judge2
	1:????????
	2:????????
	*/
	/*返回值
	0： 温度没有较大的幅度变化 
	1：30s内升高
	2：30s内下降
	3：5s内升高
	4：5s内下降
	5s的优先级高于30s的。
	*/ 
	int judge1=0,judge2=0;
	int data[10];
	int i=0,k=0;
	while(i<10)
	{
		data[i]=get_temperature();
		i=i+1;
	} 
	int average1=average_temperature(data);
	int average2=average1;
	int average;
	i=0;
	while(1)
	{
		data[i]=get_temperature();
		i=i+1;
		k=k+1;
		if(i==10)
		{
			i=0;
			average=average_temperature(data);
			if(average-average1>2000)	judge1=1;
			else if(average1-average1>2000)	judge1=2;
			average1=average;
		}
		if(k==60)
		{
			k=0;
			average=average_temperature(data);
			if(average-average2>2000)	judge2=1;
			if(average2-average>2000)	judge2=2;
			average2=average;
		} 
		if(judge1==0&&judge2==0) lcd_show_string(0, 0, 16, "%s","normal temperature");
		else if(judge2==1)	  	lcd_show_string(0, 0, 16, "%s","Large rise--30s");
		else if(judge2==2)   lcd_show_string(0, 0, 16, "%s","Large decline--30s");
		else if(judge1==1)	 lcd_show_string(0, 0, 16, "%s","Large rise--5s");
		else if(judge1==2)	 lcd_show_string(0, 0 , 16, "%s","Large decline--5s");
		judge1=0;
		judge2=0; 
	}
	if(judge1==0&&judge2==0)  return 0;
	if(judge2==1)   return 1;
	if(judge2==2)   return 2;
	if(judge1==1)   return 3;
	if(judge1==2)   return 4;
}
#endif
