/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
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
 * @file        mbedtls_app_test.c
 *
 * @brief       This is a mbedtls client sample
 *
 * @revision
 * Date         Author          Notes
 * 2018-01-22   chenyong        first version
 * 2020-03-17   OneOS Team      format and change request resource 
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os_task.h"
#include "os_errno.h"
#include "os_clock.h"

#include "tls_certificate.h"
#include "tls_client.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#define DBG_EXT_TAG                  "mbedtls"
#define DBG_EXT_LVL                  DBG_EXT_DEBUG
#include "os_dbg_ext.h"


#define MBEDTLS_TEST_DEMO_STACK_SIZE (6*1024)
#define MBEDTLS_TEST_DEMO_PRIORITY   (OS_TASK_PRIORITY_MAX/3 - 1)
#define MBEDTLS_TEST_DEMO_TICK       (5)

#define MBEDTLS_WEB_SERVER           "www.gitee.com"
#define MBEDTLS_WEB_PORT             "443"

#define MBEDTLS_READ_BUFFER          (1024)

static const char *REQUEST = "GET /tangyao10/myTest/raw/master/test HTTP/1.1\r\n" \
                             "Host: gitee.com\r\n" \
                             "\r\n";

static void mbedtls_client_test_entry(void *parament)
{
    int  i                      = 0;
    int  ret                    = 0;
    char pers[]                 = "hello_OneOS";
    MbedTLSSession *tls_session = OS_NULL;

    LOG_EXT_I("mbedtls_client_test_entry enter");

#ifdef OS_USING_SHELL
    LOG_EXT_I("Memory usage before the handshake connection is established:");
    extern os_err_t sh_exec(char *cmd, os_size_t length);
    sh_exec("list_mem", strlen("list_mem"));
#endif

    tls_session = (MbedTLSSession *) tls_malloc(sizeof(MbedTLSSession));
    if (tls_session == OS_NULL)
    {
        LOG_EXT_E("No memory for MbedTLS session object");
        return;
    }
    memset(tls_session, 0x0, sizeof(MbedTLSSession));

    tls_session->host       = tls_strdup(MBEDTLS_WEB_SERVER);
    tls_session->port       = tls_strdup(MBEDTLS_WEB_PORT);
    tls_session->buffer_len = MBEDTLS_READ_BUFFER;
    tls_session->buffer     = tls_malloc(tls_session->buffer_len);
    if (tls_session->buffer == OS_NULL)
    {
        LOG_EXT_E("No memory for MbedTLS buffer");
        if (tls_session->host)
        {
            tls_free(tls_session->host);
        }

        if (tls_session->port)
        {
            tls_free(tls_session->port);
        }

        tls_free(tls_session);
        return;
    }

    LOG_EXT_I("Start handshake tick:%d", os_tick_get());

    if ((ret = mbedtls_client_init(tls_session, (void *) pers, strlen(pers))) != 0)
    {
        LOG_EXT_E("MbedTLSClientInit err return : -0x%x", -ret);
        goto __exit;
    }

    if ((ret = mbedtls_client_context(tls_session)) < 0)
    {
        LOG_EXT_E("MbedTLSCLlientContext err return : -0x%x", -ret);
        goto __exit;
    }

    if ((ret = mbedtls_client_connect(tls_session)) != 0)
    {
        LOG_EXT_E("MbedTLSCLlientConnect err return : -0x%x", -ret);
        goto __exit;
    }

    LOG_EXT_I("Finish handshake tick:%d", os_tick_get());
    LOG_EXT_I("MbedTLS connect success");

#ifdef OS_USING_SHELL
    LOG_EXT_I("Memory usage after the handshake connection is established:");
    sh_exec("list_mem", strlen("list_mem"));
#endif

    while ((ret = mbedtls_client_write(tls_session, (const unsigned char *) REQUEST, strlen(REQUEST))) <= 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            LOG_EXT_E("mbedtls_ssl_write returned -0x%x", -ret);
            goto __exit;
        }
    }
    LOG_EXT_I("Writing HTTP request success");
    LOG_EXT_I("Getting HTTP response");

    memset(tls_session->buffer, 0x00, MBEDTLS_READ_BUFFER);
    ret = mbedtls_client_read(tls_session, (unsigned char *) tls_session->buffer, MBEDTLS_READ_BUFFER);
    if ( ret == MBEDTLS_ERR_SSL_WANT_READ  ||
         ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
         ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
    {
        LOG_EXT_E("mbedtls_client_read failed, ret = -0x%x", -ret);
        goto __exit;
    }
    
    if (ret < 0)
    {
        LOG_EXT_E("Mbedtls_ssl_read returned -0x%x", -ret);
        goto __exit;
    }

    if (ret == 0)
    {
        LOG_EXT_I("TCP server connection closed");
        goto __exit;
    }

    for (i = 0; i < ret; i++)
    {
        LOG_EXT_RAW("%c", tls_session->buffer[i]);
    }
    LOG_EXT_RAW("\n");

__exit:
    mbedtls_client_close(&tls_session);
    LOG_EXT_I("MbedTLS connection close success");

    return;
}

int mbedtls_client_test_start(void)
{
	os_task_t *tid_task;

    tid_task = os_task_create("dlog_async",
                              mbedtls_client_test_entry,
							  OS_NULL,
							  MBEDTLS_TEST_DEMO_STACK_SIZE,
							  MBEDTLS_TEST_DEMO_PRIORITY,
							  MBEDTLS_TEST_DEMO_TICK);
	if( tid_task )
	{
		os_task_startup(tid_task);
	}

    return OS_EOK;
}

#ifdef OS_USING_SHELL

#include <shell.h>
SH_CMD_EXPORT(mbedtls_test, mbedtls_client_test_start, "mbedtls client test demo");

#endif
