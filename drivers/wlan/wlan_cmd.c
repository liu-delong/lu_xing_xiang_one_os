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
 * @file        wlan_cmd.c
 *
 * @brief       wlan_cmd
 *
 * @details     wlan_cmd
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_task.h>
#include <wlan/wlan_mgnt.h>
#include <wlan/wlan_cfg.h>
#include <wlan/wlan_prot.h>
#include <os_memory.h>
#include <string.h>

struct wifi_cmd_des
{
    const char *cmd;
    int (*fun)(int argc, char *argv[]);
};

static int wifi_help(int argc, char *argv[]);
static int wifi_scan(int argc, char *argv[]);
static int wifi_status(int argc, char *argv[]);
static int wifi_join(int argc, char *argv[]);
static int wifi_ap(int argc, char *argv[]);
static int wifi_list_sta(int argc, char *argv[]);
static int wifi_disconnect(int argc, char *argv[]);
static int wifi_ap_stop(int argc, char *argv[]);

#ifdef OS_WLAN_CMD_DEBUG
/* just for debug  */
static int wifi_debug(int argc, char *argv[]);
static int wifi_debug_save_cfg(int argc, char *argv[]);
static int wifi_debug_dump_cfg(int argc, char *argv[]);
static int wifi_debug_clear_cfg(int argc, char *argv[]);
static int wifi_debug_dump_prot(int argc, char *argv[]);
static int wifi_debug_set_mode(int argc, char *argv[]);
static int wifi_debug_set_prot(int argc, char *argv[]);
static int wifi_debug_set_autoconnect(int argc, char *argv[]);
#endif

/* cmd table */
static const struct wifi_cmd_des cmd_tab[] =
{
    {"scan", wifi_scan},
    {"help", wifi_help},
    {"status", wifi_status},
    {"join", wifi_join},
    {"ap", wifi_ap},
    {"list_sta", wifi_list_sta},
    {"disc", wifi_disconnect},
    {"ap_stop", wifi_ap_stop},
    {"smartconfig", OS_NULL},
#ifdef OS_WLAN_CMD_DEBUG
    {"-d", wifi_debug},
#endif
};

#ifdef OS_WLAN_CMD_DEBUG
/* debug cmd table */
static const struct wifi_cmd_des debug_tab[] =
{
    {"save_cfg", wifi_debug_save_cfg},
    {"dump_cfg", wifi_debug_dump_cfg},
    {"clear_cfg", wifi_debug_clear_cfg},
    {"dump_prot", wifi_debug_dump_prot},
    {"mode", wifi_debug_set_mode},
    {"prot", wifi_debug_set_prot},
    {"auto", wifi_debug_set_autoconnect},
};
#endif

static int wifi_help(int argc, char *argv[])
{
    os_kprintf("wifi\n");
    os_kprintf("wifi help\n");
    os_kprintf("wifi scan [SSID]\n");
    os_kprintf("wifi join [SSID] [PASSWORD]\n");
    os_kprintf("wifi ap SSID [PASSWORD]\n");
    os_kprintf("wifi disc\n");
    os_kprintf("wifi ap_stop\n");
    os_kprintf("wifi status\n");
    os_kprintf("wifi smartconfig\n");
#ifdef OS_WLAN_CMD_DEBUG
    os_kprintf("wifi -d debug command\n");
#endif
    return 0;
}

static int wifi_status(int argc, char *argv[])
{
    int rssi;
    
    struct os_wlan_info info;

    if (argc > 2)
        return -1;

    if (os_wlan_is_connected() == 1)
    {
        rssi = os_wlan_get_rssi();
        os_wlan_get_info(&info);
        os_kprintf("Wi-Fi STA Info\n");
        os_kprintf("SSID : %-.32s\n", &info.ssid.val[0]);
        os_kprintf("MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   info.bssid[0],
                   info.bssid[1],
                   info.bssid[2],
                   info.bssid[3],
                   info.bssid[4],
                   info.bssid[5]);
        os_kprintf("Channel: %d\n", info.channel);
        os_kprintf("DataRate: %dMbps\n", info.datarate / 1000000);
        os_kprintf("RSSI: %d\n", rssi);
    }
    else
    {
        os_kprintf("wifi disconnected!\n");
    }

    if (os_wlan_ap_is_active() == 1)
    {
        os_wlan_ap_get_info(&info);
        os_kprintf("Wi-Fi AP Info\n");
        os_kprintf("SSID : %-.32s\n", &info.ssid.val[0]);
        os_kprintf("MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   info.bssid[0],
                   info.bssid[1],
                   info.bssid[2],
                   info.bssid[3],
                   info.bssid[4],
                   info.bssid[5]);
        os_kprintf("Channel: %d\n", info.channel);
        os_kprintf("DataRate: %dMbps\n", info.datarate / 1000000);
        os_kprintf("hidden: %s\n", info.hidden ? "Enable" : "Disable");
    }
    else
    {
        os_kprintf("wifi ap not start!\n");
    }
    os_kprintf("Auto Connect status:%s!\n", (os_wlan_get_autoreconnect_mode() ? "Enable" : "Disable"));
    return 0;
}

