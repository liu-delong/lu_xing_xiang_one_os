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
 * @file        wlan_dev.c
 *
 * @brief       wlan_dev
 *
 * @details     wlan_dev
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_hw.h>
#include <wlan/wlan_dev.h>
#include <wlan/wlan_prot.h>
#include <os_errno.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <drv_cfg.h>
#include <os_memory.h>
#include <string.h>
#include <os_stddef.h>
#define DRV_EXT_LVL          DBG_EXT_INFO
#define DRV_EXT_TAG         "wlan.dev" 
#include <drv_log.h>

#if defined(OS_USING_WIFI) || defined(OS_USING_WLAN)

#ifndef OS_DEVICE
#define OS_DEVICE(__device) ((os_device_t)__device)
#endif

#define WLAN_DEV_LOCK(_wlan)   (os_mutex_recursive_lock(&(_wlan)->lock, OS_IPC_WAITING_FOREVER))
#define WLAN_DEV_UNLOCK(_wlan) (os_mutex_recursive_unlock(&(_wlan)->lock))

#if OS_WLAN_SSID_MAX_LENGTH < 1
#error "SSID length is too short"
#endif

#if OS_WLAN_BSSID_MAX_LENGTH < 1
#error "BSSID length is too short"
#endif

#if OS_WLAN_PASSWORD_MAX_LENGTH < 1
#error "password length is too short"
#endif

#if OS_WLAN_DEV_EVENT_NUM < 2
#error "dev num Too little"
#endif

os_err_t os_wlan_dev_init(struct os_wlan_device *device, os_wlan_mode_t mode)
{
    os_err_t result = OS_EOK;

    /* init wlan device */
    LOG_EXT_D("F:%s L:%d is run device:0x%08x mode:%d", __FUNCTION__, __LINE__, device, mode);
    if ((device == OS_NULL) || (mode >= OS_WLAN_MODE_MAX))
    {
        LOG_EXT_E("F:%s L:%d Parameter Wrongful device:0x%08x mode:%d", __FUNCTION__, __LINE__, device, mode);
        return OS_ERROR;
    }

    if (mode == OS_WLAN_AP && device->flags & OS_WLAN_FLAG_STA_ONLY)
    {
        LOG_EXT_E("F:%s L:%d This wlan device can only be set to sta mode!", __FUNCTION__, __LINE__);
        return OS_ERROR;
    }
    else if (mode == OS_WLAN_STATION && device->flags & OS_WLAN_FLAG_AP_ONLY)
    {
        LOG_EXT_E("F:%s L:%d This wlan device can only be set to ap mode!", __FUNCTION__, __LINE__);
        return OS_ERROR;
    }

    result = os_device_init(OS_DEVICE(device));
    if (result != OS_EOK)
    {
        LOG_EXT_E("L:%d wlan init failed", __LINE__);
        return OS_ERROR;
    }
    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_MODE, (void *)&mode);
    if (result != OS_EOK)
    {
        LOG_EXT_E("L:%d wlan config mode failed", __LINE__);
        return OS_ERROR;
    }
    device->mode = mode;
    return result;
}

os_err_t os_wlan_dev_connect(struct os_wlan_device *device, struct os_wlan_info *info, const char *password, int password_len)
{
    os_err_t    result = OS_EOK;
    
    struct os_sta_info sta_info;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }
    if (info == OS_NULL)
    {
        return OS_ERROR;
    }

    if ((password_len > OS_WLAN_PASSWORD_MAX_LENGTH) || (info->ssid.len > OS_WLAN_SSID_MAX_LENGTH))
    {
        LOG_EXT_E("L:%d password or ssid is too long", __LINE__);
        return OS_ERROR;
    }
    memset(&sta_info, 0, sizeof(struct os_sta_info));
    memcpy(&sta_info.ssid, &info->ssid, sizeof(os_wlan_ssid_t));
    memcpy(sta_info.bssid, info->bssid, OS_WLAN_BSSID_MAX_LENGTH);
    if (password != OS_NULL)
    {
        memcpy(sta_info.key.val, password, password_len);
        sta_info.key.len = password_len;
    }
    sta_info.channel  = info->channel;
    sta_info.security = info->security;

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_JOIN, &sta_info);
    return result;
}

