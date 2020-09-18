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
 * @file        onepos_cell_loca.c
 *
 * @brief       cell^s location of onepos
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <os_assert.h>
#include <mo_api.h>
#include "onepos_src_info.h"

#define DBG_EXT_TAG "onepos.cell"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

static mo_object_t *cell_module = OS_NULL;

#if defined(ONEPOS_DEVICE_REC_POS_INFO)
/**
 ***********************************************************************************************************************
 * @brief           parse multiple cell position string
 *
 * @param[in]       ops_platform_lbs_info_t       to save the parse result
 * @param[out]      data_item                     position info(json format)
 *
 * @return          parse result
 * @retval          OS_EOK          parse is successful
 * @retval          OS_ERROR        paese is failed
 ***********************************************************************************************************************
 */
os_err_t onepos_parse_mul_cell_pos(ops_platform_lbs_info_t *mul_cell_info, cJSON *data_item)
{
    os_err_t ret = OS_EOK;
    cJSON *item = OS_NULL;
    static os_uint64_t latest_time = 0;
    
    if(mul_cell_info && data_item)
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
            
            mul_cell_info->time = (os_uint32_t)(item->valuedouble / 1000);
        }
        item = cJSON_GetObjectItem(data_item, "pos");
        if(item)
        {
            sscanf(item->valuestring, "%lf,%lf", &mul_cell_info->lat_coordinate,
                                               &mul_cell_info->lon_coordinate);
        }

        if(!IS_VAILD_LAT(mul_cell_info->lat_coordinate))
        {
            LOG_EXT_D("error lat_coordinate");
            return OS_ERROR;
        }

        if(!IS_VAILD_LON(mul_cell_info->lon_coordinate))
        {
            LOG_EXT_D("error lon_coordinate");
            return OS_ERROR;
        }
        
        #ifdef ONEPOS_DEBUG
        printf("\nmul_cell : at -> %d lat -> %.12lf lon -> %.12lf\n", mul_cell_info->time,
                                                             mul_cell_info->lat_coordinate,
                                                             mul_cell_info->lon_coordinate);
        #endif
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
 * @brief           clean onepos cell info 
 *
 * @param[in]       onepos_cell_info       opepos cell info
 ***********************************************************************************************************************
 */
