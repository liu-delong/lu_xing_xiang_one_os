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
 * @file        os_to_rt.c
 *
 * @brief       The file os to rt
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <shell.h>
#include <wlan/wlan_dev.h>
#include <os_memory.h>
#include <string.h>
#include <os_mq.h>
#include <os_util.h>
#include <os_sem.h>
#include <os_clock.h>
#include <os_util.h>

void rt_wlan_dev_promisc_handler(struct os_wlan_device *device, void *data, int len)
{
    os_wlan_dev_promisc_handler(device, data, len);
}
os_err_t rt_wlan_dev_register(struct os_wlan_device        *wlan,
                              const char                   *name,
                              const struct os_wlan_dev_ops *ops,
                              os_uint32_t                   flag,
                              void                         *user_data)
{
    return os_wlan_dev_register(wlan, name, ops, flag, user_data);
}
os_err_t rt_wlan_dev_report_data(struct os_wlan_device *device, void *buff, int len)
{
    return os_wlan_dev_report_data(device, buff, len);
}

void rt_wlan_dev_indicate_event_handle(struct os_wlan_device *device,
                                       os_wlan_dev_event_t    event,
                                       struct os_wlan_buff   *buff)
{
    os_wlan_dev_indicate_event_handle(device, event, buff);
}
