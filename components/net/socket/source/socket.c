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
 * @file        socket.c
 *
 * @brief       Implement socket functions
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-21   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <sys/socket.h>

#define DBG_EXT_TAG "bsd.socket"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#if defined(BSD_USING_MOLINK)

#include "mo_common.h"

int socket(int domain, int type, int protocol)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

    return mo_socket(default_module, domain, type, protocol);
}

int closesocket(int socket)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

    return mo_closesocket(default_module, socket);
}

int shutdown(int socket, int how)
{
    LOG_EXT_E("OneOS module is not support shutdown");
    return -1;
}

int bind(int socket, const struct sockaddr *name, socklen_t namelen)
{
    LOG_EXT_E("OneOS module is not support bind");
    return -1;
}

int listen(int s, int backlog)
{
    LOG_EXT_E("OneOS module is not support listen");
    return -1;
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    LOG_EXT_E("OneOS module is not support accept");
    return -1;
}

int connect(int socket, const struct sockaddr *name, socklen_t namelen)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

    return mo_connect(default_module, socket, name, namelen);
}

int sendto(int socket, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

    return mo_sendto(default_module, socket, data, size, flags, to, tolen);
}

int send(int socket, const void *data, size_t size, int flags)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

    return mo_send(default_module, socket, data, size, flags);
}

int recvfrom(int socket, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

    return mo_recvfrom(default_module, socket, mem, len, flags, from, fromlen);
}

int recv(int socket, void *mem, size_t len, int flags)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

    return mo_recv(default_module, socket, mem, len, flags);
}

int getsockopt(int socket, int level, int optname, void *optval, socklen_t *optlen)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

    return mo_getsockopt(default_module, socket, level, optname, optval, optlen);
}

int setsockopt(int socket, int level, int optname, const void *optval, socklen_t optlen)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

    return mo_setsockopt(default_module, socket, level, optname, optval, optlen);
}

struct hostent *gethostbyname(const char *name)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return OS_NULL;
    }

    return mo_gethostbyname(default_module, name);
}

int getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
	return mo_getaddrinfo(nodename, servname, hints, res);
}

void freeaddrinfo(struct addrinfo *ai)
{
    mo_freeaddrinfo(ai);
}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
    LOG_EXT_E("OneOS module is not support getpeername");
    return -1;
}

int getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
    LOG_EXT_E("OneOS module is not support getsockname");
    return -1;
}

int ioctlsocket(int s, long cmd, void *argp)
{
    LOG_EXT_E("OneOS module is not support ioctlsocket");
    return -1;
}

#elif defined(BSD_USING_LWIP)

int socket(int domain, int type, int protocol)
{
    return lwip_socket(domain, type, protocol);
}

int closesocket(int socket)
{
    return lwip_close(socket);
}

int shutdown(int socket, int how)
{
    return lwip_shutdown(socket, how);
}

int bind(int socket, const struct sockaddr *name, socklen_t namelen)
{
    return lwip_bind(socket, name, namelen);
}

int listen(int s, int backlog)
{
    return lwip_listen(s, backlog);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    return lwip_accept(s, addr, addrlen);
}

int connect(int socket, const struct sockaddr *name, socklen_t namelen)
{
    return lwip_connect(socket, name, namelen);
}

int sendto(int socket, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
    return lwip_sendto(socket, data, size, flags, to, tolen);
}

int send(int socket, const void *data, size_t size, int flags)
{
    return lwip_send(socket, data, size, flags);
}

int recvfrom(int socket, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    return lwip_recvfrom(socket, mem, len, flags, from, fromlen);
}

int recv(int socket, void *mem, size_t len, int flags)
{
    return lwip_recv(socket, mem, len, flags);
}

int getsockopt(int socket, int level, int optname, void *optval, socklen_t *optlen)
{
    return lwip_getsockopt(socket, level, optname, optval, optlen);
}

int setsockopt(int socket, int level, int optname, const void *optval, socklen_t optlen)
{
    return lwip_setsockopt(socket, level, optname, optval, optlen);
}

struct hostent *gethostbyname(const char *name)
{
    return lwip_gethostbyname(name);
}

int getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
    return lwip_getaddrinfo(nodename, servname, hints, res);
}

void freeaddrinfo(struct addrinfo *ai)
{
    lwip_freeaddrinfo(ai);
}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
    return lwip_getpeername(s, name, namelen);
}

int getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
    return lwip_getsockname(s, name, namelen);
}

int ioctlsocket(int s, long cmd, void *argp)
{
    return lwip_ioctl(s, cmd, argp);
}

const char *inet_ntop(int af, const void *src, char *dst, int32_t size)
{
    return lwip_inet_ntop(af, src, dst, size);
}

int inet_pton(int af, const char *src, void *dst)
{
    return lwip_inet_pton(af, src, dst);
}

#endif
