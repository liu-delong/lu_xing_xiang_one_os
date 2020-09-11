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
 * @file        drv_wlan.c
 *
 * @brief       The driver file for wlan.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "wifi_structures.h"
#include "wifi_constants.h"
#include <wifi/wifi_util.h>
#include <wifi/wifi_conf.h>
#include "board.h"
#include "drv_wlan.h"

struct scan_user_data
{
    struct os_completion done;
    scan_callback_fn     fun;
    void *               data;
};

extern hw_wifi_mode_t wifi_mode;

hw_wifi_mode_t hw_wifi_mode_get(void)
{
    return wifi_mode;
}

int hw_wifi_stop(void)
{
    return wifi_off();
}

int hw_wifi_start(hw_wifi_mode_t mode)
{
    if (wifi_on(mode) < 0)
    {
        os_kprintf("ERROR: wifi_on failed\n");
        return -1;
    }
    return 0;
}

int hw_wifi_connect(char *ssid, int ssid_len, char *password, int pass_len, hw_wifi_security_t security_type)
{
    int                mode;
    rtw_wifi_setting_t setting;

    mode = hw_wifi_mode_get();
    if ((mode != HW_WIFI_MODE_STA) && (mode != HW_WIFI_MODE_STA_AP))
    {
        return -1;
    }
    if (wext_get_mode(WLAN0_NAME, &mode) < 0)
    {
        os_kprintf("L:%d wifi get mode err\n", __LINE__);
        return -1;
    }
    if (wifi_connect(ssid, security_type, password, ssid_len, pass_len, -1, NULL) != RTW_SUCCESS)
    {
        os_kprintf("wifi connect fail\n");
        return -1;
    }
    os_kprintf("wifi connect success\n");
    os_kprintf("Show Wi-Fi information\n");
    wifi_get_setting(WLAN0_NAME, &setting);
    wifi_show_setting(WLAN0_NAME, &setting);
    return 0;
}

int hw_wifi_connect_bssid(char              *bssid,
                          char              *ssid,
                          int                ssid_len,
                          char              *password,
                          int                pass_len,
                          hw_wifi_security_t security_type)
{
    int                mode;
    rtw_wifi_setting_t setting;

    mode = hw_wifi_mode_get();
    if ((mode != HW_WIFI_MODE_STA) && (mode != HW_WIFI_MODE_STA_AP))
    {
        return -1;
    }
    if (wext_get_mode(WLAN0_NAME, &mode) < 0)
    {
        os_kprintf("L:%d wifi get mode err\n", __LINE__);
        return -1;
    }

    if (wifi_connect_bssid(bssid, ssid, security_type, password, 6, ssid_len, pass_len, -1, NULL) != RTW_SUCCESS)
    {
        os_kprintf("wifi connect fail\n");
        return -1;
    }
    os_kprintf("wifi connect success\n");
    os_kprintf("Show Wi-Fi information\n");
    wifi_get_setting(WLAN0_NAME, &setting);
    wifi_show_setting(WLAN0_NAME, &setting);
    return 0;
}

int hw_wifi_ap_start(char *ssid, char *password, int channel)
{
    int            mode = 0, timeout = 20;
    rtw_security_t security_type = RTW_SECURITY_WPA2_AES_PSK;
    char *         name          = OS_NULL;

    mode = hw_wifi_mode_get();
    if (mode == HW_WIFI_MODE_AP)
    {
        name = WLAN0_NAME;
    }
    else if (mode == HW_WIFI_MODE_STA_AP)
    {
        name = WLAN1_NAME;
    }
    else
    {
        return -1;
    }

    if (wext_get_mode(name, &mode) < 0)
    {
        os_kprintf("L:%d wifi get mode err\n", __LINE__);
        return -1;
    }
    if (password == OS_NULL)
    {
        security_type = RTW_SECURITY_OPEN;
    }

    if (wifi_start_ap(ssid, security_type, password, os_strlen(ssid), os_strlen(password), 6) != 0)
    {
        os_kprintf("ERROR: wifi_start_ap failed\n");
        return -1;
    }

    while (1)
    {
        char essid[33];
        if (wext_get_ssid(name, (unsigned char *)essid) > 0)
        {
            if (strcmp((const char *)essid, (const char *)ssid) == 0)
            {
                os_kprintf("%s started\n", ssid);
                break;
            }
        }
        if (timeout == 0)
        {
            os_kprintf("Start AP timeout\n");
            return -1;
        }
        os_task_delay(1 * OS_TICK_PER_SECOND);
        timeout--;
    }

    return 0;
}

