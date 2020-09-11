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
 * @file        wlan_prot.h
 *
 * @brief       wlan_prot
 *
 * @details     wlan_prot
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __WLAN_PROT_H__
#define __WLAN_PROT_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OS_WLAN_PROT_NAME_LEN
#define OS_WLAN_PROT_NAME_LEN  (8)
#endif

#ifndef OS_WLAN_PROT_MAX
#define OS_WLAN_PROT_MAX       (1)
#endif

#define OS_LWAN_ID_PREFIX      (0x5054)

typedef enum
{
    OS_WLAN_PROT_EVT_INIT_DONE = 0,
    OS_WLAN_PROT_EVT_CONNECT,
    OS_WLAN_PROT_EVT_DISCONNECT,
    OS_WLAN_PROT_EVT_AP_START,
    OS_WLAN_PROT_EVT_AP_STOP,
    OS_WLAN_PROT_EVT_AP_ASSOCIATED,
    OS_WLAN_PROT_EVT_AP_DISASSOCIATED,
    OS_WLAN_PROT_EVT_MAX,
} os_wlan_prot_event_t;

struct os_wlan_prot;
struct os_wlan_prot_ops
{
    os_err_t (*prot_recv)(struct os_wlan_device *wlan, void *buff, int len);
    struct os_wlan_prot *(*dev_reg_callback)(struct os_wlan_prot *prot, struct os_wlan_device *wlan);
    void (*dev_unreg_callback)(struct os_wlan_prot *prot, struct os_wlan_device *wlan);
};

struct os_wlan_prot
{
    char                           name[OS_WLAN_PROT_NAME_LEN];
    os_uint32_t                    id;
    const struct os_wlan_prot_ops *ops;
};

typedef void (*os_wlan_prot_event_handler)(struct os_wlan_prot *port, struct os_wlan_device *wlan, int event);

/**************************************wlan device protocol interface begin*******************************************/

os_err_t os_wlan_prot_attach(const char *dev_name, const char *prot_name);

os_err_t os_wlan_prot_attach_dev(struct os_wlan_device *wlan, const char *prot_name);

os_err_t os_wlan_prot_detach(const char *dev_name);

os_err_t os_wlan_prot_detach_dev(struct os_wlan_device *wlan);

os_err_t os_wlan_prot_regisetr(struct os_wlan_prot *prot);

os_err_t os_wlan_prot_transfer_dev(struct os_wlan_device *wlan, void *buff, int len);

os_err_t os_wlan_dev_transfer_prot(struct os_wlan_device *wlan, void *buff, int len);

os_err_t os_wlan_prot_event_register(struct os_wlan_prot *prot, os_wlan_prot_event_t event, os_wlan_prot_event_handler handler);

os_err_t os_wlan_prot_event_unregister(struct os_wlan_prot *prot, os_wlan_prot_event_t event);

int os_wlan_prot_ready(struct os_wlan_device *wlan, struct os_wlan_buff *buff);

void os_wlan_prot_dump(void);

/**************************************wlan device protocol interface end*********************************************/

#ifdef __cplusplus
}
#endif

#endif
