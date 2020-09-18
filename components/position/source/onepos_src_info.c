/**
 ***********************************************************************************************************************
 * Copyright (c)2020, China Mobile Communications Group Co.,Ltd.
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
 * @file        one_pos_src_info.c
 *
 * @brief       collect information of position to offer called by upper
 *
 * @details     
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-07   OneOs Team      First Version
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>
#include <os_sem.h>
#include <os_assert.h>
#include "cJSON.h"
#include "MQTTOneOS.h"
#include "MQTTClient.h"
#include "onepos_interface.h"

#define DBG_EXT_TAG "onepos.mqtt"
#define DBG_EXT_LVL DBG_EXT_DEBUG
#include <os_dbg_ext.h>

#if defined(ONEPOS_DEVICE_REC_POS_INFO) || defined(ONEPOS_SUPP_REMOTE_CONF)
/**
 ***********************************************************************************************************************
 * @struct      onepos_rec_pmsg_word_t
 *
 * @brief       to receive published massage from platform
 ***********************************************************************************************************************
 */
typedef struct{
    os_task_t   *rec_msg_task;
    os_sem_t    *rec_msg_lock;
    Timer        rec_msg_timer;
}onepos_rec_pmsg_work_t;
#endif

/**
 ***********************************************************************************************************************
 * @struct      onpos_mqtt_context_t
 *
 * @brief       mqtt context struct of onepos
 ***********************************************************************************************************************
 */
typedef struct
{
    Network network;
    MQTTClient client;
    
    #if defined(ONEPOS_DEVICE_REC_POS_INFO)
    os_sem_t *rec_sub_msg_sem;
    #endif

    #if defined(ONEPOS_DEVICE_REC_POS_INFO) || defined(ONEPOS_SUPP_REMOTE_CONF)
    onepos_rec_pmsg_work_t rec_pmsg_work;
    #endif
}onepos_mqtt_context_t;

static char                      pub_info_topic_str[50];
static char                      pub_conf_topic_str[50];
static char                      sub_info_topic_str[50];
static char                      sub_conf_topic_str[50];
static char                      mqtt_client_str[CLIENT_ID_LEN + 1];
static onepos_mqtt_context_t     onepos_mqtt_context = {0};
static unsigned char             onepos_sendbuf[ONEPOS_COMM_SEND_BUFF_LEN] = {0,};
static unsigned char             onepos_recebuf[ONEPOS_COMM_REC_BUFF_LEN]  = {0,};
static MQTTPacket_connectData    onepos_connectData = MQTTPacket_connectData_initializer;

#if defined(ONEPOS_DEVICE_REC_POS_INFO)
#if defined(ONEPOS_WIFI_POS)
static ops_sigle_wifi_info_t    *g_onepos_wifi_info         = OS_NULL;
static ops_platform_wifi_info_t *g_platform_wifi_src_info = OS_NULL;
#endif

#if defined(ONEPOS_CELL_POS)
static ops_platform_lbs_info_t  *g_platform_lbs_src_info  = OS_NULL;
#endif

static onepos_pos_mode_t         g_curr_use_mode          = ONEPOS_INVAILD_POS_MODE;
static os_bool_t                 g_parse_rec_err_flag     = OS_FALSE;
#endif

static void clean_loca_pub_messsge(char *msg_str);
static os_err_t onepos_mqtt_message_publish(const char *topic_name, char *pub_message);

#if defined(ONEPOS_DEVICE_REC_POS_INFO)
/**
 ***********************************************************************************************************************
 * @brief           parse receive message
 *
 * @param[in]       data       received message
 ***********************************************************************************************************************
 */
