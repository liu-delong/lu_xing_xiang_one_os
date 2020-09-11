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
 * @file        wlan_mgnt.c
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

#include <os_hw.h>
#include <os_task.h>
#include <wlan/wlan_dev.h>
#include <wlan/wlan_cfg.h>
#include <wlan/wlan_mgnt.h>
#include <wlan/wlan_prot.h>
#include <wlan/wlan_workqueue.h>
#include <os_event.h>
#include <os_errno.h>
#include <os_memory.h>
#include <string.h>
#include <os_clock.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "wlan.mgnt"
#include <drv_log.h>

#ifdef OS_WLAN_MANAGE_ENABLE

#ifndef OS_WLAN_DEVICE
#define OS_WLAN_DEVICE(__device) ((struct os_wlan_device *)__device)
#endif

#define OS_WLAN_LOG_D(_fmt, ...)  LOG_EXT_D("L:%d "_fmt"", __LINE__, ##__VA_ARGS__)
#define OS_WLAN_LOG_I(...)        LOG_EXT_I(__VA_ARGS__)
#define OS_WLAN_LOG_W(_fmt, ...)  LOG_EXT_W("F:%s L:%d "_fmt"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define OS_WLAN_LOG_E(_fmt, ...)  LOG_EXT_E("F:%s L:%d "_fmt"", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define STA_DEVICE() (_sta_mgnt.device)
#define AP_DEVICE()  (_ap_mgnt.device)

#define SRESULT_LOCK()   (os_mutex_recursive_lock(&scan_result_mutex, OS_IPC_WAITING_FOREVER))
#define SRESULT_UNLOCK() (os_mutex_recursive_unlock(&scan_result_mutex))

#define STAINFO_LOCK()   (os_mutex_recursive_lock(&sta_info_mutex, OS_IPC_WAITING_FOREVER))
#define STAINFO_UNLOCK() (os_mutex_recursive_unlock(&sta_info_mutex))

#define MGNT_LOCK()   (os_mutex_recursive_lock(&mgnt_mutex, OS_IPC_WAITING_FOREVER))
#define MGNT_UNLOCK() (os_mutex_recursive_unlock(&mgnt_mutex))

#define COMPLETE_LOCK()   (os_mutex_recursive_lock(&complete_mutex, OS_IPC_WAITING_FOREVER))
#define COMPLETE_UNLOCK() (os_mutex_recursive_unlock(&complete_mutex))

#ifdef OS_WLAN_AUTO_CONNECT_ENABLE
#define TIME_STOP()  (os_timer_stop(&reconnect_time))
#define TIME_START() (os_timer_start(&reconnect_time))
#else
#define TIME_STOP()
#define TIME_START()
#endif

#if OS_WLAN_EBOX_NUM < 1
#error "event box num Too few"
#endif

struct os_wlan_mgnt_des
{
    struct os_wlan_device *device;
    struct os_wlan_info    info;
    struct os_wlan_key     key;
    os_uint8_t             state;
    os_uint8_t             flags;
};

struct os_wlan_event_desc
{
    os_wlan_event_handler handler;
    void                 *parameter;
};

struct os_wlan_sta_list
{
    struct os_wlan_sta_list *next;
    struct os_wlan_info      info;
};

struct os_wlan_sta_des
{
    int                      num;
    struct os_wlan_sta_list *node;
};

struct os_wlan_msg
{
    os_int32_t event;
    os_int32_t len;
    void      *buff;
};

struct os_wlan_complete_des
{
    struct os_event complete;
    os_uint32_t     event_flag;
    int             index;
};

static struct os_mutex mgnt_mutex;

static struct os_wlan_mgnt_des _sta_mgnt;
static struct os_wlan_mgnt_des _ap_mgnt;

static struct os_wlan_scan_result scan_result;
static struct os_mutex            scan_result_mutex;

static struct os_wlan_sta_des sta_info;
static struct os_mutex        sta_info_mutex;

static struct os_wlan_event_desc event_tab[OS_WLAN_EVT_MAX];

static struct os_wlan_complete_des *complete_tab[5];
static struct os_mutex              complete_mutex;
static struct os_wlan_info         *scan_filter;

#ifdef OS_WLAN_AUTO_CONNECT_ENABLE
static struct os_timer reconnect_time;
#endif

OS_INLINE int _sta_is_null(void)
{
    if (_sta_mgnt.device == OS_NULL)
    {
        return 1;
    }
    return 0;
}

OS_INLINE int _ap_is_null(void)
{
    if (_ap_mgnt.device == OS_NULL)
    {
        return 1;
    }
    return 0;
}

OS_INLINE os_bool_t _is_do_connect(void)
{
    if ((os_wlan_get_autoreconnect_mode() == OS_FALSE) || (os_wlan_is_connected() == OS_TRUE) ||
        (_sta_mgnt.state & OS_WLAN_STATE_CONNECTING))
    {
        return OS_FALSE;
    }
    return OS_TRUE;
}

#ifdef OS_WLAN_WORK_TASK_ENABLE

static os_bool_t os_wlan_info_isequ(struct os_wlan_info *info1, struct os_wlan_info *info2)
{
    os_bool_t  is_equ                               = 1;
    os_uint8_t bssid_zero[OS_WLAN_BSSID_MAX_LENGTH] = {0};

    if (is_equ && (info1->security != SECURITY_UNKNOWN) && (info2->security != SECURITY_UNKNOWN))
    {
        is_equ &= info2->security == info1->security;
    }
    if (is_equ && ((info1->ssid.len > 0) && (info2->ssid.len > 0)))
    {
        is_equ &= info1->ssid.len == info2->ssid.len;
        is_equ &= memcmp(&info2->ssid.val[0], &info1->ssid.val[0], info1->ssid.len) == 0;
    }
    if (is_equ && (memcmp(&info1->bssid[0], bssid_zero, OS_WLAN_BSSID_MAX_LENGTH)) &&
        (memcmp(&info2->bssid[0], bssid_zero, OS_WLAN_BSSID_MAX_LENGTH)))
    {
        is_equ &= memcmp(&info1->bssid[0], &info2->bssid[0], OS_WLAN_BSSID_MAX_LENGTH) == 0;
    }
    if (is_equ && info1->datarate && info2->datarate)
    {
        is_equ &= info1->datarate == info2->datarate;
    }
    if (is_equ && (info1->channel >= 0) && (info2->channel >= 0))
    {
        is_equ &= info1->channel == info2->channel;
    }
    if (is_equ && (info1->rssi < 0) && (info2->rssi < 0))
    {
        is_equ &= info1->rssi == info2->rssi;
    }
    return is_equ;
}

static void os_wlan_mgnt_work(void *parameter)
{
    struct os_wlan_msg   *msg = parameter;
    void                 *user_parameter;
    os_wlan_event_handler handler   = OS_NULL;
    struct os_wlan_buff   user_buff = {0};
    os_base_t             level;

    /* Get user callback */
    if (msg->event < OS_WLAN_EVT_MAX)
    {
        level          = os_hw_interrupt_disable();
        handler        = event_tab[msg->event].handler;
        user_parameter = event_tab[msg->event].parameter;
        os_hw_interrupt_enable(level);
    }

    /* run user callback fun */
    if (handler)
    {
        user_buff.data = msg->buff;
        user_buff.len  = msg->len;
        OS_WLAN_LOG_D("wlan work thread run user callback, event:%d", msg->event);
        handler(msg->event, &user_buff, user_parameter);
    }

    switch (msg->event)
    {
    case OS_WLAN_EVT_STA_CONNECTED:
    {
        struct os_wlan_cfg_info cfg_info;

        memset(&cfg_info, 0, sizeof(cfg_info));
        /* save config */
        if (os_wlan_is_connected() == OS_TRUE)
        {
            os_enter_critical();
            cfg_info.info = _sta_mgnt.info;
            cfg_info.key  = _sta_mgnt.key;
            os_exit_critical();
            OS_WLAN_LOG_D("run save config! ssid:%s len%d", _sta_mgnt.info.ssid.val, _sta_mgnt.info.ssid.len);
#ifdef OS_WLAN_CFG_ENABLE
            os_wlan_cfg_save(&cfg_info);
#endif
        }
        break;
    }
    default:
        break;
    }

    os_free(msg);
}

