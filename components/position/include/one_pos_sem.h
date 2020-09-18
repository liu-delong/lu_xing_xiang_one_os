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
 * @file        one_pos_sem.h
 * 
 * @brief       Location service semaphore
 * 
 * @details     Location service semaphore
 * 
 * @revision
 * Date         Author          Notes
 * 2020-07-17   HuSong          First Version
 ***********************************************************************************************************************
 */
#include "one_pos_platform.h"
#include "one_pos_error.h"

#ifndef __ONE_POS_SEM_H__
#define __ONE_POS_SEM_H__

#if defined __OPS_WINDOWS__  /* Windows platform */
#include <Windows.h>

typedef HANDLE ops_sem_t;

#elif defined __OPS_ONE_OS__ /* OneOS platform */
#include <os_kernel.h>

typedef os_sem_t ops_sem_t;

#else
#error Undefined platform
#endif 

#define OPS_SEM_TIME_OUT  1  /* The amount of time the semaphore waits to timeout (unit: S) */

ops_sem_t *ops_sem_create(const char *name, ops_ushort_t value);

ops_err_t ops_sem_destroy(ops_sem_t *sem);

ops_err_t ops_sem_wait(ops_sem_t *sem);

ops_err_t ops_sem_post(ops_sem_t *sem);

#endif /* __ONE_POS_SEM_H__ */
