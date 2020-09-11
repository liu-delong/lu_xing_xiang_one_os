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
 * @file        wlan_cfg.c
 *
 * @brief       wlan_cfg
 *
 * @details     wlan_cfg
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <wlan/wlan_cfg.h>
#include <os_errno.h>
#include <os_memory.h>
#include <string.h>
#include <os_ipc.h>
#include <os_mutex.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "wlan.cfg"
#include <drv_log.h>

#ifdef OS_WLAN_CFG_ENABLE

#define WLAN_CFG_LOCK()   (os_mutex_recursive_lock(&cfg_mutex, OS_IPC_WAITING_FOREVER))
#define WLAN_CFG_UNLOCK() (os_mutex_recursive_unlock(&cfg_mutex))

#if OS_WLAN_CFG_INFO_MAX < 1
#error "The minimum configuration is 1"
#endif

struct cfg_save_info_head
{
    os_uint32_t magic;
    os_uint32_t len;
    os_uint32_t num;
    os_uint32_t crc;
};

struct os_wlan_cfg_des
{
    os_uint32_t              num;
    struct os_wlan_cfg_info *cfg_info;
};

static struct os_wlan_cfg_des *      cfg_cache;
static const struct os_wlan_cfg_ops *cfg_ops;
static struct os_mutex               cfg_mutex;

static os_uint16_t os_wlan_cal_crc(os_uint8_t *buff, int len)
{
    os_uint16_t wCRCin = 0x0000;
    os_uint16_t wCPoly = 0x1021;
    os_uint8_t  wChar  = 0;

    while (len--)
    {
        int i;

        wChar = *(buff++);
        wCRCin ^= (wChar << 8);

        for (i = 0; i < 8; i++)
        {
            if (wCRCin & 0x8000)
                wCRCin = (wCRCin << 1) ^ wCPoly;
            else
                wCRCin = wCRCin << 1;
        }
    }
    return wCRCin;
}

void os_wlan_cfg_init(void)
{
    /* init cache memory */
    if (cfg_cache == OS_NULL)
    {
        cfg_cache = os_malloc(sizeof(struct os_wlan_cfg_des));
        if (cfg_cache != OS_NULL)
        {
            memset(cfg_cache, 0, sizeof(struct os_wlan_cfg_des));
        }
        /* init mutex lock */
        os_mutex_init(&cfg_mutex, "wlan_cfg", OS_IPC_FLAG_FIFO, OS_TRUE);
    }
}

void os_wlan_cfg_set_ops(const struct os_wlan_cfg_ops *ops)
{
    os_wlan_cfg_init();

    WLAN_CFG_LOCK();
    /* save ops pointer */
    cfg_ops = ops;
    WLAN_CFG_UNLOCK();
}

os_err_t os_wlan_cfg_cache_save(void)
{
    int         len = 0;
    os_err_t    err = OS_EOK;
    
    struct cfg_save_info_head *info_pkg;

    if ((cfg_ops == OS_NULL) || (cfg_ops->write_cfg == OS_NULL))
        return OS_EOK;

    WLAN_CFG_LOCK();
    len      = sizeof(struct cfg_save_info_head) + sizeof(struct os_wlan_cfg_info) * cfg_cache->num;
    info_pkg = os_malloc(len);
    if (info_pkg == OS_NULL)
    {
        WLAN_CFG_UNLOCK();
        return OS_ENOMEM;
    }
    info_pkg->magic = OS_WLAN_CFG_MAGIC;
    info_pkg->len   = len;
    info_pkg->num   = cfg_cache->num;
    /* CRC */
    info_pkg->crc = os_wlan_cal_crc((os_uint8_t *)cfg_cache->cfg_info, sizeof(struct os_wlan_cfg_info) * cfg_cache->num);
    memcpy(((os_uint8_t *)info_pkg) + sizeof(struct cfg_save_info_head),
              cfg_cache->cfg_info, sizeof(struct os_wlan_cfg_info) * cfg_cache->num);
    if (cfg_ops->write_cfg(info_pkg, len) != len)
        err = OS_ERROR;
    os_free(info_pkg);
    WLAN_CFG_UNLOCK();
    return err;
}

