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
 * @file        esop8266_general.c
 *
 * @brief       esp8266 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp8266_general.h"

#define DBG_EXT_TAG "esp8266.general"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef ESP8266_USING_GENERAL_OPS

os_err_t esp8266_soft_reset(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_exec_cmd(parser, "AT+RST");

    os_task_mdelay(1000);

    return result;
}

os_err_t esp8266_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    return at_parser_exec_cmd(parser, "AT");
}

os_err_t esp8266_set_echo(mo_object_t *self, os_bool_t is_echo)
{
    at_parser_t *parser = &self->parser;
    
    return at_parser_exec_cmd(parser, "ATE%d", is_echo ? OS_TRUE : OS_FALSE);
}

#endif /* ESP8266_USING_GENERAL_OPS */
