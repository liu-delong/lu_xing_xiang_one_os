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
 * @file        drv_wwdt.c
 *
 * @brief       This file implements wwdt driver for lpc
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_irq.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <drv_cfg.h>
#include <drv_utick.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.utick"
#include <drv_log.h>

//static struct os_watchdog_ops lpc_wwdt_ops =
//{
//    .init = lpc_wwdt_init,
//    .control = lpc_wwdt_control,
//};

//static int stm32_wwdt_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
//{
//    struct lpc_wwdt *wwdt;

////    wwdt = os_calloc(1, sizeof(struct lpc_wwdt));

////    OS_ASSERT(wwdt);

//////    wwdg->hwwdg = (WWDG_HandleTypeDef *)dev->info;
//////    wwdg->wdg.ops = &ops;

////    if (os_hw_watchdog_register(&wwdt->wdg, dev->name, OS_DEVICE_FLAG_DEACTIVATE, OS_NULL) != OS_EOK)
////    {
////        LOG_EXT_E("wwdt device register failed.");
////        return OS_ERROR;
////    }
//    LOG_EXT_D("wwdt device register success.");
//    return OS_EOK;
//}

//OS_DRIVER_INFO stm32_wwdt_driver = {
//    .name   = "WWDG_HandleTypeDef",
//    .probe  = stm32_wwdt_probe,
//};

//OS_DRIVER_DEFINE(stm32_wwdt_driver, "1");


