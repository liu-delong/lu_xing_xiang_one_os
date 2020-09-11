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
 * @file        dlog.h
 *
 * @brief       The header for dlog.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-24   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __DLOG_H__
#define __DLOG_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_list.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Logger level, the number is compatible for syslog */
#define LOG_LVL_ERROR                   3           /* Error conditions */
#define LOG_LVL_WARNING                 4           /* Warning conditions */
#define LOG_LVL_INFO                    6           /* Informational */
#define LOG_LVL_DEBUG                   7           /* Debug-level messages */

/**
 ***********************************************************************************************************************
 * @struct      dlog_backend
 *
 * @brief       Define backend structure.
 ***********************************************************************************************************************
 */
struct dlog_backend
{
    os_slist_node_t list;
    char            name[OS_NAME_MAX + 1];          /* Name of log backend. */
    os_bool_t       support_color;                  /* Whether backend support color. */
    os_bool_t       support_isr;                    /* Whether backend support isr. */
    
    void (*init)  (struct dlog_backend *backend);   /* Initialize backend. */
    void (*deinit)(struct dlog_backend *backend);   /* De-initialize backend. */
    
    void (*output)(struct dlog_backend *backend,
                   os_uint16_t          level,
                   const char          *tag,
                   os_bool_t            is_raw,
                   const char          *log,
                   os_size_t            len);       /* Output log to backend */
                   
    void (*flush) (struct dlog_backend *backend);   /* Flush logs in the buffer of backend */
};
typedef struct dlog_backend dlog_backend_t;

extern os_err_t    dlog_init(void);
extern void        dlog_deinit(void);

extern os_err_t    dlog_backend_register(dlog_backend_t *backend);
extern os_err_t    dlog_backend_unregister(dlog_backend_t *backend);

extern os_err_t    dlog_global_lvl_set(os_uint16_t level);
extern os_uint16_t dlog_global_lvl_get(void);

#ifdef DLOG_USING_FILTER
extern os_err_t    dlog_tag_lvl_filter_set(const char *tag, os_uint16_t level);
extern os_err_t    dlog_tag_lvl_filter_get(const char *tag, os_uint16_t *level);
extern os_err_t    dlog_tag_lvl_filter_del(const char *tag);

extern void        dlog_global_filter_tag_set(const char *tag);
extern const char *dlog_global_filter_tag_get(void);
extern void        dlog_global_filter_tag_del(void);

extern void        dlog_global_filter_kw_set(const char *keyword);
extern const char *dlog_global_filter_kw_get(void);
extern void        dlog_global_filter_kw_del(void);
#endif /* DLOG_USING_FILTER */

extern void dlog_flush(void);
extern void dlog_hexdump(const char *tag, os_size_t width, os_uint8_t *buf, os_size_t size);
extern void dlog_voutput(os_uint16_t level, const char *tag, os_bool_t newline, const char *format, va_list args);
extern void dlog_output(os_uint16_t level, const char *tag, os_bool_t newline, const char *format, ...);
extern void dlog_raw(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __DLOG_H__ */

