/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 */

#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/err.h"
#include "arch/sys_arch.h"
#include "lwip/debug.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "netif/ethernetif.h"
#include "lwip/sio.h"
#include "lwip/init.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"

#include <string.h>
#include <stdio.h>
#include <os_task.h>
#include <os_sem.h>
#include <os_errno.h>
#include <os_mailbox.h>
#include <os_device.h>
#include <os_module.h>
#include <os_clock.h>
#include <os_assert.h>

/*
 * Initialize the network interface device
 *
 * @return the operation status, ERR_OK on OK, ERR_IF on error
 */
static err_t netif_device_init(struct netif *netif)
{
    struct eth_device *ethif;

    ethif = (struct eth_device *)netif->state;
    if (ethif != OS_NULL)
    {
        os_device_t *device;

        /* get device object */
        device = (os_device_t*) ethif;
        if (os_device_init(device) != OS_EOK)
        {
            return ERR_IF;
        }

        /* copy device flags to netif flags */
        netif->flags = ethif->flags;

        return ERR_OK;
    }

    return ERR_IF;
}
/*
 * Initialize the ethernetif layer and set network interface device up
 */
static void tcpip_init_done_callback(void *arg)
{
    os_device_t *device;
    struct eth_device *ethif;
    ip4_addr_t ipaddr, netmask, gw;
    struct os_list_node* node;
    struct os_object* object;
    struct os_object_info *information;

    LWIP_ASSERT("invalid arg.\n",arg);

    IP4_ADDR(&gw, 0,0,0,0);
    IP4_ADDR(&ipaddr, 0,0,0,0);
    IP4_ADDR(&netmask, 0,0,0,0);

    /* enter critical */
    os_enter_critical();

    /* for each network interfaces */
    information = os_object_get_info(OS_OBJECT_DEVICE);
    OS_ASSERT(information != OS_NULL);
    for (node = information->object_list.next;
         node != &(information->object_list);
         node = node->next)
    {
        object = os_list_entry(node, struct os_object, list);
        device = (os_device_t*)object;
        if (device->type == OS_DEVICE_TYPE_NETIF)
        {
            ethif = (struct eth_device *)device;

            /* leave critical */
            os_exit_critical();
            LOCK_TCPIP_CORE();

            netif_add(ethif->netif, &ipaddr, &netmask, &gw,
                      ethif, netif_device_init, tcpip_input);

            if (netif_default == OS_NULL)
                netif_set_default(ethif->netif);

#if LWIP_DHCP
            /* set interface up */
            netif_set_up(ethif->netif);
            /* if this interface uses DHCP, start the DHCP client */
            dhcp_start(ethif->netif);
#else
            /* set interface up */
            netif_set_up(ethif->netif);
#endif

            if (ethif->flags & ETHIF_LINK_PHYUP)
            {
                netif_set_link_up(ethif->netif);
            }

            UNLOCK_TCPIP_CORE();
            /* enter critical */
            os_enter_critical();
        }
    }

    /* leave critical */
    os_exit_critical();
    os_sem_post((os_sem_t*)arg);
}

/**
 * LwIP system initialization
 */
extern int eth_system_device_init_private(void);
int lwip_system_init(void)
{
    os_err_t rc = OS_EOK;;
    os_sem_t        *done_sem  = OS_NULL;
    static os_bool_t init_ok = OS_FALSE;

    if (init_ok)
    {
        os_kprintf("lwip system already init.\n");
        return 0;
    }

    eth_system_device_init_private();

    /* set default netif to NULL */
    netif_default = OS_NULL;

    done_sem = os_sem_create("done", 0, OS_IPC_FLAG_FIFO);

    if (rc != OS_EOK)
    {
        LWIP_ASSERT("Failed to create semaphore", 0);

        return -1;
    }

    tcpip_init(tcpip_init_done_callback, (void *)done_sem);

    /* waiting for initialization done */
    if (os_sem_wait(done_sem, OS_IPC_WAITING_FOREVER) != OS_EOK)
    {
        os_sem_destroy(done_sem);

        return -1;
    }
    os_sem_destroy(done_sem);

    /* set default ip address */
#if !LWIP_DHCP
    if (netif_default != OS_NULL)
    {
        struct ip4_addr ipaddr, netmask, gw;

        ipaddr.addr = inet_addr(LWIP_STATIC_IPADDR);
        gw.addr = inet_addr(LWIP_STATIC_GWADDR);
        netmask.addr = inet_addr(LWIP_STATIC_MSKADDR);

        netifapi_netif_set_addr(netif_default, &ipaddr, &netmask, &gw);
    }
#endif
    os_kprintf("lwIP-%d.%d.%d initialized!\n", LWIP_VERSION_MAJOR, LWIP_VERSION_MINOR, LWIP_VERSION_REVISION);

    init_ok = OS_TRUE;

    return 0;
}
OS_PREV_INIT(lwip_system_init);

