/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2018,RT - Thread Development Team
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * @file        dlog.c
 *
 * @brief       This file realize a small and functional log compoment.
 *
 * @revision
 * Date         Author          Notes
 * 2018-08-25   armink          First version.
 * 2020-03-25   OneOS Team      Format and request resource.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_task.h>
#include <os_mutex.h>
#include <os_sem.h>
#include <os_irq.h>
#include <os_list.h>
#include <os_hw.h>
#include <os_memory.h>
#include <os_assert.h>
#include <os_clock.h>
#include <os_util.h>
#include <string.h>
#include <ring_blk_buff.h>
#include <dlog.h>

#include "log_internal.h"

#ifdef OS_USING_DLOG

#ifdef DLOG_OUTPUT_FLOAT
#include <stdio.h>
#endif

#ifdef DLOG_TIME_USING_TIMESTAMP
#include <sys/time.h>
#endif

/* Buffer size for every line's log */
#ifndef DLOG_LINE_BUF_SIZE
#define DLOG_LINE_BUF_SIZE              256
#endif

/* Output filter's keyword max length */
#ifndef DLOG_FILTER_KW_MAX_LEN
#define DLOG_FILTER_KW_MAX_LEN          15
#endif

#define DLOG_NEWLINE_SIGN               "\r\n"
#define DLOG_FRAME_MAGIC                0x10

#ifdef DLOG_USING_ASYNC_OUTPUT
#ifndef DLOG_ASYNC_OUTPUT_STORE_LINES
#define DLOG_ASYNC_OUTPUT_STORE_LINES   (DLOG_ASYNC_OUTPUT_BUF_SIZE * 3 / 2 / DLOG_LINE_BUF_SIZE)
#endif
#endif

#ifdef DLOG_USING_COLOR
/*
 * CSI(Control Sequence Introducer/Initiator) sign.
 * More information on https://en.wikipedia.org/wiki/ANSI_escape_code
 */
#define CSI_START                       "\033["
#define CSI_END                         "\033[0m"

/* Output log front color */
#define F_BLACK                         "30m"
#define F_RED                           "31m"
#define F_GREEN                         "32m"
#define F_YELLOW                        "33m"
#define F_BLUE                          "34m"
#define F_MAGENTA                       "35m"
#define F_CYAN                          "36m"
#define F_WHITE                         "37m"

/* Output log default color definition */
#ifndef DLOG_COLOR_DEBUG
#define DLOG_COLOR_DEBUG                OS_NULL
#endif

#ifndef DLOG_COLOR_INFO
#define DLOG_COLOR_INFO                 (F_GREEN)
#endif

#ifndef DLOG_COLOR_WARN
#define DLOG_COLOR_WARN                 (F_YELLOW)
#endif

#ifndef DLOG_COLOR_ERROR
#define DLOG_COLOR_ERROR                (F_RED)
#endif
#endif /* DLOG_USING_COLOR */

#if DLOG_LINE_BUF_SIZE < 80
#error "The log line buffer size must more than 80"
#endif

struct dlog_ctrl_info
{
    os_bool_t           init_ok;
    os_mutex_t          log_locker;

    /* Global level */
    os_uint16_t         level_mask;
    os_uint16_t         level;
    
    /* All backends */
    os_slist_node_t     backend_list;
    
    /* The task log's line buffer */
    char                log_buf_task[DLOG_LINE_BUF_SIZE];

#ifdef DLOG_USING_ISR_LOG
    os_base_t           log_locker_isr_lvl;
    char                log_buf_isr[DLOG_LINE_BUF_SIZE];
#endif

#ifdef DLOG_USING_ASYNC_OUTPUT
    rbb_ctrl_info_t    *async_rbb;
    os_task_t          *async_task;       
    os_sem_t            async_notice_sem;
#endif

#ifdef DLOG_USING_FILTER
    struct
    {
        /* All tag's level filter */
        os_slist_node_t tag_lvl_list;

        /* Global filter tag and keyword */    
        char            tag[DLOG_FILTER_TAG_MAX_LEN + 1];
        char            keyword[DLOG_FILTER_KW_MAX_LEN + 1];
    }filter;
#endif
};

/* Tag's level filter */
struct dlog_tag_lvl_filter
{
    os_slist_node_t list;
    char            tag[DLOG_FILTER_TAG_MAX_LEN + 1];
    os_uint16_t     level_mask;
    os_uint16_t     level;
};
typedef struct dlog_tag_lvl_filter dlog_tag_lvl_filter_t;

struct dlog_frame
{
    os_uint8_t      magic;      /* Magic word is 0x10 ('lo') */
    os_uint8_t      is_raw;
    os_uint16_t     level;
    os_uint32_t     log_len;
    char           *log;
    const char     *tag;
};
typedef struct dlog_frame dlog_frame_t;

#ifdef DLOG_OUTPUT_LEVEL_INFO
/* Level output info */
static const char *gs_level_output_info[] =
{
    "EM/",
    "A/",
    "C/",
    "E/",
    "W/",
    "N/",
    "I/",
    "D/"
};
#endif /* DLOG_OUTPUT_LEVEL_INFO */

#ifdef DLOG_USING_COLOR
/* Color output info */
static const char *color_output_info[] =
{
    F_MAGENTA,              /* Compatible for LOG_EMERG */
    F_MAGENTA,              /* Compatible for LOG_ALERT */
    F_RED,                  /* Compatible for LOG_CRIT */
    DLOG_COLOR_ERROR,
    DLOG_COLOR_WARN,
    F_GREEN,                /* Compatible for LOG_NOTICE */
    DLOG_COLOR_INFO,
    DLOG_COLOR_DEBUG,
};
#endif /* DLOG_USING_COLOR */

/* Dlog local ctrl info */
static struct dlog_ctrl_info gs_dlog = {0};

/**
 ***********************************************************************************************************************
 * @brief           This function will copy memory content from source address to destination address.
 *
 * @attention       Cumulative length of this log is less than DLOG_LINE_BUF_SIZE.
 *
 * @param[in]       cur_len         The current length of log.
 * @param[in]       dst             The address of destination memory.
 * @param[in]       src             The address of source memory.
 *
 * @return          The length copied.
 ***********************************************************************************************************************
 */
os_size_t dlog_strcpy(os_size_t cur_len, char *dst, const char *src)
{
    const char *src_old;

    OS_ASSERT(dst);
    OS_ASSERT(src);

    src_old = src;
    while (*src != '\0')
    {
        /* Make sure destination has enough space */
        if (cur_len < DLOG_LINE_BUF_SIZE)
        {
            *dst = *src;
            cur_len++;
            dst++;
            src++;
        }
        else
        {
            break;
        }
    }
    
    return src - src_old;
}

OS_UNUSED static os_size_t dlog_ultoa(char *str, unsigned long value)
{
    os_size_t i;
    os_size_t j;
    os_size_t len;
    char      swap;

    i   = 0;
    j   = 0;
    len = 0;
    
    do
    {
        str[len] = value % 10 + '0';
        value    = value / 10;
        len++;
    } while (value);
    
    str[len] = '\0';

    /* Reverse string */
    for (i = 0, j = len - 1; i < j; ++i, --j)
    {
        swap   = str[i];
        str[i] = str[j];
        str[j] = swap;
    }
    
    return len;
}

