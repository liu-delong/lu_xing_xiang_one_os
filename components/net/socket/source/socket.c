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
#include <os_errno.h>

#define DBG_EXT_TAG "bsd.socket"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef OS_USING_POSIX

#include <vfs_file.h>

static int sockfs_adapter_ioctl(struct vfs_fd *file, int cmd, void *args);
static int sockfs_adapter_read(struct vfs_fd *file, void *buf, size_t count);
static int sockfs_adapter_write(struct vfs_fd *file, const void *buf, size_t count);
static int sockfs_adapter_close(struct vfs_fd *file);
static int sockfs_adapter_poll(struct vfs_fd *file, struct os_pollreq *req);

static const struct vfs_file_ops gs_sockfs_fops =
{
    OS_NULL,					// Not support open
    sockfs_adapter_close,
    sockfs_adapter_ioctl,
    sockfs_adapter_read,
    sockfs_adapter_write,
    OS_NULL,                    // Not support flush
    OS_NULL,                    // Not support lseek
    OS_NULL,					// Not support getdents
    sockfs_adapter_poll,
};
#endif

#if defined(BSD_USING_MOLINK)

#include "mo_common.h"
#include "libc_errno.h"

#ifdef OS_USING_POSIX
static int sockfs_adapter_ioctl(struct vfs_fd *file, int cmd, void *args)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_SOCKET);

    return OS_ERROR;
}

static int sockfs_adapter_read(struct vfs_fd *file, void *buf, size_t count)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_SOCKET);

    return OS_ERROR;
}

static int sockfs_adapter_write(struct vfs_fd *file, const void *buf, size_t count)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_SOCKET);
    
    return OS_ERROR;
}

static int sockfs_adapter_close(struct vfs_fd *file)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_SOCKET);
    
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return OS_ERROR;;
    }
    
    return mo_closesocket(default_module, (int)file->data);
}

static int sockfs_adapter_poll(struct vfs_fd *file, struct os_pollreq *req)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_SOCKET);
    OS_ASSERT(req != OS_NULL);
    
    return mo_poll((int)file->data, req);
}
#else
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
    return mo_select(maxfdp1, readset, writeset, exceptset, timeout);
}
#endif

int socket(int domain, int type, int protocol)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    
    if (OS_NULL == default_module)
    {
        os_set_errno(-EBADF);
        return OS_ERROR;
    }
    
    socket = mo_socket(default_module, domain, type, protocol);
    if (socket < 0)
    {    
        os_set_errno(-ENOMEM);
        return OS_ERROR;
    }

#ifdef OS_USING_POSIX
    struct vfs_fd *d;
    int fd;

    /* Allocate a fd */
    fd = fd_new();
    if (fd < 0)
    {
        mo_closesocket(default_module, socket);     
        os_set_errno(-ENOMEM);

        return OS_ERROR;
    }

    d = fd_get(fd);
    d->fops = &gs_sockfs_fops;
    d->type = FT_SOCKET;
    d->data = (void *)socket;
    // Release the ref-count of fd
    fd_put(d);

    return fd;
#else
    return socket;    
#endif
}

int closesocket(int fd)
{
    int ret;
    
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }
    
#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_EOK;
    }
    
    ret = mo_closesocket(default_module, (int)d->data);
    fd_put(d);
    
    /* release fd resource */
    fd_put(d);
#else
    ret = mo_closesocket(default_module, fd);
#endif

    return ret;
}

int shutdown(int fd, int how)
{
    LOG_EXT_E("OneOS module is not support shutdown");
    return -1;
}

int bind(int fd, const struct sockaddr *name, socklen_t namelen)
{
    LOG_EXT_E("OneOS module is not support bind");
    return -1;
}

int listen(int fd, int backlog)
{
    LOG_EXT_E("OneOS module is not support listen");
    return -1;
}

int accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    LOG_EXT_E("OneOS module is not support accept");
    return -1;
}

int connect(int fd, const struct sockaddr *name, socklen_t namelen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;    
    fd_put(d);
#else
    socket = fd;
#endif

    return mo_connect(default_module, socket, name, namelen);
}

int sendto(int fd, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }
    
#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;    
    fd_put(d);
#else
    socket = fd;
#endif

    return mo_sendto(default_module, socket, data, size, flags, to, tolen);
}

int send(int fd, const void *data, size_t size, int flags)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }
    
#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;    
    fd_put(d);
#else
    socket = fd;
#endif

    return mo_send(default_module, socket, data, size, flags);
}

int recvfrom(int fd, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;    
    fd_put(d);
#else
    socket = fd;
#endif

    return mo_recvfrom(default_module, socket, mem, len, flags, from, fromlen);
}

int recv(int fd, void *mem, size_t len, int flags)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }
    
#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;    
    fd_put(d);
