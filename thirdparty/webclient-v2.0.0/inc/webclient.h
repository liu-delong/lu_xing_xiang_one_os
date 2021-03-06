/*
 * Copyright (c) 2012-2020, CMCC IOT
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __WEBCLIENT_H__
#define __WEBCLIENT_H__

#include <string.h>
#include <stdlib.h>
#include <os_kernel.h>
#include <oneos_config.h>
#include "libc_errno.h"
#ifdef OS_USING_VFS
#include "vfs.h"
#endif

#ifdef WEBCLIENT_USING_TLS
#include <tls_client.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif   

#undef DBG_SECTION_NAME
#undef DBG_LEVEL
#undef DBG_COLOR
#undef DBG_ENABLE

#define DBG_ENABLE
#define DBG_SECTION_NAME               "WEB"
#ifdef WEBCLIENT_DEBUG
#define DBG_LEVEL                      DBG_LOG
#else
#define DBG_EXT_LEVEL                      DBG_INFO
#endif /* WEBCLIENT_DEBUG */
#define DBG_EXT_COLOR
#include <os_dbg_ext.h>

#ifndef web_malloc
#define web_malloc                     malloc
#endif

#ifndef web_calloc
#define web_calloc                     calloc
#endif

#ifndef web_realloc
#define web_realloc                    realloc
#endif

#ifndef web_free
#define web_free                       free
#endif

#ifndef web_strdup
#define web_strdup                     os_strdup
#endif

#define WEBCLIENT_SW_VERSION           "2.0.0"
#define WEBCLIENT_SW_VERSION_NUM       0x20000

#define WEBCLIENT_HEADER_BUFSZ         4096
#define WEBCLIENT_RESPONSE_BUFSZ       4096

enum WEBCLIENT_STATUS
{
    WEBCLIENT_OK,
    WEBCLIENT_ERROR,
    WEBCLIENT_TIMEOUT,
    WEBCLIENT_NOMEM,
    WEBCLIENT_NOSOCKET,
    WEBCLIENT_NOBUFFER,
    WEBCLIENT_CONNECT_FAILED,
    WEBCLIENT_DISCONNECT,
    WEBCLIENT_FILE_ERROR,
};

enum WEBCLIENT_METHOD
{
    WEBCLIENT_USER_METHOD,
    WEBCLIENT_GET,
    WEBCLIENT_POST,
};

struct  webclient_header
{
    char *buffer;
    size_t length;                      /* content header buffer size */

    size_t size;                        /* maximum support header size */
};

struct webclient_session
{
    struct webclient_header *header;    /* webclient response header information */
    int socket;
    int resp_status;

    char *host;                         /* server host */
    char *req_url;                      /* HTTP request address*/

    int chunk_sz;
    int chunk_offset;

    int content_length;
    size_t content_remainder;           /* remainder of content length */

#ifdef WEBCLIENT_USING_TLS
    MbedTLSSession *tls_session;        /* mbedtls connect session */
#endif
};

/* create webclient session and set header response size */
struct webclient_session *webclient_session_create(size_t header_sz);

/* send HTTP GET request */
int webclient_get(struct webclient_session *session, const char *URI);
int webclient_get_position(struct webclient_session *session, const char *URI, int position);

/* send HTTP POST request */
int webclient_post(struct webclient_session *session, const char *URI, const char *post_data);

/* close and release wenclient session */
int webclient_close(struct webclient_session *session);

int webclient_set_timeout(struct webclient_session *session, int millisecond);

/* send or receive data from server */
int webclient_read(struct webclient_session *session, unsigned char *buffer, size_t size);
int webclient_write(struct webclient_session *session, const unsigned char *buffer, size_t size);

/* webclient GET/POST header buffer operate by the header fields */
int webclient_header_fields_add(struct webclient_session *session, const char *fmt, ...);
const char *webclient_header_fields_get(struct webclient_session *session, const char *fields);

/* send HTTP POST/GET request, and get response data */
int webclient_response(struct webclient_session *session, unsigned char **response);
int webclient_request(const char *URI, const char *header, const char *post_data, unsigned char **response);
int webclient_resp_status_get(struct webclient_session *session);
int webclient_content_length_get(struct webclient_session *session);

#ifdef OS_USING_VFS
/* file related operations */
int webclient_get_file(const char *URI, const char *filename);
int webclient_post_file(const char *URI, const char *filename, const char *form_data);
#endif

#ifdef  __cplusplus
    }
#endif

#endif
