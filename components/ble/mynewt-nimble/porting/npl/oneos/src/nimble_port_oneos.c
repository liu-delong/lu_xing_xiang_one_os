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
 * @file        nimble_port_oneos.c
 *
 * @brief       The function to init Nimble stack and start the staks.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stddef.h>
#include <os_kernel.h>
#include <oneos_config.h>
#include "syscfg/syscfg.h"
#include "nimble/nimble_port.h"

#if NIMBLE_CFG_CONTROLLER
/* If there is not enable heap, we should use static task and stack. */
#ifndef OS_USING_HEAP
OS_ALIGN(OS_ALIGN_SIZE)
static os_uint8_t gs_ble_ll_stack[MYNEWT_VAL(BLE_CTLR_THREAD_STACK_SIZE)];
static os_task_t gs_ble_ll_task;
#endif
#endif

/* If there is not enable heap, we should use static task and stack. */
#ifndef OS_USING_HEAP
OS_ALIGN(OS_ALIGN_SIZE)
static os_uint8_t gs_ble_hs_stack[MYNEWT_VAL(BLE_HOST_THREAD_STACK_SIZE)];
static os_task_t gs_ble_hs_task;
#endif

extern void ble_ll_task(void *arg);

void nimble_port_oneos_init(void)
{
    nimble_port_init();

#if NIMBLE_CFG_CONTROLLER
    /*
     * Create task where NimBLE LL will run. This one is required as LL has its
     * own event queue and should have highest priority. The task function is
     * provided by NimBLE and in case of FreeRTOS it does not need to be wrapped
     * since it has compatible prototype.
     */
    os_task_t *task;

#ifdef OS_USING_HEAP
    task = os_task_create("ble_ll",
                          ble_ll_task,
                          OS_NULL,
                          MYNEWT_VAL(BLE_CTLR_THREAD_STACK_SIZE),
                          MYNEWT_VAL(BLE_CTLR_THREAD_PRIORITY),
                          10);
    OS_ASSERT(OS_NULL != task);
#else
    os_err_t ret;

    task = &gs_os_main_task;
    ret = os_task_init(task,
                       "ble_ll",
                       ble_ll_task,
                       OS_NULL,
                       gs_ble_ll_stack,
                       MYNEWT_VAL(BLE_CTLR_THREAD_STACK_SIZE),
                       MYNEWT_VAL(BLE_CTLR_THREAD_PRIORITY),
                       10);
    OS_ASSERT(OS_EOK == ret);
#endif

    os_task_startup(task);
#endif
    return;
}

void ble_hs_task(void *parameter)
{
    nimble_port_run();
}

void ble_hs_task_startup(void)
{
    os_task_t *task;

#ifdef OS_USING_HEAP
    task = os_task_create("ble_hs",
                          ble_hs_task,
                          OS_NULL,
                          MYNEWT_VAL(BLE_HOST_THREAD_STACK_SIZE),
                          MYNEWT_VAL(BLE_HOST_THREAD_PRIORITY),
                          10);
    OS_ASSERT(OS_NULL != task);
#else
    os_err_t ret;

    task = &gs_os_main_task;
    ret = os_task_init(task,
                       "ble_hs",
                       ble_hs_task,
                       OS_NULL,
                       gs_ble_hs_stack,
                       MYNEWT_VAL(BLE_HOST_THREAD_STACK_SIZE),
                       MYNEWT_VAL(BLE_HOST_THREAD_PRIORITY),
                       10);
    OS_ASSERT(OS_EOK == ret);
#endif

    os_task_startup(task);
    return;
}


