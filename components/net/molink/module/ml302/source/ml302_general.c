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
 * @file        ml302_general.c
 *
 * @brief       ml302 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#include "at_parser.h"
#include "ml302_general.h"

#define  DBG_EXT_TAG "ml302_general"
#define  DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define  ML302_IMEI_LEN 15
#define  ML302_IMSI_LEN 15
#define  ML302_ICCID_LEN 20

#ifdef   ML302_USING_GENERAL_OPS

os_err_t ml302_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t ml302_get_imei(mo_object_t * self,char * value,os_size_t len)
{
    OS_ASSERT(len > ML302_IMEI_LEN);

    at_parser_t *parser = &self->parser;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    if(OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CGSN=1"))
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGSN:", "+CGSN:%s", value) <= 0)
    {
        LOG_EXT_E("Get %s module imei failed.", self->name);
        return OS_ERROR;
    }

    value[ML302_IMEI_LEN] = '\0';

    LOG_EXT_D("%s module imei:%s", self->name, value);

    return OS_EOK;
}

os_err_t ml302_get_imsi(mo_object_t * self,char * value,os_size_t len)
{
    OS_ASSERT(len > ML302_IMSI_LEN);

    at_parser_t *parser = &self->parser;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    if(OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CIMI"))
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_line(&resp, resp.line_counts - 2, "%s", value) <= 0)
    {
        LOG_EXT_E("Get %s module imsi failed.");
        return OS_ERROR;
    }

    value[ML302_IMSI_LEN] = '\0';

    LOG_EXT_D("%s module imsi:%s",self->name,value);

    return OS_EOK;
}

os_err_t ml302_get_iccid(mo_object_t * self,char * value,os_size_t len)
{
    OS_ASSERT(len > ML302_ICCID_LEN);

    at_parser_t *parser = &self->parser;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    if(OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+ICCID"))
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+ICCID:", "+ICCID: %s", value) <= 0)
    {
        LOG_EXT_E("Get %s module ccid failed.", self->parser);
        return OS_ERROR;
    }

    value[ML302_ICCID_LEN] = '\0';

    return OS_EOK;
}

os_err_t ml302_get_cfun(mo_object_t * self,os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CFUN?"))
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN: %d",fun_lvl) <= 0)
    {
        LOG_EXT_E("Get %s module level of functionality failed.", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t ml302_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%d", fun_lvl);
}


#endif   /* ML302_USING_GENERAL_OPS */
