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
 * @file        esp8266_wifi.c
 *
 * @brief       esp8266 module link kit wifi api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp8266_wifi.h"
#include "esp8266.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DBG_EXT_TAG "esp8266.wifi"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define ESP8266_SCAN_RESP_BUFF_LEN 4096

#ifdef ESP8266_USING_WIFI_OPS

os_err_t esp8266_wifi_set_mode(mo_object_t *module, mo_wifi_mode_t mode)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[32] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_int8_t mode_data = 0;

    switch (mode)
    {
    case MO_WIFI_MODE_STA:
        mode_data = 1;
        break;
    case MO_WIFI_MODE_AP:
    case MO_WIFI_MODE_AP_STA:
    default:
        LOG_EXT_W("AP and AP&STA modes are not supported and AT instructions will not be executed");
        return OS_ERROR;
    }

    return at_parser_exec_cmd(parser, &resp, "AT+CWMODE_DEF=%d", mode_data);
}

mo_wifi_mode_t esp8266_wifi_get_mode(mo_object_t *module)
{
    at_parser_t   *parser    = &module->parser;
    os_int8_t      mode_data = 0;
    mo_wifi_mode_t wifi_mode = MO_WIFI_MODE_NULL;

    char resp_buff[32] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CWMODE_DEF?");
    if (result != OS_EOK)
    {
        goto __exit;
    }
    

    if (at_resp_get_data_by_kw(&resp, "+CWMODE_DEF:", "+CWMODE_DEF:%d", &mode_data) < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }
    

    switch (mode_data)
    {
    case 1:
        wifi_mode = MO_WIFI_MODE_STA;
        break;
    
    default:
        break;
    }

__exit:
    if (result != OS_EOK)
    {
        return MO_WIFI_MODE_NULL;
    }
    
    return wifi_mode;
}

mo_wifi_stat_t esp8266_wifi_get_stat(mo_object_t *module)
{
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    return esp8266->wifi_stat;
}

os_err_t esp8266_wifi_scan_info(mo_object_t *module, char *ssid, mo_wifi_scan_result_t *scan_result)
{
    at_parser_t *parser   = &module->parser;
    os_err_t     result   = OS_EOK;
	os_int32_t   ecn_mode = 0;

    const char *data_format1 = "+CWLAP:(%d,\"%[^\"]\",%d,\"%[^\"]\",%d,%*s)";
    const char *data_format2 = "+CWLAP:(%*d,\"\",%d,\"%[^\"]\",%d,%*s)";

    at_resp_t resp = {.buff      = calloc(1, ESP8266_SCAN_RESP_BUFF_LEN),
                      .buff_size = ESP8266_SCAN_RESP_BUFF_LEN,
                      .timeout   = 10 * OS_TICK_PER_SECOND};

    if (OS_NULL == resp.buff)
    {
        LOG_EXT_E("Calloc wifi scan info response memory failed!");
        result = OS_ENOMEM;
        goto __exit;
    }

    if (OS_NULL != ssid)
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CWLAP=\"%s\"", ssid);
        if (result != OS_EOK)
        {
            goto __exit;
        }
    }
    else
    {
        /* scan all ap list */
        result = at_parser_exec_cmd(parser, &resp, "AT+CWLAP");
        if (result != OS_EOK)
        {
            goto __exit;
        }
    }

    scan_result->info_array = (mo_wifi_info_t *)calloc(resp.line_counts, sizeof(mo_wifi_info_t));
    if (OS_NULL == scan_result->info_array)
    {
        LOG_EXT_E("Calloc wifi scan info memory failed!");
        result = OS_ENOMEM;
        goto __exit;
    }

    scan_result->info_num = 0;

    for (int i = 0; i < resp.line_counts; i++)
    {
        mo_wifi_info_t *tmp = &scan_result->info_array[i];

        os_int32_t get_result = at_resp_get_data_by_line(&resp,
                                                         i + 1,
                                                         data_format1,
                                                         &ecn_mode,
                                                         tmp->ssid.val,
                                                         &tmp->rssi,
                                                         tmp->bssid.bssid_str,
                                                         &tmp->channel);

        if (1 == get_result)
        {
            /* ssid is null  */
            get_result = at_resp_get_data_by_line(&resp, 
                                                  i + 1,
                                                  data_format2,
                                                  &tmp->rssi,
                                                  tmp->bssid.bssid_str,
                                                  &tmp->channel);
        }

        if (get_result > 0)
        {
            switch (ecn_mode)
            {
            case 0:
                tmp->ecn_mode = MO_WIFI_ECN_OPEN;
                break;
            case 1:
                tmp->ecn_mode = MO_WIFI_ECN_WEP;
                break;
            case 2:
                tmp->ecn_mode = MO_WIFI_ECN_WPA_PSK;
                break;
            case 3:
                tmp->ecn_mode = MO_WIFI_ECN_WPA2_PSK;
                break;
            case 4:
                tmp->ecn_mode = MO_WIFI_ECN_WPA_WPA2_PSK;
                break;
            default:
                tmp->ecn_mode = MO_WIFI_ECN_NULL;
                break;
            }

            sscanf(tmp->bssid.bssid_str,
                   "%2x:%2x:%2x:%2x:%2x:%2x",
                   (os_int32_t *)&tmp->bssid.bssid_array[0],
                   (os_int32_t *)&tmp->bssid.bssid_array[1],
                   (os_int32_t *)&tmp->bssid.bssid_array[2],
                   (os_int32_t *)&tmp->bssid.bssid_array[3],
                   (os_int32_t *)&tmp->bssid.bssid_array[4],
                   (os_int32_t *)&tmp->bssid.bssid_array[5]);

            tmp->ssid.len = strlen(tmp->ssid.val);
            scan_result->info_num++;
        }
    }

__exit:
    if (result != OS_EOK)
    {
        if (scan_result->info_array != OS_NULL)
        {
            free(scan_result->info_array);
        }
    }

    if (resp.buff != OS_NULL)
    {
        free(resp.buff);
    }

    return result;
}

void esp8266_wifi_scan_info_free(mo_wifi_scan_result_t *scan_result)
{
    if (scan_result->info_array != OS_NULL)
    {
        free(scan_result->info_array);
    }
}

os_err_t esp8266_wifi_connect_ap(mo_object_t *module, const char *ssid, const char *password)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s connect ap failed, check ssid(%s) and password(%s).", module->name, ssid, password);
    }

    return result;
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != data);

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    if (strstr(data, "WIFI CONNECTED"))
    {
        esp8266->wifi_stat = MO_WIFI_STAT_CONNECTED;
        LOG_EXT_I("ESP8266 WIFI is connected.");
    }
    else if (strstr(data, "WIFI DISCONNECT"))
    {
        esp8266->wifi_stat = MO_WIFI_STAT_DISCONNECTED;
        LOG_EXT_I("ESP8266 WIFI is disconnect.");
    }
}

static void urc_ip_func(struct at_parser *parser, const char *data, os_size_t size)
{
    LOG_EXT_D("ESP8266 WIFI is get ip");
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "WIFI CONNECTED",  .suffix = "\r\n", .func = urc_connect_func},
    {.prefix = "WIFI DISCONNECT", .suffix = "\r\n", .func = urc_connect_func},
    {.prefix = "WIFI GOT IP",     .suffix = "\r\n", .func = urc_ip_func},
};

os_err_t esp8266_wifi_init(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    return OS_EOK;
}

#endif /* ESP8266_USING_WIFI_OPS */
