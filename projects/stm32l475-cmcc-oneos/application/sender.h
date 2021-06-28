#ifndef MYSENDER_H
#define MYSENDER_H
#include "liu_mqtts_func.h"
#include "get_sennor_data.h"
const char *send_pack =           "{"
                                  "\"id\": %d,"
                                  "\"dp\": {"
                                  "\"wen_du_zheng\": [{"
                                  "\"v\": %d,"
                                  "}],"
                                  "\"wen_du_xiao\": [{"
                                  "\"v\": %d"
                                  "}]"
																	/*"\"shi_du_zheng\": [{"
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
																	"}],"
																	"\"acce_status\": [{"
																	"\"v\": %d"
																	"}]"*/
                                  "}"
                                  "}";
extern struct os_mq mqtts_mq;													
void send_func(void* para)
{
		int id=0;
		mq_msg_t mq_msg;
		mq_msg.topic_type=DATA_POINT_TOPIC;
		int rc;
		while(1)
		{
				if(en_wen_du||en_shi_du||en_guang_qiang||en_six)
				{
						memset(mq_msg.data_buf, 0x00, sizeof(mq_msg));
						snprintf(mq_msg.data_buf,400,send_pack,id++,wen_du_zheng,wen_du_xiao,shi_du_zheng,shi_du_xiao,guang_qiang_zheng,
											guang_qiang_xiao,six_x,six_y,six_z,six_status);
						
						mq_msg.data_len=strlen(mq_msg.data_buf);
						rc = os_mq_send(&mqtts_mq, (void *)&mq_msg, sizeof(mq_msg_t), 0);
						if (rc != OS_EOK)
						{
								LOG_EXT_E("mqtts_device_messagequeue_send ERR");
						}
				}
				os_task_msleep(1000);
		}
}
#endif