static int hw_wifi_disconnect(char *name)
{
    char    essid[33];
    int     timeout = 20;
    
    const os_uint8_t null_bssid[ETH_ALEN + 2] = {0, 0, 0, 0, 0, 1, 0, 0};

    if (name == OS_NULL)
        return -1;

    if (wext_get_ssid(name, (unsigned char *)essid) < 0)
    {
        os_kprintf("\nWIFI disconnected!\n");
        return -1;
    }

    if (wext_set_bssid(name, null_bssid) < 0)
    {
        os_kprintf("Failed to set bogus BSSID to disconnect");
        return -1;
    }

    while (1)
    {
        if (wext_get_ssid(name, (unsigned char *)essid) < 0)
        {
            os_kprintf("WIFI disconnected!\n");
            break;
        }

        if (timeout == 0)
        {
            os_kprintf("ERROR: Deassoc timeout!\n");
            return -1;
        }

        os_task_delay(10);
        timeout--;
    }
    return 0;
}

int hw_wifi_sta_disconnect(void)
{
    int   mode = 0;
    char *name = OS_NULL;

    mode = hw_wifi_mode_get();
    if (mode == HW_WIFI_MODE_STA)
    {
        name = WLAN0_NAME;
    }
    else if (mode == HW_WIFI_MODE_STA_AP)
    {
        name = WLAN0_NAME;
    }
    else
    {
        return -1;
    }
    return hw_wifi_disconnect(name);
}

int hw_wifi_ap_disconnect(void)
{
    int   mode = 0;
    char *name = OS_NULL;

    mode = hw_wifi_mode_get();
    if (mode == HW_WIFI_MODE_AP)
    {
        name = WLAN0_NAME;
    }
    else if (mode == HW_WIFI_MODE_STA_AP)
    {
        name = WLAN1_NAME;
    }
    else
    {
        return -1;
    }
    return hw_wifi_disconnect(name);
}

int hw_wifi_rssi_get(void)
{
    int rssi = 0;
    wifi_get_rssi(&rssi);
    return rssi;
}

void hw_wifi_channel_set(int channel)
{
    wifi_set_channel(channel);
}

int hw_wifi_channel_get(void)
{
    int channel;
    wifi_get_channel(&channel);
    return channel;
}

static rtw_result_t hw_wifi_scan_result_handler(rtw_scan_handler_result_t *malloced_scan_result)
{
    struct scan_user_data *user_data = malloced_scan_result->user_data;
    struct hw_wlan_info    info      = {0};

    if (malloced_scan_result->scan_complete != RTW_TRUE)
    {
        rtw_scan_result_t *record = &malloced_scan_result->ap_details;

        if (user_data->fun != OS_NULL)
        {
            record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */
            info.ssid                          = record->SSID.val;
            info.bssid                         = record->BSSID.octet;
            info.band                          = record->band;
            info.datarate                      = 0;
            info.channel                       = record->channel;
            info.rssi                          = record->signal_strength;
            info.security                      = record->security;
            user_data->fun(&info, user_data->data);
        }
    }
    else
    {
        user_data->fun(OS_NULL, user_data->data);
        os_free(user_data);
        os_kprintf("scan ap down\n");
    }

    return RTW_SUCCESS;
}

int hw_wifi_scan(scan_callback_fn fun, void *data)
{
    struct scan_user_data *user_data;

    if (fun == OS_NULL)
    {
        os_kprintf("scan callback fun is null\n");
        return -1;
    }
    user_data = os_malloc(sizeof(struct scan_user_data));
    if (user_data == OS_NULL)
    {
        os_kprintf("wifi scan malloc fail\n");
        return -1;
    }
    user_data->fun  = fun;
    user_data->data = data;

    if (wifi_scan_networks(hw_wifi_scan_result_handler, user_data) != RTW_SUCCESS)
    {
        os_kprintf("ERROR: wifi scan failed\n\r");
        return -1;
    }

    return 0;
}

static hw_wlan_monitor_callback_t monitor_callback;

static void hw_wifi_promisc_callback(unsigned char *buf, unsigned int len, void *userdata)
{
    if (monitor_callback)
    {
        monitor_callback(userdata, len, OS_NULL);
    }
}

void hw_wifi_monitor_callback_set(hw_wlan_monitor_callback_t callback)
{
    monitor_callback = callback;
}

void hw_wifi_monitor_enable(int enable)
{
    if (enable)
    {
        wifi_enter_promisc_mode();
        wifi_set_promisc(RTW_PROMISC_ENABLE, hw_wifi_promisc_callback, 1);
        os_kprintf("%s run \n", __FUNCTION__);
    }
    else
    {
        wifi_set_promisc(RTW_PROMISC_DISABLE, OS_NULL, 0);
        hw_wifi_stop();
    }
}
