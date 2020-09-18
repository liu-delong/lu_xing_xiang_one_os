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
 * @file        onepos_wifi_loca.c
 *
 * @brief       wifi^s location of onepos
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <os_task.h>
#include <os_assert.h>
#include <sys/time.h>
#include "cJSON.h"
#include "onepos_src_info.h"

#define DBG_EXT_TAG "onepos.wifi"
#define DBG_EXT_LVL DBG_EXT_DEBUG
#include <os_dbg_ext.h>

static mo_object_t *wifi_module = OS_NULL;

#if defined(ONEPOS_DEVICE_REC_POS_INFO)
static ops_wifi_settings_t       g_sys_settings           = {0,};
static ops_run_status            g_run_status             = {0,};

/**
 ***********************************************************************************************************************
 * @brief           deinit arithmetic
 ***********************************************************************************************************************
 */
void onepos_wifi_location_deinit(void)
{
    onepos_arthmetic_exit(&g_sys_settings);
}

/**
 ***********************************************************************************************************************
 * @brief           init something of arithmetic 
 *
 * @return          os_err_t
 * @retval          OS_EOK         init arithmetic is successful
 * @retval          OS_ERROR       init arithmetic is failed
 ***********************************************************************************************************************
 */
os_err_t onepos_wifi_location_init(void)
{
    return onepos_arithmetic_init(&g_sys_settings, &g_run_status);
}
#endif
/**
 ***********************************************************************************************************************
 * @brief           init wifi module to work
 *
 * @return          os_err_t
 * @retval          OS_EOK         init wifi modules is successful
 * @retval          OS_ERROR       init wifi modules is failed
 ***********************************************************************************************************************
 */
