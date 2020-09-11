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
 * @file        bc95_onenet_nb.c
 *
 * @brief       bc95 module link kit onenet nb api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "at_parser.h"
#include "bc95_onenet_nb.h"
#include "mo_onenet_nb.h"

#include <stdio.h>
#include <string.h>

#define DBG_EXT_TAG "bc95.onenet_nb"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>
#include <stdlib.h>

#define ONENETNB_TIMEOUT_DEFAULT (0)

typedef enum mo_config_mode
{
    ONENETNB_GUIDEMODE_DISABLE_ADDR = 0,
    ONENETNB_GUIDEMODE_ENABLE_ADDR,
    ONENETNB_RSP_TIMEOUT,
    ONENETNB_OBS_AUTOACK,
    ONENETNB_AUTH_CONFIG,
    ONENETNB_DTLS_CONFIG,
    ONENETNB_WRITE_FORMATE,
    ONENETNB_BUF_CONFIG,
} mo_config_mode_t;

os_err_t bc95_onenetnb_get_config(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    /* maximum return time out 300ms, suggest more than that. */
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;
    os_uint32_t  line;

    mo_config_mode_t config_mode_ref;
    mo_config_resp_t *config_resp  = (mo_config_resp_t *)resp;
    os_int8_t        tmp_buff[128] = {0};
    
    at_parser_set_resp(parser, 128, 0, timeout);

    if (at_parser_exec_cmd(parser, "AT+MIPLCONFIG?") != OS_EOK)
    {
        LOG_EXT_E("Get %s module onenetnb config failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    /* return mode with 7 lines */
    for(line = 1; 7 >= line; line++)
    {   
        result = at_parser_get_data_by_line(parser, 1, "+MIPLCONFIG:%d,%s", config_mode_ref, tmp_buff);
        if(result != OS_EOK)
        {
            LOG_EXT_E("Get %s module onenetnb config line:%d failed", self->name, line);
            result = OS_ERROR;
            goto __exit;
        }

        switch (config_mode_ref)
        {
        case ONENETNB_GUIDEMODE_DISABLE_ADDR:
            config_resp->guide_mode_enable = OS_FALSE;
            sscanf(tmp_buff, "%s,%hu", config_resp->ip, &config_resp->port);
            break;
        case ONENETNB_GUIDEMODE_ENABLE_ADDR:
            config_resp->guide_mode_enable = OS_TRUE;
            sscanf(tmp_buff, "%s,%hu", config_resp->ip, &config_resp->port);
            break;
        case ONENETNB_RSP_TIMEOUT:
            sscanf(tmp_buff, "%*d,%hhu", &config_resp->rsp_timeout);
            break;
        case ONENETNB_OBS_AUTOACK:
            sscanf(tmp_buff, "%d", &config_resp->obs_autoack_enable);
            break;
        case ONENETNB_AUTH_CONFIG:
            sscanf(tmp_buff, "%d,%s", &config_resp->auth_enable, config_resp->auth_code);
            break;
        case ONENETNB_DTLS_CONFIG:
            sscanf(tmp_buff, "%d,%s", &config_resp->dtls_enable, config_resp->psk);
            break;
        case ONENETNB_WRITE_FORMATE:
            sscanf(tmp_buff, "%hhu", &config_resp->write_format);
            break;
        case ONENETNB_BUF_CONFIG:
            sscanf(tmp_buff, "%hhu,%hhu", &config_resp->buf_cfg, &config_resp->buf_urc_mode);
            break;
        default:
            LOG_EXT_E("Get %s module onenetnb mode:%d invalid", self->name, config_mode_ref);
            break;
        }
        
        memset(tmp_buff, 0, sizeof(tmp_buff));
    }

__exit:
    at_parser_reset_resp(parser);

    return result;
}

os_err_t bc95_onenetnb_set_config(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    /* maximum return time out 300ms, suggest more than it */
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;

    char tmp_format[128] = "AT+MIPLCONFIG=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    result = at_parser_exec_cmd_valist(parser, tmp_format, args);

    return result;
}

os_err_t bc95_onenetnb_create(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 128, 0, timeout);

    char tmp_format[128] = "AT+MIPLCREATE";

    if (at_parser_exec_cmd(parser, tmp_format) != OS_EOK)
    {
        return OS_ERROR;
    }

    os_uint8_t ref = 0;
    
    if (at_parser_get_data_by_kw(parser, "+MIPLCREATE:", "+MIPLCREATE:%d", &ref) > 0)
    {
        *(os_uint8_t *)resp = ref;
        return OS_EOK;
    }

    return OS_ERROR;
}

os_err_t bc95_onenetnb_addobj(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 128, 0, timeout);

    char tmp_format[64] = "AT+MIPLADDOBJ=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) == OS_EOK)
    {
        return OS_EOK;
    }

    return OS_ERROR;
}

os_err_t bc95_onenetnb_discoverrsp(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 128, 0, timeout);

    char tmp_format[128] = "AT+MIPLDISCOVERRSP=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) == OS_EOK)
    {
        return OS_EOK;
    }

    return OS_ERROR;
}

