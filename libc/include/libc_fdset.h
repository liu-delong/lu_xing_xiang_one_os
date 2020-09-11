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
 * @file        libc_fdset.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __LIBC_FDSET_H__
#define __LIBC_FDSET_H__

#include <oneos_config.h>

#if defined(OS_USING_NEWLIB) || defined(_WIN32)
#include <sys/types.h>
#if defined(HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

#else

#if defined(OS_USING_POSIX) && defined(OS_USING_VFS)

#ifdef FD_SETSIZE
#undef FD_SETSIZE
#endif

#define FD_SETSIZE      VFS_FD_MAX
#endif

#ifndef   FD_SETSIZE
#define  FD_SETSIZE  32
#endif

/* Number of bits in a byte. */
#define   NBBY    8       

typedef long    fd_mask;

/* Bits per mask. */
#define   NFDBITS (sizeof (fd_mask) * NBBY)   

#ifndef   howmany
#define  howmany(x,y)    (((x) + ((y) -1 )) / (y))
#endif

/* We use a macro for fd_set so that including Sockets.h afterwards can work. */
typedef struct _types_fd_set 
{
    fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} _types_fd_set;

#define fd_set _types_fd_set

#define   FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1L << ((n) % NFDBITS)))
#define   FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1L << ((n) % NFDBITS)))
#define   FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1L << ((n) % NFDBITS)))
#define   FD_ZERO(p)       memset((void*)(p), 0, sizeof(*(p)))

#endif

#endif
