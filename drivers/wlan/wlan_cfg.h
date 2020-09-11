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
 * @file        wlan_cfg.h
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

#ifndef __WLAN_CFG_H__
#define __WLAN_CFG_H__

#include <wlan/wlan_dev.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OS_WLAN_CFG_INFO_MAX
#define OS_WLAN_CFG_INFO_MAX (3) /* min is 1 */
#endif

#define OS_WLAN_CFG_MAGIC (0x426f6d62)

struct os_wlan_cfg_info
{
    struct os_wlan_info info;
    struct os_wlan_key  key;
};

typedef int (*os_wlan_wr)(void *buff, int len);

struct os_wlan_cfg_ops
{
    int (*read_cfg)(void *buff, int len);
    int (*get_len)(void);
    int (*write_cfg)(void *buff, int len);
};

void os_wlan_cfg_init(void);

void os_wlan_cfg_set_ops(const struct os_wlan_cfg_ops *ops);

int os_wlan_cfg_get_num(void);

int os_wlan_cfg_read(struct os_wlan_cfg_info *cfg_info, int num);

int os_wlan_cfg_read_index(struct os_wlan_cfg_info *cfg_info, int index);

os_err_t os_wlan_cfg_save(struct os_wlan_cfg_info *cfg_info);

os_err_t os_wlan_cfg_cache_refresh(void);

os_err_t os_wlan_cfg_cache_save(void);

int os_wlan_cfg_delete_index(int index);

void os_wlan_cfg_delete_all(void);

void os_wlan_cfg_dump(void);

#ifdef __cplusplus
}
#endif

#endif
