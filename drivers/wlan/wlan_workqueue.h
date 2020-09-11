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
 * @file        wlan_workqueue.h
 *
 * @brief       wlan_workqueue
 *
 * @details     wlan_workqueue
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __WLAN_WORKQUEUE_H__
#define __WLAN_WORKQUEUE_H__

#include <os_workqueue.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OS_WLAN_WORKQUEUE_TASK_NAME
#define OS_WLAN_WORKQUEUE_TASK_NAME ("wlan_job")
#endif

#ifndef OS_WLAN_WORKQUEUE_TASK_SIZE
#define OS_WLAN_WORKQUEUE_TASK_SIZE (2048)
#endif

#ifndef OS_WLAN_WORKQUEUE_TASK_PRIO
#define OS_WLAN_WORKQUEUE_TASK_PRIO (20)
#endif

int os_wlan_workqueue_init(void);

os_err_t os_wlan_workqueue_dowork(void (*func)(void *parameter), void *parameter);

struct os_workqueue *os_wlan_get_workqueue(void);

#ifdef __cplusplus
}
#endif

#endif
