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
 * @brief       ec200x module link kit factory api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ec200x.h"

#include <stdlib.h>
#include <string.h>

#define DBG_EXT_TAG "ec200x"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_EC200X

#ifdef EC200X_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test   = ec200x_at_test,
    .get_imei  = ec200x_get_imei,
    .get_imsi  = ec200x_get_imsi,
    .get_iccid = ec200x_get_iccid,
    .get_cfun  = ec200x_get_cfun,
    .set_cfun  = ec200x_set_cfun,
};
#endif /* EC200X_USING_GENERAL_OPS */

#ifdef EC200X_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach     = ec200x_set_attach,
    .get_attach     = ec200x_get_attach,
    .set_reg        = ec200x_set_reg,
    .get_reg        = ec200x_get_reg,
    .set_cgact      = ec200x_set_cgact,
    .get_cgact      = ec200x_get_cgact,
    .get_csq        = ec200x_get_csq,
    .get_ipaddr     = ec200x_get_ipaddr,
    .ping           = ec200x_ping,
};
#endif /* EC200X_USING_NETSERV_OPS */
#ifdef EC200X_USING_NETCONN_OPS
extern void ec200x_netconn_init(mo_ec200x_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create        = ec200x_netconn_create,
    .destroy       = ec200x_netconn_destroy,
    .gethostbyname = ec200x_netconn_gethostbyname,
    .connect       = ec200x_netconn_connect,
    .send          = ec200x_netconn_send,
    .get_info      = ec200x_netconn_get_info,
};
#endif /* EC200X_USING_NETCONN_OPS */

static void urc_ready_func(struct at_parser *parser, const char *data, os_size_t size)
{
    LOG_EXT_D("ME initialization is successful");
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "RDY", .suffix = "\r\n", .func = urc_ready_func},
};

mo_object_t *module_ec200x_create(const char *name, os_device_t *device, os_size_t recv_len)
{
    mo_ec200x_t *module = (mo_ec200x_t *)malloc(sizeof(mo_ec200x_t));

    os_err_t result = mo_object_init(&(module->parent), name, device, recv_len);
	if (result != OS_EOK)
    {
        goto __exit;
    }

    at_parser_set_urc_table(&module->parent.parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

#ifdef EC200X_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;

    result = ec200x_set_echo(&module->parent, OS_FALSE);
    if (result != OS_EOK)
    {
        goto __exit;
    }
#endif /* EC200X_USING_GENERAL_OPS */

#ifdef EC200X_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* EC200X_USING_NETSERV_OPS */

#ifdef EC200X_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;

    ec200x_netconn_init(module);

    os_event_init(&module->netconn_evt, name, OS_IPC_FLAG_FIFO);

    os_mutex_init(&module->netconn_lock, name, OS_IPC_FLAG_FIFO, OS_TRUE);

    module->curr_connect = -1;
#endif /* EC200X_USING_NETCONN_OPS */

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

os_err_t module_ec200x_destroy(mo_object_t *self)
{
    mo_ec200x_t *module = os_container_of(self, mo_ec200x_t, parent);

    mo_object_deinit(self);

#ifdef EC200X_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);

    os_mutex_deinit(&module->netconn_lock);
#endif /* EC200X_USING_NETCONN_OPS */

    free(module);

    return OS_EOK;
}

#ifdef EC200X_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int ec200x_auto_create(void)
{
    os_device_t *device = os_device_find(EC200X_DEVICE_NAME);

    if (OS_NULL == device)
    {
        LOG_EXT_E("Auto create failed, Can not find EC200X interface device %s!", EC200X_DEVICE_NAME);
        return OS_ERROR;
    }
	
	uart_config.baud_rate = EC200X_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_object_t *module = module_ec200x_create(EC200X_NAME, device, EC200X_RECV_BUFF_LEN);

    if (OS_NULL == module)
    {
        LOG_EXT_E("Auto create failed, Can not create %s module object!", EC200X_NAME);
        return OS_ERROR;
    }

    LOG_EXT_I("Auto create %s module object success!", EC200X_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(ec200x_auto_create);

#endif /* EC200X_AUTO_CREATE */

#endif /* MOLINK_USING_EC200X */
