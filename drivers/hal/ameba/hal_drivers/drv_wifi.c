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
 * @file        drv_wifi.c
 *
 * @brief       The driver file for wifi.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <skbuff.h>
#include "board.h"
#include <string.h>
#include "drv_wlan.h"
#include "drv_wifi.h"
#include "drv_log.h"
#include "wlan_dev.h"

#define MAX_ADDR_LEN         (6)
#define HW_WLAN_SSID_MAX_LEN (32)

#define WIFI_INIT_FLAG (0x1 << 0)
#define WIFI_MAC_FLAG  (0x1 << 1)
#define WIFI_TYPE_STA  (0)
#define WIFI_TYPE_AP   (1)

#define OS_WLAN_DEVICE_STA_NAME "sta0"
#define OS_WLAN_DEVICE_AP_NAME  "ap0"
struct ameba_wifi
{
    struct os_wlan_device *wlan;
    os_uint8_t             dev_addr[MAX_ADDR_LEN];
    os_uint8_t             flag;
    int                    connected;
    int                    type;
};

extern unsigned char   rltk_wlan_running(unsigned char idx);
extern struct sk_buff *rltk_wlan_get_recv_skb(int idx);
extern unsigned char   rltk_wlan_check_isup(int idx);
extern void            rltk_wlan_tx_inc(int idx);
extern struct sk_buff *rltk_wlan_alloc_skb(unsigned int total_len);
extern void            rltk_wlan_send_skb(int idx, struct sk_buff *skb);
extern void            rltk_wlan_tx_dec(int idx);
extern void rtw_event_register(int event, void (*fun)(char *buf, int len, int flags, void *user_data), void *user_data);

static const struct os_wlan_dev_ops ops;

static struct ameba_wifi wifi_sta;
static struct ameba_wifi wifi_ap;

OS_INLINE struct ameba_wifi *hw_wifi_get_dev(int idx)
{
    int mode = hw_wifi_mode_get();

    if (mode == 1)
        return &wifi_sta;
    if (mode == 2)
        return &wifi_ap;
    if (idx == 0)
        return &wifi_sta;
    if (idx == 1)
        return &wifi_ap;
    return OS_NULL;
}

OS_INLINE int hw_wifi_get_idx(struct ameba_wifi *wifi)
{
    int mode = hw_wifi_mode_get();

    if (mode == 1)
        return 0;
    if (mode == 2)
        return 0;
    return wifi->type;
}

int hw_wifi_register(struct ameba_wifi *wifi)
{
    struct os_wlan_device *wlan = OS_NULL;

    if ((wifi->flag & WIFI_INIT_FLAG) == 0)
    {
        wlan = os_malloc(sizeof(struct os_wlan_device));
        OS_ASSERT(wlan != OS_NULL);
        if (wifi->type == WIFI_TYPE_STA)
        {
            os_wlan_dev_register(wlan, OS_WLAN_DEVICE_STA_NAME, &ops, 0, wifi);
        }
        if (wifi->type == WIFI_TYPE_AP)
        {
            os_wlan_dev_register(wlan, OS_WLAN_DEVICE_AP_NAME, &ops, 0, wifi);
        }
        wifi->flag |= WIFI_INIT_FLAG;
        wifi->wlan = wlan;
        LOG_EXT_W("F:%s L:%d wifi:0x%08x wlan:0x%08x\n", __FUNCTION__, __LINE__, wifi, wlan);
    }
    return OS_EOK;
}

void netif_post_sleep_processing(void)
{
}

void netif_pre_sleep_processing(void)
{
}

unsigned char *rltk_wlan_get_ip(int idx)
{
    struct ameba_wifi *wifi;

    wifi = hw_wifi_get_dev(idx);
    if (wifi == OS_NULL)
        return OS_NULL;

    LOG_EXT_W("F:%s L:%d is run", __FUNCTION__, __LINE__);

    return OS_NULL;
}

int netif_is_valid_IP(int idx, unsigned char *ip_dest)
{
    LOG_EXT_W("F:%s L:%d is run ip: %d:%d:%d:%d",
              __FUNCTION__,
              __LINE__,
              ip_dest[0],
              ip_dest[1],
              ip_dest[2],
              ip_dest[3]);
    return 1;
}