static void onepos_parse_rec_data(const char* data)
{
    cJSON *root = OS_NULL;
    cJSON *item = OS_NULL;
    os_bool_t result = OS_FALSE;
    root = cJSON_Parse(data);
    if(!root)
    {
        LOG_EXT_E("error position data string: %s", data);
        g_parse_rec_err_flag = OS_TRUE;
        return ;
    }
    
    item = cJSON_GetObjectItem(root, "err_code");
    if(ONEPOS_COMM_SUCC == item->valueint)
    {
        item = cJSON_GetObjectItem(root, "pos_type");

        if(item->valueint == g_curr_use_mode)
        {
            switch (item->valueint)
            {
                #if defined(ONEPOS_WIFI_POS)
                case ONEPOS_QUERY_SINGLE_WIFI_POS:
                    if(OS_EOK == onepos_parse_sig_wifi_pos(g_onepos_wifi_info, OPS_SIGNLA_SOURCE_IN_GROUP_MAX, root))
                        result = OS_TRUE;
                    break;
                case ONEPOS_MUL_WIFI_JOINT_POS:
                    if(OS_EOK == onepos_parse_mul_wifi_pos(g_platform_wifi_src_info, root))
                        result = OS_TRUE;
                    break;
                #endif

                
                #if defined(ONEPOS_CELL_POS)
                case ONEPOS_MUL_CELL_JOINT_POS:
                    if(OS_EOK == onepos_parse_mul_cell_pos(g_platform_lbs_src_info, root))
                        result = OS_TRUE;
                    break;
                #endif
                default:
                    LOG_EXT_D("pos_type is error!");
                    break;
            }
        }
        else
        {
            LOG_EXT_D("pos_type is not current using!");
        }
    }
    else if(ONEPOS_NULL_POSITION == item->valueint)
    {
        LOG_EXT_I("position result is NULL!");
        result = OS_TRUE;
    }
    else if(ONEPOS_OVER_LIMIT == item->valueint)
    {
        LOG_EXT_I("position server over call limit!");
        result = OS_TRUE;
    }
    else
    {
        LOG_EXT_E("errcode is error!!!");
    }
    cJSON_Delete(root);
    if(!result)
        g_parse_rec_err_flag = OS_TRUE;
}
#endif

#if defined(ONEPOS_SUPP_REMOTE_CONF)
static os_bool_t rec_remote_conf = OS_FALSE;
static os_bool_t remote_conf_ret = OS_FALSE;
static os_int32_t remote_conf_id = -1;
/**
 ***********************************************************************************************************************
 * @brief           parse receive config message
 *
 * @param[out]      config_id       save config id
 * @param[in]       data            received message
 *
 * @return          os_bool_t       parse is succ
 * @retval          OS_FALSE        parse is failed
 * @retval          OS_TRUE         parse is succ
 ***********************************************************************************************************************
 */