static os_err_t os_wlan_send_to_thread(os_wlan_event_t event, void *buff, int len)
{
    struct os_wlan_msg *msg;

    OS_WLAN_LOG_D("F:%s is run event:%d", __FUNCTION__, event);

    /* Event packing */
    msg = os_malloc(sizeof(struct os_wlan_msg) + len);
    if (msg == OS_NULL)
    {
        OS_WLAN_LOG_E("wlan mgnt send msg err! No memory");
        return OS_ENOMEM;
    }
    memset(msg, 0, sizeof(struct os_wlan_msg) + len);
    msg->event = event;
    if (len != 0)
    {
        msg->buff = (void *)&msg[1];
        memcpy(msg->buff, buff, len);
        msg->len = len;
    }

    /* send event to wlan thread */
    if (os_wlan_workqueue_dowork(os_wlan_mgnt_work, msg) != OS_EOK)
    {
        os_free(msg);
        OS_WLAN_LOG_E("wlan mgnt do work fail");
        return OS_ERROR;
    }
    return OS_EOK;
}
#endif

static os_err_t os_wlan_scan_result_cache(struct os_wlan_info *info, int timeout)
{
    struct os_wlan_info *ptable;
    
    int i;
    int insert = -1;
    
    os_base_t   level;
    os_err_t    err = OS_EOK;

    if (_sta_is_null() || (info == OS_NULL) || (info->ssid.len == 0))
        return OS_EOK;

    OS_WLAN_LOG_D("ssid:%s len:%d mac:%02x:%02x:%02x:%02x:%02x:%02x",
                  info->ssid.val,
                  info->ssid.len,
                  info->bssid[0],
                  info->bssid[1],
                  info->bssid[2],
                  info->bssid[3],
                  info->bssid[4],
                  info->bssid[5]);

    err = os_mutex_recursive_lock(&scan_result_mutex, os_tick_from_ms(timeout));
    if (err != OS_EOK)
        return err;

    /* scanning result filtering */
    level = os_hw_interrupt_disable();
    if (scan_filter)
    {
        struct os_wlan_info _tmp_info = *scan_filter;
        os_hw_interrupt_enable(level);
        if (os_wlan_info_isequ(&_tmp_info, info) != OS_TRUE)
        {
            os_mutex_recursive_unlock(&scan_result_mutex);
            return OS_EOK;
        }
    }
    else
    {
        os_hw_interrupt_enable(level);
    }

    /* de-duplicatio */
    for (i = 0; i < scan_result.num; i++)
    {
        if ((info->ssid.len == scan_result.info[i].ssid.len) &&
            (memcmp(&info->bssid[0], &scan_result.info[i].bssid[0], OS_WLAN_BSSID_MAX_LENGTH) == 0))
        {
            os_mutex_recursive_unlock(&scan_result_mutex);
            return OS_EOK;
        }
#ifdef OS_WLAN_SCAN_SORT
        if (insert >= 0)
        {
            continue;
        }
        /* Signal intensity comparison */
        if ((info->rssi < 0) && (scan_result.info[i].rssi < 0))
        {
            if (info->rssi > scan_result.info[i].rssi)
            {
                insert = i;
                continue;
            }
            else if (info->rssi < scan_result.info[i].rssi)
            {
                continue;
            }
        }

        /* Channel comparison */
        if (info->channel < scan_result.info[i].channel)
        {
            insert = i;
            continue;
        }
        else if (info->channel > scan_result.info[i].channel)
        {
            continue;
        }

        /* data rate comparison */
        if ((info->datarate > scan_result.info[i].datarate))
        {
            insert = i;
            continue;
        }
        else if (info->datarate < scan_result.info[i].datarate)
        {
            continue;
        }
#endif
    }

    /* Insert the end */
    if (insert == -1)
        insert = scan_result.num;

    if (scan_result.num >= OS_WLAN_SCAN_CACHE_NUM)
        return OS_EOK;

    /* malloc memory */
    ptable = os_malloc(sizeof(struct os_wlan_info) * (scan_result.num + 1));
    if (ptable == OS_NULL)
    {
        os_mutex_recursive_unlock(&scan_result_mutex);
        OS_WLAN_LOG_E("wlan info malloc failed!");
        return OS_ENOMEM;
    }
    scan_result.num++;

    /* copy info */
    for (i = 0; i < scan_result.num; i++)
    {
        if (i < insert)
        {
            ptable[i] = scan_result.info[i];
        }
        else if (i > insert)
        {
            ptable[i] = scan_result.info[i - 1];
        }
        else if (i == insert)
        {
            ptable[i] = *info;
        }
    }
    os_free(scan_result.info);
    scan_result.info = ptable;
    os_mutex_recursive_unlock(&scan_result_mutex);
    return err;
}

static os_err_t os_wlan_sta_info_add(struct os_wlan_info *info, int timeout)
{
    struct os_wlan_sta_list *sta_list;
    os_err_t                 err = OS_EOK;

    if (_ap_is_null() || (info == OS_NULL))
        return OS_EOK;

    err = os_mutex_recursive_lock(&sta_info_mutex, os_tick_from_ms(timeout));
    if (err == OS_EOK)
    {
        /* malloc memory */
        sta_list = os_malloc(sizeof(struct os_wlan_sta_list));
        if (sta_list == OS_NULL)
        {
            os_mutex_recursive_unlock(&sta_info_mutex);
            OS_WLAN_LOG_E("sta list malloc failed!");
            return OS_ENOMEM;
        }
        sta_list->next = OS_NULL;
        sta_list->info = *info;

        /* Append sta info */
        sta_list->next = sta_info.node;
        sta_info.node  = sta_list;
        /* num++ */
        sta_info.num++;
        os_mutex_recursive_unlock(&sta_info_mutex);
        OS_WLAN_LOG_I("sta associated mac:%02x:%02x:%02x:%02x:%02x:%02x",
                      info->bssid[0],
                      info->bssid[1],
                      info->bssid[2],
                      info->bssid[3],
                      info->bssid[4],
                      info->bssid[5]);
    }
    return err;
}

static os_err_t os_wlan_sta_info_del(struct os_wlan_info *info, int timeout)
{
    struct os_wlan_sta_list *sta_list, *sta_prve;
    os_err_t                 err = OS_EOK;

    if (_ap_is_null() || (info == OS_NULL))
        return OS_EOK;

    err = os_mutex_recursive_lock(&sta_info_mutex, os_tick_from_ms(timeout));
    if (err == OS_EOK)
    {
        /* traversing the list */
        for (sta_list = sta_info.node, sta_prve = OS_NULL; sta_list != OS_NULL;
             sta_prve = sta_list, sta_list = sta_list->next)
        {
            /* find mac addr */
            if (memcmp(&sta_list->info.bssid[0], &info->bssid[0], OS_WLAN_BSSID_MAX_LENGTH) == 0)
            {
                if (sta_prve == OS_NULL)
                {
                    sta_info.node = sta_list->next;
                }
                else
                {
                    sta_prve->next = sta_list->next;
                }
                sta_info.num--;
                os_free(sta_list);
                break;
            }
        }
        os_mutex_recursive_unlock(&sta_info_mutex);
        OS_WLAN_LOG_I("sta exit mac:%02x:%02x:%02x:%02x:%02x:%02x",
                      info->bssid[0],
                      info->bssid[1],
                      info->bssid[2],
                      info->bssid[3],
                      info->bssid[4],
                      info->bssid[5]);
    }
    return err;
}

static os_err_t os_wlan_sta_info_del_all(int timeout)
{
    os_err_t    err = OS_EOK;
    
    struct os_wlan_sta_list *sta_list, *sta_next;

    err = os_mutex_recursive_lock(&sta_info_mutex, os_tick_from_ms(timeout));
    if (err == OS_EOK)
    {
        /* traversing the list */
        for (sta_list = sta_info.node; sta_list != OS_NULL; sta_list = sta_next)
        {
            sta_next = sta_list->next;
            sta_info.num--;
            os_free(sta_list);
        }
        os_mutex_recursive_unlock(&sta_info_mutex);
    }
    if (sta_info.num != 0)
    {
        OS_WLAN_LOG_W("\n\n!!!Program runing exception!!!\n\n");
    }
    sta_info.num  = 0;
    sta_info.node = OS_NULL;
    return err;
}
#ifdef OS_WLAN_AUTO_CONNECT_ENABLE
static void auto_connect_clean(struct os_work *work)
{
    os_free(work);
}

