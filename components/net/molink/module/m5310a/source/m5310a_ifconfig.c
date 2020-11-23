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
 * @file        m5310a_ifconfig.c
 *
 * @brief       m5310a module link kit ifconfig api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdlib.h>
#include <string.h>
#include <mo_ipaddr.h>

#include "m5310a_general.h"
#include "m5310a_netserv.h"
#include "m5310a_ifconfig.h"

#define DBG_EXT_TAG "m5310a.ifconfig"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef M5310A_USING_IFCONFIG_OPS

os_err_t m5310a_ifconfig(mo_object_t *self)
{
    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    char imei[MO_IMEI_LEN + 1]          = {0};
    char iccid[MO_ICCID_LEN + 1]        = {0};

    os_uint8_t rssi = 0;
    os_uint8_t ber  = 0;
    os_int32_t i    = 0;

    if (m5310a_get_ipaddr(self, ipaddr) != OS_EOK)
    {
        memset(ipaddr, 0, sizeof(ipaddr));
    }

    if (m5310a_get_imei(self, imei, sizeof(imei)) != OS_EOK)
    {
        memset(imei, 0, sizeof(imei));
    }

    if (m5310a_get_iccid(self, iccid, sizeof(iccid)) != OS_EOK)
    {
        memset(iccid, 0, sizeof(iccid));
    }

    if (m5310a_get_csq(self, &rssi, &ber) != OS_EOK)
    {
        rssi = 0;
        ber  = 0;
    }

    os_kprintf("\nLIST AT MODULE INFORMATION\n");
    for (i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }

    os_kprintf("\n");
    os_kprintf("Module Name    : %s\n", self->name);
    os_kprintf("IMEI   Number  : %s\n", imei);
    os_kprintf("ICCID  Number  : %s\n", iccid);
    os_kprintf("Signal Quality : rssi(%d), ber(%d)\n", rssi, ber);
    os_kprintf("IPv4   Address : %s\n", strlen(ipaddr) ? ipaddr : "0.0.0.0");

    for (i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\n");

    return OS_EOK;
}

os_err_t m5310a_get_ipaddr(mo_object_t *self, char ip[])
{
    at_parser_t *parser = &self->parser;
    os_int8_t    ucid   = -1;
    os_int8_t    len    = -1;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGPADDR");
    if (result != OS_EOK)
    {
        LOG_EXT_E("Get ip address fail: AT+CGPADDR cmd exec fail.");
        goto __exit;
    }

    /* Response for ex: +CGPADDR:0,10.208.88.25,2409:8962:B516:123A:1:0:4C7:E50F */
    if (at_resp_get_data_by_kw(&resp, "+CGPADDR:", "+CGPADDR:%hhd,%[^,]", &ucid, ipaddr) <= 0)
    {
        LOG_EXT_E("Get ip address: parse resp fail.");
        result = OS_ERROR;
        goto __exit;
    }
    /* Delete the last useless byte of data --OD */
    ipaddr[strlen(ipaddr) - 1] = 0;

    len = strlen(ipaddr);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        LOG_EXT_D("IP address size [%hhd]", len);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        strcpy(ip, ipaddr);
        LOG_EXT_D("IP address: %s", ip);
    }

__exit:
    
    return result;
}

#endif /* M5310A_USING_IFCONFIG_OPS */
