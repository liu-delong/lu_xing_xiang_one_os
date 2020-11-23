/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * \@file        MQTTOneOS.c
 *
 * \@brief       socket port file for mqtt
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>
#include <os_assert.h>
#include <oneos_config.h>
#include "MQTTOneOS.h"
#include "certificate.h"
#include "MQTTClient.h"

#define DBG_EXT_TAG "mqtt_sample"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef MQTT_USING_TLS
#define TLS_SERVER_ADDR  "121.89.166.244"          /* private ali cloud mqtts server */
#define TLS_SERVER_PORT  8883
#else
#define TCP_SERVER_ADDR  "mq.tongxinmao.com"       /* 120.76.100.197 */
#define TCP_SERVER_PORT  18831
#endif

#define COMMAND_TIMEOUT 30000

typedef struct
{
    Network network;
    MQTTClient client;
} mqtt_context_t;

os_task_t *mqtt_sample_task = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           This function prints arrived message.
 *
 * @param[in]       data            message info.
 ***********************************************************************************************************************
 */
void messageArrived(MessageData *data)
{
    os_kprintf("Message arrived on topic %.*s: %.*s\n", 
               data->topicName->lenstring.len, 
               data->topicName->lenstring.data,
               data->message->payloadlen, 
               data->message->payload);
}

static void mqtt_sample_task_func(void *arg)
{
    /* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */
    mqtt_context_t  mqtt_context = {0};
    unsigned char   sendbuf[512] = {0};
    unsigned char   readbuf[512] = {0};
    int             rc = 0;
    int             count = 0;

#ifdef MQTT_USING_TLS
    char addr[] = TLS_SERVER_ADDR;
    int port = TLS_SERVER_PORT;
#else
    char addr[] = TCP_SERVER_ADDR;
    int port = TCP_SERVER_PORT;
#endif
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    MQTTNetworkInit(&mqtt_context.network, addr, port, g_certificate);
    MQTTClientInit(&mqtt_context.client, 
                   &mqtt_context.network, 
                   COMMAND_TIMEOUT, 
                   sendbuf, 
                   sizeof(sendbuf), 
                   readbuf, 
                   sizeof(readbuf));

    rc = mqtt_context.network.connect(&mqtt_context.network);
    if (0 != rc)
    {
        LOG_EXT_E("establish network failed, check IP and PORT");
        return;
    }
    LOG_EXT_I("establish network sucess");

#if defined(MQTT_TASK)
    if ((rc = MQTTStartTask(&mqtt_context.client)) != OS_TRUE)
        LOG_EXT_E("Return code from start tasks is %d", rc);
#endif

    connectData.MQTTVersion = 4; /*3 = 3.1 4 = 3.1.1*/
    connectData.keepAliveInterval = 60;
    connectData.cleansession = 1;
    connectData.willFlag = 0;
    connectData.clientID.cstring = "OneOS_MQTTClient";
    connectData.username.cstring = "username";
    connectData.password.cstring = "password";

    if ((rc = MQTTConnect(&mqtt_context.client, &connectData)) != 0)
        LOG_EXT_E("Return code from MQTT connect is %d", rc);
    else
        LOG_EXT_I("MQTT Connected");

    if ((rc = MQTTSubscribe(&mqtt_context.client, "/public/TEST/mqtest", QOS1, messageArrived)) != 0)
        LOG_EXT_E("Return code from MQTT subscribe is %d", rc);

    while ((count<100) && (MQTTConnect(&mqtt_context.client, &connectData)!=0))
    {
        ++count;
        MQTTMessage message;
        char payload[128];

        message.qos = QOS1;
        message.retained = 0;
        message.payload = payload;
        sprintf(payload, "message number %d", count);
        message.payloadlen = strlen(payload);

		if ((rc = MQTTPublish(&mqtt_context.client, "/public/TEST/mqtest", &message)) != 0)
		{
			LOG_EXT_I("Return code from MQTT publish is %d", rc);
		}
#if !defined(MQTT_TASK)
		if ((rc = MQTTYield(&mqtt_context.client, 1000)) != 0)
		{
			LOG_EXT_E("Return code from yield is %d", rc);
		}
#endif
    }
		
    LOG_EXT_I("MQTT stop return code by count %d",count);
    /* do not return */
}

#ifdef MQTT_USING_TLS
#define MQTT_TASK_STACK_SIZE 8192 /* MQTT using tls, MQTT thread stack size need larger than 6K */
#else
#define MQTT_TASK_STACK_SIZE 4096
#endif

/**
 ***********************************************************************************************************************
 * @brief           This function start mqtt sample.
 ***********************************************************************************************************************
 */
void mqtts_sample_start(void)
{
    mqtt_sample_task = os_task_create("mqtt_sample",
                                      mqtt_sample_task_func,
                                      OS_NULL,
                                      MQTT_TASK_STACK_SIZE,
                                      OS_TASK_PRIORITY_MAX / 2, 10);

    if (OS_NULL == mqtt_sample_task)
    {
        LOG_EXT_E("mqtt echo thread create failed");
        OS_ASSERT(OS_NULL != mqtt_sample_task);
    }

    os_task_startup(mqtt_sample_task);
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(mqtt_sample, mqtts_sample_start, "start mqtt sample");
#endif