os_err_t bc95_onenetnb_open(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    /* Lifetime : 16-268435454 (s) */
    /* Timeout  : [30]-65535   (s) */

    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;

    at_parser_set_resp(parser, 128, 6, timeout);

    char tmp_format[28] = "AT+MIPLOPEN=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) != OS_EOK)
    {
        return OS_ERROR;
    }

    if (OS_NULL != at_parser_get_line_by_kw(parser, "+MIPLEVENT:0,6"))
    {  
        return OS_EOK; /* success */
    }

    return OS_ERROR;
}

os_err_t bc95_onenetnb_notify(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    /* USER DATA <= 1000 BYTES */
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_bool_t is_ack = OS_FALSE;
    const char *idx = format;
    
    os_uint8_t  i   = 1;
    /* TODO if user had empty str after the final 11 mark \, it will be wrong */
    for (i = 1; i < 11; ++i) /* check has ack_id */
    {
        idx = strstr(idx + 1, ",");
        if (OS_NULL == idx)
        {
            break;
        }
    }
    
    if (i < 11) /* not has ack_id */
    {
        is_ack = OS_TRUE;
    }
    
    os_uint32_t ack_input = atoi(idx);
    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 256, is_ack ? 0 : 4, timeout);

    char tmp_format[128] = "AT+MIPLNOTIFY=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) != OS_EOK)
    {
        return OS_ERROR;
    }

    os_uint32_t ackid_out = 0;
    if (at_parser_get_data_by_kw(parser, "+MIPLEVENT:0,26", "+MIPLEVENT:0,26,%d", &ackid_out) <= 0)
    {
        return OS_ERROR;
    }
    
    if (ack_input == ackid_out)
    {
        return OS_EOK;
    }

    return OS_ERROR;
}

os_err_t bc95_onenetnb_update(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 128, 4, timeout);

    char tmp_format[128] = "AT+MIPLUPDATE=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) != OS_EOK)
    {
        return OS_ERROR;
    }

    os_uint8_t ref = 0;
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

os_err_t bc95_onenetnb_get_write(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);
    
    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 256, 0, timeout);

    char tmp_format[128] = "AT+MIPLMGR=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) != OS_EOK)
    {
        return OS_ERROR;
    }
    
    module_mgr_resp_t *mgr = (module_mgr_resp_t *)resp;
    if (at_parser_get_data_by_kw(parser, "+MIPLWRITE:", "+MIPLWRITE:%d,%d,%d,%d,%d,%d,%d,%s", &mgr->ref, &mgr->mid, 
        &mgr->objid, &mgr->insid, &mgr->resid, &mgr->type, &mgr->len, mgr->value) > 0)
    {
        return OS_EOK;
    }

    return OS_ERROR;
}

os_err_t bc95_onenetnb_writersp(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);
    
    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 256, 0, timeout);

    char tmp_format[128] = "AT+MIPLWRITERSP=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    if (at_parser_exec_cmd_valist(parser, tmp_format, args) == OS_EOK)
    {
        return OS_EOK;
    }

    return OS_ERROR;
}

static os_err_t urc_discover_handler(struct at_parser *parser, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<objID> */
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    return OS_EOK;
}

static os_err_t urc_observe_handler(struct at_parser *parser, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<flag>,<objID>,<insID>,<resID> */
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    return OS_EOK;
}

static os_err_t urc_read_handler(struct at_parser *parser, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<objID>,<insID>,<resID> */
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    return OS_EOK;
}

static os_err_t urc_write_handler(struct at_parser *parser, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<objID>,<insID>,<resID>,<value_type>,<len>,<value>,<flag>,<index> */
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    return OS_EOK;
}

static os_err_t urc_execute_handler(struct at_parser *parser, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<objID>,<insID>,<resID>[,<len>,<arguments>] */
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    return OS_EOK;
}

static os_err_t urc_parameter_handler(struct at_parser *parser, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<objID>,<insID>,<resID>,<len>,<parameter> */
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    return OS_EOK;
}

// static os_err_t urc_event_handler(struct at_parser *parser, const char *data, os_size_t size)
// {
//     /* <ref>,<evtID>[,<extend>][,<ackID>][,<time_stamp>,<cache_command_flag>] */
//     OS_ASSERT(OS_NULL != parser);
//     OS_ASSERT(OS_NULL != data);
//     return OS_EOK;
// }

static at_urc_t nb_urc_table[] = {
    {.prefix = "+MIPLDISCOVER:",  .suffix = "\r\n", .func = urc_discover_handler },
    {.prefix = "+MIPLOBSERVE:",   .suffix = "\r\n", .func = urc_observe_handler  },
    {.prefix = "+MIPLREAD:",      .suffix = "\r\n", .func = urc_read_handler     },
    {.prefix = "+MIPLWRITE:",     .suffix = "\r\n", .func = urc_write_handler    },
    {.prefix = "+MIPLEXECUTE:",   .suffix = "\r\n", .func = urc_execute_handler  },
    {.prefix = "+MIPLPARAMETER:", .suffix = "\r\n", .func = urc_parameter_handler},
    // {.prefix = "+MIPLEVENT:",     .suffix = "\r\n", .func = urc_event_handler    },
};

#ifdef OS_USING_SHELL
os_err_t bc95_onenetnb_all(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);
    
    at_parser_t *parser = &self->parser;
    at_parser_set_resp(parser, 256, 0, timeout);

    if (at_parser_exec_cmd_valist(parser, format, args) == OS_EOK)
    {
        return OS_EOK;
    }

    return OS_ERROR;
}
#endif
