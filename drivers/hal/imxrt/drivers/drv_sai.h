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
 * @file        drv_sai.h
 *
 * @brief       This file implements wdt driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRV_SAI_H__
#define DRV_SAI_H__

#include <os_device.h>

struct nxp_sai_info {
    I2S_Type *sai_base;
    const sai_transceiver_t *tx_config;
    const sai_transceiver_t *rx_config;
};


#endif /* __DRV_SAI_H__ */