os_err_t os_wlan_dev_disconnect(struct os_wlan_device *device)
{
    os_err_t result = OS_EOK;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_DISCONNECT, OS_NULL);
    return result;
}

os_err_t os_wlan_dev_ap_start(struct os_wlan_device *device, struct os_wlan_info *info, const char *password, int password_len)
{
    os_err_t          result = OS_EOK;
    struct os_ap_info ap_info;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }
    if (info == OS_NULL)
    {
        return OS_ERROR;
    }

    if ((password_len > OS_WLAN_PASSWORD_MAX_LENGTH) || (info->ssid.len > OS_WLAN_SSID_MAX_LENGTH))
    {
        LOG_EXT_E("L:%d password or ssid is too long", __LINE__);
        return OS_ERROR;
    }

    memset(&ap_info, 0, sizeof(struct os_ap_info));
    memcpy(&ap_info.ssid, &info->ssid, sizeof(os_wlan_ssid_t));
    if (password != OS_NULL)
    {
        memcpy(ap_info.key.val, password, password_len);
    }
    ap_info.key.len  = password_len;
    ap_info.hidden   = info->hidden;
    ap_info.channel  = info->channel;
    ap_info.security = info->security;

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_SOFTAP, &ap_info);
    return result;
}

os_err_t os_wlan_dev_ap_stop(struct os_wlan_device *device)
{
    os_err_t result = OS_EOK;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_AP_STOP, OS_NULL);
    return result;
}

os_err_t os_wlan_dev_ap_deauth(struct os_wlan_device *device, os_uint8_t mac[6])
{
    os_err_t result = OS_EOK;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_AP_DEAUTH, mac);
    return result;
}

int os_wlan_dev_get_rssi(struct os_wlan_device *device)
{
    int      rssi   = 0;
    os_err_t result = OS_EOK;

    if (device == OS_NULL)
    {
        os_set_errno(OS_EIO);
        return 0;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_GET_RSSI, &rssi);
    if (result != OS_EOK)
    {
        os_set_errno(result);
        return 0;
    }

    return rssi;
}

os_err_t os_wlan_dev_get_mac(struct os_wlan_device *device, os_uint8_t mac[6])
{
    os_err_t result = OS_EOK;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_GET_MAC, &mac[0]);
    return result;
}

os_err_t os_wlan_dev_set_mac(struct os_wlan_device *device, os_uint8_t mac[6])
{
    os_err_t result = OS_EOK;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_SET_MAC, &mac[0]);
    return result;
}

os_err_t os_wlan_dev_set_powersave(struct os_wlan_device *device, int level)
{
    os_err_t result = OS_EOK;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_SET_POWERSAVE, &level);
    return result;
}

int os_wlan_dev_get_powersave(struct os_wlan_device *device)
{
    int      level  = -1;
    os_err_t result = OS_EOK;

    if (device == OS_NULL)
    {
        os_set_errno(OS_EIO);
        return -1;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_GET_POWERSAVE, &level);
    if (result != OS_EOK)
    {
        os_set_errno(result);
    }

    return level;
}

os_err_t os_wlan_dev_register_event_handler(struct os_wlan_device    *device,
                                            os_wlan_dev_event_t       event,
                                            os_wlan_dev_event_handler handler,
                                            void *                    parameter)
{
    int       i = 0;
    os_base_t level;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }
    if (event >= OS_WLAN_DEV_EVT_MAX)
    {
        return OS_EINVAL;
    }

    level = os_hw_interrupt_disable();
    for (i = 0; i < OS_WLAN_DEV_EVENT_NUM; i++)
    {
        if (device->handler_table[event][i].handler == OS_NULL)
        {
            device->handler_table[event][i].handler   = handler;
            device->handler_table[event][i].parameter = parameter;
            os_hw_interrupt_enable(level);
            return OS_EOK;
        }
    }
    os_hw_interrupt_enable(level);

    /* No space found */
    return OS_ERROR;
}

