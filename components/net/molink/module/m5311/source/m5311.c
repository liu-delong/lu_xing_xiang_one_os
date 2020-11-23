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
 * @file        m5311.c
 *
 * @brief       m5311 factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5311.h"

#include <stdlib.h>
#include <string.h>
#define DBG_EXT_TAG "m5311"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_M5311

#define M5311_RETRY_TIMES (5)

#ifdef M5311_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test   = m5311_at_test,
    .get_imei  = m5311_get_imei,
    .get_imsi  = m5311_get_imsi,
    .get_iccid = m5311_get_iccid,
    .get_cfun  = m5311_get_cfun,
    .set_cfun  = m5311_set_cfun,
};
#endif /* M5311_USING_GENERAL_OPS */

#ifdef M5311_USING_NETSERV_OPS
extern void m5311_netserv_init(mo_m5311_t *module);

static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach       = m5311_set_attach,
    .get_attach       = m5311_get_attach,
    .set_reg          = m5311_set_reg,
    .get_reg          = m5311_get_reg,
    .set_cgact        = m5311_set_cgact,
    .get_cgact        = m5311_get_cgact,
    .get_csq          = m5311_get_csq,
    .get_radio        = m5311_get_radio,
    .set_psm          = m5311_set_psm,
    .get_psm          = m5311_get_psm,
    .set_edrx_cfg     = m5311_set_edrx_cfg,
    .get_edrx_cfg     = m5311_get_edrx_cfg,
    .get_edrx_dynamic = m5311_get_edrx_dynamic,
};
#endif /* M5311_USING_NETSERV_OPS */

#ifdef M5311_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping           = m5311_ping,
};
#endif /* M5311_USING_PING_OPS */

#ifdef M5311_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig       = m5311_ifconfig,
    .get_ipaddr     = m5311_get_ipaddr,
};
#endif /* M5311_USING_IFCONFIG_OPS */

#ifdef M5311_USING_NETCONN_OPS
extern void m5311_netconn_init(mo_m5311_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create         = m5311_netconn_create,
    .destroy        = m5311_netconn_destroy,
    .gethostbyname  = m5311_netconn_gethostbyname,
    .connect        = m5311_netconn_connect,
    .send           = m5311_netconn_send,
    .get_info       = m5311_netconn_get_info,
};
#endif /* M5311_USING_NETCONN_OPS */

#ifdef M5311_USING_ONENET_NB_OPS
static const mo_onenet_ops_t gs_onenet_ops = {
    .onenetnb_create      = m5311_onenetnb_create,
    .onenetnb_createex    = m5311_onenetnb_createex,
    .onenetnb_addobj      = m5311_onenetnb_addobj,
    .onenetnb_discoverrsp = m5311_onenetnb_discoverrsp,
    .onenetnb_nmi         = m5311_onenetnb_set_nmi,
    .onenetnb_open        = m5311_onenetnb_open,
    .onenetnb_notify      = m5311_onenetnb_notify,
    .onenetnb_update      = m5311_onenetnb_update,
    .onenetnb_get_write   = m5311_onenetnb_get_write,
    .onenetnb_writersp    = m5311_onenetnb_writersp,
#ifdef OS_USING_SHELL
    .onenetnb_all         = m5311_onenetnb_all,
#endif
};
#endif /* M5311_USING_ONENET_OPS */

static os_err_t m5311_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, M5311_RETRY_TIMES);
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

mo_object_t *module_m5311_create(const char *name, void *parser_config)
{
    mo_m5311_t *module = (mo_m5311_t *)calloc(1, sizeof(mo_m5311_t));
    if (OS_NULL == module)
    {
        LOG_EXT_E("Create %s module instance failed, no enough memory.", name);
        return OS_NULL;
    }

    os_err_t result = mo_object_init(&(module->parent), name, parser_config);
    if (result != OS_EOK)
    {
        free(module);

        return OS_NULL;
    }

    result = m5311_at_init(&(module->parent));
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef M5311_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* M5311_USING_GENERAL_OPS */

#ifdef M5311_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
    m5311_netserv_init(module);
#endif /* M5311_USING_NETSERV_OPS */

#ifdef M5311_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* M5311_USING_PING_OPS */
    
#ifdef M5311_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* M5311_USING_IFCONFIG_OPS */

#ifdef M5311_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    m5311_netconn_init(module);
    os_mutex_init(&module->netconn_lock, name, OS_IPC_FLAG_FIFO, OS_TRUE);
#endif /* M5311_USING_NETCONN_OPS */

#ifdef M5311_USING_ONENET_NB_OPS
    module->parent.ops_table[MODULE_OPS_ONENET_NB] = &gs_onenet_ops;
#endif /* M5311A_USING_ONENET_NB_OPS */

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

os_err_t module_m5311_destroy(mo_object_t *self)
{
    mo_m5311_t *module = os_container_of(self, mo_m5311_t, parent);

#ifdef M5311_USING_NETCONN_OPS
    os_mutex_deinit(&module->netconn_lock);
#endif /* M5311_USING_NETCONN_OPS */

    mo_object_deinit(self);

    free(module);

    return OS_EOK;
}

#ifdef M5311_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int m5311_auto_create(void)
{
    os_device_t *device = os_device_find(M5311_DEVICE_NAME);

    if (OS_NULL == device)
    {
        LOG_EXT_E("Auto create failed, Can not find M5311 interface device %s!", M5311_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = M5311_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = M5311_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = M5311_RECV_BUFF_LEN};

    mo_object_t *module = module_m5311_create(M5311_NAME, &parser_config);

    if (OS_NULL == module)
    {
        LOG_EXT_E("Auto create failed, Can not create %s module object!", M5311_NAME);
        return OS_ERROR;
    }

    LOG_EXT_I("Auto create %s module object success!", M5311_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(m5311_auto_create);

#endif /* M5311_AUTO_CREATE */

#endif /* MOLINK_USING_M5311 */
