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
 * @file        sai.c
 *
 * @brief       This file implements SAI driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_cfg.h>
#include <string.h>
#include <os_sem.h>
#include <os_memory.h>
#include <os_irq.h>
#include <audio/sai.h>

void os_sai_register(const char *name, os_device_sai_t *sai)
{
    OS_ASSERT(sai != OS_NULL);
    OS_ASSERT(sai->ops != OS_NULL);
    
    sai->parent.type  = OS_DEVICE_TYPE_SOUND;
    os_device_register(&sai->parent, name, OS_DEVICE_FLAG_STANDALONE);
}

