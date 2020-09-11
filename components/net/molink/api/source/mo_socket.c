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
 * @file        mo_socket.c
 *
 * @brief       module link kit socket api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_common.h"
#include "mo_socket.h"

#include <libc_errno.h>

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define DBG_EXT_TAG "molink.socket"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#define HTONS_PORT(x) ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))

#ifdef MOLINK_USING_SOCKETS_OPS

static mo_sock_t gs_sockets[MOLINK_NUM_SOCKETS] = {0};

static int alloc_socket(mo_netconn_t *netconn)
{
    int level = os_hw_interrupt_disable();

    for (int i = 0; i < MOLINK_NUM_SOCKETS; i++)
    {
        if (OS_NULL == gs_sockets[i].netconn)
        {
            netconn->socket_id = i;

            os_hw_interrupt_enable(level);

            return i;
        }
    }

    os_hw_interrupt_enable(level);

    LOG_EXT_E("Alloc socket failed!");

    return -1;
}

static mo_sock_t *get_socket(int socket)
{
    if (socket < 0 || socket >= MOLINK_NUM_SOCKETS)
    {
        LOG_EXT_E("Invalid socket number %d!", socket);
        return OS_NULL;
    }

    mo_sock_t *sock = &gs_sockets[socket];

    if (OS_NULL == sock->netconn)
    {
        LOG_EXT_E("Get %d socket failed!", socket);
        return OS_NULL;
    }

    return sock;
}

int mo_socket(mo_object_t *module, int domain, int type, int protocol)
{
    OS_ASSERT(module != OS_NULL);

    mo_netconn_t *netconn = OS_NULL;

    switch (type)
    {
    case SOCK_DGRAM:
        netconn = mo_netconn_create(module, NETCONN_TYPE_UDP);
        break;
    case SOCK_STREAM:
        netconn = mo_netconn_create(module, NETCONN_TYPE_TCP);
        break;
    default:
        LOG_EXT_W("Module %s Don't support socket type %d", module->name, type);
        break;
    }

    if (OS_NULL == netconn)
    {
        return -1;
    }

    int socket_id = alloc_socket(netconn);

    if (-1 == socket_id)
    {
        mo_netconn_destroy(module, netconn);
        return -1;
    }

    gs_sockets[socket_id].netconn      = netconn;
    gs_sockets[socket_id].lastdata     = OS_NULL;
    gs_sockets[socket_id].lastoffset   = 0;
    gs_sockets[socket_id].recv_timeout = 0;

    return socket_id;
}

int mo_closesocket(mo_object_t *module, int socket)
{
    OS_ASSERT(module != OS_NULL);

    mo_sock_t *sock = get_socket(socket);
    if (OS_NULL == sock)
    {
        LOG_EXT_E("Module %s get close socket %d failed!", module->name, socket);
        return -1;
    }

    mo_netconn_destroy(module, sock->netconn);

    free(sock->lastdata);

    sock->netconn      = OS_NULL;
    sock->lastdata     = OS_NULL;
    sock->lastoffset   = 0;
    sock->recv_timeout = 0;

    return 0;
}

/* get IP address and port by socketaddr structure information */
static int socketaddr_to_ipaddr_port(const struct sockaddr *sockaddr, ip_addr_t *addr, os_uint16_t *port)
{
    const struct sockaddr_in *sin = (const struct sockaddr_in *)(const void *)sockaddr;

#if defined(MOLINK_USING_IPV4) && defined(MOLINK_USING_IPV6)
    (*addr).u_addr.ip4.addr = sin->sin_addr.s_addr;
#elif defined(MOLINK_USING_IPV4)
    (*addr).addr = sin->sin_addr.s_addr;
#elif defined(MOLINK_USING_IPV6)
    LOG_EXT_E("Not support IPV6.");
#endif /* MOLINK_USING_IPV4 && MOLINK_USING_IPV6 */

    *port = (os_uint16_t)HTONS_PORT(sin->sin_port);

    return 0;
}

