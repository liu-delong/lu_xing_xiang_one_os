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
 * @file        poll.c
 *
 * @brief       This file implements POSIX I/O poll.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-20   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <os_clock.h>
#include <os_hw.h>
#include <os_task.h>
#include <vfs.h>
#include <vfs_file.h>
#include <vfs_posix.h>
#include <vfs_poll.h>

struct os_poll_node;

struct os_poll_table
{
    os_pollreq_t req;
    os_uint32_t triggered; /* The waited task whether triggered */
    os_task_t *polling_task;
    struct os_poll_node *nodes;
};

struct os_poll_node
{
    struct os_waitqueue_node wqn;
    struct os_poll_table *pt;
    struct os_poll_node *next;
};

static int __wqueue_pollwake(struct os_waitqueue_node *wait, void *key)
{
    struct os_poll_node *pn;

    if (key && !((os_ubase_t)key & wait->key))
    {
        return -1;
    }

    pn = os_container_of(wait, struct os_poll_node, wqn);
    pn->pt->triggered = 1;

    return os_waitqueue_default_do_wake(wait, key);
}

static void _poll_add(os_waitqueue_t *wq, os_pollreq_t *req)
{
    struct os_poll_table *pt;
    struct os_poll_node *node;

    node = os_malloc(sizeof(struct os_poll_node));
    if (node == OS_NULL)
    {
        return;
    }

    pt = os_container_of(req, struct os_poll_table, req);

    os_waitqueue_node_init(&node->wqn, __wqueue_pollwake, pt->polling_task, req->_key);

    node->next = pt->nodes;
    node->pt = pt;
    pt->nodes = node;
    os_waitqueue_add(wq, &node->wqn);
}

static void poll_table_init(struct os_poll_table *pt)
{
    pt->req._proc = _poll_add;
    pt->triggered = 0;
    pt->nodes = OS_NULL;
    pt->polling_task = (os_task_t *)os_task_self();
}

static int poll_wait_timeout(struct os_poll_table *pt, int msec)
{
    os_int32_t timeout;
    int ret = 0;
    struct os_task *task;
    os_base_t level;

    task = pt->polling_task;

    timeout = os_tick_from_ms(msec);

    level = os_hw_interrupt_disable();

    if (timeout != 0 && !pt->triggered)
    {
        extern os_err_t os_task_suspend(os_task_t *task);
        extern void     os_schedule(void);

        os_task_suspend(task);
        if (timeout > 0)
        {
            os_timer_control(&(task->task_timer),
                             OS_TIMER_CTRL_SET_TIME,
                             &timeout);
            os_timer_start(&(task->task_timer));
        }

        os_hw_interrupt_enable(level);

        os_schedule();

        level = os_hw_interrupt_disable();
    }

    ret = !pt->triggered;
    os_hw_interrupt_enable(level);

    return ret;
}

static int do_pollfd(struct pollfd *pollfd, os_pollreq_t *req)
{
    int mask = 0;
    int fd;

    fd = pollfd->fd;

    if (fd >= 0)
    {
        struct vfs_fd *f = fd_get(fd);
        mask = POLLNVAL;

        if (f)
        {
            mask = POLLMASK_DEFAULT;
            if (f->fops->poll)
            {
                req->_key = pollfd->events | POLLERR | POLLHUP;

                mask = f->fops->poll(f, req);
            }
            /* Mask out unneeded events. */
            mask &= pollfd->events | POLLERR | POLLHUP;
            fd_put(f);
        }
    }
    pollfd->revents = mask;

    return mask;
}

static int poll_do(struct pollfd *fds, nfds_t nfds, struct os_poll_table *pt, int msec)
{
    int num;
    int istimeout = 0;
    int n;
    struct pollfd *pf;

    if (msec == 0)
    {
        pt->req._proc = OS_NULL;
        istimeout = 1;
    }

    while (1)
    {
        pf = fds;
        num = 0;

        for (n = 0; n < nfds; n ++)
        {
            /* Check poll event*/
            if (do_pollfd(pf, &pt->req))
            {
                num ++;
                pt->req._proc = OS_NULL;
            }
            pf ++;
        }

        pt->req._proc = OS_NULL;

        if (num || istimeout)
        {
            break;
        }

        /* Block calling task here to wait for events */
        if (poll_wait_timeout(pt, msec))
        {
            istimeout = 1;
        }
    }

    return num;
}

static void poll_teardown(struct os_poll_table *pt)
{
    struct os_poll_node *node, *next;

    next = pt->nodes;
    while (next)
    {
        node = next;
        os_waitqueue_remove(&node->wqn);
        next = node->next;
        os_free(node);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which allows a program to monitor multiple file descriptors,
 *                  waiting until one or more of the file descriptors become"ready" for some class of I/O operation.
 *                  A file descriptor is considered  ready if it is possible to perform the corresponding I/O operation
 *                  without blocking.
 *
 * @param[in,out]   fds             The argument specifies the file descriptors to be examined and the events of interest
                                    for each file descriptor.
 * @param[in,out]   nfds            The number of items in the fds array.
 * @param[in]       timeout         The minimum interval that the function should block waiting for.
 *
 * @return          The number of structures which have nonzero revents fields.
 * @retval          >0              Sccess
 * @retval          0               Timeout
 * @retval          -1              Failure
 ***********************************************************************************************************************
 */
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    int num;
    struct os_poll_table table;

    poll_table_init(&table);

    num = poll_do(fds, nfds, &table, timeout);

    poll_teardown(&table);

    return num;
}
