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
 * @file        syscall_open.c
 *
 * @brief       This file provides an adaptation for the file open interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-14   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <yfuns.h>
#ifdef OS_USING_VFS
#include <vfs_posix.h>
#endif

#pragma module_name = "?__open"

int __open(const char *filename, int mode)
{
#ifndef OS_USING_VFS
    return _LLIO_ERROR;
#else
    int handle;
    int open_mode = O_RDONLY;

    if (mode & _LLIO_CREAT)
    {
        open_mode |= O_CREAT;

        /* Check what we should do with it if it exists. */
        if (mode & _LLIO_APPEND)
        {
            /* Append to the existing file. */
            open_mode |= O_APPEND;
        }

        if (mode & _LLIO_TRUNC)
        {
            /* Truncate the existsing file. */
            open_mode |= O_TRUNC;
        }
    }

    if (mode & _LLIO_TEXT)
    {
        /* We didn't support text mode. */
    }

    switch (mode & _LLIO_RDWRMASK)
    {
        case _LLIO_RDONLY:
            break;

        case _LLIO_WRONLY:
            open_mode |= O_WRONLY;
            break;

        case _LLIO_RDWR:
            /* The file should be opened for both reads and writes. */
            open_mode |= O_RDWR;
            break;

        default:
            return _LLIO_ERROR;
    }

    handle = open(filename, open_mode, 0);
    if (handle < 0)
    {
        return _LLIO_ERROR;
    }

    return handle;
#endif
}