int mo_connect(mo_object_t *module, int socket, const struct sockaddr *name, socklen_t namelen)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(name != OS_NULL);

    mo_sock_t *sock = get_socket(socket);
    if (OS_NULL == sock)
    {
        LOG_EXT_E("Module %s get connect socket %d failed!", module->name, socket);
        return -1;
    }

    ip_addr_t   remote_ip   = {0};
    os_uint16_t remote_port = 0;

    socketaddr_to_ipaddr_port(name, &remote_ip, &remote_port);

    os_err_t result = mo_netconn_connect(module, sock->netconn, remote_ip, remote_port);
    if(result != OS_EOK)
    {
        return -1;
    }

    return 0;
}

static int module_tcp_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    if (netconn->stat != NETCONN_STAT_CONNECT)
    {
        LOG_EXT_E("Module %s send data failed, socket %d is not connected", module->name, netconn->socket_id);
        return -1;
    }

    os_size_t sent_size = mo_netconn_send(module, netconn, data, size);
    if (sent_size <= 0)
    {
        return -1;
    }

    return sent_size;
}

static int module_udp_sendto(mo_object_t           *module,
                             mo_netconn_t          *netconn,
                             const char            *data,
                             os_size_t              size,
                             const struct sockaddr *to,
                             socklen_t              tolen)
{
    ip_addr_t   remote_ip   = {0};
    os_uint16_t remote_port = 0;

    socketaddr_to_ipaddr_port(to, &remote_ip, &remote_port);

    if (netconn->stat != NETCONN_STAT_CONNECT)
    {
        /* UDP netconn is not connected */
        os_err_t result = mo_netconn_connect(module, netconn, remote_ip, remote_port);
        if (result != OS_EOK)
        {
            return -1;
        }
    }

    os_size_t sent_size = mo_netconn_send(module, netconn, data, size);
    if (sent_size <= 0)
    {
        return -1;
    }

    return sent_size;
}

int mo_sendto(mo_object_t           *module,
              int                    socket,
              const void            *data,
              size_t                 size,
              int                    flags,
              const struct sockaddr *to,
              socklen_t              tolen)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(data != OS_NULL);

    mo_sock_t *sock = get_socket(socket);
    if (OS_NULL == sock)
    {
        LOG_EXT_E("Module %s get send socket %d failed!", module->name, socket);
        return -1;
    }

    int result = -1;
    switch (sock->netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = module_tcp_send(module, sock->netconn, (const char *)data, size);
        break;
    case NETCONN_TYPE_UDP:
        result = module_udp_sendto(module, sock->netconn, (const char *)data, size, to, tolen);
        break;
    default:
        break;
    }

    if (result < 0)
    {
        LOG_EXT_E("Module %s socket id %d send failed!", module->name, socket);
        return -1;
    }

    return result;
}

int mo_send(mo_object_t *module, int socket, const void *data, size_t size, int flags)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(data != OS_NULL);

    return mo_sendto(module, socket, data, size, flags, OS_NULL, 0);
}