static void os_wlan_auto_connect_run(struct os_work *work, void *data)
{
    char                *password = OS_NULL;
    os_base_t           level;
    static os_uint32_t  id = 0;
    
    struct os_wlan_cfg_info cfg_info;

    OS_WLAN_LOG_D("F:%s is run", __FUNCTION__);

    if (os_mutex_recursive_lock(&mgnt_mutex, 0) != OS_EOK)
        goto exit;

    /* auto connect status is disable or wifi is connect or connecting, exit */
    if (_is_do_connect() == OS_FALSE)
    {
        id = 0;
        OS_WLAN_LOG_D("not connection");
        goto exit;
    }

    /* Read the next configuration */
    memset(&cfg_info, 0, sizeof(struct os_wlan_cfg_info));
    if (os_wlan_cfg_read_index(&cfg_info, id++) == 0)
    {
        OS_WLAN_LOG_D("read cfg fail");
        id = 0;
        goto exit;
    }

    if (id >= os_wlan_cfg_get_num())
        id = 0;

    if ((cfg_info.key.len > 0) && (cfg_info.key.len <= OS_WLAN_PASSWORD_MAX_LENGTH))
    {
        cfg_info.key.val[cfg_info.key.len] = '\0';
        password                           = (char *)(&cfg_info.key.val[0]);
    }
    os_wlan_connect((char *)cfg_info.info.ssid.val, password);
exit:
    os_mutex_recursive_unlock(&mgnt_mutex);
    level = os_hw_interrupt_disable();
    // memset(work, 0, sizeof(struct os_work));
    os_hw_interrupt_enable(level);
    os_work_deinit(work, auto_connect_clean);
}

static void os_wlan_cyclic_check(void *parameter)
{
    struct os_workqueue *workqueue;
    struct os_work      *work = os_malloc(sizeof(struct os_work));
    os_base_t            level;

    if ((_is_do_connect() == OS_TRUE) && (work->func == OS_NULL))
    {
        workqueue = os_wlan_get_workqueue();
        if (workqueue != OS_NULL)
        {
            level = os_hw_interrupt_disable();
            os_work_init(work, os_wlan_auto_connect_run, OS_NULL);
            os_hw_interrupt_enable(level);
            if (os_workqueue_submit_work(workqueue, work, 0) != OS_EOK)
            {
                level = os_hw_interrupt_disable();
                // memset(&work, 0, sizeof(struct os_work));
                os_free(work);
                os_hw_interrupt_enable(level);
            }
        }
    }
}
#endif

static void os_wlan_event_dispatch(struct os_wlan_device *device,
                                   os_wlan_dev_event_t    event,
                                   struct os_wlan_buff   *buff,
                                   void                  *parameter)
{
    os_err_t *          err        = OS_NULL;
    os_wlan_event_t     user_event = OS_WLAN_EVT_MAX;
    int                 i;
    struct os_wlan_buff user_buff = {0};

    if (buff)
    {
        user_buff = *buff;
    }
    /* Event Handle */
    switch (event)
    {
    case OS_WLAN_DEV_EVT_CONNECT:
    {
        OS_WLAN_LOG_D("event: CONNECT");
        _sta_mgnt.state |= OS_WLAN_STATE_CONNECT;
        _sta_mgnt.state &= ~OS_WLAN_STATE_CONNECTING;
        user_event = OS_WLAN_EVT_STA_CONNECTED;
        TIME_STOP();
        user_buff.data = &_sta_mgnt.info;
        user_buff.len  = sizeof(struct os_wlan_info);
        OS_WLAN_LOG_I("wifi connect success ssid:%s", &_sta_mgnt.info.ssid.val[0]);
        break;
    }
    case OS_WLAN_DEV_EVT_CONNECT_FAIL:
    {
        OS_WLAN_LOG_D("event: CONNECT_FAIL");
        _sta_mgnt.state &= ~OS_WLAN_STATE_CONNECT;
        _sta_mgnt.state &= ~OS_WLAN_STATE_CONNECTING;
        _sta_mgnt.state &= ~OS_WLAN_STATE_READY;
        user_event     = OS_WLAN_EVT_STA_CONNECTED_FAIL;
        user_buff.data = &_sta_mgnt.info;
        user_buff.len  = sizeof(struct os_wlan_info);
        if (os_wlan_get_autoreconnect_mode())
        {
            TIME_START();
        }
        break;
    }
    case OS_WLAN_DEV_EVT_DISCONNECT:
    {
        OS_WLAN_LOG_D("event: DISCONNECT");
        _sta_mgnt.state &= ~OS_WLAN_STATE_CONNECT;
        _sta_mgnt.state &= ~OS_WLAN_STATE_READY;
        user_event     = OS_WLAN_EVT_STA_DISCONNECTED;
        user_buff.data = &_sta_mgnt.info;
        user_buff.len  = sizeof(struct os_wlan_info);
        if (os_wlan_get_autoreconnect_mode())
        {
            TIME_START();
        }
        break;
    }
    case OS_WLAN_DEV_EVT_AP_START:
    {
        OS_WLAN_LOG_D("event: AP_START");
        _ap_mgnt.state |= OS_WLAN_STATE_ACTIVE;
        user_event     = OS_WLAN_EVT_AP_START;
        user_buff.data = &_ap_mgnt.info;
        user_buff.len  = sizeof(struct os_wlan_info);
        break;
    }
    case OS_WLAN_DEV_EVT_AP_STOP:
    {
        OS_WLAN_LOG_D("event: AP_STOP");
        _ap_mgnt.state &= ~OS_WLAN_STATE_ACTIVE;
        user_event = OS_WLAN_EVT_AP_STOP;
        err        = (os_err_t *)os_wlan_sta_info_del_all(OS_IPC_WAITING_FOREVER);
        if (err != OS_NULL)
        {
            OS_WLAN_LOG_W("AP_STOP event handle fail");
        }
        user_buff.data = &_ap_mgnt.info;
        user_buff.len  = sizeof(struct os_wlan_info);
        break;
    }
    case OS_WLAN_DEV_EVT_AP_ASSOCIATED:
    {
        OS_WLAN_LOG_D("event: ASSOCIATED");
        user_event = OS_WLAN_EVT_AP_ASSOCIATED;
        if (user_buff.len != sizeof(struct os_wlan_info))
            break;
        err = (os_err_t *)os_wlan_sta_info_add(user_buff.data, OS_IPC_WAITING_FOREVER);
        if (err != OS_EOK)
        {
            OS_WLAN_LOG_W("AP_ASSOCIATED event handle fail");
        }
        break;
    }
    case OS_WLAN_DEV_EVT_AP_DISASSOCIATED:
    {
        OS_WLAN_LOG_D("event: DISASSOCIATED");
        user_event = OS_WLAN_EVT_AP_DISASSOCIATED;
        if (user_buff.len != sizeof(struct os_wlan_info))
            break;
        err = (os_err_t *)os_wlan_sta_info_del(user_buff.data, OS_IPC_WAITING_FOREVER);
        if (err != OS_EOK)
        {
            OS_WLAN_LOG_W("AP_DISASSOCIATED event handle fail");
        }
        break;
    }
    case OS_WLAN_DEV_EVT_AP_ASSOCIATE_FAILED:
    {
        OS_WLAN_LOG_D("event: AP_ASSOCIATE_FAILED");
        break;
    }
    case OS_WLAN_DEV_EVT_SCAN_REPORT:
    {
        OS_WLAN_LOG_D("event: SCAN_REPORT");
        user_event = OS_WLAN_EVT_SCAN_REPORT;
        if (user_buff.len != sizeof(struct os_wlan_info))
            break;
        os_wlan_scan_result_cache(user_buff.data, 0);
        break;
    }
    case OS_WLAN_DEV_EVT_SCAN_DONE:
    {
        OS_WLAN_LOG_D("event: SCAN_DONE");
        user_buff.data = &scan_result;
        user_buff.len  = sizeof(scan_result);
        user_event     = OS_WLAN_EVT_SCAN_DONE;
        break;
    }
    default:
    {
        OS_WLAN_LOG_D("event: UNKNOWN");
        return;
    }
    }

    /* send event */
    COMPLETE_LOCK();
    for (i = 0; i < sizeof(complete_tab) / sizeof(complete_tab[0]); i++)
    {
        if ((complete_tab[i] != OS_NULL))
        {
            complete_tab[i]->event_flag |= 0x1 << event;
            os_event_send(&complete_tab[i]->complete, 0x1 << event);
            OS_WLAN_LOG_D("&complete_tab[i]->complete:0x%08x", &complete_tab[i]->complete);
        }
    }
    COMPLETE_UNLOCK();
#ifdef OS_WLAN_WORK_TASK_ENABLE
    os_wlan_send_to_thread(user_event, OS_NULL, 0);
#else
    {
        void                 *user_parameter;
        os_wlan_event_handler handler = OS_NULL;
        os_base_t             level;
        /* Get user callback */
        if (user_event < OS_WLAN_EVT_MAX)
        {
            level          = os_hw_interrupt_disable();
            handler        = event_tab[user_event].handler;
            user_parameter = event_tab[user_event].parameter;
            os_hw_interrupt_enable(level);
        }

        /* run user callback fun */
        if (handler)
        {
            OS_WLAN_LOG_D("unknown thread run user callback, event:%d", user_event);
            handler(user_event, &user_buff, user_parameter);
        }
    }
#endif
}

