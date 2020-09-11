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
 * @file        wlan_test.c
 *
 * @brief       The test file for wlan.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <wlan_mgnt.h>
#include <wlan_prot.h>
#include <wlan_cfg.h>
#include <shell.h>
#include <string.h>

static int wifi_scan(int argc, char **argv)
{
    struct os_wlan_scan_result *scan_result = OS_NULL;
    struct os_wlan_info        *info        = OS_NULL;
    struct os_wlan_info         filter;

    if (argc > 3)
        return -1;

    if (argc == 3)
    {
        INVALID_INFO(&filter);
        SSID_SET(&filter, argv[2]);
        info = &filter;
    }

    /* Clean scan result */
    os_wlan_scan_result_clean();
    /* Scan ap info */
    scan_result = os_wlan_scan_with_info(info);
    if (scan_result)
    {
        int   index, num;
        char *security;

        num = scan_result->num;
        os_kprintf("             SSID                      MAC            security    rssi chn Mbps\n");
        os_kprintf("------------------------------- -----------------  -------------- ---- --- ----\n");
        for (index = 0; index < num; index++)
        {
            os_kprintf("%-32.32s", &scan_result->info[index].ssid.val[0]);
            os_kprintf("%02x:%02x:%02x:%02x:%02x:%02x  ",
                       scan_result->info[index].bssid[0],
                       scan_result->info[index].bssid[1],
                       scan_result->info[index].bssid[2],
                       scan_result->info[index].bssid[3],
                       scan_result->info[index].bssid[4],
                       scan_result->info[index].bssid[5]);
            switch (scan_result->info[index].security)
            {
            case SECURITY_OPEN:
                security = "OPEN";
                break;
            case SECURITY_WEP_PSK:
                security = "WEP_PSK";
                break;
            case SECURITY_WEP_SHARED:
                security = "WEP_SHARED";
                break;
            case SECURITY_WPA_TKIP_PSK:
                security = "WPA_TKIP_PSK";
                break;
            case SECURITY_WPA_AES_PSK:
                security = "WPA_AES_PSK";
                break;
            case SECURITY_WPA2_AES_PSK:
                security = "WPA2_AES_PSK";
                break;
            case SECURITY_WPA2_TKIP_PSK:
                security = "WPA2_TKIP_PSK";
                break;
            case SECURITY_WPA2_MIXED_PSK:
                security = "WPA2_MIXED_PSK";
                break;
            case SECURITY_WPS_OPEN:
                security = "WPS_OPEN";
                break;
            case SECURITY_WPS_SECURE:
                security = "WPS_SECURE";
                break;
            default:
                security = "UNKNOWN";
                break;
            }
            os_kprintf("%-14.14s ", security);
            os_kprintf("%-4d ", scan_result->info[index].rssi);
            os_kprintf("%3d ", scan_result->info[index].channel);
            os_kprintf("%4d\n", scan_result->info[index].datarate / 1000000);
        }
        os_wlan_scan_result_clean();
    }
    else
    {
        os_kprintf("wifi scan result is null\n");
    }

    return 0;
}

SH_CMD_EXPORT(wifi_scan, wifi_scan, "wifi_scan");

static int wifi_join(int argc, char **argv)
{
    const char *ssid = OS_NULL;
    const char *key = OS_NULL;
    struct os_wlan_cfg_info cfg_info;

    memset(&cfg_info, 0, sizeof(cfg_info));

    if (argc >= 2)
    {
        /* ssid */
        ssid = argv[1];
    }

    if (argc >= 3)
    {
        /* Password */
        key = argv[2];
    }

    os_wlan_connect(ssid, key);

    return 0;
}
SH_CMD_EXPORT(wifi_join, wifi_join, "wifi_join");

static int wifi_disconnect(int argc, char *argv[])
{
    os_wlan_disconnect();
    return 0;
}
SH_CMD_EXPORT(wifi_disconnect, wifi_disconnect, "wifi_disconnect");
