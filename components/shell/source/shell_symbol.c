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
 * @file        shell_symbol.c
 *
 * @brief       Shell symbol management.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_dbg.h>
#include <string.h>

#ifdef OS_USING_SHELL
#include <shell.h>
#include "shell_internal.h"

static sh_syscall_t *gs_syscall_table_begin  = OS_NULL;
static sh_syscall_t *gs_syscall_table_end    = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           Initialize shell symbal table.
 *
 * @param[in]       begin           The begin address of symbal table.
 * @param[in]       end             The end address of symbal table.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void sh_symbol_table_init(const void *begin, const void *end)
{
    gs_syscall_table_begin = (sh_syscall_t *)begin;
    gs_syscall_table_end   = (sh_syscall_t *)end;
}

/**
 ***********************************************************************************************************************
 * @brief           Get symbol table of shell.
 *
 * @param[out]      table_begin     The begin address's pointer of symbal table.
 * @param[out]      table_end       The end address's pointer of symbal table.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void sh_get_syscall_table(sh_syscall_t **table_begin, sh_syscall_t **table_end)
{
    *table_begin = gs_syscall_table_begin;
    *table_end   = gs_syscall_table_end;

    return;
}
#endif /* OS_USING_SHELL */