void rltk_wlan_set_netif_info(int idx, void *dev, os_uint8_t *dev_addr)
{
    struct ameba_wifi *wifi = OS_NULL;

    wifi = hw_wifi_get_dev(idx);
    if (wifi == OS_NULL)
        return;
    LOG_EXT_W("F:%s L:%d wifi:0x%08x  type:0x%x", __FUNCTION__, __LINE__, wifi, wifi->flag);
    os_memcpy(wifi->dev_addr, dev_addr, 6);
    wifi->flag |= WIFI_MAC_FLAG;
    hw_wifi_register(wifi);
    LOG_EXT_W("wifi type:%d", wifi->type);
    LOG_EXT_W("idx:%d MAC %02x:%02x:%02x:%02x:%02x:%02x",
              idx,
              dev_addr[0],
              dev_addr[1],
              dev_addr[2],
              dev_addr[3],
              dev_addr[4],
              dev_addr[5]);
}

static void rtw_connect_callbackfn(char *buf, int len, int flags, void *user_data)
{
    struct ameba_wifi *wifi = user_data;

    LOG_EXT_W("L:%d wifi connect callback flags:%d user_data:%08x", __LINE__, flags, user_data);
    if (wifi_is_connected_to_ap() == 0)
    {
        wifi->connected = 1;
        os_wlan_dev_indicate_event_handle(wifi->wlan, OS_WLAN_DEV_EVT_CONNECT, OS_NULL);
    }
}

static void rtw_connect_fail_callbackfn(char *buf, int len, int flags, void *user_data)
{
    struct ameba_wifi *wifi = user_data;

    LOG_EXT_W("L:%d wifi connect callback flags:%d", __LINE__, flags);
    wifi->connected = 0;
    os_wlan_dev_indicate_event_handle(wifi->wlan, OS_WLAN_DEV_EVT_CONNECT_FAIL, OS_NULL);
}

static void rtw_disconnect_callbackfn(char *buf, int len, int flags, void *user_data)
{
    struct ameba_wifi *wifi = user_data;

    LOG_EXT_W("L:%d wifi disconnect callback flags:%d", __LINE__, flags);
    wifi->connected = 0;
    os_wlan_dev_indicate_event_handle(wifi->wlan, OS_WLAN_DEV_EVT_DISCONNECT, OS_NULL);
}

static void rtw_sta_assoc_callbackfn(char *buf, int len, int flags, void *user_data)
{
    LOG_EXT_W("L:%d wifi sta assoc callback flags:%d", __LINE__, flags);
}

static void rtw_sta_disassoc_callbackfn(char *buf, int len, int flags, void *user_data)
{
    LOG_EXT_W("L:%d wifi sta assoc callback flags:%d buf:%08x %08x",
              __LINE__,
              flags,
              *((os_uint32_t *)buf),
              *((os_uint32_t *)buf + 4));
}

void netif_rx(int idx, unsigned int len)
{
    struct ameba_wifi *wifi = OS_NULL;
    struct sk_buff *   skb  = OS_NULL;

    wifi = hw_wifi_get_dev(idx);
    if (wifi == OS_NULL)
        return;

    LOG_EXT_W("F:%s L:%d idx:%d len:%d", __FUNCTION__, __LINE__, idx, len);
    if ((!wifi->connected) || (!rltk_wlan_running(idx)))
        return;

    skb = (struct sk_buff *)rltk_wlan_get_recv_skb(idx);
    if (!skb)
    {
        LOG_EXT_W("netif_rx rltk_wlan_get_recv_skb NULL.");
        return;
    }
    os_wlan_dev_report_data(wifi->wlan, skb->data, len);
}