static void dlog_lock(void)
{
    /* Is in task context */
    if (os_interrupt_get_nest() == 0)
    {
        os_mutex_lock(&gs_dlog.log_locker, OS_IPC_WAITING_FOREVER);
    }
    /* Is in interrupt context */
    else
    {
#ifdef DLOG_USING_ISR_LOG
        gs_dlog.log_locker_isr_lvl = os_hw_interrupt_disable();
#endif
    }

    return;
}

static void dlog_unlock(void)
{
    /* Is in task context */
    if (os_interrupt_get_nest() == 0)
    {
        os_mutex_unlock(&gs_dlog.log_locker);
    }
    else
    /* Is in interrupt context */
    {
#ifdef DLOG_USING_ISR_LOG
        os_hw_interrupt_enable(gs_dlog.log_locker_isr_lvl);
#endif
    }

    return;
}

static char *dlog_get_log_buf(void)
{
    /* Is in task context */
    if (os_interrupt_get_nest() == 0)
    {
        return gs_dlog.log_buf_task;
    }
    /* Is in interrupt context */
    else
    {
#ifdef DLOG_USING_ISR_LOG
        return gs_dlog.log_buf_isr;
#else
        os_kprintf("Error: Current mode not supported run in ISR. Please enable DLOG_USING_ISR_LOG.\r\n");
        return OS_NULL;
#endif
    }
}

static os_size_t dlog_formater(char        *log_buf,
                               os_uint16_t  level,
                               const char  *tag,
                               os_bool_t    newline,
                               const char  *format,
                               va_list      args)
{
    /* The caller has locker, so it can use static variable for reduce stack usage */
    static os_size_t  s_log_len;
    static os_size_t  s_newline_len;
    static os_int32_t s_fmt_result;

    OS_ASSERT(log_buf);
    OS_ASSERT(level <= LOG_LVL_DEBUG);
    OS_ASSERT(tag);
    OS_ASSERT(format);

    s_log_len = 0;

    if (newline)
    {
        s_newline_len = strlen(DLOG_NEWLINE_SIGN);
    }
    else
    {
        s_newline_len = 0;
    }
    
#ifdef DLOG_USING_COLOR
    /* Add CSI start sign and color info */
    if (color_output_info[level])
    {
        s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, CSI_START);
        s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, color_output_info[level]);
    }
#endif /* DLOG_USING_COLOR */

#ifdef DLOG_OUTPUT_TIME_INFO
    /* Add time info */
    {
#ifdef DLOG_TIME_USING_TIMESTAMP
        static time_t     s_now;
        static struct tm *s_tm;
        static struct tm  s_tm_tmp;

        s_now = time(OS_NULL);
        s_tm  = gmtime_r(&s_now, &s_tm_tmp);

#ifdef OS_USING_SOFT_RTC
        snprintf(log_buf + s_log_len,
                 DLOG_LINE_BUF_SIZE - s_log_len,
                 "%02d-%02d %02d:%02d:%02d.%03d",
                 s_tm->tm_mon + 1,
                 s_tm->tm_mday,
                 s_tm->tm_hour,
                 s_tm->tm_min,
                 s_tm->tm_sec,
                 os_tick_get() % 1000);
#else
        snprintf(log_buf + s_log_len,
                 DLOG_LINE_BUF_SIZE - s_log_len,
                 "%02d-%02d %02d:%02d:%02d",
                 s_tm->tm_mon + 1,
                 s_tm->tm_mday,
                 s_tm->tm_hour,
                 s_tm->tm_min,
                 s_tm->tm_sec);
#endif /* OS_USING_SOFT_RTC */

#else
        static os_size_t tick_len = 0;

        log_buf[s_log_len] = '[';
        tick_len = dlog_ultoa(log_buf + s_log_len + 1, os_tick_get());
        log_buf[s_log_len + 1 + tick_len]     = ']';
        log_buf[s_log_len + 1 + tick_len + 1] = '\0';
#endif /* DLOG_TIME_USING_TIMESTAMP */

        s_log_len += strlen(log_buf + s_log_len);
    }
#endif /* DLOG_OUTPUT_TIME_INFO */

#ifdef DLOG_OUTPUT_LEVEL_INFO

#ifdef DLOG_OUTPUT_TIME_INFO
    s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, " ");
#endif

    /* Add level info */
    s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, gs_level_output_info[level]);
#endif /* DLOG_OUTPUT_LEVEL_INFO */

#ifdef DLOG_OUTPUT_TAG_INFO

#if !defined(DLOG_OUTPUT_LEVEL_INFO) && defined(DLOG_OUTPUT_TIME_INFO)
    s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, " ");
#endif

    /* Add tag info */
    s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, tag);
#endif /* DLOG_OUTPUT_TAG_INFO */

#ifdef DLOG_OUTPUT_TASK_NAME_INFO
    /* Add task info */
    {

#if defined(DLOG_OUTPUT_TIME_INFO) || defined(DLOG_OUTPUT_LEVEL_INFO) || defined(DLOG_OUTPUT_TAG_INFO)
        s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, " ");
#endif

        /* Is not in interrupt context */
        if (os_interrupt_get_nest() == 0)
        {
            os_task_t   *task;
            char        *task_name;
            os_size_t    name_len;

            task      = os_task_self();
            task_name = os_task_name(task);
            name_len  = strlen(task_name);

            strncpy(log_buf + s_log_len, task_name, name_len);
            s_log_len += name_len;
        }
        else
        {
            s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, "ISR");
        }
    }
#endif /* DLOG_OUTPUT_TASK_NAME_INFO */

    s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, ": ");

#ifdef DLOG_OUTPUT_FLOAT
    s_fmt_result = vsnprintf(log_buf + s_log_len, DLOG_LINE_BUF_SIZE - s_log_len, format, args);
#else
    s_fmt_result = os_vsnprintf(log_buf + s_log_len, DLOG_LINE_BUF_SIZE - s_log_len, format, args);
#endif /* DLOG_OUTPUT_FLOAT */

    /* Calculate log length */
    if ((s_log_len + s_fmt_result <= DLOG_LINE_BUF_SIZE) && (s_fmt_result > -1))
    {
        s_log_len += s_fmt_result;
    }
    else
    {
        /* Using max length */
        s_log_len = DLOG_LINE_BUF_SIZE;
    }

    /* overflow check and reserve some space for CSI end sign and newline sign */
#ifdef DLOG_USING_COLOR
    if (s_log_len + (sizeof(CSI_END) - 1) + s_newline_len >= DLOG_LINE_BUF_SIZE)
    {
        /* Using max length */
        s_log_len = DLOG_LINE_BUF_SIZE;

        /* Reserve some space for CSI end sign */
        s_log_len -= (sizeof(CSI_END) - 1);

        /* Reserve some space for newline sign and '\0' */
        s_log_len -= (s_newline_len + 1);
    }
