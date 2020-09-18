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
 * @file        m5310a_general.c
 *
 * @brief       m5310-a module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5310a_general.h"
#include <string.h>

#define DBG_EXT_TAG "m5310a.general"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define M5310A_IMEI_LEN  15
#define M5310A_IMSI_LEN  15
#define M5310A_ICCID_LEN 20

#ifdef M5310A_USING_GENERAL_OPS

os_err_t m5310a_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t m5310a_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > M5310A_IMEI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGSN=1");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGSN:", "+CGSN:%s", value) <= 0)
    {
        LOG_EXT_E("Get %s module imei failed", self->name);
        return OS_ERROR;
    }

    value[M5310A_IMEI_LEN] = '\0';

    LOG_EXT_D("module %s imei:%s", value);

    return OS_EOK;
}

os_err_t m5310a_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > M5310A_IMSI_LEN);

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

    value[M5310A_IMSI_LEN] = '\0';

    LOG_EXT_D("module %s imsi:%s", value);

    return OS_EOK;
}

os_err_t m5310a_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > M5310A_ICCID_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+NCCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+NCCID:", "+NCCID:%s", value) <= 0)
    {
        LOG_EXT_E("Get %s module iccid failed", self->name);
        return OS_ERROR;
    }

    value[M5310A_ICCID_LEN] = '\0';

    LOG_EXT_D("module %s imsi:%s", value);

    return OS_EOK;
}

os_err_t m5310a_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
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

os_err_t m5310a_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%d", fun_lvl);
}

os_err_t m5310a_clear_stored_earfcn(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+NCSEARFCN");
}

os_err_t m5310a_get_app_version(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(value != OS_NULL);

    at_parser_t *parser = &self->parser;

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGMR");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    const char *version_begin = at_resp_get_line_by_kw(&resp, "APPLICATION_A,");
    if (version_begin == OS_NULL)
    {
        LOG_EXT_E("Get %s module application version failed", self->name);
        return OS_ERROR;
    }

    const char *version_end = strstr(version_begin + strlen("APPLICATION_A,"), "\r");

    if (len < version_end - version_begin - strlen("APPLICATION_A,"))
    {
        LOG_EXT_E("%s module application version buf len is not enough", self->name);
        return OS_ENOMEM;
    }

    len = version_end - version_begin - strlen("APPLICATION_A,");

    memcpy(value, version_begin + strlen("APPLICATION_A,"), len);

    LOG_EXT_D("module %s version:%s", value);

    return OS_EOK;
}

#endif /* M5310A_USING_GENERAL_OPS */
