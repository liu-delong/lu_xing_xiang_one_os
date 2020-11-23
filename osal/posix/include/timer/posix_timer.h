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
 * @file        clock_time.h
 *
 * @brief       Header file for clock time interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef POSIX_TIMER_H__
#define POSIX_TIMER_H__

#include <oneos_config.h>
/* #include <sys/types.h> */
/* #include <sys/time.h> */

/* Posix clock and timer. */
#define MILLISECOND_PER_SECOND  1000UL
#define MICROSECOND_PER_SECOND  1000000UL
#define NANOSECOND_PER_SECOND   1000000000ULL

#define MILLISECOND_PER_TICK    (MILLISECOND_PER_SECOND / OS_TICK_PER_SECOND)
#define MICROSECOND_PER_TICK    (MICROSECOND_PER_SECOND / OS_TICK_PER_SECOND)
#define NANOSECOND_PER_TICK     (NANOSECOND_PER_SECOND  / OS_TICK_PER_SECOND)

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME      1
#endif

#define CLOCK_CPUTIME_ID    2

#ifndef CLOCK_PROCESS_CPUTIME_ID
#define CLOCK_PROCESS_CPUTIME_ID CLOCK_CPUTIME_ID
#endif
#ifndef CLOCK_THREAD_CPUTIME_ID
#define CLOCK_THREAD_CPUTIME_ID  CLOCK_CPUTIME_ID
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC     4
#endif

#define SIGEV_SIGNAL	0	/* notify via signal */
#define SIGEV_NONE      1	/* other notification: meaningless */
#define SIGEV_THREAD	2	/* deliver via thread creation */

#if !defined(__GNUC__)
typedef void* timer_t;
struct itimerspec {
	struct timespec it_interval;  /* Timer interval */
	struct timespec it_value;     /* Timer expiration */
};
#endif


int timer_create(clockid_t clockid, struct sigevent *restrict evp, timer_t *restrict timerid);
int timer_delete(timer_t timerid);
int timer_getoverrun(timer_t timerid);
int timer_gettime(timer_t timerid, struct itimerspec *value);
int timer_settime(timer_t timerid, int flags, const struct itimerspec *restrict value, struct itimerspec *restrict ovalue);

#endif
