/*
 * Copyright (c) 2012-2020, CMCC IOT
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <stdint.h>
#include <stdlib.h>

#include <os_task.h>
#include <webclient.h>

#ifdef OS_USING_VFS
#include <vfs_posix.h>

/**
 * send GET request and store response data into the file.
 *
 * @param URI input server address
 * @param filename store response date to filename
 *
 * @return <0: GET request failed
 *         =0: success
 */
int webclient_get_file(const char* URI, const char* filename)
{
    int fd = -1, rc = WEBCLIENT_OK;
    size_t offset;
    size_t length, total_length = 0;
    unsigned char *ptr = OS_NULL;
    struct webclient_session* session = OS_NULL;
    int resp_status = 0;

    session = webclient_session_create(WEBCLIENT_HEADER_BUFSZ);
    if(session == OS_NULL)
    {
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }

    if ((resp_status = webclient_get(session, URI)) != 200)
    {
        LOG_EXT_E("get file failed, wrong response: %d.", resp_status);
        rc = -WEBCLIENT_ERROR;
        goto __exit;
    }

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        LOG_EXT_E("get file failed, open file(%s) error.", filename);
        rc = -WEBCLIENT_ERROR;
        goto __exit;
    }

    ptr = (unsigned char *) web_malloc(WEBCLIENT_RESPONSE_BUFSZ);
    if (ptr == OS_NULL)
    {
        LOG_EXT_E("get file failed, no memory for response buffer.");
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }
	
    if (session->content_length < 0)
    {
        while (1)
        {
            length = webclient_read(session, ptr, WEBCLIENT_RESPONSE_BUFSZ);
            if (length > 0)
            {
                write(fd, ptr, length);
                total_length += length;
                LOG_EXT_RAW(">");
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        for (offset = 0; offset < (size_t) session->content_length;)
        {
            length = webclient_read(session, ptr,
                    session->content_length - offset > WEBCLIENT_RESPONSE_BUFSZ ?
                            WEBCLIENT_RESPONSE_BUFSZ : session->content_length - offset);

            if (length > 0)
            {
                write(fd, ptr, length);
                total_length += length;
                LOG_EXT_RAW(">");
            }
            else
            {
                break;
            }

            offset += length;
        }
    }

    if (total_length)
    {
        LOG_EXT_D("save %d bytes.", total_length);
    }

__exit:
    if (fd >= 0)
    {
        close(fd);
    }

    if (session != OS_NULL)
    {
        webclient_close(session);
    }

    if (ptr != OS_NULL)
    {
        web_free(ptr);
    }

    return rc;
}

/**
 * post file to http server.
 *
 * @param URI input server address
 * @param filename post data filename
 * @param form_data  form data
 *
 * @return <0: POST request failed
 *         =0: success
 */
int webclient_post_file(const char* URI, const char* filename,
        const char* form_data)
{
    size_t length;
    char boundary[60];
    int fd = -1, rc = WEBCLIENT_OK;
    char *header = OS_NULL, *header_ptr;
    unsigned char *buffer = OS_NULL, *buffer_ptr;
    struct webclient_session* session = OS_NULL;

    fd = open(filename, O_RDONLY, 0);
    if (fd < 0)
    {
        LOG_EXT_D("post file failed, open file(%s) error.", filename);
        rc = -WEBCLIENT_FILE_ERROR;
        goto __exit;
    }

    /* get the size of file */
    length = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    buffer = (unsigned char *) web_malloc(WEBCLIENT_RESPONSE_BUFSZ);
    if (buffer == OS_NULL)
    {
        LOG_EXT_D("post file failed, no memory for response buffer.");
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }

    header = (char *) web_malloc(WEBCLIENT_HEADER_BUFSZ);
    if (header == OS_NULL)
    {
        LOG_EXT_D("post file failed, no memory for header buffer.");
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }
    header_ptr = header;

    /* build boundary */
    os_snprintf(boundary, sizeof(boundary), "----------------------------%012d", os_tick_get());

    /* build encapsulated mime_multipart information*/
    buffer_ptr = buffer;
    /* first boundary */
    buffer_ptr += os_snprintf((char*) buffer_ptr,
            WEBCLIENT_RESPONSE_BUFSZ - (buffer_ptr - buffer), "--%s\r\n", boundary);
    buffer_ptr += os_snprintf((char*) buffer_ptr,
            WEBCLIENT_RESPONSE_BUFSZ - (buffer_ptr - buffer),
            "Content-Disposition: form-data; %s\r\n", form_data);
    buffer_ptr += os_snprintf((char*) buffer_ptr,
            WEBCLIENT_RESPONSE_BUFSZ - (buffer_ptr - buffer),
            "Content-Type: application/octet-stream\r\n\r\n");
    /* calculate content-length */
    length += buffer_ptr - buffer;
    length += strlen(boundary) + 6; /* add the last boundary */

    /* build header for upload */
    header_ptr += os_snprintf(header_ptr,
            WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
            "Content-Length: %d\r\n", length);
    header_ptr += os_snprintf(header_ptr,
            WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
            "Content-Type: multipart/form-data; boundary=%s\r\n", boundary);

    session = webclient_session_create(WEBCLIENT_HEADER_BUFSZ);
    if(session == OS_NULL)
    {
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }

    session->header->buffer = web_strdup(header);

    rc = webclient_post(session, URI, NULL);
    if( rc< 0)
    {
        goto __exit;
    }

    /* send mime_multipart */
    webclient_write(session, buffer, buffer_ptr - buffer);

    /* send file data */
    while (1)
    {
        length = read(fd, buffer, WEBCLIENT_RESPONSE_BUFSZ);
        if (length <= 0)
        {
            break;
        }

        webclient_write(session, buffer, length);
    }

    /* send last boundary */
    os_snprintf((char*) buffer, WEBCLIENT_RESPONSE_BUFSZ, "\r\n--%s--\r\n", boundary);
    webclient_write(session, buffer, strlen(boundary) + 6);

__exit:
    if (fd >= 0)
    {
        close(fd);
    }

    if (session != OS_NULL)
    {
        webclient_close(session);
    }

    if (buffer != OS_NULL)
    {
        web_free(buffer);
    }

    if (header != OS_NULL)
    {
        web_free(header);
    }

    return 0;
}


int wget(int argc, char** argv)
{
    if (argc != 3)
    {
        os_kprintf("Please using: wget <URI> <filename>");
        return -1;
    }

    webclient_get_file(argv[1], argv[2]);
    return 0;
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(wget, wget, "Get file by URI: wget <URI> <filename>.");
#endif /* OS_USING_SHELL */

#endif /* OS_USING_VFS */
