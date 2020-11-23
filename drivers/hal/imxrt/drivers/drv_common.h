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
 * @file        drv_common.h
 *
 * @brief       This file provides _Error_Handler() declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_COMMON_H__
#define __DRV_COMMON_H__

#include <board.h>
#include <os_hw.h>
#include <os_device.h>

#include "fsl_adc.h"
#include "fsl_adc_etc.h"
#include "fsl_aipstz.h"
#include "fsl_aoi.h"
#include "fsl_bee.h"
#include "fsl_cache.h"
#include "fsl_clock.h"
#include "fsl_cmp.h"
#include "fsl_common.h"
#include "fsl_csi.h"
#include "fsl_dcdc.h"
#include "fsl_dcp.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "fsl_elcdif.h"
#include "fsl_enc.h"
#include "fsl_enet.h"
#include "fsl_ewm.h"
#include "fsl_flexcan.h"
#include "fsl_flexio.h"
#include "fsl_flexio_i2c_master.h"
#include "fsl_flexio_i2s.h"
#include "fsl_flexio_i2s_edma.h"
#include "fsl_flexio_spi.h"
#include "fsl_flexio_spi_edma.h"
#include "fsl_flexio_uart.h"
#include "fsl_flexio_uart_edma.h"
#include "fsl_flexram.h"
#include "fsl_flexspi.h"
#include "fsl_gpc.h"
#include "fsl_gpio.h"
#include "fsl_gpt.h"
#include "fsl_iomuxc.h"
#include "fsl_kpp.h"
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_edma.h"
#include "fsl_lpspi.h"
#include "fsl_lpspi_edma.h"
#include "fsl_lpuart.h"
#include "fsl_lpuart_edma.h"
#include "fsl_pit.h"
#include "fsl_pmu.h"
#include "fsl_pwm.h"
#include "fsl_pxp.h"
#include "fsl_qtmr.h"
#include "fsl_rtwdog.h"
#include "fsl_sai.h"
#include "fsl_sai_edma.h"
#include "fsl_semc.h"
#include "fsl_snvs_hp.h"
#include "fsl_snvs_lp.h"
#include "fsl_spdif.h"
#include "fsl_spdif_edma.h"
#include "fsl_src.h"
#include "fsl_tempmon.h"
#include "fsl_trng.h"
#include "fsl_tsc.h"
#include "fsl_usdhc.h"
#include "fsl_wdog.h"
#include "fsl_xbara.h"
#include "fsl_xbarb.h"

#include <drv_spi.h>
#include "drv_adc.h"
#include "drv_can.h"
#include "drv_gpio.h"
#include "drv_hwtimer.h"
#include "drv_i2c.h"
#include "drv_lcd.h"
#include "drv_log.h"
#include "drv_pwm.h"
#include "drv_rtc.h"
#include "drv_sdram.h"
#include "drv_spi.h"
#include "drv_uart.h"
#include "drv_wdg.h"
#include "drv_rng.h"
#include "drv_rtc.h"
#include "drv_adc.h"
#include "drv_sai.h"

#endif
