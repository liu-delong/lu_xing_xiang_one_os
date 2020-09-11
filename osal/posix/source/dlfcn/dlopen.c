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
 * @file        dlopen.c
 *
 * @brief       This file implements the posix dlopen functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <os_assert.h>
#include <os_module.h>

#include <elf.h>

#define MODULE_ROOT_DIR     "/modules"

void* dlopen(const char *filename, int flags)
{
    struct os_module *module;
    char *fullpath;
    const char*def_path = MODULE_ROOT_DIR;

    /* check parameters */
    OS_ASSERT(filename != OS_NULL);

    if (filename[0] != '/') /* it's a relative path, prefix with MODULE_ROOT_DIR */
    {
        fullpath = malloc(strlen(def_path) + strlen(filename) + 2);

        /* join path and file name */
        snprintf(fullpath, strlen(def_path) + strlen(filename) + 2,
                 "%s/%s", def_path, filename);
    }
    else
    {
        fullpath = (char*)filename; /* absolute path, use it directly */
    }

    os_enter_critical();

    /* find in module list */
    module = os_module_find(fullpath);

    if(module != OS_NULL) 
    {
        os_exit_critical();
        module->nref++;
    }
    else 
    {
        os_exit_critical();
        module = elf_object_load(fullpath);
    }

    if(fullpath != filename)
    {
        free(fullpath);
    }

    return (void*)module;
}
EXPORT_SYMBOL(dlopen);