os_err_t os_wlan_cfg_cache_refresh(void)
{
    int                        len = 0, i, j;
    struct cfg_save_info_head *head;
    void *                     data;
    struct os_wlan_cfg_info *  t_info, *cfg_info;
    os_uint32_t                crc;
    os_bool_t                  equal_flag;

    /* cache is full! exit */
    if (cfg_cache == OS_NULL || cfg_cache->num >= OS_WLAN_CFG_INFO_MAX)
        return OS_ERROR;

    /* check callback */
    if ((cfg_ops == OS_NULL) || (cfg_ops->get_len == OS_NULL) || (cfg_ops->read_cfg == OS_NULL))
        return OS_ERROR;

    WLAN_CFG_LOCK();
    /* get data len */
    if ((len = cfg_ops->get_len()) <= 0)
    {
        WLAN_CFG_UNLOCK();
        return OS_ERROR;
    }

    head = os_malloc(len);
    if (head == OS_NULL)
    {
        WLAN_CFG_UNLOCK();
        return OS_ERROR;
    }
    /* get data */
    if (cfg_ops->read_cfg(head, len) != len)
    {
        os_free(head);
        WLAN_CFG_UNLOCK();
        return OS_ERROR;
    }
    /* get config */
    data = ((os_uint8_t *)head) + sizeof(struct cfg_save_info_head);
    crc  = os_wlan_cal_crc((os_uint8_t *)data, len - sizeof(struct cfg_save_info_head));
    LOG_EXT_D("head->magic:0x%08x  OS_WLAN_CFG_MAGIC:0x%08x", head->magic, OS_WLAN_CFG_MAGIC);
    LOG_EXT_D("head->len:%d len:%d", head->len, len);
    LOG_EXT_D("head->num:%d num:%d",
              head->num,
              (len - sizeof(struct cfg_save_info_head)) / sizeof(struct os_wlan_cfg_info));
    LOG_EXT_D("hred->crc:0x%04x crc:0x%04x", head->crc, crc);
    /* check */
    if ((head->magic != OS_WLAN_CFG_MAGIC) || (head->len != len) ||
        (head->num != (len - sizeof(struct cfg_save_info_head)) / sizeof(struct os_wlan_cfg_info)) ||
        (head->crc != crc))
    {
        os_free(head);
        WLAN_CFG_UNLOCK();
        return OS_ERROR;
    }

    /* remove duplicate config */
    cfg_info = (struct os_wlan_cfg_info *)data;
    for (i = 0; i < head->num; i++)
    {
        equal_flag = OS_FALSE;
        for (j = 0; j < cfg_cache->num; j++)
        {
            if ((cfg_cache->cfg_info[j].info.ssid.len == cfg_info[i].info.ssid.len) &&
                (memcmp(&cfg_cache->cfg_info[j].info.ssid.val[0],
                        &cfg_info[i].info.ssid.val[0],
                        cfg_cache->cfg_info[j].info.ssid.len) == 0) &&
                    (memcmp(&cfg_cache->cfg_info[j].info.bssid[0], &cfg_info[i].info.bssid[0], OS_WLAN_BSSID_MAX_LENGTH) == 0))
            {
                equal_flag = OS_TRUE;
                break;
            }
        }

        if (cfg_cache->num >= OS_WLAN_CFG_INFO_MAX)
        {
            break;
        }

        if (equal_flag == OS_FALSE)
        {
            t_info = os_realloc(cfg_cache->cfg_info, sizeof(struct os_wlan_cfg_info) * (cfg_cache->num + 1));
            if (t_info == OS_NULL)
            {
                os_free(head);
                WLAN_CFG_UNLOCK();
                return OS_ERROR;
            }
            cfg_cache->cfg_info                 = t_info;
            cfg_cache->cfg_info[cfg_cache->num] = cfg_info[i];
            cfg_cache->num++;
        }
    }

    os_free(head);
    WLAN_CFG_UNLOCK();
    return OS_EOK;
}

int os_wlan_cfg_get_num(void)
{
    os_wlan_cfg_init();

    return cfg_cache->num;
}

int os_wlan_cfg_read(struct os_wlan_cfg_info *cfg_info, int num)
{
    os_wlan_cfg_init();

    if ((cfg_info == OS_NULL) || (num <= 0))
        return 0;
    /* copy data */
    WLAN_CFG_LOCK();
    num = cfg_cache->num > num ? num : cfg_cache->num;
    memcpy(&cfg_cache->cfg_info[0], cfg_info, sizeof(struct os_wlan_cfg_info) * num);
    WLAN_CFG_UNLOCK();

    return num;
}

