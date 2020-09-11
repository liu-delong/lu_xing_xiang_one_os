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
 * @file        m5311_onenet_nb.c
 *
 * @brief       m5311 module link kit onenet nb api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "at_parser.h"
#include "m5311_onenet_nb.h"
#include "mo_onenet_nb.h"
#include "mo_general.h"

#include <string.h>

#define DBG_EXT_TAG "m5311.onenet_nb"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>
#include <stdlib.h>

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_create, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 64, 0, (timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout)));

    char tmp_format[128] = "AT+MIPLCREATE=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) != OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_ERROR;
    }

    at_parser_reset_resp(parser);

    os_uint8_t ref = 0;

    if (at_parser_get_data_by_kw(parser, "+MIPLCREATE:", "+MIPLCREATE:%d", &ref) > 0)
    {
        *(os_uint8_t *)resp = ref;
        return OS_EOK;
    }

    return OS_ERROR;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_createex, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 64, 0, (timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout)));

    char tmp_format[64] = "AT+MIPLCREATEEX=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) != OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_ERROR;
    }
    at_parser_reset_resp(parser);

    os_uint8_t ref = 0;
    if (at_parser_get_data_by_kw(parser, "+MIPLCREATEEX:", "+MIPLCREATEEX:%d", &ref) > 0)
    {
        *(os_uint8_t *)resp = ref;
        return OS_EOK;
    }

    return OS_ERROR;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_addobj, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 64, 0, (timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout)));

    char tmp_format[64] = "AT+MIPLADDOBJ=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) == OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_EOK;
    }
    at_parser_reset_resp(parser);

    return OS_ERROR;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_discoverrsp, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 64, 0, (timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout)));

    char tmp_format[128] = "AT+MIPLDISCOVERRSP=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) == OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_EOK;
    }
    at_parser_reset_resp(parser);

    return OS_ERROR;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_set_nmi, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 64, 0, (timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout)));

    char tmp_format[28] = "AT+MIPLNMI=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) == OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_EOK;
    }
    at_parser_reset_resp(parser);

    return OS_ERROR;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_open, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 128, 6, (timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout)));

    char tmp_format[28] = "AT+MIPLOPEN=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) != OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_ERROR;
    }
    at_parser_reset_resp(parser);

    if (OS_NULL != at_parser_get_line_by_kw(parser, "+MIPLEVENT:0,6"))
    {
        return OS_EOK; /* success */
    }

    return OS_ERROR;
}

