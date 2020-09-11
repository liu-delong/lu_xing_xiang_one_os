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
 * @file        rmtx.c
 *
 * @brief       For IAR compiler, we recommand to define _DLIB_TASK_SUPPORT as 2 for dlib multi-task support.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-14   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_mutex.h>
#include <yfuns.h>


#if _DLIB_TASK_SUPPORT
typedef void* _Rmtx;
void _Mtxinit(_Rmtx *m)
{
    os_mutex_t mutex;

    OS_ASSERT(OS_NULL != m);
    
    mutex = (os_mutex_t)m;
    os_mutex_init(mutex, "iarMtx", OS_IPC_FLAG_FIFO);
}

void _Mtxdst(_Rmtx *m)
{
    os_mutex_t mutex;

    OS_ASSERT(OS_NULL != m);

    mutex = (os_mutex_t)m;
    os_mutex_deinit(mutex);
}

void _Mtxlock(_Rmtx *m)
{
    os_mutex_t mutex;

    OS_ASSERT(OS_NULL != m);

    mutex = (os_mutex_t)m;
    os_mutex_lock(mutex, OS_IPC_WAITING_FOREVER);
}

void _Mtxunlock(_Rmtx *m)
{
    os_mutex_t mutex;

    OS_ASSERT(OS_NULL != m);

    mutex = (os_mutex_t)m;
    os_mutex_unlock(mutex);
}
#endif