os_err_t os_wlan_dev_unregister_event_handler(struct os_wlan_device    *device,
                                              os_wlan_dev_event_t       event,
                                              os_wlan_dev_event_handler handler)
{
    int       i = 0;
    os_base_t level;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }
    if (event >= OS_WLAN_DEV_EVT_MAX)
    {
        return OS_EINVAL;
    }

    level = os_hw_interrupt_disable();
    for (i = 0; i < OS_WLAN_DEV_EVENT_NUM; i++)
    {
        if (device->handler_table[event][i].handler == handler)
        {
            memset(&device->handler_table[event][i], 0, sizeof(struct os_wlan_dev_event_desc));
            os_hw_interrupt_enable(level);
            return OS_EOK;
        }
    }
    os_hw_interrupt_enable(level);
    /* not find iteam */
    return OS_ERROR;
}

void os_wlan_dev_indicate_event_handle(struct os_wlan_device *device,
                                       os_wlan_dev_event_t    event,
                                       struct os_wlan_buff   *buff)
{
    void *                    parameter[OS_WLAN_DEV_EVENT_NUM];
    os_wlan_dev_event_handler handler[OS_WLAN_DEV_EVENT_NUM];
    int                       i;
    os_base_t                 level;

    if (device == OS_NULL)
    {
        return;
    }
    if (event >= OS_WLAN_DEV_EVT_MAX)
    {
        return;
    }

    /* get callback handle */
    level = os_hw_interrupt_disable();
    for (i = 0; i < OS_WLAN_DEV_EVENT_NUM; i++)
    {
        handler[i]   = device->handler_table[event][i].handler;
        parameter[i] = device->handler_table[event][i].parameter;
    }
    os_hw_interrupt_enable(level);

    /* run callback */
    for (i = 0; i < OS_WLAN_DEV_EVENT_NUM; i++)
    {
        if (handler[i] != OS_NULL)
        {
            handler[i](device, event, buff, parameter[i]);
        }
    }
}

os_err_t os_wlan_dev_enter_promisc(struct os_wlan_device *device)
{
    os_err_t result = OS_EOK;
    int      enable = 1;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_CFG_PROMISC, &enable);
    return result;
}

os_err_t os_wlan_dev_exit_promisc(struct os_wlan_device *device)
{
    os_err_t result = OS_EOK;
    int      enable = 0;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_CFG_PROMISC, &enable);
    return result;
}

os_err_t os_wlan_dev_set_promisc_callback(struct os_wlan_device *device, os_wlan_pormisc_callback_t callback)
{
    if (device == OS_NULL)
    {
        return OS_EIO;
    }
    device->pormisc_callback = callback;

    return OS_EOK;
}

void os_wlan_dev_promisc_handler(struct os_wlan_device *device, void *data, int len)
{
    os_wlan_pormisc_callback_t callback;

    if (device == OS_NULL)
    {
        return;
    }

    callback = device->pormisc_callback;

    if (callback != OS_NULL)
    {
        callback(device, data, len);
    }
}

os_err_t os_wlan_dev_cfg_filter(struct os_wlan_device *device, struct os_wlan_filter *filter)
{
    os_err_t result = OS_EOK;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }
    if (filter == OS_NULL)
    {
        return OS_ERROR;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_CFG_FILTER, filter);
    return result;
}

os_err_t os_wlan_dev_set_channel(struct os_wlan_device *device, int channel)
{
    os_err_t result = OS_EOK;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }
    if (channel < 0)
    {
        return OS_ERROR;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_SET_CHANNEL, &channel);
    return result;
}

int os_wlan_dev_get_channel(struct os_wlan_device *device)
{
    os_err_t result  = OS_EOK;
    int      channel = -1;

    if (device == OS_NULL)
    {
        os_set_errno(OS_EIO);
        return -1;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_GET_CHANNEL, &channel);
    if (result != OS_EOK)
    {
        os_set_errno(result);
        return -1;
    }

    return channel;
}

os_err_t os_wlan_dev_set_country(struct os_wlan_device *device, os_country_code_t country_code)
{
    int result = OS_EOK;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_SET_COUNTRY, &country_code);
    return result;
}

os_country_code_t os_wlan_dev_get_country(struct os_wlan_device *device)
{
    int result  = OS_EOK;
    
    os_country_code_t country_code = OS_COUNTRY_UNKNOWN;

    if (device == OS_NULL)
    {
        os_set_errno(OS_EIO);
        return OS_COUNTRY_UNKNOWN;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_GET_COUNTRY, &country_code);
    if (result != OS_EOK)
    {
        os_set_errno(result);
        return OS_COUNTRY_UNKNOWN;
    }

    return country_code;
}

