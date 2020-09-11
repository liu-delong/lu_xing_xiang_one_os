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
 * @file        dlclose.c
 *
 * @brief       This file implements the posix dlclose functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_errno.h>
#include <os_assert.h>
#include <os_module.h>

int dlclose(void *handle)
{
    struct os_module *module;
    int rt;

    OS_ASSERT(handle != OS_NULL);

    rt = OS_EOK;
    module = (struct os_module *)handle;

    os_enter_critical();
    if (module->nref == 0)
    {
        os_exit_critical();
        return OS_ERROR;
    }
    
    module->nref--;
    
    if (module->nref == 0)
    {
        os_exit_critical();

        rt = os_module_destroy(module);
    }
    else
    {
        os_exit_critical();
    }

    return rt;
}
EXPORT_SYMBOL(dlclose)
