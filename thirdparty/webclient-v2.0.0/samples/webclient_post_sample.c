/*
 * Copyright (c) 2012-2020, CMCC IOT
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
#include <string.h>
#include <stdlib.h>
#include <webclient.h>

#define POST_RESP_BUFSZ                1024
#define POST_HEADER_BUFSZ              1024

#define POST_LOCAL_URI                 "http://www.rt-thread.com/service/echo"

const char *post_data = "CMCC IOT is an open source IoT operating system from China!";

int webclient_post_test(int argc, char **argv)
{
    struct webclient_session* session = NULL;
    unsigned char *buffer = NULL;
    char *URI = NULL;
    int index, ret = 0;
    int bytes_read, resp_status;

    if (argc == 1)
    {
        URI = web_strdup(POST_LOCAL_URI);
        if(URI == NULL)
        {
            os_kprintf("no memory for create URI buffer.\n");
            return -1;
        }
    }
    else if (argc == 2)
    {
        URI = web_strdup(argv[1]);
        if(URI == NULL)
        {
            os_kprintf("no memory for create URI buffer.\n");
            return -1;
        }
    }
    else
    {
        os_kprintf("webclient_post_test [URI]  - webclient POST request test.\n");
        return -1;
    }

    buffer = (unsigned char *) web_malloc(POST_RESP_BUFSZ);
    if (buffer == NULL)
    {
        os_kprintf("no memory for receive response buffer.\n");
        ret = OS_ENOMEM;
        goto __exit;

    }

    /* create webclient session and set header response size */
    session = webclient_session_create(POST_HEADER_BUFSZ);
    if (session == NULL)
    {
        ret = OS_ENOMEM;
        goto __exit;
    }

    /* build header for upload */
    webclient_header_fields_add(session, "Content-Length: %d\r\n", strlen(post_data));
    webclient_header_fields_add(session, "Content-Type: application/octet-stream\r\n");

    /* send POST request by default header */
    if ((resp_status = webclient_post(session, URI, post_data)) != 200)
    {
        os_kprintf("webclient POST request failed, response(%d) error.\n", resp_status);
        ret = OS_ERROR;
        goto __exit;
    }

    os_kprintf("webclient POST request response data :\n");
    do
    {
        bytes_read = webclient_read(session, buffer, POST_RESP_BUFSZ);
        if (bytes_read <= 0)
        {
            break;
        }

        for (index = 0; index < bytes_read; index++)
        {
            os_kprintf("%c", buffer[index]);
        }
    } while (1);

    os_kprintf("\n");

__exit:
    if (session)
    {
        webclient_close(session);
    }

    if (buffer)
    {
        web_free(buffer);
    }

    if (URI)
    {
        web_free(URI);
    }

    return ret;
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(web_post_test, webclient_post_test, "webclient_post_test [URI]  - webclient POST request test.");
#endif /* OS_USING_SHELL */
