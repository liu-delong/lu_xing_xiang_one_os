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
 * @file        ml302.c
 *
 * @brief       ml302.c module api
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ml302.h"
#include <stdlib.h>

#define DBG_EXT_TAG "ml302"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_ML302

#define ML302_RETRY_TIMES (5)

#ifdef ML302_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = ml302_at_test,
    .get_imei             = ml302_get_imei,
    .get_imsi             = ml302_get_imsi,
    .get_iccid            = ml302_get_iccid,
    .get_cfun             = ml302_get_cfun,
    .set_cfun             = ml302_set_cfun,
    .get_firmware_version = ml302_get_firmware_version,
};
#endif /* ML302_USING_GENERAL_OPS */

#ifdef ML302_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach           = ml302_set_attach,
    .get_attach           = ml302_get_attach,
    .set_reg              = ml302_set_reg,
    .get_reg              = ml302_get_reg,
    .set_cgact            = ml302_set_cgact,
    .get_cgact            = ml302_get_cgact,
    .get_csq              = ml302_get_csq,
    .get_cell_info        = ml302_get_cell_info,
};
#endif /* ML302_USING_NETSERV_OPS */

#ifdef ML302_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = ml302_ping,
};
#endif /* ML302_USING_PING_OPS */

#ifdef ML302_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig             = ml302_ifconfig,
    .get_ipaddr           = ml302_get_ipaddr,
};
#endif /* ML302_USING_IFCONFIG_OPS */

#ifdef ML302_USING_NETCONN_OPS
extern void ml302_netconn_init(mo_ml302_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create               = ml302_netconn_create,
    .destroy              = ml302_netconn_destroy,
    .gethostbyname        = ml302_netconn_gethostbyname,
    .connect              = ml302_netconn_connect,
    .send                 = ml302_netconn_send,
    .get_info             = ml302_netconn_get_info,
};
#endif /* ML302_USING_NETCONN_OPS */

static os_err_t ml302_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, ML302_RETRY_TIMES);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[32] = {0};

    at_resp_t resp = {.buff = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "ATE0");
}

mo_object_t *module_ml302_create(const char *name, void *parser_config)
{
    mo_ml302_t *module = (mo_ml302_t *)malloc(sizeof(mo_ml302_t));
    if (OS_NULL == module)
    {
        LOG_EXT_E("Create %s module instance failed, no enough memory.", name);
        return OS_NULL;
    }
    
    os_task_mdelay(5000);
    /* make sure ml302 power on and be ready */
    os_err_t result = mo_object_init(&(module->parent), name, parser_config);
    if (result != OS_EOK)
    {
        free(module);

        return OS_NULL;
    }

    result = ml302_at_init(&(module->parent));
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef ML302_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif

#ifdef ML302_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* ML302_USING_NETSERV_OPS */

#ifdef ML302_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* ML302_USING_PING_OPS */

#ifdef ML302_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* ML302_USING_IFCONFIG_OPS */

#ifdef ML302_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;

    ml302_netconn_init(module);

    os_event_init(&module->netconn_evt, name, OS_IPC_FLAG_FIFO);

    os_mutex_init(&module->netconn_lock, name, OS_IPC_FLAG_FIFO, OS_TRUE);

    module->curr_connect = -1;

#endif /* ML302_USING_NETCONN_OPS */

__exit:
    if (result != OS_EOK)
    {

        if (mo_object_get_by_name(name) != OS_NULL)
        {
            mo_object_deinit(&module->parent);
        }

        free(module);

        return OS_NULL;
    }

    return &(module->parent);
}

os_err_t module_ml302_destroy(mo_object_t *self)
{
    mo_ml302_t *module = os_container_of(self, mo_ml302_t, parent);

    mo_object_deinit(self);

#ifdef ML302_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);

    os_mutex_deinit(&module->netconn_lock);
#endif /* ML302_USING_NETCONN_OPS */

    free(module);

    return OS_EOK;
}

#ifdef ML302_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int ml302_auto_create(void)
{
    os_device_t *device = os_device_find(ML302_DEVICE_NAME);
    if (OS_NULL == device)
    {
        LOG_EXT_E("Auto create failed, Can not find M5311 interface device %s!", ML302_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = ML302_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = ML302_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = ML302_RECV_BUFF_LEN};

    mo_object_t *module = module_ml302_create(ML302_NAME, &parser_config);
    if (OS_NULL == module)
    {
        LOG_EXT_E("Auto create failed, Can not create %s module object!", ML302_NAME);
        return OS_ERROR;
    }

    LOG_EXT_I("Auto create %s module object success!", ML302_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(ml302_auto_create);

#endif /* ML302_AUTO_CREATE */
#endif /* MOLINK_USING_ML302 */
