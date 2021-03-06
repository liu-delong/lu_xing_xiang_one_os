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
 * @file        bc28_ifconfig.c
 *
 * @brief       bc28 module link kit ifconfig api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <mo_ipaddr.h>
#include "bc28_general.h"
#include "bc28_netserv.h"
#include "bc28_ifconfig.h"

#include <stdlib.h>
#include <string.h>

#define DBG_EXT_TAG "bc28.ifconfig"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef BC28_USING_IFCONFIG_OPS

os_err_t bc28_ifconfig(mo_object_t *self)
{
    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    if (bc28_get_ipaddr(self, ipaddr) != OS_EOK)
    {
        memset(ipaddr, 0, sizeof(ipaddr));
    }

    char imei[MO_IMEI_LEN + 1] = {0};
    if (bc28_get_imei(self, imei, sizeof(imei)) != OS_EOK)
    {
        memset(imei, 0, sizeof(imei));
    }

    char iccid[MO_ICCID_LEN + 1] = {0};
    if (bc28_get_iccid(self, iccid, sizeof(iccid)) != OS_EOK)
    {
        memset(iccid, 0, sizeof(iccid));
    }

    os_uint8_t rssi = 0;
    os_uint8_t ber  = 0;

    if (bc28_get_csq(self, &rssi, &ber) != OS_EOK)
    {
        rssi = 0;
        ber  = 0;
    }

    os_kprintf("\nLIST AT MODULE INFORMATION\n");
    for (int i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }

    os_kprintf("\n");
    os_kprintf("Module Name    : %s\n", self->name);
    os_kprintf("IMEI   Number  : %s\n", imei);
    os_kprintf("ICCID  Number  : %s\n", iccid);
    os_kprintf("Signal Quality : rssi(%d), ber(%d)\n", rssi, ber);
    os_kprintf("IPv4   Address : %s\n", strlen(ipaddr) ? ipaddr : "0.0.0.0");

    for (int i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\n");

    return OS_EOK;
}

os_err_t bc28_get_ipaddr(mo_object_t *self, char ip[])
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

    /* Response for ex: +CGPADDR:0,100.113.120.235 */
    if (at_resp_get_data_by_kw(&resp, "+CGPADDR:", "+CGPADDR:%hhd,%[^,]", &ucid, ipaddr) <= 0)
    {
        LOG_EXT_E("Get ip address: parse resp fail.");
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(ipaddr);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        LOG_EXT_E("IP address size [%d] error.", len);
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

os_err_t bc28_set_dnsserver(mo_object_t *self, dns_server_t dns)
{
    /* BC28 must set usable dns server befor QDNS(gethostbyname), 
       otherwise both func will not be reachable before reboot! */
    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    if (OS_NULL == parser)
    {
        LOG_EXT_E("BC28 %s: at parser is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    /* it appears that module will reset dns if secondary dns is "0" */
    if (0 == strlen(dns.primary_dns) || 0 == strcmp(dns.secondary_dns, "0"))
    {
        LOG_EXT_E("BC28 %s: with invalid param.", __FUNCTION__);
        return OS_ERROR;
    }

    if (0 == strlen(dns.secondary_dns))
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+QIDNSCFG=\"%s\"", dns.primary_dns);
    }
    else
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+QIDNSCFG=\"%s\",\"%s\"", dns.primary_dns, dns.secondary_dns);
    }
    
    return result;
}

os_err_t bc28_get_dnsserver(mo_object_t *self, dns_server_t *dns)
{
    at_parser_t *parser         = &self->parser;
    os_err_t     result         = OS_EOK;

    char primary_dns[IPADDR_MAX_STR_LEN + 1]   = {0};
    char secondary_dns[IPADDR_MAX_STR_LEN + 1] = {0};

    if (OS_NULL == parser)
    {
        LOG_EXT_E("BC28 %s: at parser is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    if (OS_NULL == dns)
    {
        LOG_EXT_E("BC28 %s: dns is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    char resp_buff[256] = {0};

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 4,
                         .timeout   = 16 * OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &at_resp, "AT+QIDNSCFG?");

    /* return eg. PrimaryDns: 218.4.4.4\r\nSecondaryDns: 208.67.222.222 at BC28JAR01A04_ONT */
    if (at_resp_get_data_by_kw(&at_resp, "PrimaryDns:", "PrimaryDns: %s", primary_dns) <= 0)
    {
        LOG_EXT_E("BC28 %s: get primary dns failed.", __FUNCTION__);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "SecondaryDns:", "SecondaryDns: %s", secondary_dns) <= 0)
    {
        LOG_EXT_E("BC28 %s: get secondary dns failed.", __FUNCTION__);
        result = OS_ERROR;
        goto __exit;
    }

    strcpy(dns->primary_dns, primary_dns);
    strcpy(dns->secondary_dns, secondary_dns);

    LOG_EXT_D("BC28 %s: primary_dns[%s],secondary_dns[%s]", __FUNCTION__, dns.primary_dns, dns.secondary_dns);

__exit:
    return result;
}

#endif /* BC28_USING_IFCONFIG_OPS */
