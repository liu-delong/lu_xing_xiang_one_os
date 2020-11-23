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
 * @file        bc95_netserv.c
 *
 * @brief       bc95 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc95_netserv.h"
#include "bc95.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DBG_EXT_TAG "bc95.netserv"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef BC95_USING_NETSERV_OPS

#define BC95_EARFCN_MAX        (65535)
#define BC95_PCI_MAX           (0x1F7)

os_err_t bc95_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CGATT=%hhu", attach_stat);
}

os_err_t bc95_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+CGATT:", "+CGATT:%hhu", attach_stat) <= 0)
    {
        LOG_EXT_E("Get %s module attach state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc95_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%hhu", reg_n);
}

os_err_t bc95_get_reg(mo_object_t *self, eps_reg_info_t *info)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG:%hhu,%hhu", &info->reg_n, &info->reg_stat) <= 0)
    {
        LOG_EXT_E("Get %s module register state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc95_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CGACT=%hhu,%hhu", act_stat, cid);
}

os_err_t bc95_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGACT:", "+CGACT:%hhu,%hhu", cid, act_stat) <= 0)
    {
        LOG_EXT_E("Get %s module cgact state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc95_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    /* bc95 dosen't support ber, 99 will be set forever */
    if (at_resp_get_data_by_kw(&resp, "+CSQ:", "+CSQ:%hhu,%hhu", rssi, ber) <= 0)
    {
        LOG_EXT_E("Get %s module signal quality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc95_get_radio(mo_object_t *self, radio_info_t *radio_info)
{
    at_parser_t *parser = &self->parser;

    memset(radio_info, 0, sizeof(radio_info_t));

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+NUESTATS");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "Signal power:", "Signal power:%d", &radio_info->signal_power) <= 0)
    {
        LOG_EXT_E("Get %s module signal power failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "Cell ID:", "Cell ID:%s", &radio_info->cell_id) <= 0)
    {
        LOG_EXT_E("Get %s module cell id failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "ECL:", "ECL:%d", &radio_info->ecl) <= 0)
    {
        LOG_EXT_E("Get %s module ECL failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "SNR:", "SNR:%d", &radio_info->snr) <= 0)
    {
        LOG_EXT_E("Get %s module SNR failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "EARFCN:", "EARFCN:%d", &radio_info->earfcn) <= 0)
    {
        LOG_EXT_E("Get %s module EARFCN failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "RSRQ:", "RSRQ:%d", &radio_info->rsrq) <= 0)
    {
        LOG_EXT_E("Get %s module RSRQ failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

__exit:

    return result;
}

os_err_t bc95_set_psm(mo_object_t *self, mo_psm_info_t info)
{
    at_parser_t *parser = &self->parser;
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 1 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CPSMS=%d,%s,%s,%s,%s", 
                                              info.psm_mode, 
                                              info.periodic_rau,
                                              info.gprs_ready_timer,
                                              info.periodic_tau,
                                              info.active_time);
}

os_err_t bc95_get_psm(mo_object_t *self, mo_psm_info_t *info)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CPSMS?"))
    {
        return OS_ERROR;
    }

    memset(info, 0, sizeof(mo_psm_info_t));

    if ( 0 >= at_resp_get_data_by_kw(&resp, "+CPSMS", "+CPSMS: %d,,,%[^,],%s", &info->psm_mode, info->periodic_tau, info->active_time))
    {
        LOG_EXT_E("Get %s module psm info failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc95_set_edrx_cfg(mo_object_t *self, mo_edrx_cfg_t cfg)
{
    at_parser_t *parser = &self->parser;
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 1 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CEDRXS=%d,5,%s", 
                                              cfg.mode, 
                                              cfg.edrx.req_edrx_value);
}

os_err_t bc95_get_edrx_cfg(mo_object_t *self, mo_edrx_t *edrx_local)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 1 * OS_TICK_PER_SECOND};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CEDRXS?"))
    {
        LOG_EXT_E("%s fail: AT+CEDRXS? cmd exec fail.", __func__);
        return OS_ERROR;
    }

    memset(edrx_local, 0, sizeof(mo_edrx_t));

    if (0 >= at_resp_get_data_by_kw(&resp, "+CEDRXS", "+CEDRXS: %d,\"%[^\"]", 
                                                       &edrx_local->act_type, 
                                                        edrx_local->req_edrx_value))
    {
        LOG_EXT_E("Get %s module edrx local config failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc95_get_edrx_dynamic(mo_object_t *self, mo_edrx_t *edrx_dynamic)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 1 * OS_TICK_PER_SECOND};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CEDRXRDP"))
    {
        LOG_EXT_E("%s fail: AT+CEDRXRDP cmd exec fail.", __func__);
        return OS_ERROR;
    }

    memset(edrx_dynamic, 0, sizeof(mo_edrx_t));

    if (0 >= at_resp_get_data_by_kw(&resp, "+CEDRXRDP", "+CEDRXRDP: %d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", 
                                                         &edrx_dynamic->act_type,
                                                          edrx_dynamic->req_edrx_value,
                                                          edrx_dynamic->nw_edrx_value,
                                                          edrx_dynamic->paging_time_window))
    {
        LOG_EXT_E("Get %s module edrx dynamic config failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

#if 0
os_err_t bc95_set_band(mo_object_t *self, char band_list[], os_uint8_t num)
{
    /* invalid input check */
    if (0 == num)
    {
        LOG_EXT_E("%s input invalid band count num.", __func__);
        return OS_EINVAL;
    }

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_ERROR;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 1 * OS_TICK_PER_SECOND};

    char exec_cmd[512] = "AT+NBAND=";
    for (int idex=0; idex < num-1; idex++)
    {
        sprintf(exec_cmd, "%s%d,", exec_cmd, band_list[idex]);
    }
    sprintf(exec_cmd, "%s%d", exec_cmd, band_list[num - 1]);

    result = at_parser_exec_cmd(parser, &resp, exec_cmd);
    if (OS_EOK != result)
    {
        LOG_EXT_E("%s fail: %s cmd exec fail.", __func__, exec_cmd);
    }

    return result;
}

os_err_t bc95_set_earfcn(mo_object_t *self, mo_earfcn_t earfcn)
{
    /* bc28/95 AT+NEARFCN */
    if (BC95_EARFCN_MAX < earfcn.earfcn || BC95_PCI_MAX < earfcn.pci)
    {
        LOG_EXT_E("%s input invalid.", __func__);
        return OS_EINVAL;
    }

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_ERROR;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 1 * OS_TICK_PER_SECOND};

    /* TODO TEST THIS FORMAT */
    result = at_parser_exec_cmd(parser, &resp, "AT+NEARFCN=%u,%u,%u",
                                                earfcn.mode, 
                                                earfcn.earfcn,
                                                earfcn.pci);
    if (OS_EOK != result)
    {
        LOG_EXT_E("%s fail: AT+NEARFCN exec fail.", __func__);
    }

    return result;
}
#endif /* TODO Put on hold */

void bc95_edrx_urc_handler(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_edrx_t edrx_urc;
    memset(&edrx_urc, 0, sizeof(mo_edrx_t));
    sscanf(data, "+CEDRXP: %d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", 
                  (int *)&edrx_urc.act_type,
                   edrx_urc.req_edrx_value,
                   edrx_urc.nw_edrx_value,
                   edrx_urc.paging_time_window);
    LOG_EXT_I("Get %s module edrx urc act_type[%d]", parser->name, edrx_urc.act_type);
    LOG_EXT_I("Get %s module edrx urc req_edrx_value[%s]", parser->name, edrx_urc.req_edrx_value);
    LOG_EXT_I("Get %s module edrx urc nw_edrx_value[%s]", parser->name, edrx_urc.nw_edrx_value);
    LOG_EXT_I("Get %s module edrx urc paging_time_window[%s]", parser->name, edrx_urc.paging_time_window);

    return;
}

static at_urc_t bc95_netserv_urc_table[] = {
    {.prefix = "+CEDRXP:",  .suffix = "\r\n", .func = bc95_edrx_urc_handler},
};

void bc95_netserv_init(mo_bc95_t *module)
{
    /* Set netserv urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, bc95_netserv_urc_table, sizeof(bc95_netserv_urc_table) / sizeof(at_urc_t));
    return;
}

#endif /* BC95_USING_NETSERV_OPS */
