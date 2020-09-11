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
 * @file        shell.h
 *
 * @brief       Header file for SHELL.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __SHELL_H__
#define __SHELL_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_util.h>

#if defined(_MSC_VER)
#pragma section("FSymTab$f",read)
#endif

/* Shell system call function pointer */
typedef os_err_t (*syscall_func_t)(os_int32_t argc, char **argv);

/* System call table */
struct sh_syscall
{
    const char*     name;       /* The name of system call */

#if defined(SHELL_USING_DESCRIPTION)
    const char*     desc;       /* Description of system call */
#endif

    syscall_func_t  func;       /* The function address of system call */
};
typedef struct sh_syscall sh_syscall_t;

#ifdef __TI_COMPILER_VERSION__
#define __TI_SH_EXPORT_FUNCTION(f)      OS_PRAGMA(DATA_SECTION(f, "FSymTab"))
#endif

#ifdef SHELL_USING_DESCRIPTION
    #ifdef _MSC_VER
        #define SH_FUNCTION_EXPORT_CMD(func, cmd, desc)                         \
            const char __fsym_##cmd##_name[] = #cmd;                            \
            const char __fsym_##cmd##_desc[] = desc;                            \
            __declspec(allocate("FSymTab$f"))                                   \
            const sh_syscall_t __fsym_##cmd =                                   \
            {                                                                   \
                __fsym_##cmd##_name,                                            \
                __fsym_##cmd##_desc,                                            \
                (syscall_func_t)func                                            \
            };
        #pragma comment(linker, "/merge:FSymTab=mytext")

    #elif defined(__TI_COMPILER_VERSION__)
        #define SH_FUNCTION_EXPORT_CMD(func, cmd, desc)                         \
            __TI_SH_EXPORT_FUNCTION(__fsym_##cmd);                              \
            const char __fsym_##cmd##_name[] = #cmd;                            \
            const char __fsym_##cmd##_desc[] = desc;                            \
            const sh_syscall_t __fsym_##cmd =                                   \
            {                                                                   \
                __fsym_##cmd##_name,                                            \
                __fsym_##cmd##_desc,                                            \
                (syscall_func_t)func                                            \
            };

    #else
        #define SH_FUNCTION_EXPORT_CMD(func, cmd, desc)                         \
            const char __fsym_##cmd##_name[] = #cmd;                            \
            const char __fsym_##cmd##_desc[] = desc;                            \
            OS_USED const sh_syscall_t __fsym_##cmd OS_SECTION("FSymTab") =     \
            {                                                                   \
                __fsym_##cmd##_name,                                            \
                __fsym_##cmd##_desc,                                            \
                (syscall_func_t)func                                            \
            };

    #endif
#else
    #ifdef _MSC_VER
        #define SH_FUNCTION_EXPORT_CMD(func, cmd, desc)                         \
            const char __fsym_##cmd##_name[] = #cmd;                            \
            __declspec(allocate("FSymTab$f"))                                   \
            const sh_syscall_t __fsym_##cmd =                                   \
            {                                                                   \
                __fsym_##cmd##_name,                                            \
                (syscall_func_t)func                                            \
            };
        #pragma comment(linker, "/merge:FSymTab=mytext")

    #elif defined(__TI_COMPILER_VERSION__)
        #define SH_FUNCTION_EXPORT_CMD(func, cmd, desc)                         \
            __TI_SH_EXPORT_FUNCTION(__fsym_##cmd);                              \
            const char __fsym_##cmd##_name[] = #cmd;                            \
            const sh_syscall_t __fsym_##cmd =                                   \
            {                                                                   \
                __fsym_##cmd##_name,                                            \
                (syscall_func_t)func                                            \
            };

    #else
        #define SH_FUNCTION_EXPORT_CMD(func, cmd, desc)                         \
            const char __fsym_##cmd##_name[] = #cmd;                            \
            OS_USED const sh_syscall_t __fsym_##cmd OS_SECTION("FSymTab")=      \
            {                                                                   \
                __fsym_##cmd##_name,                                            \
                (syscall_func_t)func                                            \
            };  
    #endif
#endif /* SHELL_USING_DESCRIPTION */

#ifdef OS_USING_SHELL
/**
 ***********************************************************************************************************************
 * @def         SH_CMD_EXPORT
 *
 * @brief       This macro exports a system function with an command name to shell.
 *
 * @param       cmd             The name of command.
 * @param       func            The name of function.
 * @param       desc            The description of command, which will show in help.
 ***********************************************************************************************************************
 */
#define SH_CMD_EXPORT(cmd, func, desc)      SH_FUNCTION_EXPORT_CMD(func, __cmd_##cmd, desc)

/**
 ***********************************************************************************************************************
 * @def         SH_PRINT
 *
 * @brief       Print macro for shell.
 *
 * @param       fmt             The format.
 * @param       ...             The arguments.
 ***********************************************************************************************************************
 */
#define SH_PRINT(fmt, ...)                  os_kprintf(fmt"\r\n", ##__VA_ARGS__)
#else
#define SH_CMD_EXPORT(cmd, func, desc)
#define SH_PRINT(fmt, ...)
#endif

#endif /* __SHELL_H__ */

