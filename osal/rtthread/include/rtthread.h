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
 * @file        rtthread.h
 *
 * @brief       RT-Thread adaper all interfaces header file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __RT_THREAD_H__
#define __RT_THREAD_H__

#include <rtconfig.h>
#include <rtdef.h>
#include <rtservice.h>

#ifdef __cplusplus
extern "C" {
#endif

extern rt_tick_t   rt_tick_get(void);
extern void        rt_tick_set(rt_tick_t tick);
extern rt_tick_t   rt_tick_from_millisecond(rt_int32_t ms);

/* Timer interface */
extern void        rt_timer_init(rt_timer_t       timer,
                                 const char      *name,
                                 void           (*timeout)(void *parameter),
                                 void            *parameter,
                                 rt_tick_t        time,
                                 rt_uint8_t       flag);                               
extern rt_err_t    rt_timer_detach(rt_timer_t timer);

extern rt_timer_t  rt_timer_create(const char    *name,
                                   void         (*timeout)(void *parameter),
                                   void          *parameter,
                                   rt_tick_t      time,
                                   rt_uint8_t     flag);                           
extern rt_err_t    rt_timer_delete(rt_timer_t timer);

extern rt_err_t    rt_timer_start(rt_timer_t timer);
extern rt_err_t    rt_timer_stop(rt_timer_t timer);
extern rt_err_t    rt_timer_control(rt_timer_t timer, int cmd, void *arg);

/* Thread interface */
extern rt_err_t    rt_thread_init(struct rt_thread     *thread,
                                  const char           *name,
                                  void                (*entry)(void *parameter),
                                  void                 *parameter,
                                  void                 *stack_start,
                                  rt_uint32_t           stack_size,
                                  rt_uint8_t            priority,
                                  rt_uint32_t           tick);
extern rt_err_t    rt_thread_detach(rt_thread_t thread);

extern rt_thread_t rt_thread_create(const char *name,
                             void (*entry)(void *parameter),
                             void       *parameter,
                             rt_uint32_t stack_size,
                             rt_uint8_t  priority,
                             rt_uint32_t tick);
extern rt_err_t    rt_thread_delete(rt_thread_t thread);

extern rt_thread_t rt_thread_self(void);
extern rt_thread_t rt_thread_find(char *name);
extern rt_err_t    rt_thread_startup(rt_thread_t thread);
extern rt_err_t    rt_thread_yield(void);
extern rt_err_t    rt_thread_delay(rt_tick_t tick);
extern rt_err_t    rt_thread_mdelay(rt_int32_t ms);
extern rt_err_t    rt_thread_control(rt_thread_t thread, int cmd, void *arg);
extern rt_err_t    rt_thread_suspend(rt_thread_t thread);
extern rt_err_t    rt_thread_resume(rt_thread_t thread);

/* Idle thread interface */
#if defined(RT_USING_HOOK) || defined(RT_USING_IDLE_HOOK)
extern rt_err_t    rt_thread_idle_sethook(void (*hook)(void));
extern rt_err_t    rt_thread_idle_delhook(void (*hook)(void));
#endif

/* Schedule service interface*/
extern void        rt_schedule(void);
extern void        rt_enter_critical(void);
extern void        rt_exit_critical(void);
extern rt_uint16_t rt_critical_level(void);

/* Memory pool interface */
#ifdef RT_USING_MEMPOOL
extern rt_err_t    rt_mp_init(struct rt_mempool *mp,
                              const char        *name,
                              void              *start,
                              rt_size_t          size,
                              rt_size_t          block_size);
extern rt_err_t    rt_mp_detach(struct rt_mempool *mp);

extern rt_mp_t     rt_mp_create(const char *name,
                                rt_size_t   block_count,
                                rt_size_t   block_size);
extern rt_err_t    rt_mp_delete(rt_mp_t mp);

extern void       *rt_mp_alloc(rt_mp_t mp, rt_int32_t time);
extern void        rt_mp_free(void *block);
#endif /* RT_USING_MEMPOOL */

/* Heap memory interface */
#ifdef RT_USING_HEAP
extern void       *rt_malloc(rt_size_t nbytes);
extern void        rt_free(void *ptr);
extern void       *rt_realloc(void *ptr, rt_size_t nbytes);
extern void       *rt_calloc(rt_size_t count, rt_size_t size);
extern void       *rt_malloc_align(rt_size_t size, rt_size_t align);
extern void        rt_free_align(void *ptr);
#endif /* RT_USING_HEAP */

/* Memory heap object interface */
#ifdef RT_USING_MEMHEAP
extern rt_err_t    rt_memheap_init(struct rt_memheap *memheap, const char *name, void *start_addr, rt_size_t size);
extern rt_err_t    rt_memheap_detach(struct rt_memheap *heap);

extern void       *rt_memheap_alloc(struct rt_memheap *heap, rt_size_t size);
extern void       *rt_memheap_realloc(struct rt_memheap *heap, void *ptr, rt_size_t newsize);
extern void        rt_memheap_free(void *ptr);
#endif /* RT_USING_MEMHEAP */

/* Semaphore interface */
#ifdef RT_USING_SEMAPHORE
extern rt_err_t    rt_sem_init(rt_sem_t sem, const char *name, rt_uint32_t value, rt_uint8_t flag);
extern rt_err_t    rt_sem_detach(rt_sem_t sem);

extern rt_sem_t    rt_sem_create(const char *name, rt_uint32_t value, rt_uint8_t flag);
extern rt_err_t    rt_sem_delete(rt_sem_t sem);

extern rt_err_t    rt_sem_take(rt_sem_t sem, rt_int32_t time);
extern rt_err_t    rt_sem_trytake(rt_sem_t sem);
extern rt_err_t    rt_sem_release(rt_sem_t sem);

extern rt_err_t    rt_sem_control(rt_sem_t sem, int cmd, void *arg);
#endif /* RT_USING_SEMAPHORE */

/* Mutex interface */
#ifdef RT_USING_MUTEX
extern rt_err_t    rt_mutex_init(rt_mutex_t mutex, const char *name, rt_uint8_t flag);
extern rt_err_t    rt_mutex_detach(rt_mutex_t mutex);

extern rt_mutex_t  rt_mutex_create(const char *name, rt_uint8_t flag);
extern rt_err_t    rt_mutex_delete(rt_mutex_t mutex);

extern rt_err_t    rt_mutex_take(rt_mutex_t mutex, rt_int32_t time);
extern rt_err_t    rt_mutex_release(rt_mutex_t mutex);
extern rt_err_t    rt_mutex_control(rt_mutex_t mutex, int cmd, void *arg);
#endif /* RT_USING_MUTEX */

/* Event interface */
#ifdef RT_USING_EVENT
extern rt_err_t    rt_event_init(rt_event_t event, const char *name, rt_uint8_t flag);
extern rt_err_t    rt_event_detach(rt_event_t event);

extern rt_event_t  rt_event_create(const char *name, rt_uint8_t flag);
extern rt_err_t    rt_event_delete(rt_event_t event);

extern rt_err_t    rt_event_send(rt_event_t event, rt_uint32_t set);
extern rt_err_t    rt_event_recv(rt_event_t     event,
                                 rt_uint32_t    set,
                                 rt_uint8_t     opt,
                                 rt_int32_t     timeout,
                                 rt_uint32_t   *recved);
                                 
extern rt_err_t    rt_event_control(rt_event_t event, int cmd, void *arg);
#endif /* RT_USING_EVENT */

/* Mailbox interface */
#ifdef RT_USING_MAILBOX
extern rt_err_t     rt_mb_init(rt_mailbox_t  mb,
                               const char   *name,
                               void         *msgpool,
                               rt_size_t     size,
                               rt_uint8_t    flag);
extern rt_err_t     rt_mb_detach(rt_mailbox_t mb);

extern rt_mailbox_t rt_mb_create(const char *name, rt_size_t size, rt_uint8_t flag);
extern rt_err_t     rt_mb_delete(rt_mailbox_t mb);

extern rt_err_t     rt_mb_send(rt_mailbox_t mb, rt_uint32_t value);
extern rt_err_t     rt_mb_send_wait(rt_mailbox_t mb, rt_uint32_t value, rt_int32_t timeout);
extern rt_err_t     rt_mb_recv(rt_mailbox_t mb, rt_uint32_t *value, rt_int32_t timeout);

extern rt_err_t     rt_mb_control(rt_mailbox_t mb, int cmd, void *arg);
#endif /* RT_USING_MAILBOX */

/* Message queue interface */
#ifdef RT_USING_MESSAGEQUEUE
extern rt_err_t     rt_mq_init(rt_mq_t     mq,
                               const char *name,
                               void       *msgpool,
                               rt_size_t   msg_size,
                               rt_size_t   pool_size,
                               rt_uint8_t  flag);
extern rt_err_t     rt_mq_detach(rt_mq_t mq);

extern rt_mq_t      rt_mq_create(const char *name, rt_size_t msg_size, rt_size_t max_msgs, rt_uint8_t flag);
extern rt_err_t     rt_mq_delete(rt_mq_t mq);

extern rt_err_t     rt_mq_send(rt_mq_t mq, void *buffer, rt_size_t size);
extern rt_err_t     rt_mq_urgent(rt_mq_t mq, void *buffer, rt_size_t size);
extern rt_err_t     rt_mq_recv(rt_mq_t mq, void *buffer, rt_size_t size, rt_int32_t timeout);
                               
extern rt_err_t     rt_mq_control(rt_mq_t mq, int cmd, void *arg);
#endif /* RT_USING_MESSAGEQUEUE */


/* Interrupt interface */
extern void         rt_interrupt_enter(void);
extern void         rt_interrupt_leave(void);
extern rt_uint8_t   rt_interrupt_get_nest(void);


extern void         rt_kprintf(const char *fmt, ...);
extern void         rt_kputs(const char *str);
extern rt_int32_t   rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args);
extern rt_int32_t   rt_vsprintf(char *dest, const char *format, va_list args);
extern rt_int32_t   rt_sprintf(char *buf, const char *format, ...);
extern rt_int32_t   rt_snprintf(char *buf, rt_size_t size, const char *format, ...);

