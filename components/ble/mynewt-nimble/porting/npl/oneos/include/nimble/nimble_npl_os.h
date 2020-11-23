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
 * @file        nimble_npl_os.h
 *
 * @brief       Define the type used for stack.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _NIMBLE_NPL_OS_H_
#define _NIMBLE_NPL_OS_H_

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "libc_errno.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_NPL_OS_ALIGNMENT    4

// typedef unsigned int                    os_uint32_t;            /* 32bit unsigned integer type */
// typedef os_uint32_t                     os_tick_t;              /* Type for tick count */ 
#ifndef OS_IPC_WAITING_FOREVER
#define OS_IPC_WAITING_FOREVER    ((unsigned int)0xFFFFFFFF)
#endif

#ifndef OS_IPC_WAITING_NO
#define OS_IPC_WAITING_NO         ((unsigned int)0)
#endif

#define BLE_NPL_TIME_FOREVER    (OS_IPC_WAITING_FOREVER)

/* ble_npl_time_t have same definition with os_tick_t. */
typedef uint32_t ble_npl_time_t;
typedef int32_t ble_npl_stime_t;

struct ble_npl_task
{
    void *t;
};

struct ble_npl_event
{
    bool queued;
    ble_npl_event_fn *fn;
    void *arg;
};

struct ble_npl_eventq
{
    void *q;
};

struct ble_npl_callout
{
    void *handle;
    struct ble_npl_eventq *evq;
    struct ble_npl_event ev;
};

struct ble_npl_mutex
{
    void *handle;
};

struct ble_npl_sem
{
    void  *handle;
};

static inline 
bool ble_npl_os_started(void)
{
    return true;
}

void nimble_port_oneos_init(void);

void ble_hs_task_startup(void);


#ifdef __cplusplus
}
#endif

#endif
