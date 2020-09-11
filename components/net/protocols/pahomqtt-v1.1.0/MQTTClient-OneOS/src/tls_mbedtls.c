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
 * \@file        tls_mbedtls.c
 *
 * \@brief       socket port file for tls
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#ifdef MQTT_USING_TLS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>
#include <os_clock.h>
#include "os_errno.h"
#include "tls_mbedtls.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include <mbedtls/config.h>
#else
#include MBEDTLS_CONFIG_FILE
#endif

#define DBG_EXT_TAG "net_tls"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>


static void _ssl_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void)level);

    LOG_EXT_D("%s:%04d: %s", file, line, str);
}

static int mbedtls_client_cacert_context(MbedTLSSession *session, const char *ca_crt, size_t ca_crt_len)
{
    int ret = 0;

    ret = mbedtls_x509_crt_parse(&session->cacert, (const unsigned char *)ca_crt, ca_crt_len);
    if (ret < 0)
    {
        LOG_EXT_E("mbedtls_x509_crt_parse error,  return -0x%x", -ret);
        return ret;
    }

    LOG_EXT_I("Loading the CA root certificate success...");

    /* Hostname set here should match CN in server certificate */
    if (session->host)
    {
        ret = mbedtls_ssl_set_hostname(&session->ssl, session->host);
        if (ret != 0)
        {
            LOG_EXT_E("mbedtls_ssl_set_hostname error, return -0x%x", -ret);
            return ret;
        }
    }

    ret = mbedtls_ssl_config_defaults(&session->conf,
                                      MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0)
    {
        LOG_EXT_E("mbedtls_ssl_config_defaults error, return -0x%x", -ret);
        return ret;
    }

    mbedtls_ssl_conf_authmode(&session->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&session->conf, &session->cacert, NULL);
    mbedtls_ssl_conf_rng(&session->conf, mbedtls_ctr_drbg_random, &session->ctr_drbg);

    mbedtls_ssl_conf_dbg(&session->conf, _ssl_debug, NULL);

    ret = mbedtls_ssl_setup(&session->ssl, &session->conf);
    if (ret != 0)
    {
        LOG_EXT_E("mbedtls_ssl_setup error, return -0x%x", -ret);
        return ret;
    }
    LOG_EXT_I("mbedtls client context init success...");

    return OS_EOK;
}

