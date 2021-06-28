#ifndef MYGET_SENNOR_DATA_H
#define MYGET_SENNOR_DATA_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <os_kernel.h>
#include <os_dbg_ext.h>

#include <sensors/sensor.h>
#include "onenet_mqtts.h"

extern struct os_mq mqtts_mq;	
const char *send_pack =           "{"
                                  "\"id\": %d,"
                                  "\"dp\": {"
                                  "\"wen_du_zheng\": [{"
                                  "\"v\": %d,"
                                  "}],"
                                  "\"wen_du_xiao\": [{"
                                  "\"v\": %d"
                                  "}],"
																	"\"shi_du_zheng\": [{"
                                  "\"v\": %d"
                                  "}],"
																	"\"wen_du_xiao\": [{"
																	"\"v\": %d"
																	"}],"
																	"\"guang_qiang_zheng\": [{"
																	"\"v\": %d"
																	"}],"
																	"\"guang_qiang_xiao\": [{"
																	"\"v\": %d"
																	"}],"
																	"\"acce_x\": [{"
																	"\"v\": %d"
																	"}],"
																	"\"acce_y\": [{"
																	"\"v\": %d"
																	"}],"
																	"\"acce_z\": [{"
																	"\"v\": %d"
																	"}]"
                                  "}"
                                  "}";
int wen_du_zheng=-1;
int wen_du_xiao=-1;
int shi_du_zheng=-1;
int shi_du_xiao=-1;
int guang_qiang_zheng=-1;
int guang_qiang_xiao=-1;
int six_x=-1;
int six_y=-1;
int six_z=-1;

int en_wen_du=1;
int en_shi_du=1;
int en_guang_qiang=1;
int en_six=1;
int up=1;
void data_get_and_up(void * para)
{
		struct os_sensor_data sensor_data;
		struct os_sensor_info sensor_info;
	
		char sensor_name1[]="temp_aht10";
		os_device_t *sensor1 = os_device_find(sensor_name1);
		OS_ASSERT(sensor1 != NULL);
	
		char sensor_name2[]="humi_aht10";
		os_device_t *sensor2 = os_device_find(sensor_name2);
		OS_ASSERT(sensor2 != NULL);
	
		char sensor_name3[]="li_ap3216c";
		os_device_t *sensor3 = os_device_find(sensor_name3);
		OS_ASSERT(sensor3 != NULL);
	
		char sensor_name4[]="acce_icm20602";
		os_device_t *sensor4 = os_device_find(sensor_name4);
		OS_ASSERT(sensor4 != NULL);
	
		int id=0;
		mq_msg_t mq_msg;
		mq_msg.topic_type=DATA_POINT_TOPIC;
		int rc;
		while(1)
		{
				if(en_wen_du)
				{
						os_device_open(sensor1, OS_DEVICE_FLAG_RDWR);
						os_device_control(sensor1, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
						os_device_read(sensor1, 0, &sensor_data, sizeof(struct os_sensor_data));
					
						wen_du_zheng=sensor_data.data.temp / 1000;
						wen_du_xiao=sensor_data.data.temp % 1000;
					
						os_device_close(sensor1);
						os_task_sleep(25);
				}
				if(en_shi_du)
				{
						os_device_open(sensor2, OS_DEVICE_FLAG_RDWR);
						os_device_control(sensor2, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
						os_device_read(sensor2, 0, &sensor_data, sizeof(struct os_sensor_data));
					
						shi_du_zheng=sensor_data.data.temp / 1000;
						shi_du_xiao=sensor_data.data.temp % 1000;
					
						os_device_close(sensor2);
						os_task_sleep(25);
				}
				if(en_guang_qiang)
				{
						os_device_open(sensor3, OS_DEVICE_FLAG_RDWR);
						os_device_control(sensor3, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
						os_device_read(sensor3, 0, &sensor_data, sizeof(struct os_sensor_data));
					
						guang_qiang_zheng=sensor_data.data.temp / 1000;
						guang_qiang_xiao=sensor_data.data.temp % 1000;
					
						os_device_close(sensor3);
						os_task_sleep(25);
				}
				if(en_six)
				{
						os_device_open(sensor4, OS_DEVICE_FLAG_RDWR);
						os_device_read(sensor4, 0, &sensor_data, sizeof(struct os_sensor_data));
						
						six_x=sensor_data.data.acce.x;
						six_y=sensor_data.data.acce.y;
						six_z=sensor_data.data.acce.z;
						
						os_device_close(sensor4);
						os_task_sleep(25);
				}
				if(up&&(en_wen_du||en_shi_du||en_guang_qiang||en_six))
				{
						memset(mq_msg.data_buf, 0x00, sizeof(mq_msg.data_buf));
						snprintf(mq_msg.data_buf,384,send_pack,id++,wen_du_zheng,wen_du_xiao,shi_du_zheng,shi_du_xiao,guang_qiang_zheng,
											guang_qiang_xiao,six_x,six_y,six_z);
						
						mq_msg.data_len=strlen(mq_msg.data_buf);
						rc = os_mq_send(&mqtts_mq, (void *)&mq_msg, sizeof(mq_msg_t), 0);
						if (rc != OS_EOK)
						{
								LOG_EXT_E("mqtts_device_messagequeue_send ERR");
						}
						os_task_msleep(500);
				}
				if(id>2147483647) id=id%2147483648;
		}
}
#endif