os_err_t os_wlan_cfg_save(struct os_wlan_cfg_info *cfg_info)
{
    os_err_t    err = OS_EOK;
    int         idx = -1;
    int         i = 0;
    
    struct os_wlan_cfg_info *t_info;

    os_wlan_cfg_init();

    /* parameter check */
    if ((cfg_info == OS_NULL) || (cfg_info->info.ssid.len == 0))
    {
        return OS_EINVAL;
    }
    /* if (iteam == cache) exit */
    WLAN_CFG_LOCK();
    for (i = 0; i < cfg_cache->num; i++)
    {
        if ((cfg_cache->cfg_info[i].info.ssid.len == cfg_info->info.ssid.len) &&
            (memcmp(&cfg_cache->cfg_info[i].info.ssid.val[0],
                    &cfg_info->info.ssid.val[0],
                    cfg_cache->cfg_info[i].info.ssid.len) == 0) &&
            (memcmp(&cfg_cache->cfg_info[i].info.bssid[0], &cfg_info->info.bssid[0], OS_WLAN_BSSID_MAX_LENGTH) == 0))
        {
            idx = i;
            break;
        }
    }

    if ((idx == 0) && (cfg_cache->cfg_info[i].key.len == cfg_info->key.len) &&
        (memcmp(&cfg_cache->cfg_info[i].key.val[0], &cfg_info->key.val[0], cfg_info->key.len) == 0))
    {
        WLAN_CFG_UNLOCK();
        return OS_EOK;
    }

    /* not find iteam with cache, Add iteam to the head   */
    if ((idx == -1) && (cfg_cache->num < OS_WLAN_CFG_INFO_MAX))
    {
        t_info = os_realloc(cfg_cache->cfg_info, sizeof(struct os_wlan_cfg_info) * (cfg_cache->num + 1));
        if (t_info == OS_NULL)
        {
            WLAN_CFG_UNLOCK();
            return OS_ENOMEM;
        }
        cfg_cache->cfg_info = t_info;
        cfg_cache->num++;
    }

    /* move cache info */
    i = (i >= OS_WLAN_CFG_INFO_MAX ? OS_WLAN_CFG_INFO_MAX - 1 : i);
    for (; i; i--)
    {
        cfg_cache->cfg_info[i] = cfg_cache->cfg_info[i - 1];
    }
    /* add iteam to head */
    cfg_cache->cfg_info[i] = *cfg_info;
    WLAN_CFG_UNLOCK();

    /* save info to flash */
    err = os_wlan_cfg_cache_save();

    return err;
}

int os_wlan_cfg_read_index(struct os_wlan_cfg_info *cfg_info, int index)
{
    os_wlan_cfg_init();

    if ((cfg_info == OS_NULL) || (index < 0))
        return 0;

    WLAN_CFG_LOCK();
    if (index >= cfg_cache->num)
    {
        WLAN_CFG_UNLOCK();
        return 0;
    }
    /* copy data */
    *cfg_info = cfg_cache->cfg_info[index];
    WLAN_CFG_UNLOCK();
    return 1;
}

int os_wlan_cfg_delete_index(int index)
{
    int i;
    
    struct os_wlan_cfg_info *cfg_info;

    os_wlan_cfg_init();

    if (index < 0)
        return -1;

    WLAN_CFG_LOCK();
    if (index >= cfg_cache->num)
    {
        WLAN_CFG_UNLOCK();
        return -1;
    }

    /* malloc new mem */
    cfg_info = os_malloc(sizeof(struct os_wlan_cfg_info) * (cfg_cache->num - 1));
    if (cfg_info == OS_NULL)
    {
        WLAN_CFG_UNLOCK();
        return -1;
    }
    /* copy data to new mem */
    for (i = 0; i < cfg_cache->num; i++)
    {
        if (i < index)
        {
            cfg_info[i] = cfg_cache->cfg_info[i];
        }
        else if (i > index)
        {
            cfg_info[i - 1] = cfg_cache->cfg_info[i];
        }
    }
    os_free(cfg_cache->cfg_info);
    cfg_cache->cfg_info = cfg_info;
    cfg_cache->num--;
    WLAN_CFG_UNLOCK();

    return 0;
}

void os_wlan_cfg_delete_all(void)
{
    os_wlan_cfg_init();

    /* delete all iteam */
    WLAN_CFG_LOCK();
    cfg_cache->num = 0;
    os_free(cfg_cache->cfg_info);
    cfg_cache->cfg_info = OS_NULL;
    WLAN_CFG_UNLOCK();
}

void os_wlan_cfg_dump(void)
{
    int     index = 0;
    char   *security;
    
    struct os_wlan_info *info;
    struct os_wlan_key  *key;

    os_wlan_cfg_init();

    os_kprintf("             SSID                           PASSWORD                   MAC            security     chn\n");
    os_kprintf("------------------------------- ------------------------------- -----------------  --------------  ---\n");
    for (index = 0; index < cfg_cache->num; index ++)
    {
        info = &cfg_cache->cfg_info[index].info;
        key  = &cfg_cache->cfg_info[index].key;

        if (info->ssid.len)
            os_kprintf("%-32.32s", &info->ssid.val[0]);
        else
            os_kprintf("%-32.32s", " ");

        if (key->len)
            os_kprintf("%-32.32s", &key->val[0]);
        else
            os_kprintf("%-32.32s", " ");

        os_kprintf("%02x:%02x:%02x:%02x:%02x:%02x  ",
                   info->bssid[0],
                   info->bssid[1],
                   info->bssid[2],
                   info->bssid[3],
                   info->bssid[4],
                   info->bssid[5]);
        switch (info->security)
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
        os_kprintf("%-14.14s  ", security);
        os_kprintf("%3d    \n", info->channel);
    }
}

#endif
