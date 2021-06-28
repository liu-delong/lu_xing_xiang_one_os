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
 * @brief           creat onepos wifi loca publish message
 *
 * @param[in]       wifi_info       scanned wifi information
 * @param[in]       json_src        json data for add
 *
 * @return          json format data
 * @retval          os_err_t        error code
 ***********************************************************************************************************************
 */
static os_err_t onepos_wifi_loca_pub_message(mo_wifi_scan_result_t *wifi_info, cJSON* json_src)
{
    char       *macs_str     = OS_NULL;
    os_uint32_t macs_str_len = 0;
    os_uint32_t index        = 0;
    os_uint32_t i            = 0;

    if ((OS_NULL == wifi_info) || (0 == wifi_info->info_num) || OS_NULL == json_src)
    {
        LOG_EXT_E("input param is error");
        return OS_ERROR;
    }
    macs_str_len =
        (wifi_info->info_num * ONEPOS_WIFI_INFO_LEN) + (wifi_info->info_num - 1) * strlen(ONEPOS_MSG_SEPARATOR) + 1;
    macs_str = (char *)os_malloc(macs_str_len);
    if (macs_str)
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
            index += os_snprintf(macs_str + index, (macs_str_len - index), "%s", ONEPOS_MSG_SEPARATOR);
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

        cJSON_AddItemToObject(json_src, "macs", cJSON_CreateString(macs_str));

        os_free(macs_str);
    }
    else
    {
        LOG_EXT_E("malloc onepos pulish massage is NULL");
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           creat pulish message to multiple wifi joint positioning
 *
 * @param[in]       json_src        json data for add
 *
 * @return          message string
 ***********************************************************************************************************************
 */
os_err_t wifi_pos_pub_msg(cJSON* json_src)
{
    os_err_t              ret         = OS_EOK;
    mo_wifi_scan_result_t scan_result = {0,};

    if (OS_EOK == mo_wifi_scan_info(wifi_module, OS_NULL, &scan_result))
    {
        ret = onepos_wifi_loca_pub_message(&scan_result, json_src);
        mo_wifi_scan_info_free(&scan_result);
    }
    else
    {
        LOG_EXT_E("scan wifi result is NULL!");
        ret = OS_EIO;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           get wifi station status
 *
 * @return          os_bool_t
 * @retval          OS_TRUE         wifi is OK
 * @retval          OS_ERROR        wifi is not ready
 ***********************************************************************************************************************
 */
os_bool_t onepos_get_wifi_sta(void)
{
    return (MO_WIFI_STAT_CONNECTED == mo_wifi_get_stat(wifi_module));
}
