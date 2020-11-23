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
 * @file        cmiot_self.c
 *
 * @brief       Implement self functions
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "cmiot_typedef.h"
#include "cmiot_at_device.h"
#include "cmiot_hal.h"
#include "cmiot_os.h"

#if !defined(NET_USING_LWIP) || !defined(LWIP_USING_UDP) || !defined(LWIP_USING_TCP)
#if !defined(MOLINK_USING_SOCKETS_OPS)
#error "OTA Module is selected, lwip or at socket must be selected!\
you can select lwip by this steps:\
1,Components->NetWork->LwIP->Enable lwip stack.\
you can select at socket by this steps:\
1,Components->NetWork->Molink->Enable IoT modules support/Modules->WiFi Modules Support->ESP8266/ESP8266 Config->Enable ESP8266 Module BSD Socket Operates."
#endif
#endif

cmiot_uint16 cmiot_get_cmiot_at_rxbuf_len()
{
    return CMIOT_UARTRX_MAXLEN;
}

cmiot_uint cmiot_get_hal_data_len(void)
{
    return CMIOT_HAL_DATA_MAX_LEN;
}

cmiot_uint8 cmiot_get_default_protocol(void)
{
    return (cmiot_uint8)CMIOT_DEFAULT_NETWORK_PROTOCOL;
}

cmiot_char *cmiot_get_manufacturer(void)
{
    return (cmiot_char *)CMIOT_FOTA_SERVICE_OEM;
}

cmiot_char *cmiot_get_model_number(void)
{
    return (cmiot_char *)CMIOT_FOTA_SERVICE_MODEL;
}

cmiot_char *cmiot_get_product_id(void)
{
    return (cmiot_char *)CMIOT_FOTA_SERVICE_PRODUCT_ID;
}

cmiot_char *cmiot_get_product_sec(void)
{
    return (cmiot_char *)CMIOT_FOTA_SERVICE_PRODUCT_SEC;
}

cmiot_char *cmiot_get_device_type(void)
{
    return (cmiot_char *)CMIOT_FOTA_SERVICE_DEVICE_TYPE;
}

cmiot_char *cmiot_get_platform(void)
{
    return (cmiot_char *)CMIOT_FOTA_SERVICE_PLATFORM;
}

cmiot_char *cmiot_get_apk_version(void)
{
    return (cmiot_char *)CMIOT_FOTA_OS_VERSION;
}

cmiot_char *cmiot_get_firmware_version(void)
{
    return (cmiot_char *)CMIOT_FIRMWARE_VERSION;
}

cmiot_uint8 cmiot_get_default_segment_size(void)
{
    return (cmiot_uint8)CMIOT_DEFAULT_SEGMENT_SIZE_INDEX;
}

cmiot_uint cmiot_get_buf_len(void)
{
    return CMIOT_BUF_MAX_LEN;
}
