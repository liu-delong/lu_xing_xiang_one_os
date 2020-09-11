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
 * @file        syscall_lseek.c
 *
 * @brief       This file provides an adaptation for the file seek interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-14   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#ifdef OS_USING_VFS
#include <vfs_posix.h>
#endif
#include <yfuns.h>

#pragma module_name = "?__lseek"
long __lseek(int handle, long offset, int whence)
{
    if ((_LLIO_STDOUT == handle) || (_LLIO_STDERR == handle) || (_LLIO_STDIN == handle))
    {
        return _LLIO_ERROR;
    }

#ifdef OS_USING_VFS
    return lseek(handle, offset, whence);
#else
    return _LLIO_ERROR;
#endif
}
