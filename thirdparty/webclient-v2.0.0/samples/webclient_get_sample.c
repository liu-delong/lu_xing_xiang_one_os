/*
 * Copyright (c) 2012-2020, CMCC IOT
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
#include <string.h>
#include <stdlib.h>
#include <webclient.h>

#define GET_HEADER_BUFSZ               1024
#define GET_RESP_BUFSZ                 1024

#define GET_LOCAL_URI                  "http://www.rt-thread.com/service/rt-thread.txt"

int webclient_get_test(int argc, char **argv)
{
    struct webclient_session* session = NULL;
    unsigned char *buffer = NULL;
    char *URI = NULL;
    int index, ret = 0;
    int bytes_read, resp_status;
    int content_length = -1;

    if (argc == 1)
    {
        URI = web_strdup(GET_LOCAL_URI);
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
        os_kprintf("webclient_get_test [URI]  - webclient GET request test.\n");
        return -1;
    }

    buffer = (unsigned char *) web_malloc(GET_HEADER_BUFSZ);
    if (buffer == NULL)
    {
        os_kprintf("no memory for receive buffer.\n");
        ret = OS_ENOMEM;
        goto __exit;

    }

    /* create webclient session and set header response size */
    session = webclient_session_create(GET_HEADER_BUFSZ);
    if (session == NULL)
    {
        ret = OS_ENOMEM;
        goto __exit;
    }

    /* send GET request by default header */
    if ((resp_status = webclient_get(session, URI)) != 200)
    {
        os_kprintf("webclient GET request failed, response(%d) error.\n", resp_status);
        ret = OS_ERROR;
        goto __exit;
    }

    os_kprintf("webclient GET request response data :\n");

    content_length = webclient_content_length_get(session);
    if (content_length < 0)
    {
        os_kprintf("webclient GET request type is chunked.\n");
        do
        {
            bytes_read = webclient_read(session, buffer, GET_RESP_BUFSZ);
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
    }
    else
    {
        int content_pos = 0;

        do
        {
            bytes_read = webclient_read(session, buffer, 
                    content_length - content_pos > GET_RESP_BUFSZ ?
                            GET_RESP_BUFSZ : content_length - content_pos);
            if (bytes_read <= 0)
            {
                break;
            }

            for (index = 0; index < bytes_read; index++)
            {
                os_kprintf("%c", buffer[index]);
            }

            content_pos += bytes_read;
        } while (content_pos < content_length);

        os_kprintf("\n");
    }

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
SH_CMD_EXPORT(web_get_test, webclient_get_test, "web_get_test [URI]  webclient GET request test");
#endif /* OS_USING_SHELL */