void sys_init(void)
{
    /* nothing */
}

void lwip_sys_init(void)
{
    lwip_system_init();
}

/*
 * Create a new semaphore
 *
 * @return the operation status, ERR_OK on OK; others on error
 */
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    static unsigned short counter = 0;
    char tname[OS_NAME_MAX];
    sys_sem_t tmpsem;

    //OS_DEBUG_NOT_IN_INTERRUPT;

    snprintf(tname, OS_NAME_MAX, "%s%d", SYS_LWIP_SEM_NAME, counter);
    counter ++;

    tmpsem = os_sem_create(tname, count, OS_IPC_FLAG_FIFO);
    if (tmpsem == OS_NULL)
        return ERR_MEM;
    else
    {
        *sem = tmpsem;

        return ERR_OK;
    }
}

/*
 * Deallocate a semaphore
 */
void sys_sem_free(sys_sem_t *sem)
{
    //OS_DEBUG_NOT_IN_INTERRUPT;
    os_sem_destroy(*sem);
}

/*
 * Signal a semaphore
 */
void sys_sem_signal(sys_sem_t *sem)
{
    os_sem_post(*sem);
}

/*
 * Block the thread while waiting for the semaphore to be signaled
 *
 * @return If the timeout argument is non-zero, it will return the number of milliseconds
 *         spent waiting for the semaphore to be signaled; If the semaphore isn't signaled
 *         within the specified time, it will return SYS_ARCH_TIMEOUT; If the thread doesn't
 *         wait for the semaphore, it will return zero
 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    os_err_t ret;
    s32_t t;
    u32_t tick;

    //OS_DEBUG_NOT_IN_INTERRUPT;

    /* get the begin tick */
    tick = os_tick_get();
    if (timeout == 0)
        t = OS_IPC_WAITING_FOREVER;
    else
    {
        /* convert msecond to os tick */
        if (timeout < (1000/OS_TICK_PER_SECOND))
            t = 1;
        else
            t = timeout / (1000/OS_TICK_PER_SECOND);
    }

    ret = os_sem_wait(*sem, t);

    if (ret != OS_EOK)
        return SYS_ARCH_TIMEOUT;
    else
        ret = 1;

    /* get elapse msecond */
    tick = os_tick_get() - tick;

    /* convert tick to msecond */
    tick = tick * (1000 / OS_TICK_PER_SECOND);
    if (tick == 0)
        tick = 1;

    return tick;
}

#ifndef sys_sem_valid
/** Check if a semaphore is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_sem_valid(sys_sem_t *sem)
{
    return (int)(*sem);
}
#endif

#ifndef sys_sem_set_invalid
/** Set a semaphore invalid so that sys_sem_valid returns 0
 */
void sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = OS_NULL;
}
#endif

/* ====================== Mutex ====================== */

/** Create a new mutex
 * @param mutex pointer to the mutex to create
 * @return a new mutex
 */
err_t sys_mutex_new(sys_mutex_t *mutex)
{
    static unsigned short counter = 0;
    char tname[OS_NAME_MAX];
    sys_mutex_t tmpmutex;

    //OS_DEBUG_NOT_IN_INTERRUPT;

    snprintf(tname, OS_NAME_MAX, "%s%d", SYS_LWIP_MUTEX_NAME, counter);
    counter ++;

    tmpmutex = os_mutex_create(tname, OS_IPC_FLAG_FIFO, OS_TRUE);
    if (tmpmutex == OS_NULL)
        return ERR_MEM;
    else
    {
        *mutex = tmpmutex;

        return ERR_OK;
    }
}

/** Lock a mutex
 * @param mutex the mutex to lock
 */
void sys_mutex_lock(sys_mutex_t *mutex)
{
    //OS_DEBUG_NOT_IN_INTERRUPT;
    os_mutex_recursive_lock(*mutex, OS_IPC_WAITING_FOREVER);

    return;
}

/** Unlock a mutex
 * @param mutex the mutex to unlock
 */
void sys_mutex_unlock(sys_mutex_t *mutex)
{
    os_mutex_recursive_unlock(*mutex);
}

/** Delete a semaphore
 * @param mutex the mutex to delete
 */