void onepos_cell_info_clean(onepos_cell_info_t *onepos_cell_info)
{
    os_free(onepos_cell_info->cell_info);
    onepos_cell_info->cell_num =0;
    onepos_cell_info->cell_info = OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           creat onepos cell loca publish message 
 *
 * @param[in]       onepos_cell_info        scanned wifi information 
 * @param[in]       pos_type                onepos position type 
 * @param[in]       sev_pro                 position service provider to invoke
 *
 * @return          json format data
 * @retval          cJSON           json format message
 ***********************************************************************************************************************
 */
static cJSON *onepos_cell_loca_pub_message(onepos_cell_info_t *onepos_cell_info, 
                                            onepos_pos_mode_t pos_type, 
                                            onepos_sev_pro_t sev_pro)
{
    char        *cell_list_str  = OS_NULL;
    cJSON       *ret_msg        = OS_NULL;
    os_uint32_t  cell_list_len  = 0;
    os_uint32_t  index          = 0;
    os_uint32_t  i              =0;
    
    if((OS_NULL == onepos_cell_info) || (0 == onepos_cell_info->cell_num))
    {
        LOG_EXT_E("input param is error");
        return (cJSON*)OS_NULL;
    }
    cell_list_len = (onepos_cell_info->cell_num * ONEPOS_CELL_INFO_LEN)
                   + (onepos_cell_info->cell_num - 1) * strlen(ONEPOS_MSG_SEPARATOR) + 1;
    cell_list_str = (char*)os_malloc(cell_list_len);
    if(cell_list_str)
    {
        
        for (i = 0; i < (onepos_cell_info->cell_num - 1); i++)
        {
            index += os_snprintf(cell_list_str + index,
                                 (cell_list_len - index),
                                 ONEPOS_CELL_MSG_FORMAT,
                                 onepos_cell_info->cell_info[i].mcc,
                                 onepos_cell_info->cell_info[i].mnc,
                                 onepos_cell_info->cell_info[i].lac,
                                 onepos_cell_info->cell_info[i].cid,
                                 onepos_cell_info->cell_info[i].ss);
            index += os_snprintf(cell_list_str + index,
                                 (cell_list_len - index),
                                 "%s",
                                 ONEPOS_MSG_SEPARATOR);
        }
        os_snprintf(cell_list_str + index,
                                 (cell_list_len - index),
                                 ONEPOS_CELL_MSG_FORMAT,
                                 onepos_cell_info->cell_info[i].mcc,
                                 onepos_cell_info->cell_info[i].mnc,
                                 onepos_cell_info->cell_info[i].lac,
                                 onepos_cell_info->cell_info[i].cid,
                                 onepos_cell_info->cell_info[i].ss);
        
        ret_msg = cJSON_CreateObject();
        cJSON_AddItemToObject(ret_msg, "cell_list", cJSON_CreateString(cell_list_str));
        cJSON_AddItemToObject(ret_msg, "pos_type", cJSON_CreateNumber(pos_type));
        cJSON_AddItemToObject(ret_msg, "sev_pro", cJSON_CreateNumber(sev_pro));
        cJSON_AddItemToObject(ret_msg, "net_type", cJSON_CreateNumber(onepos_cell_info->net_type));
        
        os_free(cell_list_str);
        return ret_msg;
    }
    else
    {
        LOG_EXT_E("malloc onepos pulish massage is NULL");
        return (cJSON*)OS_NULL;
    }
}


/**
 ***********************************************************************************************************************
 * @brief           creat pulish message to multiple cell joint positioning 
 *
 * @param[in]       sev_pro       service provider of using
 *
 * @return          message string
 ***********************************************************************************************************************
 */
char *mul_cells_pos_pub_msg(onepos_sev_pro_t sev_pro)
{
    onepos_cell_info_t onepos_cell_info = {0,};
    char  *msg_str = OS_NULL;
    cJSON *msg_json = OS_NULL;
        
    if(OS_EOK == mo_get_cell_info(cell_module, &onepos_cell_info))
    {
        msg_json = onepos_cell_loca_pub_message(&onepos_cell_info, 
                                                ONEPOS_MUL_CELL_JOINT_POS,
                                                sev_pro);
        onepos_cell_info_clean(&onepos_cell_info);
        msg_str = cJSON_Print(msg_json);
        cJSON_Delete(msg_json);
    }
    else
    {
        LOG_EXT_E("get cell info is ERROR!");
    }
    return msg_str;
}

/**
 ***********************************************************************************************************************
 * @brief           init cell module to work
 *
 * @return          os_err_t
 * @retval          OS_EOK         init cell modules is successful
 * @retval          OS_ERROR       init cell modules is failed
 ***********************************************************************************************************************
 */
os_err_t init_onepos_cell_device(void)
{ 
    // uint8_t stat = MO_NET_UNDEFINE_STATE;
    cell_module = mo_object_get_by_name(ONEPOS_CELL_DEVICE_NAME);
    /* Check param */
    if(OS_NULL == cell_module)
    {
        LOG_EXT_E("onepos device : %s is error", ONEPOS_CELL_DEVICE_NAME);
        return OS_ERROR;
    }

    /*
    mo_get_netstat(cell_module, &stat);
    if (stat != MO_NET_NETWORK_REG_OK)
    {
        LOG_EXT_E("onepos device : %s is not register network OK", ONEPOS_CELL_DEVICE_NAME);
        return OS_ERROR;
    }
    */
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           get cell module status
 *
 * @return          os_bool_t
 * @retval          OS_TRUE         cell is OK
 * @retval          OS_ERROR        cell is not ready
 ***********************************************************************************************************************
 */
os_bool_t onepos_get_cell_sta(void)
{
    char ip[30] = {0,};
    
    if (OS_EOK != mo_get_ipaddr(cell_module, ip))
    {
        return OS_FALSE;
    }
    else
    {
        return OS_TRUE;
    }
}