extern rt_err_t     rt_get_errno(void);
extern void         rt_set_errno(rt_err_t no);
extern int        *_rt_errno(void);

#if !defined(RT_USING_NEWLIB) && !defined(_WIN32)
#ifndef errno
#define errno     *_rt_errno()
#endif
#endif

extern rt_int32_t   rt_strncmp(const char *cs, const char *ct, rt_ubase_t count);
extern rt_int32_t   rt_strcmp(const char *cs, const char *ct);
extern rt_size_t    rt_strlen(const char *src);
extern rt_size_t    rt_strnlen(const char *s, rt_ubase_t maxlen);

#ifdef RT_USING_HEAP
extern char        *rt_strdup(const char *s);
#endif

extern char        *rt_strstr(const char *str1, const char *str2);
extern char        *rt_strncpy(char *dest, const char *src, rt_ubase_t n);
extern rt_uint32_t  rt_strcasecmp(const char *a, const char *b);
extern void        *rt_memset(void *src, int c, rt_ubase_t n);
extern void        *rt_memcpy(void *dest, const void *src, rt_ubase_t n);
extern void        *rt_memmove(void *dest, const void *src, rt_ubase_t n);
extern rt_int32_t   rt_memcmp(const void *cs, const void *ct, rt_ubase_t count);

#define MSH_CMD_EXPORT(command, desc)
#define MSH_CMD_EXPORT_ALIAS(command, alias, desc)
 
#ifdef __cplusplus
}
#endif

#endif /* __RT_THREAD_H__ */