static int module_recv_tcp(mo_object_t *module, mo_sock_t *sock, void *mem, size_t len, int flags)
{
    os_size_t   recvd       = 0;
    void       *data_ptr    = OS_NULL;
    os_size_t   data_len    = 0;
    os_size_t   data_offset = 0;
    os_err_t    result      = OS_EOK;
    os_uint16_t copylen     = 0;

    os_size_t recv_left = (len <= OS_UINT32_MAX) ? (ssize_t)len : OS_UINT32_MAX;

    do
    {
        /* Check if there is data left from the last recv operation. */
        if (sock->lastdata != OS_NULL)
        {
            data_ptr    = sock->lastdata;
            data_offset = sock->lastoffset;
			data_len    = sock->lastlen;
        }
        else
        {
            /* No data was left from the previous operation, so we try to get some from the network. */
            result = mo_netconn_recv(module, sock->netconn, &data_ptr, &data_len, os_tick_from_ms(sock->recv_timeout));
            if (result != OS_EOK)
            {
                if (recvd > 0)
                {
                    goto recv_tcp_done;
                }

                if (OS_ERROR == result)
                {
                    LOG_EXT_E("Module %s receive error, socket %d state %d error",
                              module->name,
                              sock->netconn->socket_id,
                              sock->netconn->stat);
                    return 0;
                }
                else if (OS_ETIMEOUT == result)
                {
                    LOG_EXT_E("Module %s socket %d receive (%d ticks) timeout",
                              module->name,
                              sock->netconn->socket_id,
                              os_tick_from_ms(sock->recv_timeout));

                    errno = EAGAIN;
                    return -1;
                }
            }

            sock->lastdata   = data_ptr;
            sock->lastlen    = data_len;
            sock->lastoffset = 0;
        }

        if (recv_left > data_len)
        {
            copylen = data_len;
        }
        else
        {
            copylen = (os_uint16_t)recv_left;
        }
        if (recvd + copylen < recvd)
        {
            /* overflow */
            copylen = (os_uint16_t)(OS_UINT32_MAX - recvd);
        }

        memcpy((os_uint8_t *)mem + recvd, (os_uint8_t *)data_ptr + data_offset, copylen);

        recvd     += copylen;
        recv_left -= copylen;

        /* ... check if there is data left in the buf */
        if (sock->lastlen - copylen > 0)
        {
            sock->lastlen    -= copylen;
            sock->lastoffset += copylen;
        }
        else
        {
            sock->lastdata   = OS_NULL;
            sock->lastlen    = 0;
            sock->lastoffset = 0;

            free(data_ptr);
        }

    } while (recv_left > 0);

recv_tcp_done:

    if (recvd > 0)
    {
        os_set_errno(0); 
    }
    
    return recvd;
}

static int module_recv_udp(mo_object_t *module, mo_sock_t *sock, void *mem, size_t len, int flags)
{
    os_size_t data_len = 0;
    void *    data_tmp = OS_NULL;
    os_err_t  result   = OS_EOK;

    result = mo_netconn_recv(module, sock->netconn, &data_tmp, &data_len, os_tick_from_ms(sock->recv_timeout));
    if (OS_ERROR == result)
    {
        LOG_EXT_E("Module %s receive error, socket %d state %d error", module->name, sock->netconn->socket_id, sock->netconn->stat);
        return 0;
    }
    else if (OS_ETIMEOUT == result)
    {
        LOG_EXT_E("Module %s socket %d receive (%d ticks) timeout",
                  module->name,
                  sock->netconn->socket_id,
                  os_tick_from_ms(sock->recv_timeout));
        errno = EAGAIN;
        return -1;
    }

    if (data_len > len)
    {
        LOG_EXT_W("The actual udp data received %d is longer than expected %d, and the excess data will be truncated",
                  data_len,
                  len);
        data_len = len;
    }

    memcpy(mem, data_tmp, data_len);
    free(data_tmp);

    if (data_len > 0)
    {
        os_set_errno(0);
    }

    return data_len;
}

int mo_recvfrom(mo_object_t     *module,
                    int              socket,
                    void            *mem,
                    size_t           len,
                    int              flags,
                    struct sockaddr *from,
                    socklen_t       *fromlen)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(mem != OS_NULL);

    mo_sock_t *sock = get_socket(socket);
    if (OS_NULL == sock)
    {
        LOG_EXT_E("Module %s get recv socket %d failed!", module->name, socket);
        return -1;
    }

    if (NETCONN_TYPE_TCP == sock->netconn->type)
    {
        return module_recv_tcp(module, sock, mem, len, flags);
    }
    else /* NETCONN_TYPE_UDP */
    {
        return module_recv_udp(module, sock, mem, len, flags);
    }
}

int mo_recv(mo_object_t *module, int socket, void *mem, size_t len, int flags)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(mem != OS_NULL);

    return mo_recvfrom(module, socket, mem, len, flags, OS_NULL, OS_NULL);
}

