/**
***********************************************************************************************************************
* Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
* Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
* an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
* specific language governing permissions and limitations under the License.
*
* @file        resource_monitor.h
*
* @brief       Header file for resource moniter interface.
*
* @revision
* Date         Author          Notes
* 2020-08-18   OneOS Team      the first version
***********************************************************************************************************************
*/

#ifndef _RESOURCE_MONITOR_H_
#define _RESOURCE_MONITOR_H_

#include <oneos_config.h>
#include <os_task.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_TASK_MONITOR

#if  defined(OS_USING_TASK_PERIOD_MONITOR)

#endif /* OS_USING_TASK_PERIOD_MONITOR */

#if defined(OS_USING_TASK_SWITCH_MONITOR)

#define OS_TASK_SWITCH_INFO_COUNT                           (16)

struct task_switch_monitor_info
{
    os_uint8_t  index;                                      /* Record the index position of the task id. */
    os_uint8_t  is_full;                                    /* Whether the recording task is full. */
    os_uint8_t  sort_total;                                 /* Number of tasks sorted by task switching order. */
    os_uint8_t  unique_total;                               /* Number of tasks after removing repeate tasks. */
    os_task_t  *info[OS_TASK_SWITCH_INFO_COUNT];            /* Buffer for recording task id information when task switching */
    os_task_t  *sort_info[OS_TASK_SWITCH_INFO_COUNT];       /* Task id information sorted by task switching order */
    os_task_t  *unique_info[OS_TASK_SWITCH_INFO_COUNT];     /* Remove task id information of repeated tasks */
};

typedef struct task_switch_monitor_info task_switch_monitor_info_t;

extern void task_switch_monitor(os_task_t *from, os_task_t *to);

extern void task_switch_info_unique(void);

extern void task_switch_info_show(os_uint32_t context);


#endif /* OS_USING_TASK_SWITCH_MONITOR */

#endif /* OS_USING_TASK_MONITOR */	

#endif /* _RESOURCE_MONITOR_H_ */
