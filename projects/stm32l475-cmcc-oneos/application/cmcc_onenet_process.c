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
 * @file        cmcc_onenet_process.c
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
#include "cmcc_onenet_process.h"
#include "cmcc_sensor_process.h"

#define DBG_EXT_TAG "cmcc_onenet_process"
#define DBG_EXT_LVL DBG_EXT_INFO

#include <os_dbg_ext.h>

const char *base_dp_upload_str1 = "{"
                                 "\"id\": %d,"
                                 "\"dp\": {"
                                 "\"temperature\": [{"
                                 "\"v\": %f,"
                                 "}],"
                                 "\"humidity\": [{"
                                 "\"v\": %f"
                                 "}]"
                                 "}"
                                 "}";

extern struct os_mq mqtts_mq;

static void  onenet_upload_data_task_func(void *arg)
{
    os_err_t rc;
    cmcc_sensor_data_t sensor_data;

    char     pub_buf[128]      = {0};
    char    *pub_msg           = NULL;
    int      pub_msg_len       = 0;
    mq_msg_t mq_msg;
    int      id                = 0;
    float      temperature_value = 0;
    float      humidity_value       = 0;

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

        cmcc_sensor_data_upload(cmcc_sensor_data_result_get(), &sensor_data);
        temperature_value = sensor_data.aht10_data_temp/1000;
        humidity_value = sensor_data.aht10_data_humi/1000;
        
        snprintf(pub_buf, sizeof(pub_buf), base_dp_upload_str1, id, temperature_value, humidity_value);

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

        os_task_mdelay(5*1000);
    }
}

#define ONENET_UPLOAD_DATA_TASK_STACK_SIZE 1024
os_task_t  *onenet_upload_data_task = NULL;
static void onenet_upload_data_cycle(void)
{
    onenet_upload_data_task = os_task_create("generate_pubdata",
                                                               onenet_upload_data_task_func,
                                                               OS_NULL,
                                                               ONENET_UPLOAD_DATA_TASK_STACK_SIZE,
                                                               OS_TASK_PRIORITY_MAX / 2,
                                                               10);

    if (NULL == onenet_upload_data_task)
    {
        LOG_EXT_E("onenet mqtts client create task failed");
        OS_ASSERT(OS_NULL != onenet_upload_data_task);
    }
    os_task_startup(onenet_upload_data_task);
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(onenet_upload_data_cycle,
              onenet_upload_data_cycle,
              "publish message cycle to onenet specified topic");
#endif
