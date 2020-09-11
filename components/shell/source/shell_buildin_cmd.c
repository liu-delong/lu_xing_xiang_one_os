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
 * @file        shell_buildin_cmd.c
 *
 * @brief       Buildin command of shell.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_util.h>
#include <os_errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef OS_USING_SHELL
#include <shell.h>
#include "shell_internal.h"

/**
 ***********************************************************************************************************************
 * @brief           Provide help information.
 *
 * @param[in]       argc            Command arguments count.
 * @param[in]       argv            Command arguments
 *
 * @return          The state of executting command.
 * @retval          OS_EOK          Execute command success.
 * @retval          else            Execute command failed.
 ***********************************************************************************************************************
 */
os_err_t sh_help(os_int32_t argc, char **argv)
{
    sh_syscall_t   *syscall_tmp;
    sh_syscall_t   *table_begin;
    sh_syscall_t   *table_end;

    SH_PRINT("OneOS shell commands:");

    sh_get_syscall_table(&table_begin, &table_end);

    for (syscall_tmp = table_begin; syscall_tmp < table_end; syscall_tmp++)
    {
        if (strncmp(syscall_tmp->name, "__cmd_", 6))
        {
            continue;
        }

#if defined(SHELL_USING_DESCRIPTION)
        SH_PRINT("%-16s - %s", &syscall_tmp->name[6], syscall_tmp->desc);
#else
        SH_PRINT("%s", &syscall_tmp->name[6]);
#endif
    }

    return OS_EOK;
}
SH_CMD_EXPORT(help, sh_help, "Obtain help of commands");

#endif /* OS_USING_SHELL */

