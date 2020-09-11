/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
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
 * @file        socket.h
 *
 * @brief       This file is a posix wrapper for at_sock.h.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <oneos_config.h>

#if defined(BSD_USING_MOLINK)
#include <mo_socket.h>
#elif defined(BSD_USING_LWIP)
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#else
#error "Please select Molink or Lwip"
#endif

#if defined(BSD_USING_MOLINK) || defined(BSD_USING_LWIP)
int socket(int domain, int type, int protocol);
int closesocket(int socket);
int shutdown(int socket, int how);
int bind(int socket, const struct sockaddr *name, socklen_t namelen);
int listen(int s, int backlog);
int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int connect(int socket, const struct sockaddr *name, socklen_t namelen);
int sendto(int socket, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
int send(int socket, const void *data, size_t size, int flags);
int recvfrom(int socket, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
int recv(int socket, void *mem, size_t len, int flags);
int getsockopt(int socket, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int socket, int level, int optname, const void *optval, socklen_t optlen);
int ioctlsocket(int s, long cmd, void *argp);

struct hostent *gethostbyname(const char *name);
int  getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
void freeaddrinfo(struct addrinfo *ai);
int getpeername (int s, struct sockaddr *name, socklen_t *namelen);
int getsockname (int s, struct sockaddr *name, socklen_t *namelen);
const char *inet_ntop(int af, const void *src, char *dst, int32_t size);
int inet_pton(int af, const char *src, void *dst);
#endif

#endif