static struct os_wlan_complete_des *os_wlan_complete_create(const char *name)
{
    int i;
    
    struct os_wlan_complete_des *complete;

    complete = os_malloc(sizeof(struct os_wlan_complete_des));
    if (complete == OS_NULL)
    {
        OS_WLAN_LOG_E("complete event create failed");
        MGNT_UNLOCK();
        return complete;
    }
    os_event_init(&complete->complete, name, OS_IPC_FLAG_FIFO);
    complete->event_flag = 0;
    // protect
    COMPLETE_LOCK();
    for (i = 0; i < sizeof(complete_tab) / sizeof(complete_tab[0]); i++)
    {
        if (complete_tab[i] == OS_NULL)
        {
            complete->index = i;
            complete_tab[i] = complete;
            break;
        }
    }
    COMPLETE_UNLOCK();

    if (i >= sizeof(complete_tab) / sizeof(complete_tab[0]))
    {
        os_event_deinit(&complete->complete);
        os_free(complete);
        complete = OS_NULL;
    }

    return complete;
}

static os_err_t os_wlan_complete_wait(struct os_wlan_complete_des *complete,
                                      os_uint32_t                  event,
                                      os_uint32_t                  timeout,
                                      os_uint32_t                 *recved)
{
    if (complete == OS_NULL)
    {
        return OS_ERROR;
    }

    /* Check whether there is a waiting event */
    if (complete->event_flag & event)
    {
        *recved = complete->event_flag;
        return OS_EOK;
    }
    else
    {
        return os_event_recv(&complete->complete, event, OS_EVENT_OPTION_OR, os_tick_from_ms(timeout), recved);
    }
}

static void os_wlan_complete_delete(struct os_wlan_complete_des *complete)
{
    if (complete == OS_NULL)
    {
        return;
    }
    COMPLETE_LOCK();
    complete_tab[complete->index] = OS_NULL;
    COMPLETE_UNLOCK();
    os_event_deinit(&complete->complete);
    os_free(complete);
}

os_err_t os_wlan_set_mode(const char *dev_name, os_wlan_mode_t mode)
{
    os_device_t *             device = OS_NULL;
    os_err_t                  err;
    os_int8_t                 up_event_flag = 0;
    os_wlan_dev_event_handler handler       = OS_NULL;

    if ((dev_name == OS_NULL) || (mode >= OS_WLAN_MODE_MAX))
    {
        OS_WLAN_LOG_E("Parameter Wrongful name:%s mode:%d", dev_name, mode);
        return OS_EINVAL;
    }

    OS_WLAN_LOG_D("%s is run dev_name:%s mode:%s%s%s",
                  __FUNCTION__,
                  dev_name,
                  mode == OS_WLAN_NONE ? "NONE" : "",
                  mode == OS_WLAN_STATION ? "STA" : "",
                  mode == OS_WLAN_AP ? "AP" : "");

    /* find device */
    device = os_device_find(dev_name);
    if (device == OS_NULL)
    {
        OS_WLAN_LOG_E("not find device, set mode failed! name:%s", dev_name);
        return OS_EIO;
    }

    MGNT_LOCK();
    if (OS_WLAN_DEVICE(device)->mode == mode)
    {
        OS_WLAN_LOG_D("L:%d this device mode is set");
        MGNT_UNLOCK();
        return OS_EOK;
    }

    if ((mode == OS_WLAN_STATION) && (OS_WLAN_DEVICE(device)->flags & OS_WLAN_FLAG_AP_ONLY))
    {
        OS_WLAN_LOG_I("this device ap mode only");
        MGNT_UNLOCK();
        return OS_ERROR;
    }
    else if ((mode == OS_WLAN_AP) && (OS_WLAN_DEVICE(device)->flags & OS_WLAN_FLAG_STA_ONLY))
    {
        OS_WLAN_LOG_I("this device sta mode only");
        MGNT_UNLOCK();
        return OS_ERROR;
    }

    /*
     * device == sta  and change to ap,  should deinit
     * device == ap   and change to sta, should deinit
     */
    if (((mode == OS_WLAN_STATION) && (OS_WLAN_DEVICE(device) == AP_DEVICE())) ||
        ((mode == OS_WLAN_AP) && (OS_WLAN_DEVICE(device) == STA_DEVICE())))
    {
        err = os_wlan_set_mode(dev_name, OS_WLAN_NONE);
        if (err != OS_EOK)
        {
            OS_WLAN_LOG_E("change mode failed!");
            MGNT_UNLOCK();
            return err;
        }
    }

    /* init device */
    err = os_wlan_dev_init(OS_WLAN_DEVICE(device), mode);
    if (err != OS_EOK)
    {
        OS_WLAN_LOG_E("F:%s L:%d wlan init failed", __FUNCTION__, __LINE__);
        MGNT_UNLOCK();
        return err;
    }

    /* the mode is none */
    if (mode == OS_WLAN_NONE)
    {
        if (_sta_mgnt.device == OS_WLAN_DEVICE(device))
        {
            _sta_mgnt.device = OS_NULL;
            _sta_mgnt.state  = 0;
            up_event_flag    = 1;
            handler          = OS_NULL;
        }
        else if (_ap_mgnt.device == OS_WLAN_DEVICE(device))
        {
            _ap_mgnt.state  = 0;
            _ap_mgnt.device = OS_NULL;
            up_event_flag   = 1;
            handler         = OS_NULL;
        }
    }
    /* save sta device */
    else if (mode == OS_WLAN_STATION)
    {
        up_event_flag    = 1;
        handler          = os_wlan_event_dispatch;
        _sta_mgnt.device = OS_WLAN_DEVICE(device);
    }
    /* save ap device */
    else if (mode == OS_WLAN_AP)
    {
        up_event_flag   = 1;
        handler         = os_wlan_event_dispatch;
        _ap_mgnt.device = OS_WLAN_DEVICE(device);
    }

    /* update dev event handle */
    if (up_event_flag == 1)
    {
        if (handler)
        {
            if (mode == OS_WLAN_STATION)
            {
                os_wlan_dev_register_event_handler(OS_WLAN_DEVICE(device), OS_WLAN_DEV_EVT_CONNECT, handler, OS_NULL);
                os_wlan_dev_register_event_handler(OS_WLAN_DEVICE(device), OS_WLAN_DEV_EVT_CONNECT_FAIL, handler, OS_NULL);
                os_wlan_dev_register_event_handler(OS_WLAN_DEVICE(device), OS_WLAN_DEV_EVT_DISCONNECT, handler, OS_NULL);
                os_wlan_dev_register_event_handler(OS_WLAN_DEVICE(device), OS_WLAN_DEV_EVT_SCAN_REPORT, handler, OS_NULL);
                os_wlan_dev_register_event_handler(OS_WLAN_DEVICE(device), OS_WLAN_DEV_EVT_SCAN_DONE, handler, OS_NULL);
            }
            else if (mode == OS_WLAN_AP)
            {
                os_wlan_dev_register_event_handler(OS_WLAN_DEVICE(device), OS_WLAN_DEV_EVT_AP_START, handler, OS_NULL);
                os_wlan_dev_register_event_handler(OS_WLAN_DEVICE(device), OS_WLAN_DEV_EVT_AP_STOP, handler, OS_NULL);
                os_wlan_dev_register_event_handler(OS_WLAN_DEVICE(device), OS_WLAN_DEV_EVT_AP_ASSOCIATED, handler, OS_NULL);
                os_wlan_dev_register_event_handler(OS_WLAN_DEVICE(device), OS_WLAN_DEV_EVT_AP_DISASSOCIATED, handler, OS_NULL);
                os_wlan_dev_register_event_handler(OS_WLAN_DEVICE(device), OS_WLAN_DEV_EVT_AP_ASSOCIATE_FAILED, handler, OS_NULL);
            }
        }
        else
        {
            os_wlan_dev_event_t event;
            handler = os_wlan_event_dispatch;
            for (event = OS_WLAN_DEV_EVT_INIT_DONE; event < OS_WLAN_DEV_EVT_MAX; event++)
            {
                os_wlan_dev_unregister_event_handler(OS_WLAN_DEVICE(device), event, handler);
            }
        }
    }
    MGNT_UNLOCK();

    /* Mount protocol */
#if defined(OS_WLAN_PROT_ENABLE) && defined(OS_WLAN_DEFAULT_PROT)
    if (err == OS_EOK)
    {
        os_wlan_prot_attach(dev_name, OS_WLAN_DEFAULT_PROT);
    }
#endif
    return err;
}

