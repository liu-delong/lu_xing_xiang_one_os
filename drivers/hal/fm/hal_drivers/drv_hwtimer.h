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
 * @file        drv_hwtimer.h
 *
 * @brief       This file provides functions declaration for STM32 timer driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_HWTIMER_H__
#define __DRV_HWTIMER_H__

#include <os_task.h>

#ifdef OS_USING_BSTIMER1
#ifndef USING_BSTIMER1_CONFIG
#define USING_BSTIMER1_CONFIG                                        \
    {                                                       \
       .tim_handle.Instance     = BTIM1,                    \
       .tim_handle.periph_def     = BT1CLK,                    \
       .tim_irqn                = BTIM1_IRQn,  \
       .name                    = "btim1",                \
    }
#endif /* USING_BSTIMER1_CONFIG */
#endif /* OS_USING_BSTIMER1 */

#ifdef OS_USING_BSTIMER2
#ifndef USING_BSTIMER2_CONFIG
#define USING_BSTIMER2_CONFIG                                        \
    {                                                       \
       .tim_handle.Instance     = BTIM2,                    \
       .tim_handle.periph_def     = BT2CLK,                    \
       .tim_irqn                = BTIM2_IRQn,  \
       .name                    = "btim2",                \
    }
#endif /* USING_BSTIMER2_CONFIG */
#endif /* OS_USING_BSTIMER2 */


#ifndef TIM_DEV_INFO_CONFIG
#define TIM_DEV_INFO_CONFIG                     \
    {                                           \
        .maxfreq = 10*1000*1000,                     \
        .minfreq = 100*1000,                        \
        .maxcnt  = 0xFFFF,                      \
        .cntmode = HWTIMER_CNTMODE_UP,          \
    }
#endif /* TIM_DEV_INFO_CONFIG */

typedef struct __FM_BTIMHandleTypeDef
{
    BTIMx_Type                 *Instance;     /*!< Register base address             */
    BTIM_InitTypeDef        Init;          /*!< TIM Time Base required parameters */
    os_uint32_t       periph_def;       /*!< Active channel                    */
} FM_BTIMHandleTypeDef;

//os_uint8_t get_timer_line(TIM_TypeDef *Instance);

#endif