static os_wlan_security_t security_map_from_ameba(hw_wifi_security_t security)
{
    os_wlan_security_t result = SECURITY_OPEN;

    switch (security)
    {
    case HW_WIFI_SECURITY_OPEN:
        result = SECURITY_OPEN;
        break;
    case HW_WIFI_SECURITY_WEP_PSK:
        result = SECURITY_WEP_PSK;
        break;
    case HW_WIFI_SECURITY_WEP_SHARED:
        result = SECURITY_WEP_SHARED;
        break;
    case HW_WIFI_SECURITY_WPA_TKIP_PSK:
        result = SECURITY_WPA_TKIP_PSK;
        break;
    case HW_WIFI_SECURITY_WPA_AES_PSK:
        result = SECURITY_WPA_AES_PSK;
        break;
    case HW_WIFI_SECURITY_WPA2_AES_PSK:
        result = SECURITY_WPA2_AES_PSK;
        break;
    case HW_WIFI_SECURITY_WPA2_TKIP_PSK:
        result = SECURITY_WPA2_TKIP_PSK;
        break;
    case HW_WIFI_SECURITY_WPA2_MIXED_PSK:
        result = SECURITY_WPA2_MIXED_PSK;
        break;
    case HW_WIFI_SECURITY_WPA_WPA2_MIXED:
        result = SECURITY_WPA2_AES_PSK;
        break;
    case HW_WIFI_SECURITY_WPS_OPEN:
        result = SECURITY_WPS_OPEN;
        break;
    case HW_WIFI_SECURITY_WPS_SECURE:
        result = SECURITY_WPS_SECURE;
        break;
    default:
        result = -1;
        break;
    }

    return result;
}

static hw_wifi_security_t security_map_from_oneos(os_wlan_security_t security)
{
    os_wlan_security_t result = HW_WIFI_SECURITY_OPEN;

    switch (security)
    {
    case SECURITY_OPEN:
        result = HW_WIFI_SECURITY_OPEN;
        break;
    case SECURITY_WEP_PSK:
        result = HW_WIFI_SECURITY_WEP_PSK;
        break;
    case SECURITY_WEP_SHARED:
        result = HW_WIFI_SECURITY_WEP_SHARED;
        break;
    case SECURITY_WPA_TKIP_PSK:
        result = HW_WIFI_SECURITY_WPA_TKIP_PSK;
        break;
    case SECURITY_WPA_AES_PSK:
        result = HW_WIFI_SECURITY_WPA_AES_PSK;
        break;
    case SECURITY_WPA2_AES_PSK:
        result = HW_WIFI_SECURITY_WPA2_AES_PSK;
        break;
    case SECURITY_WPA2_TKIP_PSK:
        result = HW_WIFI_SECURITY_WPA2_TKIP_PSK;
        break;
    case SECURITY_WPA2_MIXED_PSK:
        result = HW_WIFI_SECURITY_WPA2_MIXED_PSK;
        break;
    case SECURITY_WPS_OPEN:
        result = HW_WIFI_SECURITY_WPS_OPEN;
        break;
    case SECURITY_WPS_SECURE:
        result = HW_WIFI_SECURITY_WPS_SECURE;
        break;
    default:
        result = -1;
        break;
    }

    return result;
}

static void hw_wifi_scan_callback(struct hw_wlan_info *info, void *user_data)
{
    struct os_wlan_info wlan_info = {0};
    struct os_wlan_buff buff;
    struct ameba_wifi * wifi = user_data;

    if (info == OS_NULL)
    {
        os_wlan_dev_indicate_event_handle(wifi->wlan, OS_WLAN_DEV_EVT_SCAN_DONE, OS_NULL);
        return;
    }
    memcpy(&wlan_info.bssid[0], info->bssid, 6);
    strncpy(&wlan_info.ssid.val[0], info->ssid, HW_WLAN_SSID_MAX_LEN);
    wlan_info.ssid.len = strlen(&wlan_info.ssid.val[0]);
    wlan_info.band = info->band == HW_WIFI_802_11_BAND_2_4GHZ ? HW_WIFI_802_11_BAND_2_4GHZ : HW_WIFI_802_11_BAND_5GHZ;
    wlan_info.channel  = info->channel;
    wlan_info.datarate = info->datarate * 1000;
    wlan_info.security = security_map_from_ameba(info->security);
    wlan_info.rssi     = info->rssi;

    buff.data = &wlan_info;
    buff.len  = sizeof(wlan_info);
    os_wlan_dev_indicate_event_handle(wifi->wlan, OS_WLAN_DEV_EVT_SCAN_REPORT, &buff);
}

static void hw_wlan_monitor_callback(os_uint8_t *data, int len, void *user_data)
{
    os_wlan_dev_promisc_handler(wifi_sta.wlan, data, len);
}

static os_err_t hw_wlan_init(struct os_wlan_device *wlan)
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    return OS_EOK;
}