os_wlan_mode_t os_wlan_get_mode(const char *dev_name)
{
    os_device_t *  device = OS_NULL;
    os_wlan_mode_t mode;

    if (dev_name == OS_NULL)
    {
        OS_WLAN_LOG_E("name is null");
        return OS_WLAN_NONE;
    }

    /* find device */
    device = os_device_find(dev_name);
    if (device == OS_NULL)
    {
        OS_WLAN_LOG_E("device not find! name:%s", dev_name);
        return OS_WLAN_NONE;
    }

    /* get mode */
    mode = OS_WLAN_DEVICE(device)->mode;
    OS_WLAN_LOG_D("%s is run dev_name:%s mode:%s%s%s",
                  __FUNCTION__,
                  dev_name,
                  mode == OS_WLAN_NONE ? "NONE" : "",
                  mode == OS_WLAN_STATION ? "STA" : "",
                  mode == OS_WLAN_AP ? "AP" : "");

    return mode;
}

os_bool_t os_wlan_find_best_by_cache(const char *ssid, struct os_wlan_info *info)
{
    int i;
    int ssid_len;
    
    struct os_wlan_info        *info_best;
    struct os_wlan_scan_result *result;

    ssid_len  = strlen(ssid);
    result    = &scan_result;
    info_best = OS_NULL;

    SRESULT_LOCK();
    for (i = 0; i < result->num; i++)
    {
        /* SSID is equal. */
        if ((result->info[i].ssid.len == ssid_len) &&
            (memcmp((char *)&result->info[i].ssid.val[0], ssid, ssid_len) == 0))
        {
            if (info_best == OS_NULL)
            {
                info_best = &result->info[i];
                continue;
            }
            /* Signal strength effective */
            if ((result->info[i].rssi < 0) && (info_best->rssi < 0))
            {
                /* Find the strongest signal. */
                if (result->info[i].rssi > info_best->rssi)
                {
                    info_best = &result->info[i];
                    continue;
                }
                else if (result->info[i].rssi < info_best->rssi)
                {
                    continue;
                }
            }

            /* Finding the fastest signal */
            if (result->info[i].datarate > info_best->datarate)
            {
                info_best = &result->info[i];
                continue;
            }
        }
    }
    SRESULT_UNLOCK();

    if (info_best == OS_NULL)
        return OS_FALSE;

    *info = *info_best;
    return OS_TRUE;
}