os_err_t get_onenetnb_notify_ackid(const char *format, va_list args, os_uint16_t *id)
{
    /* if has ackid, must be the 11 element */
    int        num_count = 1;
    os_int32_t id_tmp    = -1;
    os_uint8_t qualifier;

    for (; *format; ++format)
    {
        if (*format == ',')
        {
            ++num_count;
        }

        if (*format != '%')
        {
            continue;
        }
        ++format; /* ignore % */

        /* Get the conversion qualifier */
        qualifier = 0;
        if ((*format == 'h') || (*format == 'l') || (*format == 'L'))
        {
            qualifier = *format;
            ++format;

            if ((qualifier == 'l') && (*format == 'l'))
            {
                qualifier = 'L';
                ++format;
            }
        }

        switch (*format)
        {
        case 'c':
        {
            id_tmp = va_arg(args, int);
            continue;
        }
        case 's':
        {
            va_arg(args, char *);
            continue;
        }
        case 'p':
        {
            va_arg(args, void *);
            continue;
        }
        case '%':
            continue;
        case 'o':
        case 'X':
        case 'x':
        case 'd':
        case 'i':
        case 'u':
            break;
        }

        if (qualifier == 'L')
        {
            id_tmp = (os_int32_t)va_arg(args, long long);
        }
        else if (qualifier == 'l')
        {
            id_tmp = (os_int32_t)va_arg(args, os_uint32_t);
        }
        else if (qualifier == 'h')
        {
            id_tmp = (os_int32_t)va_arg(args, os_int32_t);
        }
        else
        {
            id_tmp = (os_int32_t)va_arg(args, os_uint32_t);
        }

        if (num_count == 11) /* must 11 element */
        {
            *id = (os_uint16_t)id_tmp;
            return OS_EOK;
        }
    }

    return OS_ERROR;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_notify, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_bool_t is_ack = OS_FALSE;

    at_parser_t *parser = &self->parser;
    m5311_nmi_t nmi;
    if (m5311_onenetnb_get_nmi(self, -1, &nmi, OS_NULL, args) != OS_EOK) /* get nsmi */
    {
        return OS_ERROR;
    }

    os_uint16_t ackid_in = 0;
    if (get_onenetnb_notify_ackid(format, args, &ackid_in) == OS_EOK) /* whether has ackid */
    {
        is_ack = OS_TRUE;
    }

    char value[20] = {0};
    if (mo_get_app_version(self, value, 20) != OS_EOK) /* get app version */
    {
        return OS_ERROR;
    }

    char tmp_format[128] = "AT+MIPLNOTIFY=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    int line_num = 2; /* normal: \r\n OK\r\n */
    if (strstr(value, "MCM") != OS_NULL)
    {
        line_num = ((nmi.nsmi == 1) ? line_num + 2 : line_num);
    }
    line_num = (is_ack ? line_num + 2 : line_num);

    at_parser_set_resp(parser, 64, line_num, (timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout)));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) != OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_ERROR;
    }
    at_parser_reset_resp(parser);

    if (!is_ack)
    {
        return OS_EOK;
    }

    os_int32_t ackid_out = 0;
    if (at_parser_get_data_by_kw(parser, "+MIPLEVENT:0,26", "+MIPLEVENT:0,26,%d", &ackid_out) <= 0)
    {
        return OS_ERROR;
    }

    if (ackid_in == ackid_out)
    {
        return OS_EOK;
    }

    return OS_ERROR;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_update, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 64, 4, (timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout)));

    char tmp_format[64] = "AT+MIPLUPDATE=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) != OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_ERROR;
    }
    at_parser_reset_resp(parser);

    os_uint8_t  ref      = 0;
    os_uint16_t event_id = 0;
    if (at_parser_get_data_by_kw(parser, "+MIPLEVENT:", "+MIPLEVENT:%d,%d", &ref, &event_id) <= 0)
    {
        return OS_ERROR;
    }
    if (event_id == 11)
    {
        return OS_EOK;
    }

    return OS_ERROR;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_get_write, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    module_mgr_resp_t *mgr = (module_mgr_resp_t *)resp;
    if (mgr->value == OS_NULL) /* value ptr is null */
    {
        return OS_ERROR;
    }

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 256, 0, (timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout)));

    char tmp_format[128] = "AT+MIPLMGR=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) != OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_ERROR;
    }
    at_parser_reset_resp(parser);

    if (at_parser_get_data_by_kw(parser,
                                 "+MIPLWRITE:",
                                 "+MIPLWRITE:%d,%d,%d,%d,%d,%d,%d,%s",
                                 &mgr->ref,
                                 &mgr->mid,
                                 &mgr->objid,
                                 &mgr->insid,
                                 &mgr->resid,
                                 &mgr->type,
                                 &mgr->len,
                                 mgr->value) > 0)
    {
        mgr->value[mgr->len] = '\0';
        return OS_EOK;
    }

    return OS_ERROR;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_writersp, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 64, 0, timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout));

    char tmp_format[128] = "AT+MIPLWRITERSP=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) == OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_EOK;
    }
    at_parser_reset_resp(parser);

    return OS_ERROR;
}

/* for inner */
DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_get_nmi, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);

    at_parser_t *parser = &self->parser;
    if (at_parser_exec_cmd(parser, "AT+MIPLNMI?") != OS_EOK) /* get nmi config */
    {
        return OS_ERROR;
    }

    int ref, nnmi, nsmi;
    ref = nnmi = nsmi = 0;
    if (at_parser_get_data_by_kw(parser, "+MIPLNMI:", "+MIPLNMI:%d,%d,%d", &ref, &nnmi, &nsmi) <= 0) /* parser nmi config */
    {
        return OS_ERROR;
    }
    ((m5311_nmi_t *)resp)->ref  = ref;
    ((m5311_nmi_t *)resp)->nnmi = nnmi;
    ((m5311_nmi_t *)resp)->nsmi = nsmi;
    return OS_EOK;
}

#ifdef OS_USING_SHELL
DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_all, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 256, 0, (timeout < 0 ? parser->resp.timeout : os_tick_from_ms(timeout)));

    if (at_parser_exec_cmd_valist(parser, format, args) == OS_EOK)
    {
        at_parser_reset_resp(parser);
        return OS_EOK;
    }
    at_parser_reset_resp(parser);

    return OS_ERROR;
}
#endif