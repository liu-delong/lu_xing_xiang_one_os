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
 * @file        wlan_mgnt.h
 *
 * @brief       wlan_mgnt
 *
 * @details     wlan_mgnt
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __WLAN_MGNT_H__
#define __WLAN_MGNT_H__

#include <wlan/wlan_dev.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OS_WLAN_SCAN_WAIT_MS
#define OS_WLAN_SCAN_WAIT_MS (10 * 1000)
#endif

#ifndef OS_WLAN_SCAN_CACHE_NUM
#define OS_WLAN_SCAN_CACHE_NUM (50)
#endif

#ifndef OS_WLAN_CONNECT_WAIT_MS
#define OS_WLAN_CONNECT_WAIT_MS (10 * 1000)
#endif

#ifndef OS_WLAN_STAOS_AP_WAIT_MS
#define OS_WLAN_STAOS_AP_WAIT_MS (10 * 1000)
#endif

#ifndef OS_WLAN_EBOX_NUM
#define OS_WLAN_EBOX_NUM (10)
#endif

#ifndef OS_WLAN_SCAN_RETRY_CNT
#define OS_WLAN_SCAN_RETRY_CNT (3)
#endif

#ifndef AUTO_CONNECTION_PERIOD_MS
#define AUTO_CONNECTION_PERIOD_MS (2000)
#endif

/* state fot station */
#define OS_WLAN_STATE_CONNECT    (1UL << 0)
#define OS_WLAN_STATE_CONNECTING (1UL << 1)
#define OS_WLAN_STATE_READY      (1UL << 2)
#define OS_WLAN_STATE_POWERSAVE  (1UL << 3)

/* flags fot station */
#define OS_WLAN_STATE_AUTOEN (1UL << 0)

/* state fot ap */
#define OS_WLAN_STATE_ACTIVE (1UL << 0)

typedef enum
{
    OS_WLAN_EVT_READY = 0,          /* connect and prot is ok, You can send data*/
    OS_WLAN_EVT_SCAN_DONE,          /* Scan a info */
    OS_WLAN_EVT_SCAN_REPORT,        /* Scan end */
    OS_WLAN_EVT_STA_CONNECTED,      /* connect success */
    OS_WLAN_EVT_STA_CONNECTED_FAIL, /* connection failed */
    OS_WLAN_EVT_STA_DISCONNECTED,   /* disconnect */
    OS_WLAN_EVT_AP_START,           /* AP start */
    OS_WLAN_EVT_AP_STOP,            /* AP stop */
    OS_WLAN_EVT_AP_ASSOCIATED,      /* sta associated */
    OS_WLAN_EVT_AP_DISASSOCIATED,   /* sta disassociated */
    OS_WLAN_EVT_MAX
} os_wlan_event_t;

typedef void (*os_wlan_event_handler)(int event, struct os_wlan_buff *buff, void *parameter);

struct os_wlan_scan_result
{
    os_int32_t           num;
    struct os_wlan_info *info;
};

/*********************************************wifi init interface begin***********************************************/
int            os_wlan_init(void);
os_err_t       os_wlan_set_mode(const char *dev_name, os_wlan_mode_t mode);
os_wlan_mode_t os_wlan_get_mode(const char *dev_name);
/**********************************************wifi init interface end************************************************/

/****************************************wifi station mode interface begin********************************************/
os_err_t  os_wlan_connect(const char *ssid, const char *password);
os_err_t  os_wlan_connect_adv(struct os_wlan_info *info, const char *password);
os_err_t  os_wlan_disconnect(void);
os_bool_t os_wlan_is_connected(void);
os_bool_t os_wlan_is_ready(void);
os_err_t  os_wlan_set_mac(os_uint8_t *mac);
os_err_t  os_wlan_get_mac(os_uint8_t *mac);
os_err_t  os_wlan_get_info(struct os_wlan_info *info);
int       os_wlan_get_rssi(void);
/*****************************************wifi station mode interface end*********************************************/

/*******************************************wifi ap mode interface begin**********************************************/
os_err_t          os_wlan_staos_ap(const char *ssid, const char *password);
os_err_t          os_wlan_staos_ap_adv(struct os_wlan_info *info, const char *password);
os_bool_t         os_wlan_ap_is_active(void);
os_err_t          os_wlan_ap_stop(void);
os_err_t          os_wlan_ap_get_info(struct os_wlan_info *info);
int               os_wlan_ap_get_sta_num(void);
int               os_wlan_ap_get_sta_info(struct os_wlan_info *info, int num);
os_err_t          os_wlan_ap_deauth_sta(os_uint8_t *mac);
os_err_t          os_wlan_ap_set_country(os_country_code_t country_code);
os_country_code_t os_wlan_ap_get_country(void);
/********************************************wifi ap mode interface end***********************************************/

/*********************************************wifi scan interface begin***********************************************/
os_err_t                    os_wlan_scan(void);
struct os_wlan_scan_result *os_wlan_scan_sync(void);
struct os_wlan_scan_result *os_wlan_scan_with_info(struct os_wlan_info *info);
int                         os_wlan_scan_get_info_num(void);
int                         os_wlan_scan_get_info(struct os_wlan_info *info, int num);
struct os_wlan_scan_result *os_wlan_scan_get_result(void);
void                        os_wlan_scan_result_clean(void);
int                         os_wlan_scan_find_cache(struct os_wlan_info *info, struct os_wlan_info *out_info, int num);
os_bool_t                   os_wlan_find_best_by_cache(const char *ssid, struct os_wlan_info *info);
/**********************************************wifi scan interface end************************************************/

/*****************************************wifi auto connect interface begin*******************************************/
void      os_wlan_config_autoreconnect(os_bool_t enable);
os_bool_t os_wlan_get_autoreconnect_mode(void);
/******************************************wifi auto connect interface end********************************************/

/***************************************wifi power management interface begin*****************************************/
os_err_t os_wlan_set_powersave(int level);
int      os_wlan_get_powersave(void);
/****************************************wifi power management interface end******************************************/

/***************************************wifi event management interface begin*****************************************/
os_err_t os_wlan_register_event_handler(os_wlan_event_t event, os_wlan_event_handler handler, void *parameter);
os_err_t os_wlan_unregister_event_handler(os_wlan_event_t event);
/****************************************wifi event management interface end******************************************/

/***************************************wifi management lock interface begin******************************************/
void os_wlan_mgnt_lock(void);
void os_wlan_mgnt_unlock(void);
/****************************************wifi management lock interface end*******************************************/

#ifdef __cplusplus
}
#endif

#endif