static int mbedtls_client_connect_using_recv_timeout(MbedTLSSession *session)
{
    int     ret = 0;
    char    verify_buf[128] = {0};

    ret = mbedtls_net_connect(&session->server_fd, session->host,
                              session->port, MBEDTLS_NET_PROTO_TCP);
    if (ret != 0)
    {
        LOG_EXT_E("mbedtls_net_connect error, return -0x%x", -ret);
        return ret;
    }

    LOG_EXT_I("Connected %s:%s success...", session->host, session->port);

    mbedtls_ssl_set_bio(&session->ssl, &session->server_fd, mbedtls_net_send, NULL, mbedtls_net_recv_timeout);

    while ((ret = mbedtls_ssl_handshake(&session->ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            LOG_EXT_E("mbedtls_ssl_handshake error, return -0x%x", -ret);
            return ret;
        }
    }

    ret = mbedtls_ssl_get_verify_result(&session->ssl);
    if (ret != 0)
    {
        /* Certificate verify sometimes may not necessary, so fail can also use TLS */
        LOG_EXT_E("verify peer certificate fail....");
        memset(verify_buf, 0x00, sizeof(verify_buf));
        mbedtls_x509_crt_verify_info(verify_buf, sizeof(verify_buf), "  ! ", ret);
        LOG_EXT_E("verification info: %s", verify_buf);
    }
    else
    {
        LOG_EXT_I("Certificate verified success...");
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief       This function read data from asigned session
 *
 * @param[in]   session     the pointer of  MbedTLSSession
 *
 * @return      Return status
 * @retval      OS_EOK      success.
 * @retval      others      failed.
 ***********************************************************************************************************************
 */
int mbedtls_client_read_with_timeout(MbedTLSSession *session, unsigned char *buf, size_t len)
{
    int ret = 0;

    if (session == OS_NULL || buf == OS_NULL)
    {
        return OS_ERROR;
    }

    ret = mbedtls_ssl_read(&session->ssl, (unsigned char *)buf, len);
    if (ret < 0 
        && ret != MBEDTLS_ERR_SSL_WANT_READ 
        && ret != MBEDTLS_ERR_SSL_WANT_WRITE 
        && ret != MBEDTLS_ERR_SSL_TIMEOUT)
    {
        LOG_EXT_E("mbedtls_client_read data error, return -0x%x", -ret);
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief       This function establish tls session
 *
 * @param[in]   tls_session_ptr     the pointer of  tls_session
 * @param[in]   addr                tls server address
 * @param[in]   port                server port
 * @param[in]   ca_crt              certificate buf
 * @param[in]   ca_crt_len          certificate buf lenth
 * @param[in]   client_crt          client certificate, always NULL
 * @param[in]   client_crt_len      client certificate lenth
 * @param[in]   client_key          client key, always NULL
 * @param[in]   client_key_len      client key lenth
 * @param[in]   client_pwd          client password, always NULL
 * @param[in]   client_pwd_len      client password lenth
 *
 * @return      Return status
 * @retval      OS_EOK      success.
 * @retval      others      failed.
 ***********************************************************************************************************************
 */
int mbedtls_tls_establish(uintptr_t *tls_session_ptr, const char *addr, const char *port,
                          const char *ca_crt, size_t ca_crt_len,
                          const char *client_crt, size_t client_crt_len,
                          const char *client_key, size_t client_key_len,
                          const char *client_pwd, size_t client_pwd_len)
{
    int             ret = -1;
    char           *pers = "hello_world";
    MbedTLSSession *tls_session = OS_NULL;

    if (OS_NULL == tls_session_ptr)
        return -1;

#ifdef OS_USING_SHELL
    LOG_EXT_I("Memory usage before the handshake connection is established:");
    extern int sh_exec(char *cmd, os_size_t length);
    sh_exec("list_mem", strlen("list_mem"));
#endif
    tls_session = (MbedTLSSession *)tls_malloc(sizeof(MbedTLSSession));
    if (tls_session == OS_NULL)
    {
        LOG_EXT_E("No memory for MbedTLS session object.");
        return -1;
    }
    memset(tls_session, 0x0, sizeof(MbedTLSSession));

    tls_session->host = tls_strdup(addr);
    tls_session->port = tls_strdup(port);
    tls_session->buffer_len = 0;
    tls_session->buffer = OS_NULL;

    LOG_EXT_I("Start handshake tick:%d", os_tick_get());

    if ((ret = mbedtls_client_init(tls_session, (void *)pers, strlen(pers))) != 0)
    {
        LOG_EXT_E("MbedTLSClientInit err return : -0x%x", -ret);
        goto __exit;
    }

    if ((ret = mbedtls_client_cacert_context(tls_session, ca_crt, ca_crt_len)) < 0)
    {
        LOG_EXT_E("MbedTLSCLlientContext err return : -0x%x", -ret);
        goto __exit;
    }

    if ((ret = mbedtls_client_connect_using_recv_timeout(tls_session)) != 0)
    {
        LOG_EXT_E("MbedTLSCLlientConnect err return : -0x%x", -ret);
        goto __exit;
    }

    LOG_EXT_I("Finish handshake tick:%d", os_tick_get());

    LOG_EXT_I("MbedTLS connect success...");

#ifdef OS_USING_SHELL
    LOG_EXT_I("Memory usage after the handshake connection is established:");
    sh_exec("list_mem", strlen("list_mem"));
#endif

    *tls_session_ptr = (uintptr_t)tls_session;

    return 0;

__exit:
    mbedtls_client_close(&tls_session);

    LOG_EXT_I("MbedTLS connection close success.");

    return ret;
}
#endif