#else
    if (s_log_len + s_newline_len >= DLOG_LINE_BUF_SIZE)
    {
        /* Using max length */
        s_log_len = DLOG_LINE_BUF_SIZE;

        /* Reserve some space for newline sign and '\0' */
        s_log_len -= (s_newline_len + 1);
    }
#endif /* DLOG_USING_COLOR */

    /* Package newline sign */
    if (newline)
    {
        s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, DLOG_NEWLINE_SIGN);
    }

#ifdef DLOG_USING_COLOR
    /* Add CSI end sign  */
    if (color_output_info[level])
    {
        s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, CSI_END);
    }
#endif /* DLOG_USING_COLOR */

    log_buf[s_log_len + 1] = '\0';

    return s_log_len;
}

static void dlog_output_to_all_backend(os_uint16_t  level,
                                       const char  *tag,
                                       os_bool_t    is_raw,
                                       char        *log,
                                       os_size_t    size)
{
    os_slist_node_t *node;
    dlog_backend_t  *backend;

    if (!gs_dlog.init_ok)
    {
        return;
    }
    
    /* Output for all backends */
    os_slist_for_each(node, &gs_dlog.backend_list)  
    {
        backend = os_slist_entry(node, dlog_backend_t, list);

        /* is in interrupt */
        if (os_interrupt_get_nest() > 0)
        {
            if(backend->support_isr == OS_FALSE)
            {
                continue;
            }
        }

#ifndef DLOG_USING_COLOR
        backend->output(backend, level, tag, is_raw, log, size);
#else
        if (backend->support_color || is_raw)
        {
            backend->output(backend, level, tag, is_raw, log, size);
        }
        else
        {
            /* Recalculate the log start address and log size when backend not supported color */
            os_size_t color_info_len;
            os_size_t output_size;

            color_info_len = strlen(color_output_info[level]);
            output_size    = size;
            
            if (color_info_len)
            {
                os_size_t color_hdr_len;

                color_hdr_len = strlen(CSI_START) + color_info_len;
                log += color_hdr_len;

                /* "1" is '\0' */
                output_size -= (color_hdr_len + strlen(CSI_END));
            }
            
            backend->output(backend, level, tag, is_raw, log, output_size);
        }
#endif /* DLOG_USING_COLOR */
    }
}

static void dlog_do_output(os_uint16_t level, const char *tag, os_bool_t is_raw, char *log_buf, os_size_t log_len)
{
#ifdef DLOG_USING_ASYNC_OUTPUT
    rbb_blk_t    *log_blk;
    dlog_frame_t *log_frame;

    /* allocate log frame */
    log_blk = rbb_blk_alloc(gs_dlog.async_rbb,
                            OS_ALIGN_UP(sizeof(dlog_frame_t) + log_len,
                            OS_ALIGN_SIZE));
    if (log_blk)
    {
        /* Package the log frame */
        log_frame          = (dlog_frame_t *)log_blk->buf;
        log_frame->magic   = DLOG_FRAME_MAGIC;
        log_frame->is_raw  = is_raw;
        log_frame->level   = level;
        log_frame->log_len = log_len;
        log_frame->tag     = tag;
        log_frame->log     = (char *)log_blk->buf + sizeof(dlog_frame_t);

        /* Copy log data */
        memcpy(log_blk->buf + sizeof(dlog_frame_t), log_buf, log_len);
        
        /* Put the block */
        rbb_blk_put(log_blk);
        
        /* Send a notice */
        os_sem_post(&gs_dlog.async_notice_sem);
    }
    else
    {
        static os_bool_t already_output = OS_FALSE;
        
        if (already_output == OS_FALSE)
        {
            os_kprintf("Warning: There is no enough buffer for saving async log, "
                       "please increase the ULOG_ASYNC_OUTPUT_BUF_SIZE option.\n");

            already_output = OS_TRUE;
        }
    }
#else

    /* Output to all backends */
    dlog_output_to_all_backend(level, tag, is_raw, log_buf, log_len);
        
#endif /* DLOG_USING_ASYNC_OUTPUT */
}

/**
 ***********************************************************************************************************************
 * @brief           This function will print log on backend.
 *
 * @param[in]       level           The Log level.
 * @param[in]       tag             The log tag.
 * @param[in]       newline         Whether has newline.
 * @param[in]       format          The format.
 * @param[in]       args            The Arguments.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_voutput(os_uint16_t level, const char *tag, os_bool_t newline, const char *format, va_list args)
{
    char        *log_buf;
    os_size_t    log_len;
    os_uint16_t  tag_level;
    os_uint16_t  global_level;
#ifdef DLOG_USING_FILTER
    os_err_t     ret;
#endif    
	
    OS_ASSERT(LOG_PRI(level) <= LOG_LVL_DEBUG);
    OS_ASSERT(tag);
    OS_ASSERT(format);

    if (!gs_dlog.init_ok)
    {
        return;
    }

    level     = LOG_PRI(level);
    tag_level = OS_UINT16_MAX;

#ifdef DLOG_USING_FILTER
    ret = dlog_tag_lvl_filter_get(tag, &tag_level);
    if (ret != OS_EOK)
    {
        tag_level = OS_UINT16_MAX;
    }
#endif

    if (OS_UINT16_MAX == tag_level)
    {
        global_level = dlog_global_lvl_get();
        
        if (0 == (LOG_MASK(level) & LOG_UPTO(global_level)))
        {
            return;
        }
    }
    else
    {
        if (0 == (LOG_MASK(level) & LOG_UPTO(tag_level)))
        {
            return;
        }
    }

#ifdef DLOG_USING_FILTER
    if (gs_dlog.filter.tag[0] != '\0')
    {
        if (strncmp(gs_dlog.filter.tag, tag, DLOG_FILTER_TAG_MAX_LEN))
        {
            return;
        }
    }
#endif

    log_buf = dlog_get_log_buf();
    if (!log_buf)
    {
        return;
    }

    dlog_lock();

    log_len = dlog_formater(log_buf, level, tag, newline, format, args);

#ifdef DLOG_USING_FILTER
    /* Keyword filter */
    if (gs_dlog.filter.keyword[0] != '\0')
    {
        /* Find the keyword */
        if (!strstr(log_buf, gs_dlog.filter.keyword))
        {
            dlog_unlock();
            return;
        }
    }