os_err_t os_wlan_dev_scan(struct os_wlan_device *device, struct os_wlan_info *info)
{
    struct os_scan_info  scan_info   = {0};
    struct os_scan_info *p_scan_info = OS_NULL;
    os_err_t             result      = 0;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    if (info != OS_NULL)
    {
        if (info->ssid.len > OS_WLAN_SSID_MAX_LENGTH)
        {
            LOG_EXT_E("L:%d ssid is too long", __LINE__);
            return OS_EINVAL;
        }
        memcpy(&scan_info.ssid, &info->ssid, sizeof(os_wlan_ssid_t));
        memcpy(scan_info.bssid, info->bssid, OS_WLAN_BSSID_MAX_LENGTH);
        if (info->channel > 0)
        {
            scan_info.channel_min = info->channel;
            scan_info.channel_max = info->channel;
        }
        else
        {
            scan_info.channel_min = -1;
            scan_info.channel_max = -1;
        }
        scan_info.passive = info->hidden ? OS_TRUE : OS_FALSE;
        p_scan_info       = &scan_info;
    }
    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_SCAN, p_scan_info);
    return result;
}

os_err_t os_wlan_dev_scan_stop(struct os_wlan_device *device)
{
    os_err_t result = 0;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_SCAN_STOP, OS_NULL);
    return result;
}

os_err_t os_wlan_dev_report_data(struct os_wlan_device *device, void *buff, int len)
{
#ifdef OS_WLAN_PROT_ENABLE
    return os_wlan_dev_transfer_prot(device, buff, len);
#else
    return OS_ERROR;
#endif
}

os_err_t os_wlan_dev_enter_mgnt_filter(struct os_wlan_device *device)
{
    os_err_t result = OS_EOK;
    int      enable = 1;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_CFG_MGNT_FILTER, &enable);
    return result;
}

os_err_t os_wlan_dev_exit_mgnt_filter(struct os_wlan_device *device)
{
    os_err_t result = OS_EOK;
    int      enable = 0;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    result = os_device_control(OS_DEVICE(device), OS_WLAN_CMD_CFG_MGNT_FILTER, &enable);
    return result;
}

os_err_t os_wlan_dev_set_mgnt_filter_callback(struct os_wlan_device *device, os_wlan_mgnt_filter_callback_t callback)
{
    if (device == OS_NULL)
    {
        return OS_EIO;
    }
    device->mgnt_filter_callback = callback;

    return OS_EOK;
}

void os_wlan_dev_mgnt_filter_handler(struct os_wlan_device *device, void *data, int len)
{
    os_wlan_mgnt_filter_callback_t callback;

    if (device == OS_NULL)
    {
        return;
    }

    callback = device->mgnt_filter_callback;

    if (callback != OS_NULL)
    {
        callback(device, data, len);
    }
}

int os_wlan_dev_send_raw_frame(struct os_wlan_device *device, void *buff, int len)
{
    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    if (device->ops->wlan_send_raw_frame)
    {
        return device->ops->wlan_send_raw_frame(device, buff, len);
    }

    return OS_ERROR;
}

static os_err_t _os_wlan_dev_init(os_device_t *dev)
{
    struct os_wlan_device *wlan   = (struct os_wlan_device *)dev;
    os_err_t               result = OS_EOK;

    os_mutex_init(&wlan->lock, "wlan_dev", OS_IPC_FLAG_FIFO, OS_TRUE);

    if (wlan->ops->wlan_init)
        result = wlan->ops->wlan_init(wlan);

    if (result == OS_EOK)
    {
        LOG_EXT_I("wlan init success");
    }
    else
    {
        LOG_EXT_I("wlan init failed");
    }

    return result;
}