void sys_mutex_free(sys_mutex_t *mutex)
{
    //OS_DEBUG_NOT_IN_INTERRUPT;

    os_mutex_destroy(*mutex);
}

#ifndef sys_mutex_valid
/** Check if a mutex is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_mutex_valid(sys_mutex_t *mutex)
{
    return (int)(*mutex);
}
#endif

#ifndef sys_mutex_set_invalid
/** Set a mutex invalid so that sys_mutex_valid returns 0
 */
void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    *mutex = OS_NULL;
}
#endif

/* ====================== Mailbox ====================== */

/*
 * Create an empty mailbox for maximum "size" elements
 *
 * @return the operation status, ERR_OK on OK; others on error
 */
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    static unsigned short counter = 0;
    char tname[OS_NAME_MAX];
    sys_mbox_t tmpmbox;

    //OS_DEBUG_NOT_IN_INTERRUPT;

    snprintf(tname, OS_NAME_MAX, "%s%d", SYS_LWIP_MBOX_NAME, counter);
    counter ++;

    tmpmbox = os_mb_create(tname, size, OS_IPC_FLAG_FIFO);
    if (tmpmbox != OS_NULL)
    {
        *mbox = tmpmbox;

        return ERR_OK;
    }

    return ERR_MEM;
}

/*
 * Deallocate a mailbox
 */
void sys_mbox_free(sys_mbox_t *mbox)
{
    //OS_DEBUG_NOT_IN_INTERRUPT;

    os_mb_destroy(*mbox);

    return;
}

/** Post a message to an mbox - may not fail
 * -> blocks if full, only used from tasks not from ISR
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 */
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    //OS_DEBUG_NOT_IN_INTERRUPT;

    os_mb_send(*mbox, (os_uint32_t)msg, OS_IPC_WAITING_FOREVER);

    return;
}

/*
 * Try to post the "msg" to the mailbox
 *
 * @return return ERR_OK if the "msg" is posted, ERR_MEM if the mailbox is full
 */
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    if (os_mb_send(*mbox, (os_uint32_t)msg, OS_IPC_WAITING_NO) == OS_EOK)
        return ERR_OK;

    return ERR_MEM;
}

/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return time (in milliseconds) waited for a message, may be 0 if not waited
           or SYS_ARCH_TIMEOUT on timeout
 *         The returned time has to be accurate to prevent timer jitter!
 */
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    os_err_t ret;
    s32_t t;
    u32_t tick;

    //OS_DEBUG_NOT_IN_INTERRUPT;

    /* get the begin tick */
    tick = os_tick_get();

    if(timeout == 0)
        t = OS_IPC_WAITING_FOREVER;
    else
    {
        /* convirt msecond to os tick */
        if (timeout < (1000/OS_TICK_PER_SECOND))
            t = 1;
        else
            t = timeout / (1000/OS_TICK_PER_SECOND);
    }

    ret = os_mb_recv(*mbox, (os_uint32_t *)msg, t);
    if(ret != OS_EOK)
    {
        return SYS_ARCH_TIMEOUT;
    }

    /* get elapse msecond */
    tick = os_tick_get() - tick;

    /* convert tick to msecond */
    tick = tick * (1000 / OS_TICK_PER_SECOND);
    if (tick == 0)
        tick = 1;

    return tick;
}

/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return 0 (milliseconds) if a message has been received
 *         or SYS_MBOX_EMPTY if the mailbox is empty
 */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    int ret;

    ret = os_mb_recv(*mbox, (os_uint32_t *)msg, 0);

    if (ret != OS_EOK)
        return SYS_ARCH_TIMEOUT;
    else
        ret = 1;

    return ret;
}

#ifndef sys_mbox_valid
/** Check if an mbox is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_mbox_valid(sys_mbox_t *mbox)
{
    return (int)(*mbox);
}
#endif

#ifndef sys_mbox_set_invalid
/** Set an mbox invalid so that sys_mbox_valid returns 0
 */
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = OS_NULL;
}
#endif

/* ====================== System ====================== */

/*
 * Start a new thread named "name" with priority "prio" that will begin
 * its execution in the function "thread()". The "arg" argument will be
 * passed as an argument to the thread() function
 */
sys_thread_t sys_thread_new(const char    *name,
                            lwip_thread_fn thread,
                            void          *arg,
                            int            stacksize,
                            int            prio)
{
    os_task_t* t;

    //OS_DEBUG_NOT_IN_INTERRUPT;

    /* create thread */
    t = os_task_create(name, thread, arg, stacksize, prio, 20);
    OS_ASSERT(t != OS_NULL);

    /* startup thread */
    os_task_startup(t);

    return t;
}

