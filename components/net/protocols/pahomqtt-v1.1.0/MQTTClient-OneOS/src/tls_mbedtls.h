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
 * \@file        tls_mbedtls.h
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

#if !defined(TLS_MBEDTLS_H)
#define TLS_MBEDTLS_H

#include <tls_client.h>
#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"

int mbedtls_client_read_with_timeout(MbedTLSSession *session, unsigned char *buf, size_t len);
int mbedtls_tls_establish(uintptr_t *tls_session, const char *addr, const char *port,
                          const char *ca_crt, size_t ca_crt_len,
                          const char *client_crt, size_t client_crt_len,
                          const char *client_key, size_t client_key_len,
                          const char *client_pwd, size_t client_pwd_len);

#endif
