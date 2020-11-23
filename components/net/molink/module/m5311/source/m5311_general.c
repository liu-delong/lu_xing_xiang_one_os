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
 * @file        m5311_general.c
 *
 * @brief       m5311 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "m5311_general.h"

#define DBG_EXT_TAG "m5311.general"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define M5311_PSM_QUOTES_LEN    (2)

#ifdef M5311_USING_GENERAL_OPS

os_err_t m5311_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t m5311_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMEI_LEN);

    at_parser_t *parser = &self->parser;
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+GSN");
    if(result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_line(&resp, 2, "%s", value) <= 0)
    {
        LOG_EXT_E("Get %s module imei failed", self->name);
        return OS_ERROR;
    }

    value[MO_IMEI_LEN] = '\0';

    LOG_EXT_D("module %s imei:%s", value);

    return OS_EOK;
}

os_err_t m5311_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMSI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIMI");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_line(&resp, 2, "%s", value) <= 0)
    {
        LOG_EXT_E("Get %s module imsi failed", self->name);
        return OS_ERROR;
    }

    value[MO_IMSI_LEN] = '\0';

    LOG_EXT_D("module %s imsi:%s", value);

    return OS_EOK;
}

os_err_t m5311_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_ICCID_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+ICCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+ICCID: ", "+ICCID: %s", value) <= 0)
    {
        LOG_EXT_E("Get %s module iccid failed", self->name);
        return OS_ERROR;
    }

    value[MO_ICCID_LEN] = '\0';

    LOG_EXT_D("module %s iccid: %s", value);

    return OS_EOK;
}

os_err_t m5311_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CFUN?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN:%hhu", fun_lvl) <= 0)
    {
        LOG_EXT_E("Get %s module level of functionality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%hhu", fun_lvl);
}

#endif /* M5311_USING_GENERAL_OPS */
