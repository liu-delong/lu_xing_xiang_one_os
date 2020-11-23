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
 * @file        cmiot_os.h
 *
 * @brief       The os header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_OS_H__
#define __CMIOT_OS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CMIOT_ONEOS 1

/* if defined os, use os's config */
#if defined(CMIOT_ONEOS)
#include "oneos_config.h"
#else
/* Message queue, do not close */
#define CMIOT_QUEUE_MSG
/* Timer, do not close */
#define CMIOT_TIMERS
#endif

/* Debug mode if 1, else user mode */
#define CMIOT_APP_DEBUG 1

#ifdef __cplusplus
}
#endif

#endif
