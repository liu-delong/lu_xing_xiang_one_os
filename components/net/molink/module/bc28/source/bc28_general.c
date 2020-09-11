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
 * @file        bc28_general.c
 *
 * @brief       bc28 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc28_general.h"

#define DBG_EXT_TAG "bc28.general"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define BC28_IMEI_LEN  15
#define BC28_IMSI_LEN  15
#define BC28_ICCID_LEN 20

#ifdef BC28_USING_GENERAL_OPS

os_err_t bc28_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT");
}

os_err_t bc28_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > BC28_IMEI_LEN);

    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CGSN=1");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_parser_get_data_by_kw(parser, "+CGSN:", "+CGSN:%s", value) <= 0)
    {
        LOG_EXT_E("Get %s module imei failed", self->name);
        return OS_ERROR;
    }

    value[BC28_IMEI_LEN] = '\0';

    LOG_EXT_D("module %s imei:%s", value);

    return OS_EOK;
}

os_err_t bc28_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > BC28_IMSI_LEN);

    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CIMI");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_parser_get_data_by_line(parser, 2, "%s", value) <= 0)
    {
        LOG_EXT_E("Get %s module imsi failed", self->name);
        return OS_ERROR;
    }

    value[BC28_IMSI_LEN] = '\0';

    LOG_EXT_D("module %s imsi:%s", value);

    return OS_EOK;
}

os_err_t bc28_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > BC28_ICCID_LEN);

    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+NCCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_parser_get_data_by_kw(parser, "+NCCID:", "+NCCID:%s", value) <= 0)
    {
        LOG_EXT_E("Get %s module iccid failed", self->name);
        return OS_ERROR;
    }

    value[BC28_ICCID_LEN] = '\0';

    LOG_EXT_D("module %s imsi:%s", value);

    return OS_EOK;
}

os_err_t bc28_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+CFUN?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_parser_get_data_by_kw(parser, "+CFUN:", "+CFUN:%d", fun_lvl) <= 0)
    {
        LOG_EXT_E("Get %s module level of functionality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc28_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT+CFUN=%d", fun_lvl);
}

#endif /* BC28_USING_GENERAL_OPS */
