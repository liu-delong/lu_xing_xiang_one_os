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
 * @file        mo_onenet_nb.c
 *
 * @brief       module link kit onenet nb api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_onenet_nb.h"
#include "mo_common.h"

#define DBG_EXT_TAG "module.onenet_nb"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#define CALL_MODULE_FUNC(FUNC_NAME, OPT_NAME)                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        os_err_t error = OS_ERROR;                                                                                     \
        va_list  args  = {0};                                                                                          \
        va_start(args, format);                                                                                        \
                                                                                                                       \
        if (OS_NULL == self)                                                                                           \
        {                                                                                                              \
            self = mo_get_default();                                                                                   \
        }                                                                                                              \
        mo_onenet_ops_t *ops = module_get_onenet_ops(self);                                                            \
        if (OS_NULL == ops)                                                                                            \
        {                                                                                                              \
            return error;                                                                                              \
        }                                                                                                              \
        if (OS_NULL == ops->FUNC_NAME)                                                                                 \
        {                                                                                                              \
            LOG_EXT_E("Module %s does not support %s operate", self->name, OPT_NAME);                                  \
            return OS_ERROR;                                                                                           \
        }                                                                                                              \
        error = ops->FUNC_NAME(self, timeout, resp, format, args);                                                     \
        va_end(args);                                                                                                  \
                                                                                                                       \
        return error;                                                                                                  \
    } while (0)

OS_INLINE mo_onenet_ops_t *module_get_onenet_ops(mo_object_t *self)
{
    mo_onenet_ops_t *ops = (mo_onenet_ops_t *)self->ops_table[MODULE_OPS_ONENET_NB];

    if (OS_NULL == ops)
    {
        LOG_EXT_E("Module %s does not support onenet operates", self->name);
    }

    return ops;
}

os_err_t mo_onenetnb_get_config(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_get_config, "MIPLGETCONFIG");
}

os_err_t mo_onenetnb_set_config(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_set_config, "MIPLSETCONFIG");
}

os_err_t mo_onenetnb_create(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_create, "MIPLCREATE");
}

os_err_t mo_onenetnb_createex(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_createex, "MIPLCREATEEX");
}

os_err_t mo_onenetnb_addobj(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_addobj, "MIPLADDOBJ");
}

os_err_t mo_onenetnb_discoverrsp(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_discoverrsp, "MIPLDISCOVERRSP");
}

os_err_t mo_onenetnb_nmi(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_nmi, "MIPLNMI");
}

os_err_t mo_onenetnb_open(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_open, "MIPLOPEN");
}

os_err_t mo_onenetnb_notify(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_notify, "MIPLNOTIFY");
}

os_err_t mo_onenetnb_update(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_update, "MIPLUPDATE");
}

os_err_t mo_onenetnb_get_write(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_get_write, "MIPLMGR");
}

os_err_t mo_onenetnb_writersp(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_writersp, "MIPLWRITERSP");
}

#ifdef OS_USING_SHELL

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