static os_err_t _os_wlan_dev_control(os_device_t *dev, int cmd, void *args)
{
    struct os_wlan_device *wlan = (struct os_wlan_device *)dev;
    os_err_t               err  = OS_EOK;

    OS_ASSERT(dev != OS_NULL);

    WLAN_DEV_LOCK(wlan);

    switch (cmd)
    {
    case OS_WLAN_CMD_MODE:
    {
        os_wlan_mode_t mode = *((os_wlan_mode_t *)args);

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_MODE, "OS_WLAN_CMD_MODE");
        if (wlan->ops->wlan_mode)
            err = wlan->ops->wlan_mode(wlan, mode);
        break;
    }
    case OS_WLAN_CMD_SCAN:
    {
        struct os_scan_info *scan_info = args;

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_SCAN, "OS_WLAN_CMD_SCAN");
        if (wlan->ops->wlan_scan)
            err = wlan->ops->wlan_scan(wlan, scan_info);
        break;
    }
    case OS_WLAN_CMD_JOIN:
    {
        struct os_sta_info *sta_info = args;

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_JOIN, "OS_WLAN_CMD_JOIN");
        if (wlan->ops->wlan_join)
            err = wlan->ops->wlan_join(wlan, sta_info);
        break;
    }
    case OS_WLAN_CMD_SOFTAP:
    {
        struct os_ap_info *ap_info = args;

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_SOFTAP, "OS_WLAN_CMD_SOFTAP");
        if (wlan->ops->wlan_softap)
            err = wlan->ops->wlan_softap(wlan, ap_info);
        break;
    }
    case OS_WLAN_CMD_DISCONNECT:
    {
        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_DISCONNECT, "OS_WLAN_CMD_DISCONNECT");
        if (wlan->ops->wlan_disconnect)
            err = wlan->ops->wlan_disconnect(wlan);
        break;
    }
    case OS_WLAN_CMD_AP_STOP:
    {
        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_AP_STOP, "OS_WLAN_CMD_AP_STOP");
        if (wlan->ops->wlan_ap_stop)
            err = wlan->ops->wlan_ap_stop(wlan);
        break;
    }
    case OS_WLAN_CMD_AP_DEAUTH:
    {
        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_AP_DEAUTH, "OS_WLAN_CMD_AP_DEAUTH");
        if (wlan->ops->wlan_ap_deauth)
            err = wlan->ops->wlan_ap_deauth(wlan, args);
        break;
    }
    case OS_WLAN_CMD_SCAN_STOP:
    {
        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_SCAN_STOP, "OS_WLAN_CMD_SCAN_STOP");
        if (wlan->ops->wlan_scan_stop)
            err = wlan->ops->wlan_scan_stop(wlan);
        break;
    }
    case OS_WLAN_CMD_GET_RSSI:
    {
        int *rssi = args;

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_GET_RSSI, "OS_WLAN_CMD_GET_RSSI");
        if (wlan->ops->wlan_get_rssi)
            *rssi = wlan->ops->wlan_get_rssi(wlan);
        break;
    }
    case OS_WLAN_CMD_SET_POWERSAVE:
    {
        int level = *((int *)args);

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_SET_POWERSAVE, "OS_WLAN_CMD_SET_POWERSAVE");
        if (wlan->ops->wlan_set_powersave)
            err = wlan->ops->wlan_set_powersave(wlan, level);
        break;
    }
    case OS_WLAN_CMD_GET_POWERSAVE:
    {
        int *level = args;

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_GET_POWERSAVE, "OS_WLAN_CMD_GET_POWERSAVE");
        if (wlan->ops->wlan_get_powersave)
            *level = wlan->ops->wlan_get_powersave(wlan);
        break;
    }
    case OS_WLAN_CMD_CFG_PROMISC:
    {
        os_bool_t start = *((os_bool_t *)args);

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_CFG_PROMISC, "OS_WLAN_CMD_CFG_PROMISC");
        if (wlan->ops->wlan_cfg_promisc)
            err = wlan->ops->wlan_cfg_promisc(wlan, start);
        break;
    }
    case OS_WLAN_CMD_CFG_FILTER:
    {
        struct os_wlan_filter *filter = args;

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_CFG_FILTER, "OS_WLAN_CMD_CFG_FILTER");
        if (wlan->ops->wlan_cfg_filter)
            err = wlan->ops->wlan_cfg_filter(wlan, filter);
        break;
    }
    case OS_WLAN_CMD_CFG_MGNT_FILTER:
    {
        os_bool_t start = *((os_bool_t *)args);

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_CFG_MGNT_FILTER, "OS_WLAN_CMD_CFG_MGNT_FILTER");
        if (wlan->ops->wlan_cfg_mgnt_filter)
            err = wlan->ops->wlan_cfg_mgnt_filter(wlan, start);
        break;
    }
    case OS_WLAN_CMD_SET_CHANNEL:
    {
        int channel = *(int *)args;
        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_SET_CHANNEL, "OS_WLAN_CMD_SET_CHANNEL");
        if (wlan->ops->wlan_set_channel)
            err = wlan->ops->wlan_set_channel(wlan, channel);
        break;
    }
    case OS_WLAN_CMD_GET_CHANNEL:
    {
        int *channel = args;

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_GET_CHANNEL, "OS_WLAN_CMD_GET_CHANNEL");
        if (wlan->ops->wlan_get_channel)
            *channel = wlan->ops->wlan_get_channel(wlan);
        break;
    }
    case OS_WLAN_CMD_SET_COUNTRY:
    {
        os_country_code_t country = *(os_country_code_t *)args;

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_SET_COUNTRY, "OS_WLAN_CMD_SET_COUNTRY");
        if (wlan->ops->wlan_set_country)
            err = wlan->ops->wlan_set_country(wlan, country);
        break;
    }
    case OS_WLAN_CMD_GET_COUNTRY:
    {
        os_country_code_t *country = args;
        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_GET_COUNTRY, "OS_WLAN_CMD_GET_COUNTRY");
        if (wlan->ops->wlan_get_country)
            *country = wlan->ops->wlan_get_country(wlan);
        break;
    }
    case OS_WLAN_CMD_SET_MAC:
    {
        os_uint8_t *mac = args;

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_SET_MAC, "OS_WLAN_CMD_SET_MAC");
        if (wlan->ops->wlan_set_mac)
            err = wlan->ops->wlan_set_mac(wlan, mac);
        break;
    }
    case OS_WLAN_CMD_GET_MAC:
    {
        os_uint8_t *mac = args;

        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, OS_WLAN_CMD_GET_MAC, "OS_WLAN_CMD_GET_MAC");
        if (wlan->ops->wlan_get_mac)
            err = wlan->ops->wlan_get_mac(wlan, mac);
        break;
    }
    default:
        LOG_EXT_D("%s %d cmd[%d]:%s  run......", __FUNCTION__, __LINE__, -1, "UNKUOWN");
        break;
    }

    WLAN_DEV_UNLOCK(wlan);

    return err;
}