os_err_t init_onepos_wifi_device(void)
{
    wifi_module = mo_get_by_name(ONEPOS_WIFI_DEVICE_NAME);
    if (OS_NULL == wifi_module)
    {
        LOG_EXT_E("onepos wifi device : %s is error", ONEPOS_WIFI_DEVICE_NAME);
        return OS_ERROR;
    }

    /* Check the wifi module mode */
    if (MO_WIFI_MODE_STA != mo_wifi_get_mode(wifi_module))
    {
        LOG_EXT_E("the wifi modules : %s should be STATION mode!");
        return OS_ERROR;
    }

    /* waite wlan is ready */
    if (MO_WIFI_STAT_CONNECTED != mo_wifi_get_stat(wifi_module))
    {
        LOG_EXT_I("wifi is not ready, will return");
    }
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           remalloc ap_info space
 *
 * @param[in]       wifi_src        space of already malloc 
 * @param[in]       item_num        will increase item num
 *
 * @return          new space of ap_info
 ***********************************************************************************************************************
 */
static ops_wifi_src_info_t* creat_wifi_pos_src(ops_wifi_src_info_t* wifi_src, os_uint32_t item_num)
{
    ops_wifi_src_info_t* new_src = wifi_src;

    if(item_num)
    {
        new_src = os_realloc(wifi_src, item_num * sizeof(ops_wifi_src_info_t));
    }

    return new_src;
}

/**
 ***********************************************************************************************************************
 * @brief           free ap_info space
 ***********************************************************************************************************************
 */
void clean_ap_pos_info(ops_wifi_src_grp_t* src)
{
    if(src && src->sig_src)
    {
        os_free(src->sig_src);
        src->sig_src = OS_NULL;
    }
}

#if defined(ONEPOS_DEVICE_REC_POS_INFO)
/**
 ***********************************************************************************************************************
 * @brief           parse single wifi position string
 *
 * @param[in]       sig_wifi_info       to save the parse result
 * @param[in]       src                 the num to sava wifi 
 * @param[out]      data_item           position info(json format)
 *
 * @return          parse result
 * @retval          OS_EOK          parse is successful
 * @retval          OS_ERROR        paese is failed
 ***********************************************************************************************************************
 */
os_err_t onepos_parse_sig_wifi_pos(ops_sigle_wifi_info_t *sig_wifi_info, os_uint16_t src_num, cJSON *data_item)
{
    static  os_uint64_t latest_time = 0;
    cJSON  *item                    = OS_NULL;

    char    bssid_val[MO_WIFI_BSSID_MAX_LENGTH + 1];
    char   *point     = 0;
    char   *tmp       = OS_NULL;
    int     wifi_num  = 0;
    
    os_int32_t   rad_preci  = 0;
    os_int32_t   rssi       = 0;
    os_err_t     ret        = OS_EOK;
    
    os_int32_t   mac[OPS_SOURCE_MAC_MAX]     =  {0};
    ops_wifi_src_info_t  ap_info        = {0,};
    ops_wifi_src_info_t *ap_info_point  = OS_NULL;
    ops_wifi_src_grp_t   wifi_src_grp   = {0,};

    #ifdef ONEPOS_DEBUG
    struct tm    TM = {0};
    printf(" HEAD     DATE      TIME         MAC         RSSI         LAT                  LON            RADIUS   \n");
    printf("------ ---------- -------- ----------------- ---- -------------------- -------------------- ---------- \n");
    #endif

    memset(&wifi_src_grp, 0, sizeof(ops_wifi_src_grp_t));
    
    if(data_item)
    {
        item = cJSON_GetObjectItem(data_item, "at");
        if(item)
        {
            if(latest_time >= item->valuedouble)
            {
                LOG_EXT_E("error time stamp");
                return OS_ERROR;
            }
            else
                latest_time = item->valuedouble;
            
            wifi_src_grp.time = (os_uint32_t)(item->valuedouble / 1000);
        }
        item = cJSON_GetObjectItem(data_item, "pos");
        tmp = item->valuestring;

        if(item)
        {
            if(OPS_SOURCE_MAC_MAX != sscanf(tmp, "%02x:%02x:%02x:%02x:%02x:%02x", 
                                  &mac[0],
                                  &mac[1],
                                  &mac[2],
                                  &mac[3],
                                  &mac[4],
                                  &mac[5]))
            {
                sscanf(tmp, "%lf,%lf", &sig_wifi_info->lat_coordinate,
                                               &sig_wifi_info->lon_coordinate);
                if(!IS_VAILD_LAT(sig_wifi_info->lat_coordinate))
                {
                    LOG_EXT_I("error lat_coordinate");
                    return OS_ERROR;
                }

                if(!IS_VAILD_LON(sig_wifi_info->lon_coordinate))
                {
                    LOG_EXT_I("error lon_coordinate");
                    return OS_ERROR;
                }

                #ifdef ONEPOS_DEBUG
                gmtime_r((time_t*)&wifi_src_grp.time, &TM);
                printf("$APMSG %04d/%02d/%02d %02d:%02d:%02d --:--:--:--:--:-- %-3d %-20lf %-20lf %-10d\n",
                TM.tm_year + 1900, TM.tm_mon + 1, TM.tm_mday, TM.tm_hour + 8, TM.tm_min, TM.tm_sec, -99, 
                                sig_wifi_info->lat_coordinate, sig_wifi_info->lon_coordinate, 0);
                #endif
                
            }
            else
            {
                point = tmp;
                do{
                    memset(&ap_info, 0, sizeof(ops_wifi_src_info_t));

                    /* no limit num */
                    /* 
                    if(!(wifi_num < src_num))
                        break;
                    */
                    
                    sscanf(item->valuestring + (point - tmp), "%[^,],%d,%u,%lf,%lf", bssid_val, 
                                                                           &rssi,
                                                                           &rad_preci,
                                                                           &ap_info.lat_coordinate,
                                                                           &ap_info.lon_coordinate);
                    
                    sscanf(bssid_val, "%02x:%02x:%02x:%02x:%02x:%02x", 
                                      &mac[0],
                                      &mac[1],
                                      &mac[2],
                                      &mac[3],
                                      &mac[4],
                                      &mac[5]);

                    ap_info.rssi = (os_uint16_t)rssi;
                    ap_info.mac[0] = (char)mac[0];
                    ap_info.mac[1] = (char)mac[1];
                    ap_info.mac[2] = (char)mac[2];
                    ap_info.mac[3] = (char)mac[3];
                    ap_info.mac[4] = (char)mac[4];
                    ap_info.mac[5] = (char)mac[5];

                    if(!IS_VAILD_LAT(ap_info.lat_coordinate))
                    {
                        LOG_EXT_I("error lat_coordinate");
                        return OS_ERROR;
                    }

                    if(!IS_VAILD_LON(ap_info.lon_coordinate))
                    {
                        LOG_EXT_I("error lon_coordinate");
                        return OS_ERROR;
                    }

                    /* If AP haven^t position, will not save this AP */
                    /* If AP is valid, will malloc space and save the ap_info */
                    if(0 != rad_preci)
                    {
                        ap_info_point = creat_wifi_pos_src(ap_info_point, (wifi_num + 1));
                        memcpy(&ap_info_point[wifi_num], &ap_info, sizeof(ops_wifi_src_info_t));
                        wifi_num ++;
                    }
                    #ifdef ONEPOS_DEBUG
                    gmtime_r((time_t*)&wifi_src_grp.time, &TM);
                    printf("$APMSG %04d/%02d/%02d %02d:%02d:%02d %02x:%02x:%02x:%02x:%02x:%02x %-3d %-20lf %-20lf %-10d\n",
                    TM.tm_year + 1900, TM.tm_mon + 1, TM.tm_mday, TM.tm_hour + 8, TM.tm_min, TM.tm_sec, ap_info.mac[0], 
                    ap_info.mac[1], ap_info.mac[2], ap_info.mac[3], ap_info.mac[4], ap_info.mac[5], ap_info.rssi, 
                                    ap_info.lat_coordinate, ap_info.lon_coordinate, rad_preci);
                    #endif
                    point = strstr(tmp + (point - tmp + 1), ONEPOS_MSG_SEPARATOR) + 1;
                }while(0x01 != (os_uint32_t)point);
                wifi_src_grp.src_num = wifi_num;
                wifi_src_grp.sig_src = ap_info_point;
                onepos_arithmetic_run(&g_run_status,
                                      &wifi_src_grp,  
                                      &g_sys_settings,
                                      &sig_wifi_info->lat_coordinate,
                                      &sig_wifi_info->lon_coordinate);
                }
            sig_wifi_info->time = wifi_src_grp.time;
            
            #ifdef ONEPOS_DEBUG
            printf("------ ---------- -------- ----------------- ---- -------------------- -------------------- ---------- \n");
            #endif
            
            /* call free */
            clean_ap_pos_info(&wifi_src_grp);
        }
    }
    else
    {
        LOG_EXT_E("input param is error!");
        ret = OS_ERROR;
    }
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           parse multiple wifi position string
 *
 * @param[in]       mul_wifi_info       to save the parse result
 * @param[out]      data_item           position info(json format) 
 *
 * @return          parse result
 * @retval          OS_EOK          parse is successful
 * @retval          OS_ERROR        paese is failed
 ***********************************************************************************************************************
 */
os_err_t onepos_parse_mul_wifi_pos(ops_platform_wifi_info_t *mul_wifi_info, cJSON *data_item)
{
    os_err_t ret = OS_EOK;
    cJSON *item = OS_NULL;
    static os_uint64_t latest_time = 0;
    
    if(mul_wifi_info && data_item)
    {
        item = cJSON_GetObjectItem(data_item, "at");
        if(item)
        {
            if(latest_time >= item->valuedouble)
            {
                LOG_EXT_E("error time stamp");
                return OS_ERROR;
            }
            else
                latest_time = item->valuedouble;
            
            mul_wifi_info->time = (os_uint32_t)(item->valuedouble / 1000);
        }
        item = cJSON_GetObjectItem(data_item, "pos");
        if(item)
        {
            sscanf(item->valuestring, "%lf,%lf", &mul_wifi_info->lat_coordinate,
                                               &mul_wifi_info->lon_coordinate);
        }
        if(!IS_VAILD_LAT(mul_wifi_info->lat_coordinate))
        {
            LOG_EXT_D("error lat_coordinate");
            return OS_ERROR;
        }

        if(!IS_VAILD_LON(mul_wifi_info->lon_coordinate))
        {
            LOG_EXT_D("error lon_coordinate");
            return OS_ERROR;
        }
    }
    else
    {
        LOG_EXT_E("input param is error!");
        ret = OS_ERROR;
    }
    return ret;
}
#endif
/**
 ***********************************************************************************************************************
 * @brief           creat onepos wifi loca publish message 
 *
 * @param[in]       wifi_info       scanned wifi information 
 * @param[in]       pos_type        onepos position type 
 * @param[in]       sev_pro         position service provider to invoke
 *
 * @return          json format data
 * @retval          char           json format message
 ***********************************************************************************************************************
 */
static char *onepos_wifi_loca_pub_message(mo_wifi_scan_result_t *wifi_info,
                                          onepos_pos_mode_t pos_type,
                                          onepos_sev_pro_t sev_pro)
{
    char        *macs_str       = OS_NULL;
    char        *msg_str        = OS_NULL;
    cJSON       *ret_msg        = OS_NULL;
    os_uint32_t  macs_str_len   = 0;
    os_uint32_t  index          = 0;
    os_uint32_t  i              =0;

    if ((OS_NULL == wifi_info) || (0 == wifi_info->info_num))
    {
        LOG_EXT_E("input param is error");
        return (char*)OS_NULL;
    }
    macs_str_len = (wifi_info->info_num * ONEPOS_WIFI_INFO_LEN) 
                            + (wifi_info->info_num - 1) * strlen(ONEPOS_MSG_SEPARATOR) + 1;
    macs_str = (char*)os_malloc(macs_str_len);
    if(macs_str)
    {
        for (i = 0; i < (wifi_info->info_num - 1); i++)
        {
            index += os_snprintf(macs_str + index,
                                 (macs_str_len - index),
                                 ONEPOS_WIFI_MSG_FORMAT,
                                 wifi_info->info_array[i].bssid.bssid_array[0],
                                 wifi_info->info_array[i].bssid.bssid_array[1],
                                 wifi_info->info_array[i].bssid.bssid_array[2],
                                 wifi_info->info_array[i].bssid.bssid_array[3],
                                 wifi_info->info_array[i].bssid.bssid_array[4],
                                 wifi_info->info_array[i].bssid.bssid_array[5],
                                 wifi_info->info_array[i].rssi);
            index += os_snprintf(macs_str + index,
                                 (macs_str_len - index),
                                 "%s",
                                 ONEPOS_MSG_SEPARATOR);
        }
        os_snprintf(macs_str + index,
                    (macs_str_len - index),
                    ONEPOS_WIFI_MSG_FORMAT,
                    wifi_info->info_array[i].bssid.bssid_array[0],
                    wifi_info->info_array[i].bssid.bssid_array[1],
                    wifi_info->info_array[i].bssid.bssid_array[2],
                    wifi_info->info_array[i].bssid.bssid_array[3],
                    wifi_info->info_array[i].bssid.bssid_array[4],
                    wifi_info->info_array[i].bssid.bssid_array[5],
                    wifi_info->info_array[i].rssi);
        
        ret_msg = cJSON_CreateObject();
        if(ret_msg)
        {
            cJSON_AddItemToObject(ret_msg, "macs", cJSON_CreateString(macs_str));
            cJSON_AddItemToObject(ret_msg, "pos_type", cJSON_CreateNumber(pos_type));
            cJSON_AddItemToObject(ret_msg, "sev_pro", cJSON_CreateNumber(sev_pro));
            
            msg_str = cJSON_Print(ret_msg);
            
            cJSON_Delete(ret_msg);
        }
        else
        {
            LOG_EXT_E("creat onepos pulish massage json is NULL");
        }
        os_free(macs_str);
    }
    else
    {
        LOG_EXT_E("malloc onepos pulish massage is NULL");
    }
    
    return msg_str;
}
/**
 ***********************************************************************************************************************
 * @brief           creat pulish message to query single wifi position 
 *
 * @param[in]       sev_pro       service provider of using
 *
 * @return          message string
 ***********************************************************************************************************************
 */
char *single_wifi_query_pos_pub_msg(onepos_sev_pro_t sev_pro)
{
    
    char                        *msg_str     = OS_NULL;
    os_err_t                     result      = OS_ERROR;
    mo_wifi_scan_result_t        scan_result = {0,};

    result = mo_wifi_scan_info(wifi_module, OS_NULL, &scan_result);
    
    if(OS_EOK == result)
    {
        msg_str = onepos_wifi_loca_pub_message(&scan_result, 
                                                ONEPOS_QUERY_SINGLE_WIFI_POS,
                                                sev_pro);
    }
    else
    {
        LOG_EXT_E("scan wifi result is NULL!");
    }
    mo_wifi_scan_info_free(&scan_result);

    return msg_str;
}

/**
 ***********************************************************************************************************************
 * @brief           creat pulish message to multiple wifi joint positioning 
 *
 * @param[in]       sev_pro       service provider of using
 *
 * @return          message string
 ***********************************************************************************************************************
 */
char *mul_wifis_pos_pub_msg(onepos_sev_pro_t sev_pro)
{
    
    char                  *msg_str     = OS_NULL;
    os_err_t               result      = OS_ERROR;
    mo_wifi_scan_result_t  scan_result = {0,};

    result = mo_wifi_scan_info(wifi_module, OS_NULL, &scan_result);

    if(OS_EOK == result)
    {
        msg_str = onepos_wifi_loca_pub_message(&scan_result, 
                                                ONEPOS_MUL_WIFI_JOINT_POS,
                                                sev_pro);
    }
    else
    {
        LOG_EXT_E("scan wifi result is NULL!");
    }
    mo_wifi_scan_info_free(&scan_result);
    
    return msg_str;
}

/**
 ***********************************************************************************************************************
 * @brief           get wifi station status
 *
 * @return          os_bool_t
 * @retval          OS_TRUE          wifi is OK
 * @retval          OS_ERROR        wifi is not ready
 ***********************************************************************************************************************
 */
os_bool_t onepos_get_wifi_sta(void)
{
    return (MO_WIFI_STAT_CONNECTED == mo_wifi_get_stat(wifi_module));
}                                            
