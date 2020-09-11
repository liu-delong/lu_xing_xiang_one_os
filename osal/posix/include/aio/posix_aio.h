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
 * @file        posix_aio.h
 *
 * @brief       Header file for posix aio interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __POSIX_AIO_H__
#define __POSIX_AIO_H__

#include <sys/time.h>
#include <libc_signal.h>
#include <os_workqueue.h>
#include <vfs_posix.h>

struct aiocb
{
    int             aio_fildes;       /* File descriptor. */
    off_t           aio_offset;       /* File offset. */
    volatile void   *aio_buf;         /* Location of buffer. */
    size_t          aio_nbytes;       /* Length of transfer. */
    int             aio_reqprio;      /* Request priority offset. */
    struct sigevent aio_sigevent;     /* Signal number and value. */
    int             aio_lio_opcode;   /* Operation to be performed. */
    int             aio_result;
    struct os_work  aio_work;
};

int      aio_cancel(int fd, struct aiocb *cb);
int      aio_error(const struct aiocb *cb);
int      aio_fsync(int op, struct aiocb *cb);
int      aio_read(struct aiocb *cb);
ssize_t  aio_return(struct aiocb *cb);
int      aio_suspend(const struct aiocb *const list[], int nent, const struct timespec *timeout);
int      aio_write(struct aiocb *cb);
int      lio_listio(int mode, struct aiocb * const list[], int nent, struct sigevent *sig);

#endif
