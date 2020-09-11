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
 * @file        wlan_workqueue.c
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

#include <os_hw.h>
#include <os_task.h>
#include <wlan/wlan_workqueue.h>
#include <os_workqueue.h>
#include <os_memory.h>
#include <string.h>
#include <os_errno.h>

#define DRV_EXT_LVL          DBG_EXT_INFO
#define DRV_EXT_TAG         "wlan.work" 
#include <drv_log.h>

#ifdef OS_WLAN_WORK_TASK_ENABLE

struct os_wlan_work
{
    struct os_work work;
    void (*fun)(void *parameter);
    void *parameter;
};

static struct os_workqueue *wlan_workqueue;

static void wlan_workqueue_clean(struct os_work *work)
{
    os_free(work->data);
}

static void os_wlan_workqueue_fun(struct os_work *work, void *data)
{
    struct os_wlan_work *wlan_work = (struct os_wlan_work *)data;

    if (OS_NULL != wlan_work->fun)
    {
        wlan_work->fun(wlan_work->parameter);
    }
    os_work_deinit(work, wlan_workqueue_clean);
}

struct os_workqueue *os_wlan_get_workqueue(void)
{
    return wlan_workqueue;
}

os_err_t os_wlan_workqueue_dowork(void (*func)(void *parameter), void *parameter)
{
    struct os_wlan_work *wlan_work;
    os_err_t             err = OS_EOK;

    LOG_EXT_D("F:%s is run", __FUNCTION__);
    if (func == OS_NULL)
    {
        LOG_EXT_E("F:%s L:%d func is null", __FUNCTION__, __LINE__);
        return OS_EINVAL;
    }

    if (wlan_workqueue == OS_NULL)
    {
        LOG_EXT_E("F:%s L:%d not init wlan work queue", __FUNCTION__, __LINE__);
        return OS_ERROR;
    }

    wlan_work = os_malloc(sizeof(struct os_wlan_work));
    if (wlan_work == OS_NULL)
    {
        LOG_EXT_E("F:%s L:%d create work failed", __FUNCTION__, __LINE__);
        return OS_ENOMEM;
    }
    wlan_work->fun       = func;
    wlan_work->parameter = parameter;
    os_work_init(&wlan_work->work, os_wlan_workqueue_fun, wlan_work);
    err = os_workqueue_submit_work(wlan_workqueue, &wlan_work->work, 0);
    if (err != OS_EOK)
    {
        LOG_EXT_E("F:%s L:%d do work failed", __FUNCTION__, __LINE__);
        os_free(wlan_work);
        return err;
    }
    return err;
}

int os_wlan_workqueue_init(void)
{
    static os_int8_t _init_flag = 0;

    if (_init_flag == 0)
    {
        wlan_workqueue =
            os_workqueue_create(OS_WLAN_WORKQUEUE_TASK_NAME, OS_WLAN_WORKQUEUE_TASK_SIZE, OS_WLAN_WORKQUEUE_TASK_PRIO);
        if (wlan_workqueue == OS_NULL)
        {
            LOG_EXT_E("F:%s L:%d wlan work queue create failed", __FUNCTION__, __LINE__);
            return -1;
        }
        _init_flag = 1;
        return 0;
    }
    return 0;
}
OS_PREV_INIT(os_wlan_workqueue_init);

#endif