#endif /* DLOG_USING_FILTER */

    /* Do log output */
    dlog_do_output(level, tag, OS_FALSE, log_buf, log_len);

    dlog_unlock();

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Output the log.
 *
 * @param[in]       level           The Log level.
 * @param[in]       tag             The log tag.
 * @param[in]       newline         Whether has newline.
 * @param[in]       format          The format.
 * @param[in]       ...             The arguments.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_output(os_uint16_t level, const char *tag, os_bool_t newline, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    dlog_voutput(level, tag, newline, format, args);
    va_end(args);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Output raw string format log.
 *
 * @param[in]       format          The format.
 * @param[in]       ...             The arguments.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_raw(const char *format, ...)
{
    os_size_t   log_len;
    char       *log_buf;
    va_list     args;
    os_int32_t  fmt_result;

    OS_ASSERT(gs_dlog.init_ok);

    log_len = 0;
    log_buf = OS_NULL;
    
    /* Get log buffer */
    log_buf = dlog_get_log_buf();
    if (!log_buf)
    {
        return;
    }

    dlog_lock();
    
    va_start(args, format);

#ifdef DLOG_OUTPUT_FLOAT
    fmt_result = vsnprintf(log_buf, DLOG_LINE_BUF_SIZE, format, args);
#else
    fmt_result = os_vsnprintf(log_buf, DLOG_LINE_BUF_SIZE, format, args);
#endif /* DLOG_OUTPUT_FLOAT */

    va_end(args);

    /* Calculate log length */
    if ((fmt_result > -1) && (fmt_result < DLOG_LINE_BUF_SIZE))
    {
        log_len = fmt_result;
    }
    else
    {
        log_len = DLOG_LINE_BUF_SIZE - 1;
    }

    log_buf[log_len + 1] = '\0';

    /* Do log output */
    dlog_do_output(LOG_LVL_DEBUG, NULL, OS_TRUE, log_buf, log_len);

    dlog_unlock();
}

/**
 ***********************************************************************************************************************
 * @brief           Dump the hex format data to log.
 *
 * @param[in]       tag             Name for hex object, it will show on log header.
 * @param[in]       width           Hex number for every line, such as: 16, 32
 * @param[in]       buf             Hex buffer.
 * @param[in]       size            Hex buffer size.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_hexdump(const char *tag, os_size_t width, os_uint8_t *buf, os_size_t size)
{
#define __is_print(ch)       ((unsigned int)((ch) - ' ') < 127u - ' ')

    os_size_t    i;
    os_size_t    j;
    os_size_t    log_len;
    os_size_t    name_len;
    char        *log_buf;
    char         dump_string[8];
    int          fmt_result;
    os_uint16_t  tag_level;
    os_uint16_t  global_level;
#ifdef DLOG_USING_FILTER
    os_err_t     ret;
#endif

    OS_ASSERT(gs_dlog.init_ok);

    tag_level = OS_UINT16_MAX;

#ifdef DLOG_USING_FILTER
    ret = dlog_tag_lvl_filter_get(tag, &tag_level);
    if (ret != OS_EOK)
    {
        tag_level = OS_UINT16_MAX;
    }
#endif

    if (OS_UINT16_MAX == tag_level)
    {
        global_level = dlog_global_lvl_get();
        
        if (0 == (LOG_MASK(LOG_LVL_DEBUG) & LOG_UPTO(global_level)))
        {
            return;
        }
    }
    else
    {
        if (0 == (LOG_MASK(LOG_LVL_DEBUG) & LOG_UPTO(tag_level)))
        {
            return;
        }
    }

#ifdef DLOG_USING_FILTER
    if (gs_dlog.filter.tag[0] != '\0')
    {
        if (strncmp(gs_dlog.filter.tag, tag, DLOG_FILTER_TAG_MAX_LEN))
        {
            return;
        }
    }
#endif

    log_len  = 0;
    name_len = strlen(tag);

    /* Get log buffer */
    log_buf = dlog_get_log_buf();
    if (!log_buf)
    {
        return;
    }

    dlog_lock();

    for (i = 0, log_len = 0; i < size; i += width)
    {
        /* Package header */
        if (i == 0)
        {
            log_len += dlog_strcpy(log_len, log_buf + log_len, "D/HEX ");
            log_len += dlog_strcpy(log_len, log_buf + log_len, tag);
            log_len += dlog_strcpy(log_len, log_buf + log_len, ": ");
        }
        else
        {
            log_len = 6 + name_len + 2;
            memset(log_buf, ' ', log_len);
        }
        
        fmt_result = os_snprintf(log_buf + log_len, DLOG_LINE_BUF_SIZE, "%04X-%04X: ", i, i + width);

        /* Calculate log length */
        if ((fmt_result > -1) && (fmt_result <= DLOG_LINE_BUF_SIZE))
        {
            log_len += fmt_result;
        }
        else
        {
            log_len = DLOG_LINE_BUF_SIZE;
        }
        
        /* Dump hex */
        for (j = 0; j < width; j++)
        {
            if (i + j < size)
            {
                os_snprintf(dump_string, sizeof(dump_string), "%02X ", buf[i + j]);
            }
            else
            {
                strncpy(dump_string, "   ", sizeof(dump_string));
            }
            
            log_len += dlog_strcpy(log_len, log_buf + log_len, dump_string);
            if ((j + 1) % 8 == 0)
            {
                log_len += dlog_strcpy(log_len, log_buf + log_len, " ");
            }
        }
        
        log_len += dlog_strcpy(log_len, log_buf + log_len, "  ");

        /* Dump char for hex */
        for (j = 0; j < width; j++)
        {
            if (i + j < size)
            {
                os_snprintf(dump_string, sizeof(dump_string), "%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
                log_len += dlog_strcpy(log_len, log_buf + log_len, dump_string);
            }
        }
        
        /* Overflow check and reserve some space for newline sign */
        if (log_len + strlen(DLOG_NEWLINE_SIGN) > DLOG_LINE_BUF_SIZE)
        {
            log_len = DLOG_LINE_BUF_SIZE - strlen(DLOG_NEWLINE_SIGN);
        }
        
        /* Package newline sign */
        log_len += dlog_strcpy(log_len, log_buf + log_len, DLOG_NEWLINE_SIGN);
        
        /* Do log output */
        dlog_do_output(LOG_LVL_DEBUG, NULL, OS_TRUE, log_buf, log_len);
    }
    
    dlog_unlock();

    return;
}

#ifdef DLOG_USING_FILTER
/**
 ***********************************************************************************************************************
 * @brief           Set new tag filter.
 *
 * @param[in]       tag             The tag of new filter.
 * @param[in]       level           The level of new filter.
 *
 * @return          Set new tag filer result.
 * @retval          OS_EOK          Set new tag filer success.
 * @retval          else            Set new tag filer failed.
 ***********************************************************************************************************************
 */
os_err_t dlog_tag_lvl_filter_set(const char *tag, os_uint16_t level)
{
    os_slist_node_t       *node;
    dlog_tag_lvl_filter_t *tag_lvl;
    os_int32_t             ret;
    os_bool_t              found;

    OS_ASSERT(tag);
    OS_ASSERT(LOG_PRI(level) <= LOG_LVL_DEBUG);

    if (!gs_dlog.init_ok)
    {
        return OS_ERROR;
    }

    dlog_lock();

    found = OS_FALSE;
    os_slist_for_each(node, &gs_dlog.filter.tag_lvl_list) 
    {
        tag_lvl = os_slist_entry(node, dlog_tag_lvl_filter_t, list);
        if (!strncmp(tag_lvl->tag, tag, DLOG_FILTER_TAG_MAX_LEN))
        {
            found = OS_TRUE;
        
            tag_lvl->level_mask = LOG_UPTO(LOG_PRI(level));
            tag_lvl->level      = LOG_PRI(level);
            
            break;
        }
    }
    
    dlog_unlock();
    
    ret  = OS_EOK;
    if (!found)
    {
        /* New a tag's level filter */
        tag_lvl = (dlog_tag_lvl_filter_t *)os_malloc(sizeof(dlog_tag_lvl_filter_t));
        if (tag_lvl)
        {
            memset(tag_lvl->tag, 0 , sizeof(tag_lvl->tag));
            strncpy(tag_lvl->tag, tag, DLOG_FILTER_TAG_MAX_LEN);
            tag_lvl->level_mask = LOG_UPTO(LOG_PRI(level));;
            tag_lvl->level      = LOG_PRI(level);

            dlog_lock();
            os_slist_add_tail(&gs_dlog.filter.tag_lvl_list, &tag_lvl->list);
            dlog_unlock();
        }
        else
        {
            ret = OS_ENOMEM;
        }
    }
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Query log level of specified filter.
 *
 * @param[in]       tag             The tag of specified filter.
 * @param[out]      level           The log level of specified filter. If query failed, the "level" is invalid.
 *
 * @return          Query result.
 * @retval          OS_EOK          Query success.
 * @retval          else            Query failed.
 ***********************************************************************************************************************
 */
os_err_t dlog_tag_lvl_filter_get(const char *tag, os_uint16_t *level)
{
    os_slist_node_t       *node;
    dlog_tag_lvl_filter_t *tag_lvl;
    os_bool_t              found;
    os_err_t               ret;

    OS_ASSERT(tag);
    OS_ASSERT(level);
    
    if (!gs_dlog.init_ok)
    {
        return OS_ERROR;
    }
    
    dlog_lock();

    found = OS_FALSE;
    os_slist_for_each(node, &gs_dlog.filter.tag_lvl_list) 
    {
        tag_lvl = os_slist_entry(node, dlog_tag_lvl_filter_t, list);
        if (!strncmp(tag_lvl->tag, tag, DLOG_FILTER_TAG_MAX_LEN))
        {
            found  = OS_TRUE;
            *level = tag_lvl->level;
            
            break;
        }
    }
    
    dlog_unlock();

    if (found)
    {
        ret = OS_EOK;
    }
    else
    {
        ret = OS_ERROR;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete specified filter.
 *
 * @param[in]       tag             The tag of specified filter.
 *
 * @return          Delete specified filter result.
 * @retval          OS_EOK          Delete success.
 * @retval          else            Delete failed.
 ***********************************************************************************************************************
 */
os_err_t dlog_tag_lvl_filter_del(const char *tag)
{
    os_slist_node_t       *node;
    dlog_tag_lvl_filter_t *tag_lvl;
    os_bool_t              found;
    os_err_t               ret;

    OS_ASSERT(tag);
    
    if (!gs_dlog.init_ok)
    {
        return OS_ERROR;
    }
    
    dlog_lock();

    found = OS_FALSE;
    os_slist_for_each(node, &gs_dlog.filter.tag_lvl_list) 
    {
        tag_lvl = os_slist_entry(node, dlog_tag_lvl_filter_t, list);
        if (!strncmp(tag_lvl->tag, tag, DLOG_FILTER_TAG_MAX_LEN))
        {
            found  = OS_TRUE; 
            os_slist_del(&gs_dlog.filter.tag_lvl_list, &tag_lvl->list);
            os_free(tag_lvl);
            break;
        }
    }
    
    dlog_unlock();

    if (found)
    {
        ret = OS_EOK;
    }
    else
    {
        ret = OS_ERROR;
    }

    return ret;

}

/**
 ***********************************************************************************************************************
 * @brief           Set tag for global filter.
 *
 * @param[in]       tag             New tag for global filter.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_global_filter_tag_set(const char *tag)
{
    OS_ASSERT(tag);

    memset(gs_dlog.filter.tag, 0, sizeof(gs_dlog.filter.tag));
    strncpy(gs_dlog.filter.tag, tag, DLOG_FILTER_TAG_MAX_LEN);
    
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Get tag for global filter.
 *
 * @param           None.
 *
 * @return          The tag for global filter.
 * @retval          OS_NULL         There is no tag for global filter.
 * @retval          else            The tag string for global filter.
 ***********************************************************************************************************************
 */
const char *dlog_global_filter_tag_get(void)
{
    return gs_dlog.filter.tag;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete tag for global filter.
 *
 * @param           None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_global_filter_tag_del(void)
{
    memset(gs_dlog.filter.tag, 0, sizeof(gs_dlog.filter.tag));

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Set keyword for global filter.
 *
 * @param[in]       keyword         New keyword for global filter.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_global_filter_kw_set(const char *keyword)
{
    OS_ASSERT(keyword);

    memset(gs_dlog.filter.keyword, 0, sizeof(gs_dlog.filter.keyword));
    strncpy(gs_dlog.filter.keyword, keyword, DLOG_FILTER_KW_MAX_LEN);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Get keyword for global filter.
 *
 * @param           None.
 *
 * @return          The keyword for global filter.
 * @retval          OS_NULL         There is no keyword for global filter.
 * @retval          else            The keyword string for global filter.
 ***********************************************************************************************************************
 */
const char *dlog_global_filter_kw_get(void)
{
    return gs_dlog.filter.keyword;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete keyword for global filter.
 *
 * @param           None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_global_filter_kw_del(void)
{
    memset(gs_dlog.filter.keyword, 0, sizeof(gs_dlog.filter.keyword));

    return;
}
#endif /* DLOG_USING_FILTER */

/**
 ***********************************************************************************************************************
 * @brief           Register backend to dlog module.
 *
 * @param[in]       backend         The backend to be registered.
 *
 * @return          Register backend result.
 * @retval          OS_EOK          Register success.
 * @retval          else            Register failed.
 ***********************************************************************************************************************
 */
os_err_t dlog_backend_register(dlog_backend_t *backend)
{
    os_base_t level;

    OS_ASSERT(backend);
    OS_ASSERT(backend->output);
    OS_ASSERT((strlen(backend->name) > 0) && (strlen(backend->name) <= OS_NAME_MAX));
    OS_ASSERT(gs_dlog.init_ok);

    if (backend->init)
    {
        backend->init(backend);
    }

    level = os_hw_interrupt_disable();
    os_slist_add_tail(&gs_dlog.backend_list, &backend->list);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Unregister backend from dlog module.
 *
 * @param[in]       backend         The backend to be unregistered.
 *
 * @return          Unregister backend result.
 * @retval          OS_EOK          Unregister success.
 * @retval          else            Unregister failed.
 ***********************************************************************************************************************
 */
os_err_t dlog_backend_unregister(dlog_backend_t *backend)
{
    os_base_t level;

    OS_ASSERT(backend);
    OS_ASSERT(gs_dlog.init_ok);

    if (backend->deinit)
    {
        backend->deinit(backend);
    }

    level = os_hw_interrupt_disable();
    os_slist_del(&gs_dlog.backend_list, &backend->list);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

#ifdef DLOG_USING_ASYNC_OUTPUT
static void dlog_async_output(void)
{
    rbb_blk_t    *log_blk;
    dlog_frame_t *log_frame;

    log_blk = rbb_blk_get(gs_dlog.async_rbb);
    while (log_blk != NULL)
    {
        log_frame = (dlog_frame_t *)log_blk->buf;
        if (log_frame->magic == DLOG_FRAME_MAGIC)
        {
            /* Output to all backends */
            dlog_output_to_all_backend(log_frame->level,
                                       log_frame->tag,
                                       log_frame->is_raw,
                                       log_frame->log,
                                       log_frame->log_len);
        }
        
        rbb_blk_free(gs_dlog.async_rbb, log_blk);
        log_blk = rbb_blk_get(gs_dlog.async_rbb);
    }

    return;
}

static void dlog_async_waiting_log(os_tick_t time)
{
    os_uint16_t count;

    count = 0;
    os_sem_control(&gs_dlog.async_notice_sem, OS_IPC_CMD_RESET, (void *)&count);
    
    os_sem_wait(&gs_dlog.async_notice_sem, time);

    return;
}

static void dlog_async_output_task_entry(void *param)
{
    dlog_async_output();

    while (1)
    {
        dlog_async_waiting_log(OS_IPC_WAITING_FOREVER);
        dlog_async_output();
    }
}
#endif /* DLOG_USING_ASYNC_OUTPUT */


/**
 ***********************************************************************************************************************
 * @brief           Flush logs in the buffer(dlog buffer and backend buffer).
 *
 * @param           None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_flush(void)
{
    os_slist_node_t *node;
    dlog_backend_t  *backend;

    if (!gs_dlog.init_ok)
    {
        return;
    }
    
#ifdef DLOG_USING_ASYNC_OUTPUT
    dlog_async_output();
#endif

    /* Flush all backends */
    os_slist_for_each(node, &gs_dlog.backend_list)
    {
        backend = os_slist_entry(node, dlog_backend_t, list);
        if (backend->flush)
        {
            backend->flush(backend);
        }
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Set global level.
 *
 * @param[in]       level           The new global level.
 *
 * @return          Set global level result.
 * @retval          OS_EOK          Set global level success.
 * @retval          else            Set global level failed.
 ***********************************************************************************************************************
 */
os_err_t dlog_global_lvl_set(os_uint16_t level)
{
    if (LOG_PRI(level) > LOG_LVL_DEBUG)
    {
        os_kprintf("Invalid para, level(%u)\r\n", level);
        return OS_EINVAL;
    }

    gs_dlog.level_mask = LOG_UPTO(LOG_PRI(level));
    gs_dlog.level      = LOG_PRI(level);
    
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Get global level.
 *
 * @param           None.
 *
 * @return          The global level.
 ***********************************************************************************************************************
 */
os_uint16_t dlog_global_lvl_get(void)
{
    return gs_dlog.level;
}

/**
 ***********************************************************************************************************************
 * @brief           Initialize dlog compoment.
 *
 * @param           None.
 *
 * @return          Initialize dlog compoment result.
 * @retval          OS_EOK          Initialize dlog compoment success.
 * @retval          else            Initialize dlog compoment failed.
 ***********************************************************************************************************************
 */
os_err_t dlog_init(void)
{
    os_err_t ret;

    if (gs_dlog.init_ok)
    {
        return OS_EOK;
    }
    
    ret = os_mutex_init(&gs_dlog.log_locker, "dlog_lock", OS_IPC_FLAG_FIFO, OS_FALSE);
    if (OS_EOK != ret)
    {
        os_kprintf("Error: Dlog init failed! Create mutex failed! \r\n");
        return OS_ERROR;
    }
    
    os_slist_init(&gs_dlog.backend_list);

#ifdef DLOG_USING_FILTER
    os_slist_init(&gs_dlog.filter.tag_lvl_list);
#endif

#ifdef DLOG_USING_ASYNC_OUTPUT
    OS_ASSERT(DLOG_ASYNC_OUTPUT_STORE_LINES >= 2);

    /* Async output ring block buffer */
    gs_dlog.async_rbb = rbb_create(OS_ALIGN_UP(DLOG_ASYNC_OUTPUT_BUF_SIZE, OS_ALIGN_SIZE),
                                   DLOG_ASYNC_OUTPUT_STORE_LINES);
    if (!gs_dlog.async_rbb)
    {
        os_kprintf("Error: Dlog init failed! No memory for async rbb.\r\n");
        
        (void)os_mutex_deinit(&gs_dlog.log_locker);
        return OS_ENOMEM;
    }
    
    /* Async output task */
    gs_dlog.async_task = os_task_create("dlog_async",
                                        dlog_async_output_task_entry,
                                        &gs_dlog,
                                        DLOG_ASYNC_OUTPUT_TASK_STACK,
                                        DLOG_ASYNC_OUTPUT_TASK_PRIORITY,
                                        20);
    if (!gs_dlog.async_task)
    {
        os_kprintf("Error: Dlog init failed! Create async output task failed.\r\n");
        
        (void)os_mutex_deinit(&gs_dlog.log_locker);
        rbb_destroy(gs_dlog.async_rbb);

        return OS_ERROR;
    }

    ret = os_sem_init(&gs_dlog.async_notice_sem, "dlog_sem", 0, OS_IPC_FLAG_FIFO);
    if (OS_EOK != ret)
    {
        os_kprintf("Error: Dlog init failed! Create async semphore failed.\r\n");
        
        (void)os_mutex_deinit(&gs_dlog.log_locker);
        rbb_destroy(gs_dlog.async_rbb);
        (void)os_task_destroy(gs_dlog.async_task);
        
        return OS_ERROR;
    }
    
    /* Async output task startup */
    (void)os_task_startup(gs_dlog.async_task);
#endif /* DLOG_USING_ASYNC_OUTPUT */

    dlog_global_lvl_set(DLOG_GLOBAL_LEVEL);

    gs_dlog.init_ok = OS_TRUE;

    return OS_EOK;
}
OS_PREV_INIT(dlog_init);

/**
 ***********************************************************************************************************************
 * @brief           De-initialize dlog compoment.
 *
 * @param           None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_deinit(void)
{
    os_slist_node_t *node;
    os_slist_node_t *next;
    dlog_backend_t  *backend;

    if (!gs_dlog.init_ok)
    {
        return;
    }
    
    /* Deinit all backends */
    os_slist_for_each_safe(node, next, &gs_dlog.backend_list)
    {
        backend = os_slist_entry(node, dlog_backend_t, list);
        os_slist_del(&gs_dlog.backend_list, &backend->list);
        
        if (backend->deinit)
        {
            backend->deinit(backend);
        }
    }

#ifdef DLOG_USING_FILTER
    /* Deinit tag's level filter */
    {
        dlog_tag_lvl_filter_t *tag_lvl;
        
        os_slist_for_each_safe(node, next, &gs_dlog.filter.tag_lvl_list)
        {
            tag_lvl = os_slist_entry(node, dlog_tag_lvl_filter_t, list);
            os_slist_del(&gs_dlog.filter.tag_lvl_list, &tag_lvl->list);
            os_free(tag_lvl);
        }
    }
#endif /* DLOG_USING_FILTER */

    (void)os_mutex_deinit(&gs_dlog.log_locker);

#ifdef DLOG_USING_ASYNC_OUTPUT
    rbb_destroy(gs_dlog.async_rbb);
    (void)os_task_destroy(gs_dlog.async_task);
    (void)os_sem_deinit(&gs_dlog.async_notice_sem);
#endif

    gs_dlog.init_ok = OS_FALSE;

    return;
}

#ifdef OS_USING_SHELL
#include <shell.h>
#include <stdlib.h>

#include <option_parse.h>

struct dlog_commond_ctrl_info
{
#ifdef DLOG_USING_FILTER
    char                    tag_name[DLOG_FILTER_TAG_MAX_LEN+1];
    char                    keyword [DLOG_FILTER_KW_MAX_LEN+1];
#endif
    os_int32_t              control_info;
    os_uint16_t             level;
};
typedef struct dlog_commond_ctrl_info  dlog_command_ctrl_info_t;

static dlog_command_ctrl_info_t        gs_dlog_ctrl_info;

#define COMMAND_SET_OPTION      1
#define COMMAND_GET_OPTION      2
#define COMMAND_DEL_OPTION      3

os_err_t dlog_ctrl_info_get(os_int32_t argc, char * const *argv, dlog_command_ctrl_info_t *ctrl_info)
{
    opt_state_t state;
    os_int32_t  opt_ret;
    os_int32_t  ret;

    memset(ctrl_info, 0 , sizeof(dlog_command_ctrl_info_t));
    
    memset(&state, 0, sizeof(state));
    opt_init(&state, 1);

    ret = OS_EOK;
    while (1)
    {
        opt_ret = opt_get(argc, argv, "n:sgdl:k:", &state);
        if (opt_ret == OPT_EOF)
        {
            break;
        }

        if ((opt_ret == OPT_BADOPT) || (opt_ret == OPT_BADARG))
        {
            ret = OS_ERROR;
            break;
        }
    
        switch (opt_ret)
        {
        case 's':
            ctrl_info->control_info = COMMAND_SET_OPTION;
            break;

        case 'g':
            ctrl_info->control_info = COMMAND_GET_OPTION;
            break;
            
        case 'd':
            ctrl_info->control_info = COMMAND_DEL_OPTION;
            break;
            
        case 'l':
            ctrl_info->level = (os_uint16_t)atoi(state.opt_arg);
            break;
            
#ifdef DLOG_USING_FILTER
        case 'n':
            memset(ctrl_info->tag_name, 0, DLOG_FILTER_TAG_MAX_LEN);
            strncpy(ctrl_info->tag_name, state.opt_arg, DLOG_FILTER_TAG_MAX_LEN);
            break;

        case 'k':
            memset(ctrl_info->keyword, 0, DLOG_FILTER_KW_MAX_LEN);
            strncpy(ctrl_info->keyword, state.opt_arg, DLOG_FILTER_KW_MAX_LEN);
            break;
#endif            

        default:
            os_kprintf("Invalid option: %c\r\n", (char)opt_ret);

            ret = OS_EINVAL;
            break;
        }

        if (ret != OS_EOK)
        {
            break;
        }
    }

    return ret;
}

static os_err_t sh_dlog_global_level_control(os_int32_t argc, char **argv)
{
    os_err_t   ret;
    os_uint16_t level;

    if(argc < 2)
    {
        SH_PRINT("Command format:");
        SH_PRINT("dlog_glvl_ctrl [-s | -g] [-l global level]");
        SH_PRINT("parameter Usage:");
  
        SH_PRINT("         -s     Set global level option.");
        SH_PRINT("         -g     Get global level option.");
        SH_PRINT("         -l     Specify a global level that want to be set.");
        SH_PRINT("                level: 3-error, 4-warning, 6-info, 7-debug");

        return OS_EINVAL;
    }
    
    ret = dlog_ctrl_info_get(argc,argv,&gs_dlog_ctrl_info);
    
    if (ret != OS_EOK)
    {
        SH_PRINT("Wrong parameter, usage:");
        SH_PRINT("dlog_glvl_ctrl [-s | -g] [-l level]");
        return ret;
    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_GET_OPTION)
    {
        level = dlog_global_lvl_get();

        SH_PRINT("Level is: %u", level);
    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_SET_OPTION)
    {
        if (gs_dlog_ctrl_info.level > LOG_LVL_DEBUG)
        {
            SH_PRINT("Invalid level(%u)", gs_dlog_ctrl_info.level);
            return OS_EINVAL;
        }

        if (gs_dlog_ctrl_info.level == 0)
        {
            gs_dlog_ctrl_info.level = DLOG_GLOBAL_LEVEL;
        }
    
        ret = dlog_global_lvl_set(gs_dlog_ctrl_info.level);
        if (OS_EOK != ret)
        {
            SH_PRINT("Set global level(%u) of dlog failed", gs_dlog_ctrl_info.level);
            return OS_ERROR;
        }
        else
        {
            SH_PRINT("Set global level(%u) of dlog success", gs_dlog_ctrl_info.level);
        }

    }

    return ret; 

}
SH_CMD_EXPORT(dlog_glvl_ctrl, sh_dlog_global_level_control, "dlog global level control");


#ifdef DLOG_USING_FILTER

static os_err_t sh_dlog_tag_level_control(os_int32_t argc, char **argv)
{
    os_err_t   ret;
    os_uint16_t level;

    if(argc < 2)
    {
        SH_PRINT("Command format:");
        SH_PRINT("dlog_tlvl_ctrl [-s | -g | -d] [-n tag name] [-l tag level]");
        SH_PRINT("parameter Usage:");
  
        SH_PRINT("         -s     Set tag level option.");
        SH_PRINT("         -g     Get tag level option.");
        SH_PRINT("         -d     Delete tag level option.");
        SH_PRINT("         -n     Specify the tag name that want set level.");
        SH_PRINT("         -l     Specify a tag level that want to be set.");
        SH_PRINT("                level: 3-error, 4-warning, 6-info, 7-debug");
        return OS_EINVAL;
    }
    ret = dlog_ctrl_info_get(argc,argv,&gs_dlog_ctrl_info);
    
    if (ret != OS_EOK)
    {
        SH_PRINT("Wrong parameter, usage:");
        SH_PRINT("dlog_tlvl_ctrl [-s | -g | -d] [-n tag name] [-l level]");
        return ret;
    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_GET_OPTION)
    {
        if (strlen(gs_dlog_ctrl_info.tag_name) == OS_FALSE)
        {
            SH_PRINT("Wrong parameter, usage:");
            SH_PRINT("dlog_tlvl_ctrl [-g] [-n tag name] ");
            return OS_EINVAL;
        }
    
        ret = dlog_tag_lvl_filter_get(gs_dlog_ctrl_info.tag_name, &level);
        if (OS_EOK != ret)
        {
           SH_PRINT("Get level of tag(%s) failed", gs_dlog_ctrl_info.tag_name);
           return OS_ERROR;
        }

        SH_PRINT("Level of tag(%s) is: %u", gs_dlog_ctrl_info.tag_name, level);

    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_SET_OPTION)
    {
        if (strlen(gs_dlog_ctrl_info.tag_name) == OS_FALSE)
        {
            SH_PRINT("Wrong parameter, usage:");
            SH_PRINT("dlog_tlvl_ctrl [-s] [-n tag name] [-l level]");
            return OS_EINVAL;
        }
    
        if (gs_dlog_ctrl_info.level > LOG_LVL_DEBUG || 
            gs_dlog_ctrl_info.level < LOG_LVL_ERROR || 
            gs_dlog_ctrl_info.level == 5)
        {
            SH_PRINT("Invalid level(%u)", gs_dlog_ctrl_info.level);
            SH_PRINT("level: 3-error, 4-warning, 6-info, 7-debug");
            return OS_EINVAL;
        }

        if (gs_dlog_ctrl_info.level == 0)
        {
            SH_PRINT("Wrong parameter, usage:");
            SH_PRINT("dlog_tlvl_ctrl [-s] [-n tag name] [-l level]");
            return OS_EINVAL;
        }

        ret = dlog_tag_lvl_filter_set(gs_dlog_ctrl_info.tag_name, gs_dlog_ctrl_info.level);
        if (OS_EOK != ret)
        {
            SH_PRINT("Set tag level of dlog failed, tag: %s, level: %u", gs_dlog_ctrl_info.tag_name, gs_dlog_ctrl_info.level);
        }
        else
        {
            SH_PRINT("Set tag level of dlog success, tag: %s, level: %u",gs_dlog_ctrl_info.tag_name, gs_dlog_ctrl_info.level);
        }

    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_DEL_OPTION)
    {
        if (strlen(gs_dlog_ctrl_info.tag_name) == OS_FALSE)
        {
            SH_PRINT("Wrong parameter, usage:");
            SH_PRINT("dlog_tlvl_ctrl [-d] [-n tag name]");
            return OS_EINVAL;
        }
    
        ret = dlog_tag_lvl_filter_del(gs_dlog_ctrl_info.tag_name);
        if (OS_EOK == ret)
        {
            SH_PRINT("Del tag(%s) level success", gs_dlog_ctrl_info.tag_name);
        }
        else
        {
            SH_PRINT("Del tag(%s) level failed", gs_dlog_ctrl_info.tag_name);
        }
    }

    return ret; 

}
SH_CMD_EXPORT(dlog_tlvl_ctrl, sh_dlog_tag_level_control, "dlog tag level control");

static os_err_t sh_dlog_global_tag_control(os_int32_t argc, char **argv)
{
    os_err_t   ret;
    const char *tag;
    
    if(argc < 2)
    {
        SH_PRINT("Command format:");
        SH_PRINT("dlog_gtag_ctrl [-s | -g | -d] [-n tag name] ");
        SH_PRINT("parameter Usage:");
  
        SH_PRINT("         -s     Set global tag option.");
        SH_PRINT("         -g     Get global tag option.");
        SH_PRINT("         -d     Delete global tag option.");
        SH_PRINT("         -n     Specify the global tag name.\r\n");
    }

    ret = dlog_ctrl_info_get(argc,argv,&gs_dlog_ctrl_info);
    
    if (ret != OS_EOK)
    {
        SH_PRINT("Wrong parameter, usage:");
        SH_PRINT("dlog_gtag_ctrl [-s | -g | -d] [-n tag name] ");
        return ret;
    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_GET_OPTION)
    {
        tag = dlog_global_filter_tag_get();
        
        SH_PRINT("The global filter tag of dlog is %s", tag);
    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_SET_OPTION)
    {
        if (strlen(gs_dlog_ctrl_info.tag_name) == OS_FALSE)
        {
            SH_PRINT("Wrong parameter, usage:");
            SH_PRINT("dlog_gtag_ctrl [-s] [-n tag name] ");
            return OS_EINVAL;
        }
    
        dlog_global_filter_tag_set(gs_dlog_ctrl_info.tag_name);
        
        SH_PRINT("Set global filter tag(%s) of dlog success", gs_dlog_ctrl_info.tag_name);
    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_DEL_OPTION)
    {
        dlog_global_filter_tag_del();
        
        SH_PRINT("Del global filter tag of dlog success");
    }

    return ret; 

}
SH_CMD_EXPORT(dlog_gtag_ctrl, sh_dlog_global_tag_control, "dlog global tag control");

static os_err_t sh_dlog_global_keyword_control(os_int32_t argc, char **argv)
{
    os_err_t    ret;
    const char *keyword;

    if(argc < 2)
    {
        SH_PRINT("Command format:");
        SH_PRINT("dlog_gkw_ctrl [-s | -g | -d] [-k keyword] ");
        SH_PRINT("parameter Usage:");
  
        SH_PRINT("         -s     Set global keyword option.");
        SH_PRINT("         -g     Get global keyword option.");
        SH_PRINT("         -d     Delete global keyword option.");
        SH_PRINT("         -k     Specify the gobal keyword.\r\n");
    }
    
    ret = dlog_ctrl_info_get(argc,argv,&gs_dlog_ctrl_info);
    
    if (ret != OS_EOK)
    {
        SH_PRINT("Wrong parameter, usage:");
        SH_PRINT("dlog_gkw_ctrl [-s | -g | -d] [-k keyword] ");
        return ret;
    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_GET_OPTION)
    {
        keyword = dlog_global_filter_kw_get();
        
        SH_PRINT("The global filter keyword of dlog is %s", keyword);

    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_SET_OPTION)
    {
        if (strlen(gs_dlog_ctrl_info.keyword) == OS_FALSE)
        {
            SH_PRINT("Wrong parameter, usage:");
            SH_PRINT("dlog_gkw_ctrl [-s] [-k keyword] ");
            return OS_EINVAL;
        }
    
        dlog_global_filter_kw_set(gs_dlog_ctrl_info.keyword);
        
        SH_PRINT("Set global filter keyword(%s) of dlog success", gs_dlog_ctrl_info.keyword);
    }

    if(gs_dlog_ctrl_info.control_info == COMMAND_DEL_OPTION)
    {
        dlog_global_filter_kw_del();
        
        SH_PRINT("Del global filter keyword of dlog success");
    }

    return ret; 

}
SH_CMD_EXPORT(dlog_gkw_ctrl, sh_dlog_global_keyword_control, "dlog global keyword control");



#endif /* DLOG_USING_FILTER */

#endif /* OS_USING_SHELL */

#endif /* OS_USING_DLOG */

