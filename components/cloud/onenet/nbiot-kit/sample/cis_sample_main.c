/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * \@file        cis_sample_main.c
 *
 * \@brief       cis sample shell 
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <os_task.h>
#include <cis_def.h>
#include <cis_api.h>
#include <cis_config.h>
#include <oneos_config.h>

extern int cis_sample_entry(cis_user_cfg_t *cis_user_cfg);
extern int sample_delete_object(void);
extern int sample_add_object(void);
extern int cis_sample_exit(void);

#define SERVER_IP   "183.230.40.39"
#define SERVER_PORT "5683"

cis_user_cfg_t g_cis_user_cfg;

void cis_sample_thread(void *lpParam)
{
    g_cis_user_cfg.bs_enabled   = true;
    g_cis_user_cfg.dtls_enabled = false;
    g_cis_user_cfg.ip           = (uint8_t *)SERVER_IP;
    g_cis_user_cfg.port         = (uint8_t *)SERVER_PORT;
    g_cis_user_cfg.auth_code    = (uint8_t *)CIS_AUTH_CODE;
    g_cis_user_cfg.psk          = (uint8_t *)CIS_PSK_CODE;

    cis_sample_entry(&g_cis_user_cfg);
}

void cis_sample_init(void)
{
    cis_sample_thread(NULL);
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(cis_sample_start, cis_sample_init, "start onenet-nbiot cis sample.");
SH_CMD_EXPORT(sample_delete_object, sample_delete_object, "delete cis sample object");
SH_CMD_EXPORT(sample_add_object, sample_add_object, "add cis sample object");
SH_CMD_EXPORT(cis_sample_end, cis_sample_exit, "end onenet-nbiot cis sample.");
#endif