#ifdef OS_USING_DEVICE_OPS
const static struct os_device_ops wlan_ops =
{
    _os_wlan_dev_init,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    _os_wlan_dev_control
};
#endif

os_err_t os_wlan_dev_register(struct os_wlan_device        *wlan,
                              const char                   *name,
                              const struct os_wlan_dev_ops *ops,
                              os_uint32_t                   flag,
                              void                         *user_data)
{
    os_err_t err = OS_EOK;

    if ((wlan == OS_NULL) || (name == OS_NULL) || (ops == OS_NULL) ||
        (flag & OS_WLAN_FLAG_STA_ONLY && flag & OS_WLAN_FLAG_AP_ONLY))
    {
        LOG_EXT_E("F:%s L:%d parameter Wrongful", __FUNCTION__, __LINE__);
        return (os_err_t)OS_NULL;
    }

    memset(wlan, 0, sizeof(struct os_wlan_device));

#ifdef OS_USING_DEVICE_OPS
    wlan->device.ops = &wlan_ops;
#else
    wlan->device.init    = _os_wlan_dev_init;
    wlan->device.open    = OS_NULL;
    wlan->device.close   = OS_NULL;
    wlan->device.read    = OS_NULL;
    wlan->device.write   = OS_NULL;
    wlan->device.control = _os_wlan_dev_control;
#endif

    wlan->device.user_data = OS_NULL;

    wlan->device.type = OS_DEVICE_TYPE_NETIF;

    wlan->ops       = ops;
    wlan->user_data = user_data;

    wlan->flags = flag;
    err         = os_device_register(&wlan->device, name, OS_DEVICE_FLAG_RDWR);

    LOG_EXT_D("F:%s L:%d run", __FUNCTION__, __LINE__);

    return err;
}

#endif