static os_bool_t onepos_parse_config_msg(os_int32_t *config_id, const char *data)
{
    cJSON *root = OS_NULL;
    cJSON *item = OS_NULL;
    os_bool_t result = OS_TRUE;
    
    root = cJSON_Parse(data);
    if(!root)
    {
        LOG_EXT_E("error config string: %s", data);
        result = OS_FALSE;
        goto __exit;
    }
    
    item = cJSON_GetObjectItem(root, "config_id");
    if(config_id && item)
    {
        *config_id = item->valueint;
        if((os_int32_t)*config_id < 0u || *config_id > OS_UINT32_MAX)
        {
            LOG_EXT_E("error config id is : %d!", *config_id);
            result = OS_FALSE;
            goto __exit;
        }
        else
        {
            LOG_EXT_D("config id is : %d!", *config_id);
        }
        
        item = cJSON_GetObjectItem(root, "interval");
        if(item &&
           onepos_set_pos_interval(item->valueint))
        {
            LOG_EXT_D("interval is : %d!", item->valueint);
        }
        else
        
{
            LOG_EXT_E("error interval is : %d!", item->valueint);
            result = OS_FALSE;
            goto __exit;
        }
        
        item = cJSON_GetObjectItem(root, "pos_type");
        if(item && IS_VALID_POS_MODE(item->valueint) &&
           onepos_set_pos_mode((onepos_pos_mode_t)item->valueint))
        {
            LOG_EXT_D("pos_mode is : %d!", item->valueint);
        }
        else
        {
            LOG_EXT_E("error pos_mode is : %d!", item->valueint);
            result = OS_FALSE;
            goto __exit;
        }
           
        item = cJSON_GetObjectItem(root, "prov_type");
        if(item &&IS_VAILD_SEV_PRO(item->valueint)&&
           onepos_set_sev_pro((onepos_sev_pro_t)item->valueint))
        {            
            LOG_EXT_D("prov_type is : %d!", item->valueint);
        }
        else
        {
            LOG_EXT_E("error prov_type is : %d!", item->valueint);
            result = OS_FALSE;
            goto __exit;
        }
        
        item = cJSON_GetObjectItem(root, "pos_error");
        if(item &&
           onepos_set_pos_err(item->valueint))
        {
            LOG_EXT_D("pos_error is : %d!", item->valueint);
        }
        else
        {
            LOG_EXT_E("error pos_error is : %d!", item->valueint);
            result = OS_FALSE;
            goto __exit;
        }
        
        item = cJSON_GetObjectItem(root, "rept_type");
        if(item &&IS_VAILD_SEV_TYPE(item->valueint)&&
           onepos_set_server_type((onepos_serv_type)item->valueint))
        {            
            LOG_EXT_D("rept_type is : %d!", item->valueint);
        }
        else
        {
            LOG_EXT_E("error rept_type is : %d!", item->valueint);
            result = OS_FALSE;
            goto __exit;
        }
        
        item = cJSON_GetObjectItem(root, "pos_on");
        if(item)
        {
            LOG_EXT_D("pos_on is : %d!", item->valueint);
            if(0 == item->valueint)
            {
                onepos_start_server();
            }
            else if(1 == item->valueint)
            {
                onepos_stop_server();
            }
            else
            {
                LOG_EXT_E("error pos_on is : %d!", item->valueint);
                result = OS_FALSE;
                goto __exit;
            }
        }
        else
        {
            result = OS_FALSE;
            goto __exit;
        }
    }
    else
    {
        result = OS_FALSE;
    }

__exit:
    cJSON_Delete(root);
    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           reply config message
 *
 * @param[in]       config_id       config id
 * @param[in]       conf_result     config result
 ***********************************************************************************************************************
 */
static void onepos_config_reply(os_uint32_t config_id, os_bool_t conf_result)
{
    char *reply_str         = OS_NULL;
    cJSON *reply_json       = OS_NULL;
    
    onepos_conf_err_code_t err_code = conf_result?ONEPOS_CONFIG_SUCC:ONEPOS_CONFIG_FAIL;
        
    memset(pub_conf_topic_str, 0, 50);
    snprintf(pub_conf_topic_str, 50, "%s%s%s", ONEPOS_TOPIC_PRE, ONEPOS_DEVICE_ID, ONEPOS_CONIFG_PUB_TOPIC_SUFF);
    
    reply_json = cJSON_CreateObject();
    if(reply_json)
    {
        cJSON_AddItemToObject(reply_json, "config_id", cJSON_CreateNumber(config_id));
        cJSON_AddItemToObject(reply_json, "err_code", cJSON_CreateNumber((double)err_code));
        reply_str = cJSON_Print(reply_json);
        cJSON_Delete(reply_json);
        if(reply_str)
        {
            onepos_mqtt_message_publish(pub_conf_topic_str, reply_str);
            clean_loca_pub_messsge(reply_str);
        }
        else
        {
            LOG_EXT_E("config message reply string is null!");
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           will call this func while config message arrived 
 *
 * @param[in]       data            subscribed message info.
 ***********************************************************************************************************************
 */
static void onepos_config_msg_arrived_call_back_fun(MessageData *data)
{
    #if defined(ONEPOS_DEBUG)
    printf("receive_message : %s\n", (char*)data->message->payload);
    #endif
    
    rec_remote_conf = OS_TRUE;
    remote_conf_ret = onepos_parse_config_msg(&remote_conf_id, data->message->payload);
    rec_remote_conf = OS_FALSE;

    memset(data->message->payload, 0, data->message->payloadlen);
}
#endif

/**
 ***********************************************************************************************************************
 * @brief           report status inforamtion of device to paltform
 ***********************************************************************************************************************
 */
void onepos_rep_device_sta(void)
{
    char reply_str[128]     = {0,};
      
    memset(pub_conf_topic_str, 0, 50);
    snprintf(pub_conf_topic_str, 50, "%s%s%s", ONEPOS_TOPIC_PRE, ONEPOS_DEVICE_ID, ONEPOS_CONIFG_PUB_TOPIC_SUFF);
    
    #if defined(ONEPOS_SUPP_REMOTE_CONF)
    if(!rec_remote_conf)
    {
    #endif
    memset(reply_str, 0, 128);
    
    sprintf(reply_str, "{\"payload\":{\"pos_on\":%u,\"pos_type\":%u,\"interval\":%u,\"rept_type\":%u,\"pos_error\":%u,\"prov_type\":%u}}",
            onepos_get_server_sta() == ONEPOS_CLOSING ? 1u : 0u,
            onepos_get_pos_mode(), onepos_get_pos_interval(),
            onepos_get_server_type(), onepos_get_sev_pos_err(),
            onepos_get_sev_pro());
    
    onepos_mqtt_message_publish(pub_conf_topic_str, reply_str);
    #if defined(ONEPOS_SUPP_REMOTE_CONF)
    }
    #endif
}

/**
 ***********************************************************************************************************************
 * @brief           will call this func while message arrived 
 *
 * @param[in]       data            subscribed message info.
 ***********************************************************************************************************************
 */
#if defined(ONEPOS_DEVICE_REC_POS_INFO)
static void onepos_subed_msg_arrived_call_back_fun(MessageData *data)
{
    #if defined(ONEPOS_DEBUG)
    printf("receive_message : %s\n", (char*)data->message->payload);
    #endif
    
    onepos_parse_rec_data((char*)data->message->payload);
    
    memset(data->message->payload, 0, data->message->payloadlen);
    os_sem_post(onepos_mqtt_context.rec_sub_msg_sem);
}
#endif

#if defined(ONEPOS_DEVICE_REC_POS_INFO) || defined(ONEPOS_SUPP_REMOTE_CONF)
extern int cycle(MQTTClient* c, Timer* timer);
/**
 ***********************************************************************************************************************
 * @brief           onepos wait message published by paltform task function
 *
 * @param[in]       parameter       input param(onepos_mqtt_context_t)
 ***********************************************************************************************************************
 */
static void onepos_rec_pmsg_task_func(void *parameter)
{
    onepos_mqtt_context_t* context = (onepos_mqtt_context_t*)parameter;
    onepos_rec_pmsg_work_t* work = &context->rec_pmsg_work;

    while(1)
    {
        if(onepos_mqtt_is_connected())
        {
            os_sem_wait(work->rec_msg_lock, OS_IPC_WAITING_FOREVER);
            TimerCountdownMS(&work->rec_msg_timer, ONEPOS_WAIT_PUB_MSG_TIMEOUT); /* Don't wait too long if no traffic is incoming */
    		cycle(&context->client, &work->rec_msg_timer);
            os_sem_post(work->rec_msg_lock);
                        
            #if defined(ONEPOS_SUPP_REMOTE_CONF)
            if(-1 != remote_conf_id)
            {
                onepos_config_reply(remote_conf_id, remote_conf_ret);
                onepos_rep_device_sta();
                remote_conf_id = -1;
            }
            #endif
        }
        else
        {
            os_task_mdelay(ONEPOS_WAIT_MQTT_READY);
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           init sonmething of wait message published by paltform task need
 *
 * @param[in]       context      context of onepos mqtt communtion
 *
 * @return          os_err_t
 * @retval          OS_EOK       init successful
 * @retval          OS_ERROR     init failed
 ***********************************************************************************************************************
 */
static os_err_t onepos_rec_pmsg_work_init(onepos_mqtt_context_t* context)
{
    onepos_rec_pmsg_work_t* work = &context->rec_pmsg_work;
    os_err_t ret = OS_EOK;

    if(work && OS_NULL == os_task_find("onepos_rec_pmsg"))
    {
        TimerInit(&work->rec_msg_timer);
        work->rec_msg_lock = os_sem_create("onepos_rec_pmsg", 1, OS_IPC_FLAG_FIFO);
        work->rec_msg_task = os_task_create("onepos_rec_pmsg", onepos_rec_pmsg_task_func, (void*)context, 
                                            ONEPOS_WAIT_PUB_MSG_TASK_STACK_SIZE, ONEPOS_TASK_PRIORITY, 
                                            ONEPOS_TASK_TIMESLICE);
        if(!(work->rec_msg_lock && work->rec_msg_task && work->rec_msg_timer.xTimeOut))
        {
            ret = OS_ERROR;
            goto exit;
        }
        else
        {
            os_task_startup(work->rec_msg_task);
            return ret;
        }
    }
    else
    {
        return OS_ERROR;
    }

    exit:
        if(work->rec_msg_lock)
            os_sem_destroy(work->rec_msg_lock);
        if(work->rec_msg_task)
            os_task_destroy(work->rec_msg_task);
        if(work->rec_msg_timer.xTimeOut)
            TimerRelease(&work->rec_msg_timer);
            
        return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           deinit sonmething of wait message published by paltform task using
 *
 * @param[in]       work      onepos_rec_pmsg_work_t
 ***********************************************************************************************************************
 */
static void onepos_rec_pmsg_work_deinit(onepos_rec_pmsg_work_t* work)
{
    if(work)
    {
        if(work->rec_msg_task)
        {
            os_sem_wait(work->rec_msg_lock, OS_IPC_WAITING_FOREVER);
            os_task_destroy(work->rec_msg_task);
        }
        if(work->rec_msg_timer.xTimeOut)
            TimerRelease(&work->rec_msg_timer);
        if(work->rec_msg_lock)
            os_sem_destroy(work->rec_msg_lock);
    }
}
#endif
/**
 ***********************************************************************************************************************
 * @brief           init mqtt communication context and sub topic of received
 *
 * @return          os_err_t
 * @retval          OS_EOK       init and subscribe topic successful
 * @retval          OS_ERROR     init and subscribe topic failed
 ***********************************************************************************************************************
 */
static os_err_t onepos_mqtt_comm_init(void)
{
    int         rc = 0;
    os_err_t    ret = OS_EOK;

    if(DEVICE_ID_LEN != strlen(ONEPOS_DEVICE_ID))
    {
        LOG_EXT_E("ONEPOS_DEVICE_ID is error, pls check!");
        return OS_ERROR;
    }

    memset(mqtt_client_str, 0, CLIENT_ID_LEN + 1);
    snprintf(mqtt_client_str, CLIENT_ID_LEN + 1, "%s%s", ONEPOS_DEVICE_ID, ONEPOS_MQTT_CLIENT_ID_SUFF);
    
    /* init mqtt communication context */
    onepos_connectData.MQTTVersion = ONEPOS_MQTT_VERSION;
    onepos_connectData.keepAliveInterval = ONEPOS_MQTT_COMM_ALIVE_TIME;
    onepos_connectData.cleansession = 1;
    onepos_connectData.willFlag = 0;
    onepos_connectData.clientID.cstring = mqtt_client_str;
    onepos_connectData.username.cstring = ONEPOS_MQTT_USER_NAME;
    onepos_connectData.password.cstring = ONEPOS_MQTT_PASSWD;
    
    rc = MQTTNetworkInit(&onepos_mqtt_context.network, ONEPOS_PLATFORM_ADDR, ONEPOS_PLATFORM_PORT, OS_NULL);
    onepos_mqtt_context.network.handle = (uintptr_t)-1;

    MQTTClientInit(&onepos_mqtt_context.client, 
                   &onepos_mqtt_context.network, 
                   ONEPOS_PLATFORM_COMM_TIMEOUT, 
                   onepos_sendbuf, 
                   sizeof(onepos_sendbuf), 
                   onepos_recebuf, 
                   sizeof(onepos_recebuf));

    #if defined(ONEPOS_DEVICE_REC_POS_INFO)
    onepos_mqtt_context.rec_sub_msg_sem = os_sem_create("ops_rec_sub_msg", 0, OS_IPC_FLAG_FIFO);
    if(OS_NULL == onepos_mqtt_context.rec_sub_msg_sem)
    {
        LOG_EXT_E("creat onepos receive sub_msg semphare is error");
        rc = -1;
    }
    #endif

    #if defined(ONEPOS_DEVICE_REC_POS_INFO) || defined(ONEPOS_SUPP_REMOTE_CONF)
    rc = onepos_rec_pmsg_work_init(&onepos_mqtt_context);
    #endif
    
    ret = (rc == 0)?OS_EOK:OS_ERROR;
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           deinti some param of onepos_mqtt 
 *
 ***********************************************************************************************************************
 */
static void onepos_mqtt_comm_deinit(void)
{
    TimerRelease(&onepos_mqtt_context.client.last_sent);
    TimerRelease(&onepos_mqtt_context.client.last_received);
    
    #if defined(ONEPOS_DEVICE_REC_POS_INFO) || defined(ONEPOS_SUPP_REMOTE_CONF)
    onepos_rec_pmsg_work_deinit(&onepos_mqtt_context.rec_pmsg_work);
    #endif
    
    #if defined(ONEPOS_DEVICE_REC_POS_INFO)
    if(onepos_mqtt_context.rec_sub_msg_sem)
        os_sem_destroy(onepos_mqtt_context.rec_sub_msg_sem);
    #endif
    memset(&onepos_connectData, 0, sizeof(MQTTPacket_connectData));
    memset(&onepos_mqtt_context, 0, sizeof(onepos_mqtt_context_t));
    onepos_mqtt_context.network.handle = (uintptr_t)-1;
}

/**
 ***********************************************************************************************************************
 * @brief           disconnect mqtt connect
 *
 ***********************************************************************************************************************
 */
void onepos_mqtt_disconnect(void)
{
    if(onepos_mqtt_is_connected())
    {
        #if defined(ONEPOS_DEVICE_REC_POS_INFO)
        memset(sub_info_topic_str, 0, 50);
        snprintf(sub_info_topic_str, 50, "%s%s%s", ONEPOS_TOPIC_PRE, ONEPOS_DEVICE_ID, ONEPOS_SUB_TOPIC_SUFF);
        MQTTUnsubscribe(&onepos_mqtt_context.client, sub_info_topic_str);
        #endif
        
        #if defined(ONEPOS_SUPP_REMOTE_CONF)
        memset(sub_info_topic_str, 0, 50);
        snprintf(sub_info_topic_str, 50, "%s%s%s", ONEPOS_TOPIC_PRE, ONEPOS_DEVICE_ID, ONEPOS_CONFIG_SUB_TOPIC_SUFF);
        MQTTUnsubscribe(&onepos_mqtt_context.client, sub_info_topic_str);
        #endif
    }
    
    MQTTDisconnect(&onepos_mqtt_context.client);

    if((uintptr_t)-1 != onepos_mqtt_context.network.handle)
        onepos_mqtt_context.network.disconnect(&(onepos_mqtt_context.network));
}

/**
 ***********************************************************************************************************************
 * @brief           build mqtt connected
 *
 * @return          os_err_t
 * @retval          OS_EOK       connected
 * @retval          OS_ERROR     not connected
 ***********************************************************************************************************************
 */
os_err_t onepos_mqtt_connect(void)

{
    int rc                 = 0;
    os_err_t ret           = OS_EOK;
    
    rc = onepos_mqtt_context.network.connect(&onepos_mqtt_context.network);
    if (0 != rc)
    {
        LOG_EXT_E("establish network failed, check IP and PORT");
        ret = OS_ERROR;
        goto exit;
    }
    LOG_EXT_D("establish network sucess");

    if ((rc = MQTTConnect(&onepos_mqtt_context.client, &onepos_connectData)) != 0)
    {
        LOG_EXT_E("Return code from MQTT connect is %d", rc);
        ret = OS_ERROR;
        goto exit;
    }
    else
        LOG_EXT_D("MQTT Connected");

    #if defined(ONEPOS_DEVICE_REC_POS_INFO)
    memset(sub_info_topic_str, 0, 50);
    snprintf(sub_info_topic_str, 50, "%s%s%s", ONEPOS_TOPIC_PRE, ONEPOS_DEVICE_ID, ONEPOS_SUB_TOPIC_SUFF);
    
    if(MQTTSubscribe(&onepos_mqtt_context.client, sub_info_topic_str, 
                            ONEPOS_MQTT_COMM_QOS, onepos_subed_msg_arrived_call_back_fun) != 0)
    {
        LOG_EXT_E("return code from info_topic sub is %d", rc);
        ret = OS_ERROR;
        goto exit;
    }
    else
        LOG_EXT_D("mqtt sub info_topic succ");
    #endif

    #if defined(ONEPOS_SUPP_REMOTE_CONF)
    memset(sub_conf_topic_str, 0, 50);
    snprintf(sub_conf_topic_str, 50, "%s%s%s", ONEPOS_TOPIC_PRE, ONEPOS_DEVICE_ID, ONEPOS_CONFIG_SUB_TOPIC_SUFF);
    
    if(MQTTSubscribe(&onepos_mqtt_context.client, sub_conf_topic_str, 
                            ONEPOS_MQTT_COMM_QOS, onepos_config_msg_arrived_call_back_fun) != 0)
    {
        LOG_EXT_E("return code from conf_topic sub is %d", rc);
        ret = OS_ERROR;
        goto exit;
    }
    else
        LOG_EXT_D("mqtt sub conf_topic succ");
    #endif
    
exit:
    if(OS_EOK != ret)
    {
        onepos_mqtt_disconnect();
    }
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           check onepos mqtt connected status
 *
 * @return          connected or not
 * @retval          os_true       connected
 * @retval          os_false      not connecred
 ***********************************************************************************************************************
 */
os_bool_t onepos_mqtt_is_connected(void)
{
    os_int32_t rc = 0;
    rc = MQTTIsConnected(&onepos_mqtt_context.client);
    if(0 == rc)
    {
        return OS_FALSE;
    }
    else
    {
        return OS_TRUE;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           publish massage to topic of mqtt
 *
 * @param[in]       topic_name       mqtt topic name 
 * @param[out]      pub_message      message string will be published
 *
 * @return          os_err_t
 * @retval          OS_EOK       publish message to the topic is successfully
 * @retval          OS_ERROR     publish message to the topic is failed
 ***********************************************************************************************************************
 */
static os_err_t onepos_mqtt_message_publish(const char *topic_name, char *pub_message)
{
    int         rc = 0;
    os_err_t    ret = OS_EOK;
    MQTTMessage message;

    message.qos = ONEPOS_MQTT_COMM_QOS;
    message.retained = 0;
    message.payload = pub_message;
    message.payloadlen = strlen(pub_message);
    
    if (onepos_mqtt_is_connected() != 0)
	{
        #if defined(ONEPOS_DEBUG)
        printf("publish_message : %s\n", pub_message);
        #endif
    
        #if defined(ONEPOS_DEVICE_REC_POS_INFO) || defined(ONEPOS_SUPP_REMOTE_CONF)
        os_sem_t *lock = onepos_mqtt_context.rec_pmsg_work.rec_msg_lock;
        os_sem_wait(lock, OS_IPC_WAITING_FOREVER);
        #endif
        
        rc = MQTTPublish(&onepos_mqtt_context.client, topic_name, &message);
        if(0 != rc)
        {
            ret = OS_ERROR;
		    LOG_EXT_E("Return code from MQTT publish is %d", rc);
        }
        
        #if defined(ONEPOS_DEVICE_REC_POS_INFO) || defined(ONEPOS_SUPP_REMOTE_CONF)
        os_sem_post(lock);
        #endif
	}
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           wait mqtt subscribed message
 *
 * @param[in]       c               mqtt client
 * @param[out]      timeout_ms      wait mqtt subscribed message max time
 *
 * @return          wait subscribed message result 
 * @retval          OS_ETIMEOUT     wait timeout
 * @retval          OS_EOK          successful
 ***********************************************************************************************************************
 */
#if defined(ONEPOS_DEVICE_REC_POS_INFO)
extern os_tick_t os_tick_from_ms(os_uint32_t ms);
static os_err_t onepos_mqtt_wait_sub_message(MQTTClient* c, os_uint32_t timeout_ms)
{
    os_err_t ret = OS_EOK;
    
    ret = os_sem_wait(onepos_mqtt_context.rec_sub_msg_sem, os_tick_from_ms(timeout_ms));
    if(OS_EOK != ret)
        LOG_EXT_I("wait sub_msg error, ret is %d!", ret);
    
    return ret;
}
#endif
/**
 ***********************************************************************************************************************
 * @brief           free loca publish message space
 *
 * @param[in]       msg_str       message space to free
 ***********************************************************************************************************************
 */
static void clean_loca_pub_messsge(char *msg_str)
{
    if(msg_str)
    {
        os_free(msg_str);
        msg_str = OS_NULL;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           init device of onepos used
 *
 * @return          result of init device
 * @retval          OS_EOK          init device successful
 * @retval          OS_ERROR        init device failed
 ***********************************************************************************************************************
 */
static os_err_t onepos_init_device(void)
{
    os_err_t ret = OS_EOK;
    
    #if defined(ONEPOS_WIFI_POS)
    if(OS_EOK != init_onepos_wifi_device())
    {
        LOG_EXT_E("init wifi device is error");
        ret = OS_ERROR;
    }
    #endif

    #if defined(ONEPOS_CELL_POS)
    if((OS_EOK == ret) && (OS_EOK != init_onepos_cell_device()))
    {
        LOG_EXT_E("init cell module is error");
        ret = OS_ERROR;
    }
    #endif

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           init device of onepos used
 *
 * @return          result of init device
 * @retval          OS_EOK          init device successful
 * @retval          OS_ERROR        init device failed
 ***********************************************************************************************************************
 */
os_bool_t onepos_get_dev_sta(void)
{
    os_err_t ret = OS_TRUE;
    
    #if defined(ONEPOS_WIFI_POS)
    if(OS_TRUE != onepos_get_wifi_sta())
    {
        LOG_EXT_E("wifi device status is not ready");
        ret = OS_ERROR;
    }
    #endif

    #if defined(ONEPOS_CELL_POS)
    if((OS_EOK == ret) && (OS_EOK != onepos_get_cell_sta()))
    {
        LOG_EXT_E("cell module is not ready");
        ret = OS_ERROR;
    }
    #endif

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           init something of onepos
 *
 * @return          init succ or not
 * @retval          os_true         init successfuls
 * @retval          os_false        init failed
 ***********************************************************************************************************************
 */
os_bool_t onepos_init(void)
{
   #if defined(ONEPOS_WIFI_POS) && defined(ONEPOS_DEVICE_REC_POS_INFO)
    if(OPS_EOK != onepos_wifi_location_init())
    {
        LOG_EXT_E("onepos single wifi arithmetic init error, will return\n");
        return OS_FALSE;
    }
    #endif

    if(OS_EOK != onepos_mqtt_comm_init())
    {
        LOG_EXT_E("onepos mqtt init error, will return\n");
        return OS_FALSE;
    }

    if(OS_EOK != onepos_init_device())
    {
        LOG_EXT_E("onepos init onepos device is error, will return\n");
        return OS_FALSE;
    }

    if(OS_EOK != onepos_mqtt_connect())
    {
        LOG_EXT_I("onepos mqtt connect error\n");
        return OS_FALSE;
    }
    
    return OS_TRUE;
}

/**
 ***********************************************************************************************************************
 * @brief           deinit something of onepos
 ***********************************************************************************************************************
 */
void onepos_deinit(void)
{
    #if defined(ONEPOS_WIFI_POS) && defined(ONEPOS_DEVICE_REC_POS_INFO)
    onepos_wifi_location_deinit();
    #endif
    
    onepos_mqtt_disconnect();
    onepos_mqtt_comm_deinit();
}

/**
 ***********************************************************************************************************************
 * @brief           init something of onepos
 *
 * @param[in]       src             point for save location result
 * @param[in]       mode            the mode of this time using
 * @param[in]       sev_pro         service provider for using
 *
 * @return          location succ or not
 * @retval          os_true         location successfuls
 * @retval          os_false        location failed
 ***********************************************************************************************************************
 */
os_bool_t onepos_location(ops_src_info_t* src, onepos_pos_mode_t mode, onepos_sev_pro_t sev_pro)
{
    char *pub_msg = OS_NULL;
    os_bool_t ret = OS_TRUE;
        
    if(!src)
    {
        LOG_EXT_E("input param : src is NULL!");
        return OS_FALSE;
    }

    memset(pub_info_topic_str, 0, 50);
    snprintf(pub_info_topic_str, 50, "%s%s%s", ONEPOS_TOPIC_PRE, ONEPOS_DEVICE_ID, ONEPOS_INFO_PUB_TOPIC_SUFF);

    switch (mode)
    {
        #if defined(ONEPOS_WIFI_POS)
        case ONEPOS_QUERY_SINGLE_WIFI_POS:
        {
            #if defined(ONEPOS_DEVICE_REC_POS_INFO)    
            g_onepos_wifi_info = src->sigle_wifi_src_info;
            #endif
            pub_msg = single_wifi_query_pos_pub_msg(sev_pro);
        }
        break;
        
        case ONEPOS_MUL_WIFI_JOINT_POS:
        {
            #if defined(ONEPOS_DEVICE_REC_POS_INFO)
            g_platform_wifi_src_info = src->platform_wifi_src_info;
            #endif
            pub_msg = mul_wifis_pos_pub_msg(sev_pro);
        }
        break;
        #endif
        
        #if defined(ONEPOS_CELL_POS)
        case ONEPOS_MUL_CELL_JOINT_POS:
        {
            #if defined(ONEPOS_DEVICE_REC_POS_INFO)
            g_platform_lbs_src_info = src->platform_lbs_src_info;
            #endif
            pub_msg = mul_cells_pos_pub_msg(sev_pro);
        }
        break;
        #endif
        
        default:
        {
            LOG_EXT_E("error mode!");
            return OS_FALSE;
        }
    }
    
    if(pub_msg)
    {
        #if defined(ONEPOS_DEVICE_REC_POS_INFO)
        g_curr_use_mode = mode;
        g_parse_rec_err_flag = OS_FALSE;
        #endif
        
        if(OS_EOK == onepos_mqtt_message_publish(pub_info_topic_str, pub_msg))
        {
            #if defined(ONEPOS_DEVICE_REC_POS_INFO)
            if(OS_EOK != onepos_mqtt_wait_sub_message(&onepos_mqtt_context.client, ONEPOS_REC_SUB_MSG_TIMEOUT)
                || g_parse_rec_err_flag == OS_TRUE)
            {
                LOG_EXT_I("wait sub msg error, parse flag : %d", g_parse_rec_err_flag);
                ret = OS_FALSE;
            }
            #endif
        }
        else
        {
            LOG_EXT_E("mqtt publish message is ERR!");
            ret = OS_FALSE;
        }
        clean_loca_pub_messsge(pub_msg);
        pub_msg = OS_NULL;
    }
    else
    {
        LOG_EXT_E("pub_msg is NULL!");
    }

    return ret;
}
