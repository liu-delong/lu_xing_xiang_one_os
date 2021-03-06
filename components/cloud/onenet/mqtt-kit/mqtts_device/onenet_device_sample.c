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
 * @file        onenet_device_sample.c
 * 
 * @brief       Demonstrate a sample which publish messages into message queue periodic.  
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */

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

#define DBG_EXT_TAG "onenet_device.sample"
#define DBG_EXT_LVL DBG_EXT_INFO

#include <os_dbg_ext.h>

const char *base_dp_upload_str = "{"
                                 "\"id\": %d,"
                                 "\"dp\": {"
                                 "\"temperaturez\": [{"
                                 "\"v\": %d,"
                                 "}],"
                                 "\"temperatures\": [{"
                                 "\"v\": %d"
                                 "}]"
                                 "}"
                                 "}";

extern struct os_mq mqtts_mq;

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

        os_task_mdelay(10 * 1000);
    }
}

#define GENERATE_ONENET_PUBLISH_DATA_CYCLE_THREAD_STACK_SIZE 1024
os_task_t  *generate_onenet_publish_data_cycle_thread = NULL;
static void generate_onenet_publish_data_cycle(void)
{
    generate_onenet_publish_data_cycle_thread = os_task_create("generate_pubdata",
                                                               generate_onenet_publish_data_cycle_thread_func,
                                                               OS_NULL,
                                                               GENERATE_ONENET_PUBLISH_DATA_CYCLE_THREAD_STACK_SIZE,
                                                               OS_TASK_PRIORITY_MAX / 2,
                                                               10);

    if (NULL == generate_onenet_publish_data_cycle_thread)
    {
        LOG_EXT_E("onenet mqtts client create thread failed");
        OS_ASSERT(OS_NULL != generate_onenet_publish_data_cycle_thread);
    }
    os_task_startup(generate_onenet_publish_data_cycle_thread);

}
static void stop_onenet_publish_data_cycle(void)
{
    os_task_destroy(generate_onenet_publish_data_cycle_thread);
    LOG_EXT_I("onenet publish_data_cycle thread stop");
    return;
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(generate_onenet_publish_data_cycle,
              generate_onenet_publish_data_cycle,
              "publish message cycle to onenet specified topic");
SH_CMD_EXPORT(stop_onenet_publish_data_cycle,
              stop_onenet_publish_data_cycle,
              "stop publishing message cycle to onenet specified topic");
#endif
