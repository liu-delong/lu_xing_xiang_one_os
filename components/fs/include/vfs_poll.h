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
 * @file        vfs_poll.h
 *
 * @brief       Header file for synchronous I/O multiplexing interface poll.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-20   OneOS team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_POLL_H__
#define __VFS_POLL_H__


#include <os_task.h>
#include <os_assert.h>
#include <extend/os_waitqueue.h>

#ifdef OS_USING_POSIX
#include <sys/time.h> /* For struct timeval */

#if !defined(POLLIN) && !defined(POLLOUT)
#define POLLIN             (0x01)
#define POLLRDNORM         (0x01)
#define POLLRDBAND         (0x01)
#define POLLPRI            (0x01)

#define POLLOUT            (0x02)
#define POLLWRNORM         (0x02)
#define POLLWRBAND         (0x02)

#define POLLERR            (0x04)
#define POLLHUP            (0x08)
#define POLLNVAL           (0x10)

typedef unsigned int nfds_t;

/**
 ***********************************************************************************************************************
 * @struct      pollfd
 *
 * @brief       Define the structure used for poll() to monitor.
 ***********************************************************************************************************************
 */
struct pollfd
{
    int   fd;                    /* File descriptor */
    short events;                /* Requested events */
    short revents;               /* Returned events */
};
#endif /* !defined(POLLIN) && !defined(POLLOUT) */

struct os_pollreq;
typedef void (*poll_queue_proc)(os_waitqueue_t *, struct os_pollreq *);

typedef struct os_pollreq
{
    poll_queue_proc _proc;
    short           _key;
} os_pollreq_t;

OS_INLINE void os_poll_add(os_waitqueue_t *wq, struct os_pollreq *req)
{
    if (req && req->_proc && wq)
    {
        req->_proc(wq, req);
    }
}

#define POLLMASK_DEFAULT (POLLIN | POLLOUT | POLLRDNORM | POLLWRNORM)
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
#endif /* OS_USING_POSIX */

#endif /* __VFS_POLL_H__ */
