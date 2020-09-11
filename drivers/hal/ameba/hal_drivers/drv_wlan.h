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
 * @file        drv_wlan.h
 *
 * @brief       The header file for wlan.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_WLAN_H__
#define __DRV_WLAN_H__

typedef enum
{
    HW_WIFI_MODE_NONE = 0,
    HW_WIFI_MODE_STA,
    HW_WIFI_MODE_AP,
    HW_WIFI_MODE_STA_AP,
    HW_WIFI_MODE_PROMISC,
    HW_WIFI_MODE_P2P
} hw_wifi_mode_t;

#define SHARED_ENABLED 0x00008000
#define WPA_SECURITY   0x00200000
#define WPA2_SECURITY  0x00400000
#define WPS_ENABLED    0x10000000
#define WEP_ENABLED    0x0001
#define TKIP_ENABLED   0x0002
#define AES_ENABLED    0x0004
#define WSEC_SWFLAG    0x0008

typedef enum {
    HW_WIFI_SECURITY_OPEN           = 0,                                                /**< Open security                           */
    HW_WIFI_SECURITY_WEP_PSK        = WEP_ENABLED,                                      /**< WEP Security with open authentication   */
    HW_WIFI_SECURITY_WEP_SHARED     = ( WEP_ENABLED | SHARED_ENABLED ),                 /**< WEP Security with shared authentication */
    HW_WIFI_SECURITY_WPA_TKIP_PSK   = ( WPA_SECURITY  | TKIP_ENABLED ),                 /**< WPA Security with TKIP                  */
    HW_WIFI_SECURITY_WPA_AES_PSK    = ( WPA_SECURITY  | AES_ENABLED ),                  /**< WPA Security with AES                   */
    HW_WIFI_SECURITY_WPA2_AES_PSK   = ( WPA2_SECURITY | AES_ENABLED ),                  /**< WPA2 Security with AES                  */
    HW_WIFI_SECURITY_WPA2_TKIP_PSK  = ( WPA2_SECURITY | TKIP_ENABLED ),                 /**< WPA2 Security with TKIP                 */
    HW_WIFI_SECURITY_WPA2_MIXED_PSK = ( WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED ),   /**< WPA2 Security with AES & TKIP           */
    HW_WIFI_SECURITY_WPA_WPA2_MIXED = ( WPA_SECURITY  | WPA2_SECURITY ),                /**< WPA/WPA2 Security                       */

    HW_WIFI_SECURITY_WPS_OPEN       = WPS_ENABLED,                                      /**< WPS with open security                  */
    HW_WIFI_SECURITY_WPS_SECURE     = (WPS_ENABLED | AES_ENABLED),                      /**< WPS with AES security                   */

    HW_WIFI_SECURITY_UNKNOWN        = -1,                                               /**< May be returned by scan function if security is unknown. Do not pass this to the join function! */

    HW_WIFI_SECURITY_FORCE_32_BIT   = 0x7fffffff                                        /**< Exists only to force rtw_security_t type to 32 bits */
} hw_wifi_security_t;

typedef enum {
    HW_WIFI_EVENT_CONNECT                = 0,
    HW_WIFI_EVENT_DISCONNECT             = 1,
    HW_WIFI_EVENT_FOURWAY_HANDSHAKE_DONE = 2,
    HW_WIFI_EVENT_SCAN_RESULT_REPORT     = 3,
    HW_WIFI_EVENT_SCAN_DONE              = 4,
    HW_WIFI_EVENT_RECONNECTION_FAIL      = 5,
    HW_WIFI_EVENT_SEND_ACTION_DONE       = 6,
    HW_WIFI_EVENT_RX_MGNT                = 7,
    HW_WIFI_EVENT_STA_ASSOC              = 8,
    HW_WIFI_EVENT_STA_DISASSOC           = 9,
    HW_WIFI_EVENT_STA_WPS_START          = 10,
    HW_WIFI_EVENT_WPS_FINISH             = 11,
    HW_WIFI_EVENT_EAPOL_START            = 12,
    HW_WIFI_EVENT_EAPOL_RECVD            = 13,
    HW_WIFI_EVENT_NO_NETWORK             = 14,
    HW_WIFI_EVENT_BEACON_AFTER_DHCP      = 15,
    HW_WIFI_EVENT_MAX,
} hw_wifi_event_indicate_t;

typedef enum {
    HW_WIFI_802_11_BAND_5GHZ   = 0, /* Denotes 5GHz radio band   */
    HW_WIFI_802_11_BAND_2_4GHZ = 1  /* Denotes 2.4GHz radio band */
} hw_wifi_802_11_band_t;

struct hw_wlan_info
{
    char                 *ssid;
    os_uint8_t           *bssid;
    hw_wifi_802_11_band_t band;
    os_uint32_t           datarate;
    os_uint16_t           channel;
    os_int16_t            rssi;
    hw_wifi_security_t    security;
};

typedef void (*scan_callback_fn)(struct hw_wlan_info *info, void *user_data);
typedef void (*hw_wlan_monitor_callback_t)(os_uint8_t *data, int len, void *user_data);

hw_wifi_mode_t hw_wifi_mode_get(void);
int            hw_wifi_stop(void);
int            hw_wifi_start(hw_wifi_mode_t mode);
int  hw_wifi_connect(char *ssid, int ssid_len, char *password, int pass_len, hw_wifi_security_t security_type);
int  hw_wifi_connect_bssid(char              *bssid,
                           char              *ssid,
                           int                ssid_len,
                           char              *password,
                           int                pass_len,
                           hw_wifi_security_t security_type);
int  hw_wifi_ap_start(char *ssid, char *password, int channel);
int  hw_wifi_rssi_get(void);
void hw_wifi_channel_set(int channel);
int  hw_wifi_channel_get(void);
int  hw_wifi_sta_disconnect(void);
int  hw_wifi_ap_disconnect(void);
int  hw_wifi_scan(scan_callback_fn fun, void *data);
void hw_wifi_monitor_enable(int enable);
void hw_wifi_monitor_callback_set(hw_wlan_monitor_callback_t callback);
#endif
