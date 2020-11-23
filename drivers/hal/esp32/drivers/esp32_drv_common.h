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
 * @file        esp_drv_common.h
 *
 * @brief       
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_COMMON_H__
#define __DRV_COMMON_H__

#include <board.h>
#include <os_hw.h>
#include <os_device.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/timer.h"
struct esp32_timer_info{
    timer_group_t group;
    timer_idx_t id;
};



#ifdef __cplusplus
}
#endif

#endif