static os_err_t hw_wlan_mode(struct os_wlan_device *wlan, os_wlan_mode_t mode)
{
    struct ameba_wifi *wifi = (struct ameba_wifi *)(wlan->user_data);

    LOG_EXT_W("F:%s L:%d mode:%d", __FUNCTION__, __LINE__, mode);

    if (mode == OS_WLAN_STATION)
    {
        if (wifi->type != WIFI_TYPE_STA)
        {
            LOG_EXT_W("this wlan not support sta mode");
            return OS_ERROR;
        }
    }
    else if (mode == OS_WLAN_AP)
    {
        if (wifi->type != WIFI_TYPE_AP)
        {
            LOG_EXT_W("this wlan not support ap mode");
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

static os_err_t hw_wlan_scan(struct os_wlan_device *wlan, struct os_scan_info *scan_info)
{
    struct ameba_wifi *wifi = (struct ameba_wifi *)(wlan->user_data);

    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    LOG_EXT_W("F:%s L:%d wifi:0x%08x type:0x%x", __FUNCTION__, __LINE__, wifi, wifi->type);

    if (wifi->type != WIFI_TYPE_STA)
    {
        LOG_EXT_W("this wlan not support scan mode");
        return OS_ERROR;
    }
    if (hw_wifi_mode_get() == HW_WIFI_MODE_NONE)
    {
        if (hw_wifi_start(HW_WIFI_MODE_STA_AP) != OS_EOK)
        {
            LOG_EXT_W("L:%d wifistart failed...", __LINE__);
            return -1;
        }
    }
    hw_wifi_scan(hw_wifi_scan_callback, wifi);
    return OS_EOK;
}

static os_err_t hw_wlan_join(struct os_wlan_device *wlan, struct os_sta_info *sta_info)
{
    struct ameba_wifi *wifi   = (struct ameba_wifi *)(wlan->user_data);
    int                result = 0, i;
    char *             ssid = OS_NULL, *key = OS_NULL;

    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    if (wifi->type != WIFI_TYPE_STA)
    {
        LOG_EXT_E("this wlan not support sta mode");
        return OS_ERROR;
    }
    if ((hw_wifi_mode_get() != HW_WIFI_MODE_STA) && (hw_wifi_mode_get() != HW_WIFI_MODE_STA_AP))
    {
        hw_wifi_stop();
        os_task_delay(OS_TICK_PER_SECOND / 10);
        if (hw_wifi_start(HW_WIFI_MODE_STA_AP) != OS_EOK)
        {
            LOG_EXT_E("wifi on failed, join fail");
            return OS_ERROR;
        }
    }
    for (i = 0; i < OS_WLAN_BSSID_MAX_LENGTH; i++)
    {
        if (sta_info->bssid[i] != 0xff || sta_info->bssid[i] != 0x00)
            break;
    }
    if (i < OS_WLAN_BSSID_MAX_LENGTH)
    {
        if (sta_info->ssid.len > 0)
            ssid = &sta_info->ssid.val[0];
        if (sta_info->key.len > 0)
            key = &sta_info->key.val[0];
        LOG_EXT_W("bssid connect bssid: %02x:%02x:%02x:%02x:%02x:%02x ssid:%s ssid_len:%d key:%s key_len%d",
                  sta_info->bssid[0],
                  sta_info->bssid[1],
                  sta_info->bssid[2],
                  sta_info->bssid[3],
                  sta_info->bssid[4],
                  sta_info->bssid[5],
                  ssid,
                  sta_info->ssid.len,
                  key,
                  sta_info->key.len);
        result = hw_wifi_connect_bssid(sta_info->bssid,
                                       ssid,
                                       sta_info->ssid.len,
                                       key,
                                       sta_info->key.len,
                                       security_map_from_oneos(sta_info->security));
    }
    else
    {
        result = hw_wifi_connect(sta_info->ssid.val,
                                 sta_info->ssid.len,
                                 sta_info->key.val,
                                 sta_info->key.len,
                                 security_map_from_oneos(sta_info->security));
    }
    if (result != 0)
    {
        LOG_EXT_E("amebaz_wifi_connect failed...");
        return OS_ERROR;
    }
    /* netif_set_connected((struct ameba_wifi *)wlan, 1); */
    LOG_EXT_W("amebaz_wifi_connect do");
    return OS_EOK;
}

static os_err_t hw_wlan_softap(struct os_wlan_device *wlan, struct os_ap_info *ap_info)
{
    struct ameba_wifi *wifi = (struct ameba_wifi *)(wlan->user_data);

    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);

    if (wifi->type != WIFI_TYPE_AP)
    {
        LOG_EXT_E("this wlan not support ap mode");
        return OS_ERROR;
    }
    if (hw_wifi_ap_start(&ap_info->ssid.val[0], &ap_info->key.val[0], ap_info->channel) != 0)
    {
        os_wlan_dev_indicate_event_handle(wifi->wlan, OS_WLAN_DEV_EVT_AP_STOP, OS_NULL);
        wifi->connected = 0;
        return OS_ERROR;
    }
    os_wlan_dev_indicate_event_handle(wifi->wlan, OS_WLAN_DEV_EVT_AP_START, OS_NULL);
    wifi->connected = 1;
    return OS_EOK;
}

static os_err_t hw_wlan_disconnect(struct os_wlan_device *wlan)
{
    struct ameba_wifi *wifi = (struct ameba_wifi *)(wlan->user_data);

    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);

    if (wifi->type != WIFI_TYPE_STA)
    {
        LOG_EXT_E("this wlan not support sta mode");
        return OS_ERROR;
    }
    wifi->connected = 0;
    hw_wifi_sta_disconnect();
    os_wlan_dev_indicate_event_handle(wifi->wlan, OS_WLAN_DEV_EVT_AP_STOP, OS_NULL);
    return OS_EOK;
}

static os_err_t hw_wlan_ap_stop(struct os_wlan_device *wlan)
{
    struct ameba_wifi *wifi = (struct ameba_wifi *)(wlan->user_data);

    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);

    if (wifi->type != WIFI_TYPE_AP)
    {
        LOG_EXT_E("this wlan not support ap mode");
        return OS_ERROR;
    }

    hw_wifi_ap_disconnect();
    return OS_EOK;
}