sys_prot_t sys_arch_protect(void)
{
    os_base_t level;

    /* disable interrupt */
    level = os_hw_interrupt_disable();

    return level;
}

void sys_arch_unprotect(sys_prot_t pval)
{
    /* enable interrupt */
    os_hw_interrupt_enable(pval);

    return;
}

void sys_arch_assert(const char *file, int line)
{
    os_kprintf("\nAssertion: %d in %s, thread %s\n",
               line, file, os_task_self()->parent.name);
    OS_ASSERT(0);
}

u32_t sys_jiffies(void)
{
    return os_tick_get();
}

u32_t sys_now(void)
{
    return os_tick_get() * (1000 / OS_TICK_PER_SECOND);
}


OS_WEAK
void mem_init(void)
{
}

void *mem_calloc(mem_size_t count, mem_size_t size)
{
    return calloc(count, size);
}

void *mem_trim(void *mem, mem_size_t size)
{
    // return os_realloc(mem, size);
    /* not support trim yet */
    return mem;
}

void *mem_malloc(mem_size_t size)
{
    return malloc(size);
}

void  mem_free(void *mem)
{
    free(mem);
}

#ifdef LWIP_USING_PPP
u32_t sio_read(sio_fd_t fd, u8_t *buf, u32_t size)
{
    u32_t len;

    OS_ASSERT(fd != OS_NULL);

    len = os_device_read((os_device_t*)fd, 0, buf, size);
    if (len <= 0)
        return 0;

    return len;
}

u32_t sio_write(sio_fd_t fd, u8_t *buf, u32_t size)
{
    OS_ASSERT(fd != OS_NULL);

    return os_device_write((os_device_t*)fd, 0, buf, size);
}

void sio_read_abort(sio_fd_t fd)
{
    os_kprintf("read_abort\n");
}

void ppp_trace(int level, const char *format, ...)
{
    va_list args;
 
    va_start(args, format);
    os_kprintf(format, args);
    va_end(args);

    return;
}
#endif

/*
 * export bsd socket symbol
 */
#if LWIP_SOCKET
#include <lwip/sockets.h>
EXPORT_SYMBOL(lwip_accept);
EXPORT_SYMBOL(lwip_bind);
EXPORT_SYMBOL(lwip_shutdown);
EXPORT_SYMBOL(lwip_getpeername);
EXPORT_SYMBOL(lwip_getsockname);
EXPORT_SYMBOL(lwip_getsockopt);
EXPORT_SYMBOL(lwip_setsockopt);
EXPORT_SYMBOL(lwip_close);
EXPORT_SYMBOL(lwip_connect);
EXPORT_SYMBOL(lwip_listen);
EXPORT_SYMBOL(lwip_recv);
EXPORT_SYMBOL(lwip_read);
EXPORT_SYMBOL(lwip_recvfrom);
EXPORT_SYMBOL(lwip_send);
EXPORT_SYMBOL(lwip_sendto);
EXPORT_SYMBOL(lwip_socket);
EXPORT_SYMBOL(lwip_write);
EXPORT_SYMBOL(lwip_select);
EXPORT_SYMBOL(lwip_ioctl);
EXPORT_SYMBOL(lwip_fcntl);

EXPORT_SYMBOL(lwip_htons);
EXPORT_SYMBOL(lwip_htonl);

#if LWIP_DNS
#include <lwip/netdb.h>
EXPORT_SYMBOL(lwip_gethostbyname);
EXPORT_SYMBOL(lwip_gethostbyname_r);
EXPORT_SYMBOL(lwip_freeaddrinfo);
EXPORT_SYMBOL(lwip_getaddrinfo);
#endif

#endif

#if LWIP_DHCP
#include <lwip/dhcp.h>
EXPORT_SYMBOL(dhcp_start);
EXPORT_SYMBOL(dhcp_renew);
EXPORT_SYMBOL(dhcp_stop);
#endif

#if LWIP_NETIF_API
#include <lwip/netifapi.h>
EXPORT_SYMBOL(netifapi_netif_set_addr);
#endif

#if LWIP_NETIF_LINK_CALLBACK
EXPORT_SYMBOL(netif_set_link_callback);
#endif

#if LWIP_NETIF_STATUS_CALLBACK
EXPORT_SYMBOL(netif_set_status_callback);
#endif

EXPORT_SYMBOL(netif_find);
EXPORT_SYMBOL(netif_set_addr);
EXPORT_SYMBOL(netif_set_ipaddr);
EXPORT_SYMBOL(netif_set_gw);
EXPORT_SYMBOL(netif_set_netmask);
