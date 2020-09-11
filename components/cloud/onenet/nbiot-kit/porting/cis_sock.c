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
 * \@file        cis_sock.c
 *
 * \@brief       socket port file for cis
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <cis_sock.h>
#include <stdlib.h>
#include <stdio.h>
#include <cis_log.h>
#include "os_libc.h"

int net_Init(void)
{
    return 1;
}

void net_Uninit(void)
{
}

void net_Close(int fd)
{
    closesocket(fd);
}

int net_SetNonBlock(int socket, int enable)
{
    return ioctlsocket(socket, FIONBIO, &enable) == 0;
}

int net_Socket(int family, int socktype, int proto)
{
    int fd;

    fd = socket(family, socktype, proto);
    if (fd == -1)
    {
        LOGE("cannot create socket, error no: %d.", net_errno);
        return -1;
    }

    net_SetNonBlock(fd, 1);
    return fd;
}