static os_err_t hw_wlan_ap_deauth(struct os_wlan_device *wlan, os_uint8_t mac[])
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    return OS_EOK;
}

static os_err_t hw_wlan_scan_stop(struct os_wlan_device *wlan)
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    return OS_EOK;
}

static int hw_wlan_get_rssi(struct os_wlan_device *wlan)
{
    struct ameba_wifi *wifi = (struct ameba_wifi *)(wlan->user_data);

    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);

    if (wifi->type != WIFI_TYPE_STA)
    {
        LOG_EXT_E("this wlan not support sta mode");
        return OS_ERROR;
    }

    return hw_wifi_rssi_get();
}

static os_err_t hw_wlan_set_powersave(struct os_wlan_device *wlan, int level)
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    return OS_EOK;
}

static int hw_wlan_get_powersave(struct os_wlan_device *wlan)
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    return 0;
}

static os_err_t hw_wlan_cfg_promisc(struct os_wlan_device *wlan, os_bool_t start)
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);

    if (start)
    {
        hw_wifi_monitor_callback_set(hw_wlan_monitor_callback);
        hw_wifi_monitor_enable(1);
    }
    else
    {
        hw_wifi_monitor_callback_set(OS_NULL);
        hw_wifi_monitor_enable(0);
    }
    return OS_EOK;
}

static os_err_t hw_wlan_cfg_filter(struct os_wlan_device *wlan, struct os_wlan_filter *filter)
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    return OS_EOK;
}

static os_err_t hw_wlan_set_channel(struct os_wlan_device *wlan, int channel)
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);

    hw_wifi_channel_set(channel);

    return OS_EOK;
}

static int hw_wlan_get_channel(struct os_wlan_device *wlan)
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);

    return hw_wifi_channel_get();
}

static os_err_t hw_wlan_set_country(struct os_wlan_device *wlan, os_country_code_t country_code)
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    return OS_EOK;
}

static os_country_code_t hw_wlan_get_country(struct os_wlan_device *wlan)
{
    LOG_EXT_W("F:%s L:%d\n", __FUNCTION__, __LINE__);
    return OS_EOK;
}

static os_err_t hw_wlan_set_mac(struct os_wlan_device *wlan, os_uint8_t mac[])
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    return OS_ERROR;
}

static os_err_t hw_wlan_get_mac(struct os_wlan_device *wlan, os_uint8_t mac[])
{
    struct ameba_wifi *wifi = (struct ameba_wifi *)wlan->user_data;

    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    if (mac == OS_NULL)
    {
        return OS_ERROR;
    }
    memcpy(mac, wifi->dev_addr, MAX_ADDR_LEN);
    return OS_EOK;
}