int mo_getsockopt(mo_object_t *module, int socket, int level, int optname, void *optval, socklen_t *optlen)
{
    OS_ASSERT(optval != OS_NULL);
    OS_ASSERT(optlen != OS_NULL);

    mo_sock_t *sock = get_socket(socket);
    if (OS_NULL == sock)
    {
        LOG_EXT_E("Module %s get getsockopt socket %d failed!", module->name, socket);
        return -1;
    }

    os_int32_t timeout = 0;

    switch (level)
    {
    case SOL_SOCKET:
        switch (optname)
        {
        case SO_RCVTIMEO:
            timeout                               = sock->recv_timeout;
            ((struct timeval *)(optval))->tv_sec  = (timeout) / 1000U;
            ((struct timeval *)(optval))->tv_usec = (timeout % 1000U) * 1000U;
            break;
        default:
            LOG_EXT_E("Module %s socket %d not support option name :%d.", module->name, socket, optname);
            return -1;
        }
        break;
    default:
        LOG_EXT_E("Module %s socket %d not support option level : %d.", module->name, socket, level);
        return -1;
    }

    return 0;
}

int mo_setsockopt(mo_object_t *module, int socket, int level, int optname, const void *optval, socklen_t optlen)
{
    OS_ASSERT(optval != OS_NULL);

    mo_sock_t *sock = get_socket(socket);
    if (OS_NULL == sock)
    {
        LOG_EXT_E("Module %s get setsockopt socket %d failed!", module->name, socket);
        return -1;
    }

    switch (level)
    {
    case SOL_SOCKET:
        switch (optname)
        {
        case SO_RCVTIMEO:
            sock->recv_timeout = ((const struct timeval *)optval)->tv_sec * 1000 +
                                 ((const struct timeval *)optval)->tv_usec / 1000;
            break;
        case SO_SNDTIMEO:
            break;
        default:
            LOG_EXT_E("Module %s socket %d not support option name :%d.", module->name, socket, optname);
            return -1;
        }
        break;
    default:
        LOG_EXT_E("Module %s socket %d not support option level : %d.", module->name, socket, level);
        return -1;
    }

    return 0;
}

struct hostent *mo_gethostbyname(mo_object_t *module, const char *name)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(name != OS_NULL);

    ip_addr_t addr;

    /* buffer variables for mo_gethostbyname() */
    static struct hostent s_hostent;
    static char *s_aliases;
    static ip_addr_t s_hostent_addr;
    static ip_addr_t *s_phostent_addr[2];
    static char s_hostname[MOLINK_DNS_MAX_NAME_LEN + 1];

    os_err_t result = mo_netconn_gethostbyname(module, name, &addr);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s gethostbyname failed", module->name);
        return OS_NULL;
    }

    /* fill hostent */
    s_hostent_addr     = addr;
    s_phostent_addr[0] = &s_hostent_addr;
    s_phostent_addr[1] = OS_NULL;

    strncpy(s_hostname, name, MOLINK_DNS_MAX_NAME_LEN);
    s_hostname[MOLINK_DNS_MAX_NAME_LEN] = 0;

    s_hostent.h_name      = s_hostname;
    s_aliases             = OS_NULL;
    s_hostent.h_aliases   = &s_aliases;
    s_hostent.h_addrtype  = AF_INET;
    s_hostent.h_length    = sizeof(ip_addr_t);
    s_hostent.h_addr_list = (char **)&s_phostent_addr;

    return &s_hostent;
}

