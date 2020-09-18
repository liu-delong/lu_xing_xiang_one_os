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
 * @file        esp8266.c
 *
 * @brief       esp8266 module link kit factory api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp8266.h"

#include <stdlib.h>
#include <string.h>

#define DBG_EXT_TAG "esp8266"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_ESP8266

#ifdef ESP8266_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .soft_reset = esp8266_soft_reset,
    .at_test    = esp8266_at_test,
};
#endif /* ESP8266_USING_GENERAL_OPS */

#ifdef ESP8266_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .get_ipaddr     = esp8266_get_ipaddr,
    .ping           = esp8266_ping,
};
#endif /* ESP8266_USING_NETSERV_OPS */

#ifdef ESP8266_USING_NETCONN_OPS
extern os_err_t esp8266_netconn_init(mo_esp8266_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create        = esp8266_netconn_create,
    .destroy       = esp8266_netconn_destroy,
    .gethostbyname = esp8266_netconn_gethostbyname,
    .connect       = esp8266_netconn_connect,
    .send          = esp8266_netconn_send,
    .get_info      = esp8266_netconn_get_info,
};
#endif /* ESP8266_USING_NETCONN_OPS */

#ifdef ESP8266_USING_WIFI_OPS
extern os_err_t esp8266_wifi_init(mo_object_t *module);

static const struct mo_wifi_ops gs_wifi_ops = {
    .set_mode       = esp8266_wifi_set_mode,
    .get_mode       = esp8266_wifi_get_mode,
    .get_stat       = esp8266_wifi_get_stat,
    .connect_ap     = esp8266_wifi_connect_ap,
    .scan_info      = esp8266_wifi_scan_info,
    .scan_info_free = esp8266_wifi_scan_info_free,
};
#endif /* ESP8266_USING_WIFI_OPS */

static void urc_ready_func(struct at_parser *parser, const char *data, os_size_t size)
{
    LOG_EXT_D("AT firmware started successfully");
}

static void urc_busy_p_func(struct at_parser *parser, const char *data, os_size_t size)
{
    LOG_EXT_D("system is processing a commands and it cannot respond to the current commands.");
}

static void urc_busy_s_func(struct at_parser *parser, const char *data, os_size_t size)
{
    LOG_EXT_D("system is sending data and it cannot respond to the current commands.");
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "ready",  .suffix = "\r\n", .func = urc_ready_func},
    {.prefix = "busy p", .suffix = "\r\n", .func = urc_busy_p_func},
    {.prefix = "busy s", .suffix = "\r\n", .func = urc_busy_s_func},
};

mo_object_t *module_esp8266_create(const char *name, os_device_t *device, os_size_t recv_len)
{
    mo_esp8266_t *module = (mo_esp8266_t *)malloc(sizeof(mo_esp8266_t));

    os_err_t result = mo_object_init(&(module->parent), name, device, recv_len);
	if (result != OS_EOK)
    {
        goto __exit;
    }

    at_parser_set_urc_table(&module->parent.parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

#ifdef ESP8266_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;

    result = esp8266_soft_reset(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = esp8266_set_echo(&module->parent, OS_FALSE);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#endif /* ESP8266_USING_GENERAL_OPS */

#ifdef ESP8266_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* ESP8266_USING_NETSERV_OPS */

#ifdef ESP8266_USING_WIFI_OPS
    result = esp8266_wifi_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = esp8266_wifi_set_mode(&module->parent, MO_WIFI_MODE_STA);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    module->parent.ops_table[MODULE_OPS_WIFI] = &gs_wifi_ops;

    module->wifi_stat = MO_WIFI_STAT_INIT;
#endif /* ESP8266_USING_WIFI_OPS */

#ifdef ESP8266_USING_NETCONN_OPS
    result = esp8266_netconn_init(module);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;

    os_event_init(&module->netconn_evt, name, OS_IPC_FLAG_FIFO);

    os_mutex_init(&module->netconn_lock, name, OS_IPC_FLAG_FIFO, OS_TRUE);

    module->curr_connect = -1;
#endif /* ESP8266_USING_NETCONN_OPS */

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

os_err_t module_esp8266_destroy(mo_object_t *self)
{
    mo_esp8266_t *module = os_container_of(self, mo_esp8266_t, parent);

    mo_object_deinit(self);

#ifdef ESP8266_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);

    os_mutex_deinit(&module->netconn_lock);
#endif /* ESP8266_USING_NETCONN_OPS */

    free(module);

    return OS_EOK;
}

#ifdef ESP8266_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int esp8266_auto_create(void)
{
    os_device_t *device = os_device_find(ESP8266_DEVICE_NAME);

    if (OS_NULL == device)
    {
        LOG_EXT_E("Auto create failed, Can not find ESP8266 interface device %s!", ESP8266_DEVICE_NAME);
        return OS_ERROR;
    }
	
	uart_config.baud_rate = ESP8266_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_object_t *module = module_esp8266_create(ESP8266_NAME, device, ESP8266_RECV_BUFF_LEN);

    if (OS_NULL == module)
    {
        LOG_EXT_E("Auto create failed, Can not create %s module object!", ESP8266_NAME);
        return OS_ERROR;
    }

#ifdef ESP8266_AUTO_CONNECT_AP
    os_err_t result = mo_wifi_connect_ap(module, ESP8266_CONNECT_SSID, ESP8266_CONNECT_PASSWORD);
#endif /* ESP8266_AUTO_CONNECT_AP */

    LOG_EXT_I("Auto create %s module object success!", ESP8266_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(esp8266_auto_create);

#endif /* ESP8266_AUTO_CREATE */

#endif /* MOLINK_USING_ESP8266 */
