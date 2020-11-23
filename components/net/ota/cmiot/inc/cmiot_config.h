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
 * @file        cmiot_config.h
 *
 * @brief       The config header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_CONFIG_H__
#define __CMIOT_CONFIG_H__

#include "cmiot_os.h"
#include <fal_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CMIOT_ONEOS_APP    OS_APP_NAME
#define CMIOT_ONEOS_UPDATE OS_DL_PART_NAME

#define CMIOT_SLIM_RES

/* ram size for restore, if use wosun algorithm set it to all available ram, else lusun set it to no more than a sector
 * and no more than 0xFFFF
 */
#ifndef CMIOT_WORKING_BUFFER_LEN
#ifdef CMIOT_ALGORITHM_WOSUN
#define CMIOT_WORKING_BUFFER_LEN (STM32_SRAM_SIZE - 10) * 1024
#else
#define CMIOT_WORKING_BUFFER_LEN 0x400
#endif
#endif

/* driver macro, user to configure */
#ifndef CMIOT_ONEOS
/* 0 means lusun algorithm, 1 means low-level wosun algorithm, low-level wosun for ram less than 512k, 2 means advanced
 * wosun algorithm, advanced wosun for ram greater than 512k
 */
#define CMIOT_FOTA_ALGORITHM 1
#define STM32L4_NUCLEO
/* if set debug, open debug mode, else user mode */
#if (CMIOT_BOOTLOADER_DEBUG == 1) || (CMIOT_APP_DEBUG == 1)
#ifndef CMIOT_DEBUG_MODE
#define CMIOT_DEBUG_MODE
#endif
#endif
/* Download segment, config by user */
#define CMIOT_DEFAULT_SEGMENT_SIZE_INDEX CMIOT_SEGMENT_SIZE_512_INDEX
#ifndef CMIOT_ONEOS_UPDATE
#define CMIOT_ONEOS_UPDATE "download"
#endif
#ifndef CMIOT_ONEOS_APP
#define CMIOT_ONEOS_APP "app"
#endif
/* Config device */
/* Sector size */
#define CMIOT_DEFAULT_SECTOR_SIZE 0x00000800
/* Flash base address */
#define CMIOT_FLASH_BASE_ADDR 0x08000000
/* Bootloader size */
#define CMIOT_BL_SIZE 0x00010000
/* App address */
#define CMIOT_FLASH_APP_ADDR 0x08010000
/* App size */
#define CMIOT_APP_SIZE 0x40000
/* update partition address */
#define CMIOT_UPDATE_ADDR 0x08050000
/* update partition size, include diff package size and 2 sectors size */
#define CMIOT_UPDATE_SIZE 0x30000

/* OEM */
#define CMIOT_FOTA_SERVICE_OEM "L452RE"
/* Device model*/
#define CMIOT_FOTA_SERVICE_MODEL "L452RE"
/* Product id */
#define CMIOT_FOTA_SERVICE_PRODUCT_ID "1551332713"
/* Product secret */
#define CMIOT_FOTA_SERVICE_PRODUCT_SEC "43d1d10afb934ec997f8a3ac2c2dde77"
/* Device type */
#define CMIOT_FOTA_SERVICE_DEVICE_TYPE "box"
/* Platform */
#define CMIOT_FOTA_SERVICE_PLATFORM "stm32l4"
/* Firmware version*/
#define CMIOT_FIRMWARE_VERSION "1.0"
/* Network type */
#define CMIOT_NETWORK_TYPE "NB"
#define CMIOT_KEY_VERSION  2

/* IOT5.0_LUSUN11_R50426 is lusun algorithm, else if wosun algorithm, wosun default is IOT4.0_R42641 */
#define CMIOT_FOTA_LUSUN_VERSION "IOT5.0_LUSUN11_R50426"
#define CMIOT_FOTA_WOSUN_VERSION "IOT4.0_R42641"
#if (CMIOT_FOTA_ALGORITHM == 0)
#define CMIOT_FOTA_SDK_VER CMIOT_FOTA_LUSUN_VERSION
#else
#define CMIOT_FOTA_SDK_VER CMIOT_FOTA_WOSUN_VERSION
#endif
#define CMIOT_FOTA_APP_VERSION "CMIOT_V4.0"
#else
#if (CMIOT_BOOTLOADER_DEBUG == 1)
#ifndef CMIOT_DEBUG_MODE
#define CMIOT_DEBUG_MODE
#endif
#endif
#define CMIOT_FOTA_OS_VERSION "ONEOS_V1.0"
#define CMIOT_KEY_VERSION     3
#endif

#define CMIOT_HW_VERSION "HW01"
#define CMIOT_SW_VERSION "SW01"

#define CMIOT_SEGMENT_SIZE_16_INDEX  0
#define CMIOT_SEGMENT_SIZE_32_INDEX  1
#define CMIOT_SEGMENT_SIZE_64_INDEX  2
#define CMIOT_SEGMENT_SIZE_128_INDEX 3
#define CMIOT_SEGMENT_SIZE_256_INDEX 4
#define CMIOT_SEGMENT_SIZE_512_INDEX 5

typedef enum
{
    /* Quectel */
    CMIOT_MODULE_BC28 = 0,
    /* CMCC */
    CMIOT_MODULE_M5310 = 20,
    /* ESPRESSIF */
    CMIOT_MODULE_ESP8266 = 40,
    CMIOT_MODULE_ESP07S  = 41,
    /* SIMCom */
    CMIOT_MODULE_SIM7020C = 60,
    CMIOT_MODULE_SIM7600  = 61,
    /* Gosuncn */
    CMIOT_MODULE_ME3630 = 80,
    /* Meig */
    CMIOT_MODULE_SLM152 = 100,

    CMIOT_MODULE_MAX = 0xFFFFF
} cmiot_module_t;

/* CMIOT_DEFAULT_NETWORK_PROTOCOL can be set to 1 or 2 */
#define CMIOT_PROTOCOL_NONE      0
#define CMIOT_PROTOCOL_COAP      1
#define CMIOT_PROTOCOL_HTTP      2
#define CMIOT_PROTOCOL_COAP_HTTP (CMIOT_PROTOCOL_COAP | CMIOT_PROTOCOL_HTTP)

/* Try count */
#define CMIOT_TRY_COUNT 6
#if defined(CMIOT_MODULE_ESP8266_ENABLE) || defined(CMIOT_MODULE_ESP07S_ENABLE)
#define CMIOT_WIFI_SSID_PWD
#endif

#ifdef __cplusplus
}
#endif

#endif
