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
 * @file        select.c
 *
 * @brief       This file implements POSIX I/O select.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <vfs.h>
#include <vfs_fs.h>
#include <vfs_posix.h>

#include <vfs_poll.h>
#include <vfs_select.h>

static void fdszero(fd_set *set, int nfds)
{
    fd_mask *m;
    int n;

    /*
      The 'sizeof(fd_set)' of the system space may differ from user space,
      so the actual size of the 'fd_set' is determined here with the parameter 'nfds'
    */
    m = (fd_mask *)set;
    for (n = 0; n < nfds; n += (sizeof(fd_mask) * 8))
    {
        memset(m, 0, sizeof(fd_mask));
        m++;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function is a POSIX compliant version, which allows a program to monitor multiple file descriptors,
 *                  waiting until one or more of the file descriptors become"ready" for some class of I/O operation.
 *                  A file descriptor is considered  ready if it is possible to perform the corresponding I/O operation
 *                  without blocking.
 *
 * @param[in]       nfds            The highest-numbered file descriptor in any of the three sets, plus 1.
 * @param[in,out]   readfds         The file descriptors set which will be watched to see if characters become available for reading.
 * @param[in,out]   writefds        The file descriptors set which will be watched to see if a write will not block.
 * @param[in,out]   exceptfds       The file descriptors set which will be watched watched for exceptions.
 * @param[in]       timeout         The minimum interval that the function should block waiting for.
 *
 * @return          The number of file descriptors contained in the three returned descriptor sets.
 * @retval          >=0             Sccess
 * @retval          -1              Failure
 ***********************************************************************************************************************
 */
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int fd;
    int npfds;
    int msec;
    int ndx;
    int ret;
    struct pollfd *pollset = OS_NULL;

    /* How many pollfd structures do we need to allocate? */
    for (fd = 0, npfds = 0; fd < nfds; fd++)
    {
        /* Check if any monitor operation is requested on this fd */
        if ((readfds   && FD_ISSET(fd, readfds))  ||
            (writefds  && FD_ISSET(fd, writefds)) ||
            (exceptfds && FD_ISSET(fd, exceptfds)))
        {
            npfds++;
        }
    }

    /* Allocate the descriptor list for poll() */
    if (npfds > 0)
    {
        pollset = (struct pollfd *)os_calloc(npfds, sizeof(struct pollfd));
        if (!pollset)
        {
            return -1;
        }
    }

    /* Initialize the descriptor list for poll() */
    for (fd = 0, ndx = 0; fd < nfds; fd++)
    {
        int incr = 0;

        /* The reavfs set holds the set of FDs that the caller can be assured
         * of reading from without blocking.  Note that POLLHUP is included as
         * a read-able condition.  POLLHUP will be reported at the end-of-file
         * or when a connection is lost.  In either case, the read() can then
         * be performed without blocking.
         */

        if (readfds && FD_ISSET(fd, readfds))
        {
            pollset[ndx].fd         = fd;
            pollset[ndx].events |= POLLIN;
            incr = 1;
        }

        if (writefds && FD_ISSET(fd, writefds))
        {
            pollset[ndx].fd      = fd;
            pollset[ndx].events |= POLLOUT;
            incr = 1;
        }

        if (exceptfds && FD_ISSET(fd, exceptfds))
        {
            pollset[ndx].fd = fd;
            incr = 1;
        }

        ndx += incr;
    }

    OS_ASSERT(ndx == npfds);

    /* Convert the timeout to milliseconds */
    if (timeout)
    {
        msec = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
    }
    else
    {
        msec = -1;
    }

    /* Then let poll do all of the real work. */

    ret = poll(pollset, npfds, msec);

    /* Now set up the return values */
    if (readfds)
    {
        fdszero(readfds, nfds);
    }

    if (writefds)
    {
        fdszero(writefds, nfds);
    }

    if (exceptfds)
    {
        fdszero(exceptfds, nfds);
    }

    /* Convert the poll descriptor list back into selects 3 bitsets */

    if (ret > 0)
    {
        ret = 0;
        for (ndx = 0; ndx < npfds; ndx++)
        {
            /* Check for read conditions.  Note that POLLHUP is included as a
             * read condition.  POLLHUP will be reported when no more data will
             * be available (such as when a connection is lost).  In either
             * case, the read() can then be performed without blocking.
             */

            if (readfds)
            {
                if (pollset[ndx].revents & (POLLIN | POLLHUP))
                {
                    FD_SET(pollset[ndx].fd, readfds);
                    ret++;
                }
            }

            /* Check for write conditions */
            if (writefds)
            {
                if (pollset[ndx].revents & POLLOUT)
                {
                    FD_SET(pollset[ndx].fd, writefds);
                    ret++;
                }
            }

            /* Check for exceptions */
            if (exceptfds)
            {
                if (pollset[ndx].revents & POLLERR)
                {
                    FD_SET(pollset[ndx].fd, exceptfds);
                    ret++;
                }
            }
        }
    }

    if (pollset)
    {
        os_free(pollset);
    }

    return ret;
}
