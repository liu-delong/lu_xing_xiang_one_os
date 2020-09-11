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
 * \@file        cis_sock.h
 *
 * \@brief       socket header file for cis
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#ifndef _NETWORK_IO_HEADER_
#define _NETWORK_IO_HEADER_

/*#include <stdint.h> */
#include <cis_def.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>

/*#   define ENETUNREACH WSAENETUNREACH*/
#define net_errno (errno)

int  net_Init(void);
void net_Close(int fd);
int  net_Socket(int family, int socktype, int proto);

#endif /* _BASE_NETWORK_IO_HEADER_ */