static int hw_wlan_recv(struct os_wlan_device *wlan, void *buff, int len)
{
    LOG_EXT_W("F:%s L:%d", __FUNCTION__, __LINE__);
    return OS_EOK;
}

static int hw_wlan_send(struct os_wlan_device *wlan, void *buff, int len)
{
    struct ameba_wifi *wifi = (struct ameba_wifi *)wlan->user_data;
    int                idx  = hw_wifi_get_idx(wifi);
    os_base_t          level;
    struct sk_buff    *skb = OS_NULL;

    LOG_EXT_W("F:%s L:%d len:%d", __FUNCTION__, __LINE__, len);

    level = os_hw_interrupt_disable();
    if (!wifi->connected || !rltk_wlan_check_isup(idx))
    {
        os_hw_interrupt_enable(level);
        return OS_ERROR;
    }
    rltk_wlan_tx_inc(idx);
    os_hw_interrupt_enable(level);

    skb = (struct sk_buff *)rltk_wlan_alloc_skb(len);
    if (skb == OS_NULL)
    {
        LOG_EXT_W("rltk_wlan_alloc_skb NULL for WIFI TX.");
        goto exit;
    }
    /* copy buff to a whole ETH frame */
    memcpy(skb->tail, buff, len);
    skb_put(skb, len);
    rltk_wlan_send_skb(idx, skb);

exit:
    level = os_hw_interrupt_disable();
    rltk_wlan_tx_dec(idx);
    os_hw_interrupt_enable(level);

    LOG_EXT_W("F:%s L:%d end", __FUNCTION__, __LINE__);

    return OS_EOK;
}

static const struct os_wlan_dev_ops ops = 
{
    .wlan_init          = hw_wlan_init,
    .wlan_mode          = hw_wlan_mode,
    .wlan_scan          = hw_wlan_scan,
    .wlan_join          = hw_wlan_join,
    .wlan_softap        = hw_wlan_softap,
    .wlan_disconnect    = hw_wlan_disconnect,
    .wlan_ap_stop       = hw_wlan_ap_stop,
    .wlan_ap_deauth     = hw_wlan_ap_deauth,
    .wlan_scan_stop     = hw_wlan_scan_stop,
    .wlan_get_rssi      = hw_wlan_get_rssi,
    .wlan_set_powersave = hw_wlan_set_powersave,
    .wlan_get_powersave = hw_wlan_get_powersave,
    .wlan_cfg_promisc   = hw_wlan_cfg_promisc,
    .wlan_cfg_filter    = hw_wlan_cfg_filter,
    .wlan_set_channel   = hw_wlan_set_channel,
    .wlan_get_channel   = hw_wlan_get_channel,
    .wlan_set_country   = hw_wlan_set_country,
    .wlan_get_country   = hw_wlan_get_country,
    .wlan_set_mac       = hw_wlan_set_mac,
    .wlan_get_mac       = hw_wlan_get_mac,
    .wlan_recv          = hw_wlan_recv,
    .wlan_send          = hw_wlan_send,
};

int hw_wifi_low_init(void)
{
    static os_int8_t _init_flag = 0;

    if (_init_flag)
    {
        return 1;
    }
    os_memset(&wifi_sta, 0, sizeof(wifi_sta));
    os_memset(&wifi_ap, 0, sizeof(wifi_ap));
    wifi_sta.type = WIFI_TYPE_STA;
    wifi_ap.type  = WIFI_TYPE_AP;
    if (hw_wifi_start(HW_WIFI_MODE_STA_AP) != OS_EOK)
    {
        LOG_EXT_E("amebaz_wifi_start failed...");
        return -1;
    }
    LOG_EXT_I("amebaz_wifi_start success");
    LOG_EXT_W("F:%s L:%d wifi_sta:0x%08x   wifi_ap:0x%08x", __FUNCTION__, __LINE__, &wifi_sta, &wifi_ap);

    wifi_reg_event_handler(HW_WIFI_EVENT_FOURWAY_HANDSHAKE_DONE, rtw_connect_callbackfn, &wifi_sta);
    wifi_reg_event_handler(HW_WIFI_EVENT_DISCONNECT, rtw_disconnect_callbackfn, &wifi_sta);

    _init_flag = 1;

    return 0;
}
OS_APP_INIT(hw_wifi_low_init);
