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
 * @file        esp8266.c
 *
 * @brief       esp8266 module link kit network service api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp8266_netserv.h"
#include "esp8266.h"

#include <stdlib.h>
#include <string.h>

#ifdef MOLINK_USING_IP
#include <mo_ipaddr.h>
#endif /* MOLINK_USING_IP */

#define DBG_EXT_TAG "esp8266.netserv"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef ESP8266_USING_NETSERV_OPS

os_err_t esp8266_get_ipaddr(mo_object_t *self, char ip[])
{
    at_parser_t *parser = &self->parser;
    os_int8_t    len    = -1;

    char ipaddr[IP_SIZE] = {0};

    at_parser_set_resp(parser, 128, 0, os_tick_from_ms(5000));
    os_err_t result = at_parser_exec_cmd(parser, "AT+CIFSR");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    /* Response for ex: +CIFSR:STAIP,"100.113.120.235" */
    if (at_parser_get_data_by_kw(parser, "+CIFSR:STAIP", "+CIFSR:STAIP,\"%[^\"]", ipaddr) <= 0)
    {
        LOG_EXT_E("Get ip address: parse resp fail.");
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(ipaddr);
    if ((len < MIN_IP_SIZE) || (len >= IP_SIZE))
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
    
    at_parser_reset_resp(parser);

    return result;
}

os_err_t esp8266_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint16_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser    = &self->parser;
    os_uint32_t  ping_time = 0;

    char ip_addr[IP_SIZE] = {0};    

    os_err_t result = at_parser_exec_cmd(parser, "AT+CIPDOMAIN=\"%s\"", host);

    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser, "+CIPDOMAIN:", "+CIPDOMAIN:%s", ip_addr) < 0)
    {
        LOG_EXT_E("ping: get the IP address failed");
        result = OS_ERROR;
        goto __exit;
    }

    at_parser_set_resp(parser, 64, 0, timeout);

    /* send ping commond "AT+PING=<IP>" and wait response */
    result = at_parser_exec_cmd(parser, "AT+PING=\"%s\"", ip_addr);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_parser_get_data_by_kw(parser,  "+", "+%d", &ping_time) < 0)
    {
        LOG_EXT_E("ping: get the ping time error");
        result = OS_ERROR;
        goto __exit;
    }

    if (ping_time != 0)
    {
        inet_aton(ip_addr, &(resp->ip_addr));
        resp->data_len = len;
        resp->ttl      = 0;
        resp->time     = ping_time;
    }

__exit:
    
    at_parser_reset_resp(parser);

    return result;
}

#endif /* ESP8266_USING_NETSERV_OPS */