os_err_t os_wlan_connect(const char *ssid, const char *password)
{
    os_err_t                     err      = OS_EOK;
    int                          ssid_len = 0;
    struct os_wlan_info          info;
    struct os_wlan_complete_des *complete;
    os_uint32_t                  set = 0, recved = 0;
    os_uint32_t                  scan_retry = OS_WLAN_SCAN_RETRY_CNT;

    /* sta dev Can't be NULL */
    if (_sta_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run ssid:%s password:%s", __FUNCTION__, ssid, password);
    if (ssid == OS_NULL)
    {
        OS_WLAN_LOG_E("ssid is null!");
        return OS_EINVAL;
    }
    ssid_len = strlen(ssid);
    if (ssid_len > OS_WLAN_SSID_MAX_LENGTH)
    {
        OS_WLAN_LOG_E("ssid is to long! ssid:%s len:%d", ssid, ssid_len);
        return OS_EINVAL;
    }

    if ((os_wlan_is_connected() == OS_TRUE) && (strcmp((char *)&_sta_mgnt.info.ssid.val[0], ssid) == 0))
    {
        OS_WLAN_LOG_I("wifi is connect ssid:%s", ssid);
        return OS_EOK;
    }
    /* get info from cache */
    INVALID_INFO(&info);
    MGNT_LOCK();
    while (scan_retry-- && os_wlan_find_best_by_cache(ssid, &info) != OS_TRUE)
    {
        os_wlan_scan_sync();
    }
    os_wlan_scan_result_clean();

    if (info.ssid.len <= 0)
    {
        OS_WLAN_LOG_W("not find ap! ssid:%s", ssid);
        MGNT_UNLOCK();
        return OS_ERROR;
    }

    OS_WLAN_LOG_D("find best info ssid:%s mac: %02x %02x %02x %02x %02x %02x",
                  info.ssid.val,
                  info.bssid[0],
                  info.bssid[1],
                  info.bssid[2],
                  info.bssid[3],
                  info.bssid[4],
                  info.bssid[5]);

    /* create event wait complete */
    complete = os_wlan_complete_create("join");
    if (complete == OS_NULL)
    {
        MGNT_UNLOCK();
        return OS_ENOMEM;
    }
    /* run connect adv */
    err = os_wlan_connect_adv(&info, password);
    if (err != OS_EOK)
    {
        os_wlan_complete_delete(complete);
        MGNT_UNLOCK();
        return err;
    }

    /* Initializing events that need to wait */
    set |= 0x1 << OS_WLAN_DEV_EVT_CONNECT;
    set |= 0x1 << OS_WLAN_DEV_EVT_CONNECT_FAIL;
    /* Check whether there is a waiting event */
    os_wlan_complete_wait(complete, set, OS_WLAN_CONNECT_WAIT_MS, &recved);
    os_wlan_complete_delete(complete);
    /* check event */
    set = 0x1 << OS_WLAN_DEV_EVT_CONNECT;
    if (!(recved & set))
    {
        OS_WLAN_LOG_I("wifi connect failed!");
        MGNT_UNLOCK();
        return OS_ERROR;
    }
    MGNT_UNLOCK();
    return err;
}

os_err_t os_wlan_connect_adv(struct os_wlan_info *info, const char *password)
{
    int      password_len = 0;
    os_err_t err          = OS_EOK;

    if (_sta_is_null())
    {
        return OS_EIO;
    }
    if (info == OS_NULL)
    {
        OS_WLAN_LOG_E("info is null!");
        return OS_EINVAL;
    }
    OS_WLAN_LOG_D("%s is run ssid:%s password:%s", __FUNCTION__, info->ssid.val, password);
    /* Parameter checking */
    if (password != OS_NULL)
    {
        password_len = strlen(password);
        if (password_len > OS_WLAN_PASSWORD_MAX_LENGTH)
        {
            OS_WLAN_LOG_E("password is to long! password:%s len:%d", password, password_len);
            return OS_EINVAL;
        }
    }
    if (info->ssid.len == 0 || info->ssid.len > OS_WLAN_SSID_MAX_LENGTH)
    {
        OS_WLAN_LOG_E("ssid is zero or to long! ssid:%s len:%d", info->ssid.val, info->ssid.len);
        return OS_EINVAL;
    }
    /* is connect ? */
    MGNT_LOCK();
    if (os_wlan_is_connected())
    {
        if ((_sta_mgnt.info.ssid.len == info->ssid.len) && (_sta_mgnt.key.len == password_len) &&
            (memcmp(&_sta_mgnt.info.ssid.val[0], &info->ssid.val[0], info->ssid.len) == 0) &&
            (memcmp(&_sta_mgnt.info.bssid[0], &info->bssid[0], OS_WLAN_BSSID_MAX_LENGTH) == 0) &&
            (memcmp(&_sta_mgnt.key.val[0], password, password_len) == 0))
        {
            OS_WLAN_LOG_I("wifi Already Connected");
            MGNT_UNLOCK();
            return OS_EOK;
        }

        err = os_wlan_disconnect();
        if (err != OS_EOK)
        {
            MGNT_UNLOCK();
            return err;
        }
    }

    /* save info */
    os_enter_critical();
    _sta_mgnt.info = *info;
    memcpy(&_sta_mgnt.key.val, password, password_len);
    _sta_mgnt.key.len               = password_len;
    _sta_mgnt.key.val[password_len] = '\0';
    os_exit_critical();
    /* run wifi connect */
    _sta_mgnt.state |= OS_WLAN_STATE_CONNECTING;
    err = os_wlan_dev_connect(_sta_mgnt.device, info, password, password_len);
    if (err != OS_EOK)
    {
        os_enter_critical();
        memset(&_sta_mgnt.info, 0, sizeof(struct os_wlan_ssid));
        memset(&_sta_mgnt.key, 0, sizeof(struct os_wlan_key));
        os_exit_critical();
        _sta_mgnt.state &= ~OS_WLAN_STATE_CONNECTING;
        MGNT_UNLOCK();
        return err;
    }

    MGNT_UNLOCK();
    return err;
}

os_err_t os_wlan_disconnect(void)
{
    os_err_t                     err;
    struct os_wlan_complete_des *complete;
    os_uint32_t                  recved = 0, set = 0;

    /* ap dev Can't be empty */
    if (_sta_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run", __FUNCTION__);

    /* run disconnect */
    MGNT_LOCK();
    /* create event wait complete */
    complete = os_wlan_complete_create("disc");
    if (complete == OS_NULL)
    {
        MGNT_UNLOCK();
        return OS_ENOMEM;
    }
    err = os_wlan_dev_disconnect(_sta_mgnt.device);
    if (err != OS_EOK)
    {
        OS_WLAN_LOG_E("wifi disconnect fail");
        os_wlan_complete_delete(complete);
        MGNT_UNLOCK();
        return err;
    }
    /* Initializing events that need to wait */
    set |= 0x1 << OS_WLAN_DEV_EVT_DISCONNECT;
    /* Check whether there is a waiting event */
    os_wlan_complete_wait(complete, set, OS_WLAN_CONNECT_WAIT_MS, &recved);
    os_wlan_complete_delete(complete);
    /* check event */
    set = 0x1 << OS_WLAN_DEV_EVT_DISCONNECT;
    if (!(recved & set))
    {
        OS_WLAN_LOG_E("disconnect failed!");
        MGNT_UNLOCK();
        return OS_ERROR;
    }
    OS_WLAN_LOG_I("disconnect success!");
    MGNT_UNLOCK();
    return err;
}

os_bool_t os_wlan_is_connected(void)
{
    os_bool_t _connect;

    if (_sta_is_null())
    {
        return OS_FALSE;
    }
    _connect = _sta_mgnt.state & OS_WLAN_STATE_CONNECT ? OS_TRUE : OS_FALSE;
    OS_WLAN_LOG_D("%s is run : %s", __FUNCTION__, _connect ? "connect" : "disconnect");
    return _connect;
}

os_bool_t os_wlan_is_ready(void)
{
    os_bool_t _ready;

    if (_sta_is_null())
    {
        return OS_FALSE;
    }
    _ready = _sta_mgnt.state & OS_WLAN_STATE_READY ? OS_TRUE : OS_FALSE;
    OS_WLAN_LOG_D("%s is run : %s", __FUNCTION__, _ready ? "ready" : "not ready");
    return _ready;
}

os_err_t os_wlan_set_mac(os_uint8_t mac[6])
{
    os_err_t err = OS_EOK;

    if (_sta_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run mac: %02x:%02x:%02x:%02x:%02x:%02x",
                  __FUNCTION__,
                  mac[0],
                  mac[1],
                  mac[2],
                  mac[3],
                  mac[4],
                  mac[5]);

    MGNT_LOCK();
    err = os_wlan_dev_set_mac(STA_DEVICE(), mac);
    if (err != OS_EOK)
    {
        OS_WLAN_LOG_E("set sta mac addr fail");
        MGNT_UNLOCK();
        return err;
    }
    MGNT_UNLOCK();
    return err;
}

os_err_t os_wlan_get_mac(os_uint8_t mac[6])
{
    os_err_t err = OS_EOK;

    if (_sta_is_null())
    {
        return OS_EIO;
    }
    MGNT_LOCK();
    err = os_wlan_dev_get_mac(STA_DEVICE(), mac);
    if (err != OS_EOK)
    {
        OS_WLAN_LOG_E("get sta mac addr fail");
        MGNT_UNLOCK();
        return err;
    }
    OS_WLAN_LOG_D("%s is run mac: %02x:%02x:%02x:%02x:%02x:%02x",
                  __FUNCTION__,
                  mac[0],
                  mac[1],
                  mac[2],
                  mac[3],
                  mac[4],
                  mac[5]);
    MGNT_UNLOCK();
    return err;
}

os_err_t os_wlan_get_info(struct os_wlan_info *info)
{
    if (_sta_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run", __FUNCTION__);

    if (os_wlan_is_connected() == OS_TRUE)
    {
        *info      = _sta_mgnt.info;
        info->rssi = os_wlan_get_rssi();
        return OS_EOK;
    }
    return OS_ERROR;
}

int os_wlan_get_rssi(void)
{
    int rssi = 0;

    if (_sta_is_null())
    {
        return OS_EIO;
    }

    MGNT_LOCK();
    rssi = os_wlan_dev_get_rssi(STA_DEVICE());
    OS_WLAN_LOG_D("%s is run rssi:%d", __FUNCTION__, rssi);
    MGNT_UNLOCK();
    return rssi;
}

os_err_t os_wlan_staos_ap(const char *ssid, const char *password)
{
    os_err_t                     err      = OS_EOK;
    int                          ssid_len = 0;
    struct os_wlan_info          info;
    struct os_wlan_complete_des *complete;
    os_uint32_t                  set = 0, recved = 0;

    if (_ap_is_null())
    {
        return OS_EIO;
    }
    if (ssid == OS_NULL)
        return OS_EINVAL;

    memset(&info, 0, sizeof(struct os_wlan_info));
    OS_WLAN_LOG_D("%s is run ssid:%s password:%s", __FUNCTION__, ssid, password);
    if (password)
    {
        info.security = SECURITY_WPA2_AES_PSK;
    }
    ssid_len = strlen(ssid);
    if (ssid_len > OS_WLAN_SSID_MAX_LENGTH)
    {
        OS_WLAN_LOG_E("ssid is to long! len:%d", ssid_len);
    }

    /* copy info */
    memcpy(&info.ssid.val, ssid, ssid_len);
    info.ssid.len = ssid_len;
    info.channel  = 6;
    info.band     = OS_802_11_BAND_2_4GHZ;

    /* Initializing events that need to wait */
    MGNT_LOCK();
    /* create event wait complete */
    complete = os_wlan_complete_create("staos_ap");
    if (complete == OS_NULL)
    {
        MGNT_UNLOCK();
        return OS_ENOMEM;
    }

    /* start ap */
    err = os_wlan_staos_ap_adv(&info, password);
    if (err != OS_EOK)
    {
        os_wlan_complete_delete(complete);
        OS_WLAN_LOG_I("start ap failed!");
        MGNT_UNLOCK();
        return err;
    }

    /* Initializing events that need to wait */
    set |= 0x1 << OS_WLAN_DEV_EVT_AP_START;
    set |= 0x1 << OS_WLAN_DEV_EVT_AP_STOP;
    /* Check whether there is a waiting event */
    os_wlan_complete_wait(complete, set, OS_WLAN_STAOS_AP_WAIT_MS, &recved);
    os_wlan_complete_delete(complete);
    /* check event */
    set = 0x1 << OS_WLAN_DEV_EVT_AP_START;
    if (!(recved & set))
    {
        OS_WLAN_LOG_I("start ap failed!");
        MGNT_UNLOCK();
        return OS_ERROR;
    }
    OS_WLAN_LOG_I("start ap successs!");
    MGNT_UNLOCK();
    return err;
}

os_err_t os_wlan_staos_ap_adv(struct os_wlan_info *info, const char *password)
{
    os_err_t err          = OS_EOK;
    int      password_len = 0;

    if (_ap_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run", __FUNCTION__);
    if (password != OS_NULL)
    {
        password_len = strlen(password);
    }
    if (password_len > OS_WLAN_PASSWORD_MAX_LENGTH)
    {
        OS_WLAN_LOG_E("key is to long! len:%d", password_len);
        return OS_EINVAL;
    }
    /* is start up ? */
    MGNT_LOCK();
    if (os_wlan_ap_is_active())
    {
        if ((_ap_mgnt.info.ssid.len == info->ssid.len) && (_ap_mgnt.info.security == info->security) &&
            (_ap_mgnt.info.channel == info->channel) && (_ap_mgnt.info.hidden == info->hidden) &&
            (_ap_mgnt.key.len == password_len) &&
            (memcmp(&_ap_mgnt.info.ssid.val[0], &info->ssid.val[0], info->ssid.len) == 0) &&
            (memcmp(&_ap_mgnt.key.val[0], password, password_len)))
        {
            OS_WLAN_LOG_D("wifi Already Start");
            MGNT_UNLOCK();
            return OS_EOK;
        }
    }

    err = os_wlan_dev_ap_start(AP_DEVICE(), info, password, password_len);
    if (err != OS_EOK)
    {
        MGNT_UNLOCK();
        return err;
    }
    memcpy(&_ap_mgnt.info, info, sizeof(struct os_wlan_info));
    memcpy(&_ap_mgnt.key.val, password, password_len);
    _ap_mgnt.key.len = password_len;

    MGNT_UNLOCK();
    return err;
}

os_bool_t os_wlan_ap_is_active(void)
{
    os_bool_t _active = OS_FALSE;

    if (_ap_is_null())
    {
        return OS_FALSE;
    }

    _active = _ap_mgnt.state & OS_WLAN_STATE_ACTIVE ? OS_TRUE : OS_FALSE;
    OS_WLAN_LOG_D("%s is run active:%s", __FUNCTION__, _active ? "Active" : "Inactive");
    return _active;
}

os_err_t os_wlan_ap_stop(void)
{
    os_err_t                     err = OS_EOK;
    struct os_wlan_complete_des *complete;
    os_uint32_t                  set = 0, recved = 0;

    if (_ap_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run", __FUNCTION__);

    MGNT_LOCK();
    /* create event wait complete */
    complete = os_wlan_complete_create("stop_ap");
    if (complete == OS_NULL)
    {
        MGNT_UNLOCK();
        return OS_ENOMEM;
    }
    err = os_wlan_dev_ap_stop(AP_DEVICE());
    if (err != OS_EOK)
    {
        OS_WLAN_LOG_E("ap stop fail");
        os_wlan_complete_delete(complete);
        MGNT_UNLOCK();
        return err;
    }
    /* Initializing events that need to wait */
    set |= 0x1 << OS_WLAN_DEV_EVT_AP_STOP;
    /* Check whether there is a waiting event */
    os_wlan_complete_wait(complete, set, OS_WLAN_STAOS_AP_WAIT_MS, &recved);
    os_wlan_complete_delete(complete);
    /* check event */
    set = 0x1 << OS_WLAN_DEV_EVT_AP_STOP;
    if (!(recved & set))
    {
        OS_WLAN_LOG_I("ap stop failed!");
        MGNT_UNLOCK();
        return OS_ERROR;
    }
    OS_WLAN_LOG_I("ap stop success!");
    MGNT_UNLOCK();
    return err;
}

os_err_t os_wlan_ap_get_info(struct os_wlan_info *info)
{
    if (_ap_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run", __FUNCTION__);

    if (os_wlan_ap_is_active() == OS_TRUE)
    {
        *info = _ap_mgnt.info;
        return OS_EOK;
    }
    return OS_ERROR;
}

/* get sta number  */
int os_wlan_ap_get_sta_num(void)
{
    int sta_num = 0;

    STAINFO_LOCK();
    sta_num = sta_info.num;
    STAINFO_UNLOCK();
    OS_WLAN_LOG_D("%s is run num:%d", __FUNCTION__, sta_num);
    return sta_num;
}

/* get sta info */
int os_wlan_ap_get_sta_info(struct os_wlan_info *info, int num)
{
    int                      sta_num = 0, i = 0;
    struct os_wlan_sta_list *sta_list;

    STAINFO_LOCK();
    /* sta_num = min(sta_info.num, num) */
    sta_num = sta_info.num > num ? num : sta_info.num;
    for (sta_list = sta_info.node; sta_list != OS_NULL && i < sta_num; sta_list = sta_list->next)
    {
        info[i] = sta_list->info;
        i++;
    }
    STAINFO_UNLOCK();
    OS_WLAN_LOG_D("%s is run num:%d", __FUNCTION__, i);
    return i;
}

/* deauth sta */
os_err_t os_wlan_ap_deauth_sta(os_uint8_t *mac)
{
    os_err_t                 err = OS_EOK;
    struct os_wlan_sta_list *sta_list;
    os_bool_t                find_flag = OS_FALSE;

    if (_ap_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run mac: %02x:%02x:%02x:%02x:%02x:%02x:%d",
                  __FUNCTION__,
                  mac[0],
                  mac[1],
                  mac[2],
                  mac[3],
                  mac[4],
                  mac[5]);

    if (mac == OS_NULL)
    {
        OS_WLAN_LOG_E("mac addr is null");
        return OS_EINVAL;
    }

    MGNT_LOCK();
    if (sta_info.node == OS_NULL || sta_info.num == 0)
    {
        OS_WLAN_LOG_E("No AP");
        MGNT_UNLOCK();
        return OS_ERROR;
    }

    STAINFO_LOCK();
    /* Search for MAC address from sta list */
    for (sta_list = sta_info.node; sta_list != OS_NULL; sta_list = sta_list->next)
    {
        if (memcmp(&sta_list->info.bssid[0], &mac[0], OS_WLAN_BSSID_MAX_LENGTH) == 0)
        {
            find_flag = OS_TRUE;
            break;
        }
    }
    STAINFO_UNLOCK();

    /* No MAC address was found. return */
    if (find_flag != OS_TRUE)
    {
        OS_WLAN_LOG_E("Not find mac addr");
        MGNT_UNLOCK();
        return OS_ERROR;
    }

    /* Kill STA */
    err = os_wlan_dev_ap_deauth(AP_DEVICE(), mac);
    if (err != OS_EOK)
    {
        OS_WLAN_LOG_E("deauth sta failed");
        MGNT_UNLOCK();
        return err;
    }

    MGNT_UNLOCK();
    return err;
}

os_err_t os_wlan_ap_set_country(os_country_code_t country_code)
{
    os_err_t err = OS_EOK;

    if (_ap_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run country:%d", __FUNCTION__, country_code);
    MGNT_LOCK();
    err = os_wlan_dev_set_country(AP_DEVICE(), country_code);
    MGNT_UNLOCK();
    return err;
}

os_country_code_t os_wlan_ap_get_country(void)
{
    os_country_code_t country_code = OS_COUNTRY_UNKNOWN;

    if (_ap_is_null())
    {
        return country_code;
    }
    MGNT_LOCK();
    country_code = os_wlan_dev_get_country(AP_DEVICE());
    OS_WLAN_LOG_D("%s is run country:%d", __FUNCTION__, country_code);
    MGNT_UNLOCK();
    return country_code;
}

void os_wlan_config_autoreconnect(os_bool_t enable)
{
#ifdef OS_WLAN_AUTO_CONNECT_ENABLE
    OS_WLAN_LOG_D("%s is run enable:%d", __FUNCTION__, enable);

    MGNT_LOCK();
    if (enable)
    {
        TIME_START();
        _sta_mgnt.flags |= OS_WLAN_STATE_AUTOEN;
    }
    else
    {
        TIME_STOP();
        _sta_mgnt.flags &= ~OS_WLAN_STATE_AUTOEN;
    }
    MGNT_UNLOCK();
#endif
}

os_bool_t os_wlan_get_autoreconnect_mode(void)
{
#ifdef OS_WLAN_AUTO_CONNECT_ENABLE
    os_bool_t enable = 0;

    enable = _sta_mgnt.flags & OS_WLAN_STATE_AUTOEN ? 1 : 0;
    OS_WLAN_LOG_D("%s is run enable:%d", __FUNCTION__, enable);
    return enable;
#else
    return OS_FALSE;
#endif
}

/* Call the underlying scan function, which is asynchronous.
The hotspots scanned are returned by callbacks */
os_err_t os_wlan_scan(void)
{
    os_err_t err = OS_EOK;

    if (_sta_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run", __FUNCTION__);

    MGNT_LOCK();
    err = os_wlan_dev_scan(STA_DEVICE(), OS_NULL);
    MGNT_UNLOCK();
    return err;
}

struct os_wlan_scan_result *os_wlan_scan_sync(void)
{
    struct os_wlan_scan_result *result;

    /* Execute synchronous scan function */
    MGNT_LOCK();
    result = os_wlan_scan_with_info(OS_NULL);
    MGNT_UNLOCK();
    return result;
}

struct os_wlan_scan_result *os_wlan_scan_with_info(struct os_wlan_info *info)
{
    os_err_t                     err = OS_EOK;
    struct os_wlan_complete_des *complete;
    os_uint32_t                  set = 0, recved = 0;
    static struct os_wlan_info   scan_filter_info;
    os_base_t                    level;
    struct os_wlan_scan_result  *result;

    if (_sta_is_null())
    {
        return OS_NULL;
    }
    OS_WLAN_LOG_D("%s is run", __FUNCTION__);
    if (info != OS_NULL && info->ssid.len > OS_WLAN_SSID_MAX_LENGTH)
    {
        OS_WLAN_LOG_E("ssid is to long!");
        return OS_NULL;
    }

    /* Create an event that needs to wait. */
    MGNT_LOCK();
    complete = os_wlan_complete_create("scan");
    if (complete == OS_NULL)
    {
        MGNT_UNLOCK();
        return &scan_result;
    }

    /* add scan info filter */
    if (info)
    {
        scan_filter_info = *info;
        level            = os_hw_interrupt_disable();
        scan_filter      = &scan_filter_info;
        os_hw_interrupt_enable(level);
    }

    /* run scan */
    err = os_wlan_dev_scan(STA_DEVICE(), info);
    if (err != OS_EOK)
    {
        os_wlan_complete_delete(complete);
        OS_WLAN_LOG_E("scan sync fail");
        result = OS_NULL;
        goto scan_exit;
    }

    /* Initializing events that need to wait */
    set |= 0x1 << OS_WLAN_DEV_EVT_SCAN_DONE;
    /* Check whether there is a waiting event */
    os_wlan_complete_wait(complete, set, OS_WLAN_CONNECT_WAIT_MS, &recved);
    os_wlan_complete_delete(complete);
    /* check event */
    set = 0x1 << OS_WLAN_DEV_EVT_SCAN_DONE;
    if (!(recved & set))
    {
        OS_WLAN_LOG_E("scan wait timeout!");
        result = &scan_result;
        goto scan_exit;
    }

scan_exit:
    MGNT_UNLOCK();
    level       = os_hw_interrupt_disable();
    scan_filter = OS_NULL;
    os_hw_interrupt_enable(level);
    result = &scan_result;
    return result;
}

int os_wlan_scan_get_info_num(void)
{
    int num = 0;

    num = scan_result.num;
    OS_WLAN_LOG_D("%s is run num:%d", __FUNCTION__, num);
    return num;
}

int os_wlan_scan_get_info(struct os_wlan_info *info, int num)
{
    int _num = 0;

    SRESULT_LOCK();
    if (scan_result.num && num > 0)
    {
        _num = scan_result.num > num ? num : scan_result.num;
        memcpy(info, scan_result.info, _num * sizeof(struct os_wlan_info));
    }
    SRESULT_UNLOCK();
    return _num;
}

struct os_wlan_scan_result *os_wlan_scan_get_result(void)
{
    return &scan_result;
}

void os_wlan_scan_result_clean(void)
{
    MGNT_LOCK();
    SRESULT_LOCK();

    /* If there is data */
    if (scan_result.num)
    {
        scan_result.num = 0;
        os_free(scan_result.info);
        scan_result.info = OS_NULL;
    }
    SRESULT_UNLOCK();
    MGNT_UNLOCK();
}

int os_wlan_scan_find_cache(struct os_wlan_info *info, struct os_wlan_info *out_info, int num)
{
    int                  i = 0, count = 0;
    struct os_wlan_info *scan_info;
    os_bool_t            is_equ;

    if ((out_info == OS_NULL) || (info == OS_NULL) || (num <= 0))
    {
        return 0;
    }
    SRESULT_LOCK();
    /* Traversing the cache to find a qualified hot spot information */
    for (i = 0; (i < scan_result.num) && (count < num); i++)
    {
        scan_info = &scan_result.info[i];
        is_equ    = os_wlan_info_isequ(scan_info, info);
        /* Determine whether to find */
        if (is_equ)
        {
            memcpy(&out_info[count], scan_info, sizeof(struct os_wlan_info));
            count++;
        }
    }
    SRESULT_UNLOCK();

    return count;
}

os_err_t os_wlan_set_powersave(int level)
{
    os_err_t err = OS_EOK;

    if (_sta_is_null())
    {
        return OS_EIO;
    }
    OS_WLAN_LOG_D("%s is run", __FUNCTION__);
    MGNT_LOCK();
    err = os_wlan_dev_set_powersave(STA_DEVICE(), level);
    MGNT_UNLOCK();
    return err;
}

int os_wlan_get_powersave(void)
{
    int level;

    if (_sta_is_null())
    {
        return -1;
    }
    OS_WLAN_LOG_D("%s is run", __FUNCTION__);
    MGNT_LOCK();
    level = os_wlan_dev_get_powersave(STA_DEVICE());
    MGNT_UNLOCK();
    return level;
}

os_err_t os_wlan_register_event_handler(os_wlan_event_t event, os_wlan_event_handler handler, void *parameter)
{
    os_base_t level;

    if (event >= OS_WLAN_EVT_MAX)
    {
        return OS_EINVAL;
    }
    OS_WLAN_LOG_D("%s is run event:%d", __FUNCTION__, event);

    MGNT_LOCK();
    /* Registering Callbacks */
    level                      = os_hw_interrupt_disable();
    event_tab[event].handler   = handler;
    event_tab[event].parameter = parameter;
    os_hw_interrupt_enable(level);
    MGNT_UNLOCK();
    return OS_EOK;
}

os_err_t os_wlan_unregister_event_handler(os_wlan_event_t event)
{
    os_base_t level;

    if (event >= OS_WLAN_EVT_MAX)
    {
        return OS_EINVAL;
    }
    OS_WLAN_LOG_D("%s is run event:%d", __FUNCTION__, event);
    MGNT_LOCK();
    /* unregister*/
    level                      = os_hw_interrupt_disable();
    event_tab[event].handler   = OS_NULL;
    event_tab[event].parameter = OS_NULL;
    os_hw_interrupt_enable(level);
    MGNT_UNLOCK();
    return OS_EOK;
}

void os_wlan_mgnt_lock(void)
{
    MGNT_LOCK();
}

void os_wlan_mgnt_unlock(void)
{
    MGNT_UNLOCK();
}

int os_wlan_prot_ready_event(struct os_wlan_device *wlan, struct os_wlan_buff *buff)
{
    os_base_t level;

    if ((wlan == OS_NULL) || (_sta_mgnt.device != wlan) || (!(_sta_mgnt.state & OS_WLAN_STATE_CONNECT)))
    {
        return -1;
    }
    if (_sta_mgnt.state & OS_WLAN_STATE_READY)
    {
        return 0;
    }
    level = os_hw_interrupt_disable();
    _sta_mgnt.state |= OS_WLAN_STATE_READY;
    os_hw_interrupt_enable(level);
#ifdef OS_WLAN_WORK_TASK_ENABLE
    os_wlan_send_to_thread(OS_WLAN_EVT_READY, buff->data, buff->len);
#else
    {
        void                 *user_parameter;
        os_wlan_event_handler handler = OS_NULL;

        level          = os_hw_interrupt_disable();
        handler        = event_tab[OS_WLAN_EVT_READY].handler;
        user_parameter = event_tab[OS_WLAN_EVT_READY].parameter;
        os_hw_interrupt_enable(level);
        if (handler)
        {
            handler(OS_WLAN_EVT_READY, buff, user_parameter);
        }
    }
#endif
    return 0;
}

int os_wlan_init(void)
{
    static os_int8_t _init_flag = 0;

    /* Execute only once */
    if (_init_flag == 0)
    {
        memset(&_sta_mgnt, 0, sizeof(struct os_wlan_mgnt_des));
        memset(&_ap_mgnt, 0, sizeof(struct os_wlan_mgnt_des));
        memset(&scan_result, 0, sizeof(struct os_wlan_scan_result));
        memset(&sta_info, 0, sizeof(struct os_wlan_sta_des));
        os_mutex_init(&mgnt_mutex, "mgnt", OS_IPC_FLAG_FIFO, OS_TRUE);
        os_mutex_init(&scan_result_mutex, "scan", OS_IPC_FLAG_FIFO, OS_TRUE);
        os_mutex_init(&sta_info_mutex, "sta", OS_IPC_FLAG_FIFO, OS_TRUE);
        os_mutex_init(&complete_mutex, "complete", OS_IPC_FLAG_FIFO, OS_TRUE);
#ifdef OS_WLAN_AUTO_CONNECT_ENABLE
        os_timer_init(&reconnect_time,
                      "wifi_tim",
                      os_wlan_cyclic_check,
                      OS_NULL,
                      os_tick_from_ms(AUTO_CONNECTION_PERIOD_MS),
                      OS_TIMER_FLAG_PERIODIC | OS_TIMER_FLAG_SOFT_TIMER);
#endif
        _init_flag = 1;
    }
    return 0;
}
OS_PREV_INIT(os_wlan_init);

#endif