#else
    socket = fd;
#endif

    return mo_recv(default_module, socket, mem, len, flags);
}

int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }
    
#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;    
    fd_put(d);
#else
    socket = fd;
#endif

    return mo_getsockopt(default_module, socket, level, optname, optval, optlen);
}

int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }
    
#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;    
    fd_put(d);
#else
    socket = fd;
#endif

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

int getpeername(int fd, struct sockaddr *name, socklen_t *namelen)
{
    LOG_EXT_E("OneOS module is not support getpeername");
    return -1;
}

int getsockname(int fd, struct sockaddr *name, socklen_t *namelen)
{
    LOG_EXT_E("OneOS module is not support getsockname");
    return -1;
}

int ioctlsocket(int fd, long cmd, void *argp)
{
    LOG_EXT_E("OneOS module is not support ioctlsocket");
    return -1;
}

#elif defined(BSD_USING_LWIP)

#ifdef OS_USING_POSIX
static int sockfs_adapter_ioctl(struct vfs_fd *file, int cmd, void *args)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_SOCKET);

    return OS_ERROR;
}

static int sockfs_adapter_read(struct vfs_fd *file, void *buf, size_t count)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_SOCKET);

    return OS_ERROR;
}

static int sockfs_adapter_write(struct vfs_fd *file, const void *buf, size_t count)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_SOCKET);
    
    return OS_ERROR;
}

static int sockfs_adapter_close(struct vfs_fd *file)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_SOCKET);

    return lwip_close((int)file->data);
}

static int sockfs_adapter_poll(struct vfs_fd *file, struct os_pollreq *req)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_SOCKET);
    OS_ASSERT(req != OS_NULL);

    return lwip_posix_poll((int)file->data, req);
}
#else
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
    return lwip_select(maxfdp1, readset, writeset, exceptset, timeout);
}
#endif

int socket(int domain, int type, int protocol)
{
    int socket;
    
    socket = lwip_socket(domain, type, protocol);
    if (socket < 0)
    {
        os_set_errno(-ENOMEM);
        return OS_ERROR;
    }
    
#ifdef OS_USING_POSIX
    struct vfs_fd *d;
    int fd;

    /* Allocate a fd */
    fd = fd_new();
    if (fd < 0)
    {
        lwip_close(socket);     
        os_set_errno(-ENOMEM);

        return OS_ERROR;
    }
    
    d = fd_get(fd);
    d->fops = &gs_sockfs_fops;
    d->type = FT_SOCKET;
    d->data = (void *)socket;
    // Release the ref-count of fd
    fd_put(d);

    return fd;
#else
    return socket;    
#endif    
}

int closesocket(int fd)
{
    int ret;
    
#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_EOK;
    }
    
    ret = lwip_close((int)d->data);
    fd_put(d);
    
    /* release fd resource */
    fd_put(d);
#else
    ret = lwip_close(fd);
#endif

    return ret;
}

int shutdown(int fd, int how)
{
    int socket;
    
#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_shutdown(socket, how);
}

int bind(int fd, const struct sockaddr *name, socklen_t namelen)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_bind(socket, name, namelen);
}

int listen(int fd, int backlog)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_listen(socket, backlog);
}

int accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_accept(socket, addr, addrlen);
}

int connect(int fd, const struct sockaddr *name, socklen_t namelen)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_connect(socket, name, namelen);
}

int sendto(int fd, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_sendto(socket, data, size, flags, to, tolen);
}

int send(int fd, const void *data, size_t size, int flags)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_send(socket, data, size, flags);
}

int recvfrom(int fd, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_recvfrom(socket, mem, len, flags, from, fromlen);
}

int recv(int fd, void *mem, size_t len, int flags)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_recv(socket, mem, len, flags);
}

int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_getsockopt(socket, level, optname, optval, optlen);
}

int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

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

int getpeername(int fd, struct sockaddr *name, socklen_t *namelen)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_getpeername(socket, name, namelen);
}

int getsockname(int fd, struct sockaddr *name, socklen_t *namelen)
{
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_getsockname(socket, name, namelen);
}

int ioctlsocket(int fd, long cmd, void *argp)
{    
    int socket;

#ifdef OS_USING_POSIX
    struct vfs_fd *d;

    d = fd_get(fd);
    if (OS_NULL == d)
    {
        return OS_ERROR;
    }
    
    socket = (int)d->data;
    fd_put(d);
#else
    socket = fd;
#endif

    return lwip_ioctl(socket, cmd, argp);
}

#ifdef NET_USING_LWIP212
const char *inet_ntop(int af, const void *src, char *dst, int32_t size)
{
    return lwip_inet_ntop(af, src, dst, size);
}

int inet_pton(int af, const char *src, void *dst)
{
    return lwip_inet_pton(af, src, dst);
}
#endif

#endif