int mo_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
    mo_object_t *module = mo_get_default();
    if (OS_NULL == module)
    {
        return EAI_FAIL;
    }

    int ai_family;

    if (OS_NULL == res)
    {
        return EAI_FAIL;
    }

    *res = OS_NULL;

    if ((OS_NULL == nodename) && (OS_NULL == servname))
    {
        return EAI_NONAME;
    }

    if (hints != OS_NULL)
    {
        ai_family = hints->ai_family;
        if (ai_family != AF_UNSPEC && ai_family != AF_INET)
        {
            return EAI_FAMILY;
        }
    }

    int port_nr = 0;

    if (servname != OS_NULL)
    {
        /* service name specified: convert to port number */
        port_nr = atoi(servname);
        if ((port_nr <= 0) || (port_nr > 0xffff))
        {
            return EAI_SERVICE;
        }
    }

    ip_addr_t addr;

    if (nodename != OS_NULL)
    {
        /* service location specified, try to resolve */
        if ((hints != OS_NULL) && (hints->ai_flags & AI_NUMERICHOST))
        {
            /* no DNS lookup, just parse for an address string */
            if (!inet_aton(nodename, &addr))
            {
                return EAI_NONAME;
            }

            if (AF_INET == ai_family)
            {
                return EAI_NONAME;
            }
        }
        else
        {
            if(inet_addr(nodename) == IPADDR_NONE)
            {
                if (mo_netconn_gethostbyname(module, nodename, &addr) != OS_EOK)
                {
                    return EAI_FAIL;
                }
            }
        }
    }
    else
    {
        /* TODO: service location specified, use loopback address */
    }

    os_size_t total_size = sizeof(struct addrinfo) + sizeof(struct sockaddr_storage);
    os_size_t namelen    = 0;
    if (nodename != OS_NULL)
    {
        namelen = strlen(nodename);
        if (namelen > MOLINK_DNS_MAX_NAME_LEN)
        {
            /* invalid name length */
            return EAI_FAIL;
        }
        OS_ASSERT(total_size + namelen + 1 > total_size);
        total_size += namelen + 1;
    }

    OS_ASSERT(total_size <= sizeof(struct addrinfo) + sizeof(struct sockaddr_storage) + MOLINK_DNS_MAX_NAME_LEN + 1);
    struct addrinfo *ai = calloc(1, total_size);
    if (OS_NULL == ai)
    {
        return EAI_MEMORY;
    }

    /* cast through void* to get rid of alignment warnings */
    struct sockaddr_storage *sa = (struct sockaddr_storage *)(void *)((uint8_t *)ai + sizeof(struct addrinfo));
    struct sockaddr_in *sa4 = (struct sockaddr_in *) sa;
    /* set up sockaddr */
#if defined(MOLINK_USING_IPV4) && defined(MOLINK_USING_IPV6)
    sa4->sin_addr.s_addr = addr.u_addr.ip4.addr;
    sa4->type = IPADDR_TYPE_V4;
#elif defined(MOLINK_USING_IPV4)
    sa4->sin_addr.s_addr = addr.addr;
#elif defined(MOLINK_USING_IPV6)
#error "Not support IPV6."
#endif /* MOLINK_USING_IPV4 && MOLINK_USING_IPV6 */
    sa4->sin_family = AF_INET;
    sa4->sin_len    = sizeof(struct sockaddr_in);
    sa4->sin_port   = htons((os_uint16_t)port_nr);
    ai->ai_family   = AF_INET;

        /* set up addrinfo */
    if (hints != OS_NULL)
    {
        /* copy socktype & protocol from hints if specified */
        ai->ai_socktype = hints->ai_socktype;
        ai->ai_protocol = hints->ai_protocol;
    }
    if (nodename != OS_NULL)
    {
        /* copy nodename to canonname if specified */
        ai->ai_canonname = ((char *) ai + sizeof(struct addrinfo) + sizeof(struct sockaddr_storage));
        memcpy(ai->ai_canonname, nodename, namelen);
        ai->ai_canonname[namelen] = 0;
    }
    ai->ai_addrlen = sizeof(struct sockaddr_storage);
    ai->ai_addr = (struct sockaddr *) sa;

    *res = ai;

    return 0;
}

void mo_freeaddrinfo(struct addrinfo *ai)
{
    struct addrinfo *next = OS_NULL;

    while (ai != OS_NULL)
    {
        next = ai->ai_next;
        free(ai);
        ai = next;
    }
}

#endif /* MOLINK_USING_SOCKETS_OPS */
