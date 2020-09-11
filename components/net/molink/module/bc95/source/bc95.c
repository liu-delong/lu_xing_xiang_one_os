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
 * @file        bc95.c
 *
 * @brief       bc95 factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc95.h"

#include <stdlib.h>
#include <string.h>

#define DBG_EXT_TAG "bc95"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_BC95

#ifdef BC95_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test   = bc95_at_test,
    .get_imei  = bc95_get_imei,
    .get_imsi  = bc95_get_imsi,
    .get_iccid = bc95_get_iccid,
    .get_cfun  = bc95_get_cfun,
    .set_cfun  = bc95_set_cfun,
};
#endif /* BC95_USING_GENERAL_OPS */

#ifdef BC95_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach     = bc95_set_attach,
    .get_attach     = bc95_get_attach,
    .set_reg        = bc95_set_reg,
    .get_reg        = bc95_get_reg,
    .set_cgact      = bc95_set_cgact,
    .get_cgact      = bc95_get_cgact,
    .get_csq        = bc95_get_csq,
    .get_radio      = bc95_get_radio,
    .get_ipaddr     = bc95_get_ipaddr,
    .set_netstat    = bc95_set_netstat,
    .get_netstat    = bc95_get_netstat,
    .set_dnsserver  = bc95_set_dnsserver,
    .get_dnsserver  = bc95_get_dnsserver,
    .ping           = bc95_ping,
};
#endif /* BC95_USING_NETSERV_OPS */

#ifdef BC95_USING_NETCONN_OPS
extern void bc95_netconn_init(mo_bc95_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create        = bc95_netconn_create,
    .destroy       = bc95_netconn_destroy,
    .gethostbyname = bc95_netconn_gethostbyname,
    .connect       = bc95_netconn_connect,
    .send          = bc95_netconn_send,
};
#endif /* BC95_USING_NETCONN_OPS */

#ifdef BC95_USING_ONENET_NB_OPS
static const mo_onenet_ops_t gs_onenet_ops = {
    .onenetnb_get_config  = bc95_onenetnb_get_config,
    .onenetnb_set_config  = bc95_onenetnb_set_config,
    .onenetnb_create      = bc95_onenetnb_create,
    .onenetnb_addobj      = bc95_onenetnb_addobj,
    .onenetnb_discoverrsp = bc95_onenetnb_discoverrsp,
    .onenetnb_nmi         = bc95_onenetnb_nmi,
    .onenetnb_open        = bc95_onenetnb_open,
    .onenetnb_notify      = bc95_onenetnb_notify,
    .onenetnb_update      = bc95_onenetnb_update,
    .onenetnb_get_write   = bc95_onenetnb_get_write,
    .onenetnb_writersp    = bc95_onenetnb_writersp,
#ifdef OS_USING_SHELL
    .onenetnb_all         = bc95_onenetnb_all,
#endif
};
#endif /* BC95_USING_ONENET_OPS */

mo_object_t *module_bc95_create(const char *name, os_device_t *device, os_size_t recv_len)
{
    mo_bc95_t *module = (mo_bc95_t *)malloc(sizeof(mo_bc95_t));

    os_err_t result = mo_object_init(&(module->parent), name, device, recv_len);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef BC95_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* BC95_USING_GENERAL_OPS */

#ifdef BC95_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* BC95_USING_NETSERV_OPS */

#ifdef BC95_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    bc95_netconn_init(module);
    os_mutex_init(&module->netconn_lock, name, OS_IPC_FLAG_FIFO, OS_TRUE);
#endif /* BC95_USING_NETCONN_OPS */

#ifdef BC95_USING_ONENET_NB_OPS
    module->parent.ops_table[MODULE_OPS_ONENET_NB] = &gs_onenet_ops;
#endif /* BC95_USING_ONENET_NB_OPS */
	
__exit:
    if (result != OS_EOK)
    {
        free(module);
        
        return OS_NULL;
    }

    return &(module->parent);
}

os_err_t module_bc95_destroy(mo_object_t *self)
{
    mo_bc95_t *module = os_container_of(self, mo_bc95_t, parent);
    
    os_mutex_deinit(&module->netconn_lock);
    
    mo_object_deinit(self);

    free(module);

    return OS_EOK;
}

#ifdef BC95_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int bc95_auto_create(void)
{
    os_device_t *device = os_device_find(BC95_DEVICE_NAME);

    if (OS_NULL == device)
    {
        LOG_EXT_E("Auto create failed, Can not find BC95 interface device %s!", BC95_DEVICE_NAME);
        return OS_ERROR;
    }
	
	uart_config.baud_rate = BC95_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_object_t *module = module_bc95_create(BC95_NAME, device, BC95_RECV_BUFF_LEN);

    if (OS_NULL == module)
    {
        LOG_EXT_E("Auto create failed, Can not create %s module object!", BC95_NAME);
        return OS_ERROR;
    }

    LOG_EXT_I("Auto create %s module object success!", BC95_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(bc95_auto_create);

#endif /* BC95_AUTO_CREATE */

#endif /* MOLINK_USING_BC95 */
