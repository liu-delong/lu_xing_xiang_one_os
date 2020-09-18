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
 * @file        ec200x.c
 *
 * @brief       ec200x module link kit general api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ec200x_general.h"

#define DBG_EXT_TAG "ec200x.general"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define EC200X_IMEI_LEN  15
#define EC200X_IMSI_LEN  15
#define EC200X_ICCID_LEN 20

#ifdef EC200X_USING_GENERAL_OPS

os_err_t ec200x_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t ec200x_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > EC200X_IMEI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+GSN");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_line(&resp, 2, "%s", value) <= 0)
    {
        LOG_EXT_E("Get %s module imei failed", self->name);
        return OS_ERROR;
    }

    value[EC200X_IMEI_LEN] = '\0';

    LOG_EXT_D("module %s imei:%s", value);

    return OS_EOK;
}

os_err_t ec200x_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > EC200X_IMSI_LEN);

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

    value[EC200X_IMSI_LEN] = '\0';

    LOG_EXT_D("module %s imsi:%s", value);

    return OS_EOK;
}

os_err_t ec200x_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > EC200X_ICCID_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+QCCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+QCCID:", "+QCCID: %s", value) <= 0)
    {
        LOG_EXT_E("Get %s module iccid failed", self->name);
        return OS_ERROR;
    }

    value[EC200X_ICCID_LEN] = '\0';

    LOG_EXT_D("module %s imsi:%s", value);

    return OS_EOK;
}

os_err_t ec200x_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CFUN?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN:%d", fun_lvl) <= 0)
    {
        LOG_EXT_E("Get %s module level of functionality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t ec200x_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%d", fun_lvl);
}

os_err_t ec200x_set_echo(mo_object_t *self, os_bool_t is_echo)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "ATE%d", is_echo ? OS_TRUE : OS_FALSE);
}

#endif /* EC200X_USING_GENERAL_OPS */