os_err_t module_onenetnb_create_sh(int argc, char *argv[])
{
    os_uint8_t ref_num = 0;

    if (argc < 2)
    {
        printf("Usage:mo_onenetnb_create len config\n");
        return OS_EOK;
    }

    if (mo_onenetnb_create(OS_NULL, 5000, &ref_num, "%d,%s,0,%d,0", atoi(argv[1]), argv[2], atoi(argv[1])) == OS_EOK)
    {
        printf("nb device instance:%d\n", ref_num);
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_create, module_onenetnb_create_sh, "create nb device instance");

os_err_t module_onenetnb_createex_sh(int argc, char *argv[])
{
    os_uint8_t ref_num = 0;

    if (mo_onenetnb_createex(OS_NULL, 5000, &ref_num, "\"183.230.40.39\",1") == OS_EOK)
    {
        printf("nb device instance:%d\n", ref_num);
    }
    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_createex, module_onenetnb_createex_sh, "createex nb device instance");

os_err_t module_onenetnb_addobj_sh(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("Usage:mo_onenetnb_addobj ref_num\n");
        return OS_EOK;
    }

    if (mo_onenetnb_addobj(OS_NULL, 2000, OS_NULL, "%d,3200,1,\"1\",0,0", atoi(argv[1])) == OS_EOK)
    {
        printf("add obj obj:3200, inscount:1, bitmap:1, atts:0, acts:0\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_addobj, module_onenetnb_addobj_sh, "add nb obj");

os_err_t module_onenetnb_discoverrsp_sh(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage:mo_onenetnb_discoverrsp ref_num\n");
        return OS_EOK;
    }

    if (mo_onenetnb_discoverrsp(OS_NULL, 2000, OS_NULL, "%d,3200,1,14,\"5500;5501;5750\"", atoi(argv[1])) == OS_EOK)
    {
        printf("discoverrsp obj:3200, result:1, length:4, data:5500;5501;5750\n");
    }
    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_discoverrsp, module_onenetnb_discoverrsp_sh, "add nb resource");

os_err_t module_onenetnb_nmi_sh(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage:mo_onenetnb_nmi ref_num\n");
        return OS_EOK;
    }

    if (mo_onenetnb_nmi(OS_NULL, 2000, OS_NULL, "%d,1,1", atoi(argv[1])) == OS_EOK)
    {
        printf("set numi nnmi:1, nsmi:1\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_nmi, module_onenetnb_nmi_sh, "set nmi");

os_err_t module_onenetnb_open_sh(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage:mo_onenetnb_open ref_num lifetime\n");
        return OS_EOK;
    }

    if (mo_onenetnb_open(OS_NULL, 30000, OS_NULL, "%d,%d,30", atoi(argv[1]), atoi(argv[2])) == OS_EOK)
    {
        printf("open successed!\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_open, module_onenetnb_open_sh, "reg to onenet");

os_err_t module_onenetnb_notify_sh(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage:mo_onenetnb_notify ref_num len value\n");
        return OS_EOK;
    }

    static os_uint8_t uip = 0;
    if (mo_onenetnb_notify(OS_NULL,
                           15000,
                           OS_NULL,
                           "0,%d,3200,0,5750,1,%d,\"%s\",0,0,%d",
                           atoi(argv[1]),
                           atoi(argv[2]),
                           argv[3],
                           uip) == OS_EOK)
    {
        ++uip;
        printf("notify mid:0, obj:3200, insid:0, resid:5750, type:1, len: %d, value: %s, index:0, flag:0, ack_id:%d\n",
               atoi(argv[1]),
               argv[2],
               uip);
    }
    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_notify, module_onenetnb_notify_sh, "notify data");

os_err_t module_onenetnb_update_sh(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage:mo_onenetnb_update ref_num\n");
        return OS_EOK;
    }

    if (mo_onenetnb_update(OS_NULL, 15000, OS_NULL, "%d,3600,1", atoi(argv[1])) == OS_EOK)
    {
        printf("update lifetime success!\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_update, module_onenetnb_update_sh, "update lifetime");

os_err_t module_onenetnb_get_write_sh(int argc, char *argv[])
{
    module_mgr_resp_t mgr;
    mgr.value = malloc(50);
    if (mo_onenetnb_get_write(OS_NULL, 2000, &mgr, "0") == OS_EOK)
    {
        printf("result ref:%d, mid:%d, objid:%d, insid:%d, resid:%d, type:%d, len:%d, value:%s\n",
               mgr.ref,
               mgr.mid,
               mgr.objid,
               mgr.insid,
               mgr.resid,
               mgr.type,
               mgr.len,
               mgr.value);
    }
    free(mgr.value);

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_get_write, module_onenetnb_get_write_sh, "mgr");

os_err_t module_onenetnb_writersp_sh(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage:mo_onenetnb_writersp ref_num mid\n");
        return OS_EOK;
    }

    if (mo_onenetnb_writersp(OS_NULL, 8000, OS_NULL, "%d,%d,2", atoi(argv[1]), atoi(argv[2])) == OS_EOK)
    {
        printf("write resp ok!\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_writersp, module_onenetnb_writersp_sh, "write resp");

os_err_t mo_onenetnb_all(mo_object_t *self, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_all, format);
}

os_err_t module_onenetnb_all_sh(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage:mo_onenetnb_all at_cmd config\n");
        return OS_EOK;
    }

    if (mo_onenetnb_all(OS_NULL, 15000, OS_NULL, argv[1], argv[2]) == OS_EOK)
    {
        printf("write resp ok!\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_all, module_onenetnb_all_sh, "for all set");

#endif /* OS_USING_SHELL */
