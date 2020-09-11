/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2018,RT - Thread Development Team
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
 * @file        console_be.c
 *
 * @brief       Console backend realization.
 *
 * @revision
 * Date         Author          Notes
 * 2018-09-04   armink          First version.
 * 2020-03-25   OneOS Team      Format and request resource.
 ***********************************************************************************************************************
 */

#include <os_hw.h>
#include <os_device.h>
#include <os_errno.h>
#include <string.h>
#include <dlog.h>

#ifdef DLOG_BACKEND_USING_CONSOLE

static dlog_backend_t gs_console;

#ifdef DLOG_BACKEND_USING_FILESYSTEM

static dlog_backend_t gs_console_filesystem;

#endif


/**
 ***********************************************************************************************************************
 * @brief           Output log to console.
 *
 * @param[in]       backend         The backend.
 * @param[in]       level           Log level.
 * @param[in]       tag             Log tag.
 * @param[in]       is_raw          Whether is raw.
 * @param[in]       log             Log buffer.
 * @param[in]       len             Log length. 
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void dlog_console_backend_output(dlog_backend_t *backend,
                                 os_uint16_t     level,
                                 const char     *tag,
                                 os_bool_t       is_raw,
                                 const char     *log,
                                 os_size_t       len)
{
#if defined(OS_USING_DEVICE) && defined(OS_USING_CONSOLE)
    os_device_t *dev;
    
    dev = os_console_get_device();
    if (OS_NULL == dev)
    {
        os_hw_console_output(log);
    }
    else
    {
        os_uint16_t old_flag = dev->open_flag;

        dev->open_flag |= OS_DEVICE_FLAG_STREAM;
        os_device_write(dev, 0, log, len);
        dev->open_flag = old_flag;
    }
#else
    os_hw_console_output(log);
#endif

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Initialize console backend and register backend to dlog.
 *
 * @param           None.
 *
 * @return          Initialize console backend result.
 * @retval          OS_EOK          Initialize console backend success.
 * @retval          else            Initialize console backend failed.
 ***********************************************************************************************************************
 */
os_err_t dlog_console_backend_init(void)
{
    dlog_init();

    memset(&gs_console, 0, sizeof(dlog_backend_t));
    strncpy(gs_console.name, "console", OS_NAME_MAX);
    gs_console.support_isr   = OS_TRUE;
    gs_console.support_color = OS_TRUE;
    gs_console.output        = dlog_console_backend_output;
    
    (void)dlog_backend_register(&gs_console);

    return OS_EOK;
}
OS_PREV_INIT(dlog_console_backend_init);


#ifdef DLOG_BACKEND_USING_FILESYSTEM

#ifdef OS_USING_SHELL
#include <shell.h>
#endif 

extern void dlog_console_backend_filesystem_output(dlog_backend_t *backend,
                                                                   os_uint16_t     level,
                                                                   const char     *tag,
                                                                   os_bool_t       is_raw,
                                                                   const char     *log,
                                                                   os_size_t       len);

extern void dlog_console_backend_filesystem_init(struct dlog_backend *backend);

extern void dlog_console_backend_filesystem_deinit(struct dlog_backend *backend);

/**
 ***********************************************************************************************************************
 * @brief           Initialize console backend and register filesystem backend to dlog.
 *
 * @param           None.
 *
 * @return          Initialize console backend result.
 * @retval          OS_EOK          Register console backend success.
 * @retval          else            Register console backend failed.
 ***********************************************************************************************************************
 */ 
os_err_t dlog_backend_register_filesystem()
{
    memset(&gs_console_filesystem, 0, sizeof(dlog_backend_t));
    strncpy(gs_console_filesystem.name, "backend_fs", OS_NAME_MAX);
    gs_console_filesystem.support_isr   = OS_TRUE;
    gs_console_filesystem.support_color = OS_FALSE;
    gs_console_filesystem.output        = dlog_console_backend_filesystem_output;
    gs_console_filesystem.init          = dlog_console_backend_filesystem_init;
    gs_console_filesystem.deinit        = dlog_console_backend_filesystem_deinit;
    
    (void)dlog_backend_register(&gs_console_filesystem);

    return OS_EOK;
}
#ifdef OS_USING_SHELL
SH_CMD_EXPORT(dlog_backend_fs_register, dlog_backend_register_filesystem, "register filesystem operation to dlog backend");
#endif

/**
 ***********************************************************************************************************************
 * @brief           Unregister filesystem backend to dlog.
 *
 * @param           None.
 *
 * @return          Uninitialize console backend result.
 * @retval          OS_EOK          Unregister console backend success.
 * @retval          else            Unregister console backend failed.
 ***********************************************************************************************************************
 */ 
os_err_t dlog_backend_unregister_filesystem()
{
    (void)dlog_backend_unregister(&gs_console_filesystem);

    memset(&gs_console_filesystem, 0, sizeof(dlog_backend_t));

    return OS_EOK;
}
#ifdef OS_USING_SHELL
SH_CMD_EXPORT(dlog_backend_fs_unregister, dlog_backend_unregister_filesystem, "unregister filesystem operation to dlog backend");
#endif

#endif
#endif /* DLOG_BACKEND_USING_CONSOLE */

