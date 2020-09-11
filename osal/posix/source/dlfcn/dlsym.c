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
 * @file        dlsym.c
 *
 * @brief       This file implements the posix dlsym functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <os_module.h>
#include <os_assert.h>

void* dlsym(void *handle, const char* symbol)
{
    int i;
    struct os_module *module;
    
    OS_ASSERT(handle != OS_NULL);

    module = (struct os_module *)handle;

    for(i=0; i<module->nsym; i++)
    {
        if (strcmp(module->symtab[i].name, symbol) == 0)
        {
            return (void*)module->symtab[i].addr;
        }

    }

    return OS_NULL;
}
EXPORT_SYMBOL(dlsym)
