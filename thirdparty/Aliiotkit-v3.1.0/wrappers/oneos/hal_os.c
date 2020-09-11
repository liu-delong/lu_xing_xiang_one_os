/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * Copyright (c) 2006-2018 RT-Thread Development Team.
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
 * @file        hal_os.c
 *
 * @brief     a port file of os for iotkit
 *
 * @details  
 *
 * @revision
 * Date               Author             Notes
 * 2019-07-21         MurphyZhao         first edit
 * 2020-06-10         OneOS Team         format and change request resource
 ***********************************************************************************************************************
 */

#include <stdlib.h>

#include "os_kernel.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "wrappers_defs.h"

#define DBG_EXT_TAG "ali.os"
#define DBG_EXT_LVL DBG_EXT_INFO
#include "os_dbg_ext.h"

static char log_buf[OS_LOG_BUFF_SIZE];

void *HAL_MutexCreate(void)
{
    os_mutex_t *mutex = os_mutex_create("ali_ld_mutex", OS_IPC_FLAG_FIFO, 0);
    if (!mutex)
    {
        LOG_EXT_E("mutex create failed!");
    }
    return mutex;
}

void HAL_MutexDestroy(void *mutex)
{
    os_err_t err_num;

    if (0 != (err_num = os_mutex_destroy((os_mutex_t *)mutex)))
    {
        LOG_EXT_E("destroy mutex failed, err num: %d", err_num);
    }
}

void HAL_MutexLock(void *mutex)
{
    os_err_t err_num;

    if (0 != (err_num = os_mutex_lock((os_mutex_t *)mutex, OS_IPC_WAITING_FOREVER)))
    {
        LOG_EXT_E("lock mutex failed, err num: %d", err_num);
    }
}

void HAL_MutexUnlock(void *mutex)
{
    os_err_t err_num;

    if (0 != (err_num = os_mutex_unlock((os_mutex_t *)mutex)))
    {
        LOG_EXT_E("unlock mutex failed, err num: %d", err_num);
    }
}

/**
 * @brief   create a semaphore
 *
 * @return semaphore handle.
 * @see None.
 * @note not more than 100 semaphore.
 */
void *HAL_SemaphoreCreate(void)
{
    char           name[10] = {0};
    static uint8_t sem_num  = 0;
    os_snprintf(name, sizeof(name), "sem%02d", ((++sem_num) % 100));
    os_sem_t *sem = os_sem_create(name, 0, OS_IPC_FLAG_FIFO);
    if (!sem)
    {
        LOG_EXT_E("Semaphore create failed!");
    }
    return (void *)sem;
}

/**
 * @brief   destory a semaphore
 *
 * @param[in] sem @n the specified sem.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SemaphoreDestroy(void *sem)
{
    os_err_t     err = OS_EOK;
    os_object_t *obj = sem;

    if (!obj)
    {
        LOG_EXT_E("In param (sem) is NULL!");
        return;
    }

    if (obj->type == OS_OBJECT_SEMAPHORE)
    {
        err = os_sem_destroy((os_sem_t *)obj);
        if (err != OS_EOK)
        {
            LOG_EXT_E("sem delete failed! errno:%d", err);
        }
    }
    else
    {
        LOG_EXT_E("Error sem handler!");
    }
    return;
}

/**
 * @brief   signal thread wait on a semaphore
 *
 * @param[in] sem @n the specified semaphore.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SemaphorePost(void *sem)
{
    os_err_t     err = OS_EOK;
    os_object_t *obj = sem;

    if (!obj)
    {
        LOG_EXT_E("In param (sem) is NULL!");
        return;
    }

    if (obj->type == OS_OBJECT_SEMAPHORE)
    {
        err = os_sem_post((os_sem_t *)obj);
        if (err != OS_EOK)
        {
            LOG_EXT_E("sem release failed! errno:%d", err);
        }
    }
    else
    {
        LOG_EXT_E("Error sem handler!");
    }
    return;
}

/**
 * @brief   wait on a semaphore
 *
 * @param[in] sem @n the specified semaphore.
 * @param[in] timeout_ms @n timeout interval in millisecond.
     If timeout_ms is PLATFORM_WAIT_INFINITE, the function will return only when the semaphore is signaled.
 * @return
   @verbatim
   =  0: The state of the specified object is signaled.
   =  -1: The time-out interval elapsed, and the object's state is nonsignaled.
   @endverbatim
 * @see None.
 * @note None.
 */
int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms)
{
    os_err_t     err = OS_EOK;
    os_object_t *obj = sem;

    if (!obj)
    {
        LOG_EXT_E("In param (sem) is NULL!");
        return -1;
    }

    if (obj->type == OS_OBJECT_SEMAPHORE)
    {
        err = os_sem_wait((os_sem_t *)obj, timeout_ms);
        if (err != OS_EOK)
        {
            LOG_EXT_E("sem take failed! errno:%d", err);
        }
    }
    else
    {
        LOG_EXT_E("Error sem handler!");
    }
    return (err == OS_EOK ? 0 : -1);
}

void *HAL_Malloc(uint32_t size)
{
    return os_malloc(size);
}

void HAL_Free(void *ptr)
{
    os_free(ptr);
}

uint64_t HAL_UptimeMs(void)
{
    uint64_t tick;
    tick = os_tick_get();

    tick = tick * 1000;

    return (tick + OS_TICK_PER_SECOND - 1) / OS_TICK_PER_SECOND;
}

void HAL_SleepMs(uint32_t ms)
{
    os_task_mdelay(ms);
}

void HAL_Srandom(uint32_t seed)
{
    srand(seed);
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (rand() % region) : 0;
}

int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = os_vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap)
{
    return os_vsnprintf(str, len, format, ap);
}

void HAL_Printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    os_vsnprintf(log_buf, OS_LOG_BUFF_SIZE, fmt, args);
    va_end(args);
    os_kprintf("%s", log_buf);
}

#ifdef HAL_KV
OS_WEAK int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
    return 0;
}

OS_WEAK int HAL_Kv_Get(const char *key, void *buffer, int *buffer_len)
{
    return 0;
}

OS_WEAK int HAL_Kv_Del(const char *key)
{
    return 0;
}
#endif