static int wifi_scan(int argc, char *argv[])
{
    struct os_wlan_scan_result *scan_result = OS_NULL;
    struct os_wlan_info *       info        = OS_NULL;
    struct os_wlan_info         filter;

    if (argc > 3)
        return -1;

    if (argc == 3)
    {
        INVALID_INFO(&filter);
        SSID_SET(&filter, argv[2]);
        info = &filter;
    }

    /* clean scan result */
    os_wlan_scan_result_clean();
    /* scan ap info */
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

static int wifi_join(int argc, char *argv[])
{
    const char  *ssid = OS_NULL;
    const char  *key  = OS_NULL;
    
    struct os_wlan_cfg_info cfg_info;

    memset(&cfg_info, 0, sizeof(cfg_info));
    
    if (argc == 2)
    {
#ifdef OS_WLAN_CFG_ENABLE
        /* get info to connect */
        if (os_wlan_cfg_read_index(&cfg_info, 0) == 1)
        {
            ssid = (char *)(&cfg_info.info.ssid.val[0]);
            if (cfg_info.key.len)
                key = (char *)(&cfg_info.key.val[0]);
        }
        else
#endif
        {
            os_kprintf("not find connect info\n");
        }
    }
    else if (argc == 3)
    {
        /* ssid */
        ssid = argv[2];
    }
    else if (argc == 4)
    {
        ssid = argv[2];
        /* password */
        key = argv[3];
    }
    else
    {
        return -1;
    }
    os_wlan_connect(ssid, key);
    return 0;
}

static int wifi_ap(int argc, char *argv[])
{
    const char *ssid = OS_NULL;
    const char *key  = OS_NULL;

    if (argc == 3)
    {
        ssid = argv[2];
    }
    else if (argc == 4)
    {
        ssid = argv[2];
        key  = argv[3];
    }
    else
    {
        return -1;
    }

    os_wlan_staos_ap(ssid, key);
    return 0;
}

static int wifi_list_sta(int argc, char *argv[])
{
    struct os_wlan_info *sta_info;
    int                  num, i;

    if (argc > 2)
        return -1;
    num      = os_wlan_ap_get_sta_num();
    sta_info = os_malloc(sizeof(struct os_wlan_info) * num);
    if (sta_info == OS_NULL)
    {
        os_kprintf("num:%d\n", num);
        return 0;
    }
    os_wlan_ap_get_sta_info(sta_info, num);
    os_kprintf("num:%d\n", num);
    for (i = 0; i < num; i++)
    {
        os_kprintf("sta mac  %02x:%02x:%02x:%02x:%02x:%02x\n",
                   sta_info[i].bssid[0],
                   sta_info[i].bssid[1],
                   sta_info[i].bssid[2],
                   sta_info[i].bssid[3],
                   sta_info[i].bssid[4],
                   sta_info[i].bssid[5]);
    }
    os_free(sta_info);
    return 0;
}

static int wifi_disconnect(int argc, char *argv[])
{
    if (argc != 2)
    {
        return -1;
    }

    os_wlan_disconnect();
    return 0;
}

static int wifi_ap_stop(int argc, char *argv[])
{
    if (argc != 2)
    {
        return -1;
    }

    os_wlan_ap_stop();
    return 0;
}

#ifdef OS_WLAN_CMD_DEBUG

static int wifi_debug_help(int argc, char *argv[])
{
    os_kprintf("save_cfg ssid [password]\n");
    os_kprintf("dump_cfg\n");
    os_kprintf("clear_cfg\n");
    os_kprintf("dump_prot\n");
    os_kprintf("mode sta/ap dev_name\n");
    os_kprintf("prot lwip dev_name\n");
    os_kprintf("auto enable/disable\n");
    return 0;
}

static int wifi_debug_save_cfg(int argc, char *argv[])
{
    struct os_wlan_cfg_info cfg_info;
    
    int     len;
    char   *ssid        = OS_NULL;
    char   *password    = OS_NULL;

    memset(&cfg_info, 0, sizeof(cfg_info));
    INVALID_INFO(&cfg_info.info);
    if (argc == 2)
    {
        ssid = argv[1];
    }
    else if (argc == 3)
    {
        ssid     = argv[1];
        password = argv[2];
    }
    else
    {
        return -1;
    }

    if (ssid != OS_NULL)
    {
        len = strlen(ssid);
        if (len > OS_WLAN_SSID_MAX_LENGTH)
        {
            os_kprintf("ssid is to long");
            return 0;
        }
        memcpy(&cfg_info.info.ssid.val[0], ssid, len);
        cfg_info.info.ssid.len = len;
    }

    if (password != OS_NULL)
    {
        len = strlen(password);
        if (len > OS_WLAN_PASSWORD_MAX_LENGTH)
        {
            os_kprintf("password is to long");
            return 0;
        }
        memcpy(&cfg_info.key.val[0], password, len);
        cfg_info.key.len = len;
    }
#ifdef OS_WLAN_CFG_ENABLE
    os_wlan_cfg_save(&cfg_info);
#endif
    return 0;
}

static int wifi_debug_dump_cfg(int argc, char *argv[])
{
    if (argc == 1)
    {
#ifdef OS_WLAN_CFG_ENABLE
        os_wlan_cfg_dump();
#endif
    }
    else
    {
        return -1;
    }
    return 0;
}

static int wifi_debug_clear_cfg(int argc, char *argv[])
{
    if (argc == 1)
    {
#ifdef OS_WLAN_CFG_ENABLE
        os_wlan_cfg_delete_all();
        os_wlan_cfg_cache_save();
#endif
    }
    else
    {
        return -1;
    }
    return 0;
}

static int wifi_debug_dump_prot(int argc, char *argv[])
{
    if (argc == 1)
    {
        os_wlan_prot_dump();
    }
    else
    {
        return -1;
    }
    return 0;
}

static int wifi_debug_set_mode(int argc, char *argv[])
{
    os_wlan_mode_t mode;

    if (argc != 3)
        return -1;

    if (strcmp("sta", argv[1]) == 0)
    {
        mode = OS_WLAN_STATION;
    }
    else if (strcmp("ap", argv[1]) == 0)
    {
        mode = OS_WLAN_AP;
    }
    else if (strcmp("none", argv[1]) == 0)
    {
        mode = OS_WLAN_NONE;
    }
    else
        return -1;

    os_wlan_set_mode(argv[2], mode);
    return 0;
}

static int wifi_debug_set_prot(int argc, char *argv[])
{
    if (argc != 3)
    {
        return -1;
    }

    os_wlan_prot_attach(argv[2], argv[1]);
    return 0;
}

static int wifi_debug_set_autoconnect(int argc, char *argv[])
{
    if (argc == 2)
    {
        if (strcmp(argv[1], "enable") == 0)
            os_wlan_config_autoreconnect(OS_TRUE);
        else if (strcmp(argv[1], "disable") == 0)
            os_wlan_config_autoreconnect(OS_FALSE);
    }
    else
    {
        return -1;
    }
    return 0;
}

static int wifi_debug(int argc, char *argv[])
{
    int i;
    int result = 0;
    
    const struct wifi_cmd_des *run_cmd = OS_NULL;

    if (argc < 3)
    {
        wifi_debug_help(0, OS_NULL);
        return 0;
    }

    for (i = 0; i < sizeof(debug_tab) / sizeof(debug_tab[0]); i++)
    {
        if (strcmp(debug_tab[i].cmd, argv[2]) == 0)
        {
            run_cmd = &debug_tab[i];
            break;
        }
    }

    if (run_cmd == OS_NULL)
    {
        wifi_debug_help(0, OS_NULL);
        return 0;
    }

    if (run_cmd->fun != OS_NULL)
    {
        result = run_cmd->fun(argc - 2, &argv[2]);
    }

    if (result)
    {
        wifi_debug_help(argc - 2, &argv[2]);
    }
    return 0;
}

#endif

#include <shell.h>

static int wifi_cmd(int argc, char *argv[])
{
    int i;
    int result = 0;
    
    const struct wifi_cmd_des *run_cmd = OS_NULL;

    if (argc == 1)
    {
        wifi_help(argc, argv);
        return 0;
    }

    /* find fun */
    for (i = 0; i < sizeof(cmd_tab) / sizeof(cmd_tab[0]); i++)
    {
        if (strcmp(cmd_tab[i].cmd, argv[1]) == 0)
        {
            run_cmd = &cmd_tab[i];
            break;
        }
    }

    /* not find fun, print help */
    if (run_cmd == OS_NULL)
    {
        wifi_help(argc, argv);
        return 0;
    }

    /* run fun */
    if (run_cmd->fun != OS_NULL)
    {
        result = run_cmd->fun(argc, argv);
    }

    if (result)
    {
        wifi_help(argc, argv);
    }
    return 0;
}

SH_CMD_EXPORT(wifi, wifi_cmd, "wifi command.");
