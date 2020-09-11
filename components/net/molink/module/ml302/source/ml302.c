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

#ifdef ML302_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test   = ml302_at_test,
    .get_imei  = ml302_get_imei,
    .get_imsi  = ml302_get_imsi,
    .get_iccid = ml302_get_iccid,
    .get_cfun  = ml302_get_cfun,
    .set_cfun  = ml302_set_cfun,
	  .set_echo  = ml302_set_echo,
};
#endif /* ML302_USING_GENERAL_OPS */

#ifdef ML302_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach     = ml302_set_attach,
    .get_attach     = ml302_get_attach,
    .set_reg        = ml302_set_reg,
    .get_reg        = ml302_get_reg,
    .set_cgact      = ml302_set_cgact,
    .get_cgact      = ml302_get_cgact,
    .get_csq        = ml302_get_csq,
    .get_ipaddr     = ml302_get_ipaddr,
    .set_netstat    = ml302_set_netstat,
    .get_netstat    = ml302_get_netstat,
    .ping           = ml302_ping,
	  .get_cell_info  = ml302_get_cell_info,

};
#endif /* ML302_USING_NETSERV_OPS */

#ifdef ML302_USING_NETCONN_OPS
extern void ml302_netconn_init(mo_ml302_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create        = ml302_netconn_create,
    .destroy       = ml302_netconn_destroy,
    .gethostbyname = ml302_netconn_gethostbyname,
    .connect       = ml302_netconn_connect,
    .send          = ml302_netconn_send,
};
#endif /* ML302_USING_NETCONN_OPS */

mo_object_t *module_ml302_create(const char *name, os_device_t *device, os_size_t recv_len)
{
    mo_ml302_t *module = (mo_ml302_t *)malloc(sizeof(mo_ml302_t));

    os_err_t result = mo_object_init(&(module->parent), name, device, recv_len);
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

int ml302_auto_create(void)
{
    os_device_t *device = os_device_find(ML302_DEVICE_NAME);
	if(OS_NULL == device)
	{
        LOG_EXT_E("Auto create failed, Can not find M5311 interface device %s!", ML302_DEVICE_NAME);
		return OS_ERROR;
	}

    mo_object_t *module = module_ml302_create(ML302_NAME, device,ML302_RECV_BUFF_LEN);
	if(OS_NULL == module)
	{
        LOG_EXT_E("Auto create failed, Can not create %s module object!", ML302_NAME);
		return OS_ERROR;
	}

	LOG_EXT_I("Auto create %s module object success!",ML302_NAME);
	return OS_EOK;
}
OS_CMPOENT_INIT(ml302_auto_create);

#endif /* ML302_AUTO_CREATE */
#endif /* MOLINK_USING_ML302 */
