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
 * @file        shell_main.c
 *
 * @brief       Main routine of shell.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <os_errno.h>
#include <os_task.h>
#include <os_hw.h>
#include <shell.h>
#include <os_util.h>
#include <os_memory.h>
#include <os_assert.h>
#include <os_dbg.h>
#include <os_device.h>
#include <string.h>
#include <stdio.h>

#include "shell_internal.h"

#ifdef OS_USING_SHELL

#if (SHELL_TASK_PRIORITY > OS_TASK_PRIORITY_MAX - 1)
#error "Shell task priority is greater than or equal to OS_TASK_PRIORITY_MAX"
#endif

#ifndef OS_USING_HEAP
static os_task_t             _gs_shell_task;
OS_ALIGN(OS_ALIGN_SIZE)
static char                  gs_shell_task_stack[SHELL_TASK_STACK_SIZE];

static shell_ctrl_info_t     _gs_shell;
#endif

static os_task_t            *gs_shell_task          = OS_NULL;
static shell_ctrl_info_t    *gs_shell               = OS_NULL;

static char                 *gs_shell_prompt_custom = OS_NULL;

#ifdef OS_USING_HEAP
/**
 ***********************************************************************************************************************
 * @brief           Set prompt for shell.
 *
 * @param[in]       prompt          New prompt string.
 *
 * @return          Set prompt result.
 * @retval          OS_EOK          Set prompt success.
 * @retval          else            Set prompt failed.
 ***********************************************************************************************************************
 */
os_err_t sh_set_prompt(const char *prompt)
{
    if (gs_shell_prompt_custom)
    {
        os_free(gs_shell_prompt_custom);
        gs_shell_prompt_custom = OS_NULL;
    }

    /* Strdup */
    if (prompt)
    {
        gs_shell_prompt_custom = os_malloc(strlen(prompt) + 1);
        if (gs_shell_prompt_custom)
        {
            strcpy(gs_shell_prompt_custom, prompt);
        }
    }

    return OS_EOK;
}
#endif /* OS_USING_HEAP */

#if defined(OS_USING_VFS)
#include <vfs_posix.h>
#endif

/**
 ***********************************************************************************************************************
 * @brief           Get prompt of shell.
 *
 * @param           None.
 *
 * @return          The prompt string.
 ***********************************************************************************************************************
 */
const char *sh_get_prompt(void)
{
#define _PROMPT     "sh"
    static char s_shell_prompt[OS_LOG_BUFF_SIZE + 1];

    /* check prompt mode */
    if (!gs_shell->enable_prompt)
    {
        s_shell_prompt[0] = '\0';
        return s_shell_prompt;
    }

    if (gs_shell_prompt_custom)
    {
        strncpy(s_shell_prompt, gs_shell_prompt_custom, sizeof(s_shell_prompt) - 1);
        s_shell_prompt[sizeof(s_shell_prompt) - 1] = '\0';

        return s_shell_prompt;
    }

    strcpy(s_shell_prompt, _PROMPT);

#if defined(OS_USING_VFS) && defined(VFS_USING_WORKDIR)
    strcat(s_shell_prompt, " ");

    /* Get current working directory */
    getcwd(&s_shell_prompt[strlen(s_shell_prompt)], OS_LOG_BUFF_SIZE - strlen(s_shell_prompt));
#endif

    strcat(s_shell_prompt, ">");

    return s_shell_prompt;
}

#ifndef OS_USING_POSIX
static os_err_t sh_rx_ind(os_device_t *dev, os_size_t size)
{
    OS_ASSERT(OS_NULL != gs_shell);

    /* Release semaphore to let shell task rx data */
    os_sem_post(&gs_shell->rx_sem);;

    return OS_EOK;
}

static void sh_set_device(const char *device_name)
{
    os_device_t *dev;
    os_err_t    ret;

    OS_ASSERT(OS_NULL != gs_shell);

    dev = os_device_find(device_name);
    if (OS_NULL == dev)
    {
        os_kprintf("Can not find device: %s\r\n", device_name);
        return;
    }
    
    if (dev == gs_shell->device)
    {
        return;
    }
    
    /* Open this device and set the new device in shell */

    ret = os_device_open(dev, OS_DEVICE_OFLAG_RDWR | OS_DEVICE_FLAG_INT_RX | OS_DEVICE_FLAG_STREAM);
    if (OS_EOK == ret)
    {
        if (OS_NULL != gs_shell->device)
        {
            /* Close old shell device */
            os_device_close(gs_shell->device);
            os_device_set_rx_indicate(gs_shell->device, OS_NULL);
        }

        /* Clear line buffer before switch to new device */
        memset(gs_shell->line, 0, sizeof(gs_shell->line));
        gs_shell->line_curpos   = 0;
        gs_shell->line_position = 0;

        gs_shell->device = dev;
        os_device_set_rx_indicate(dev, sh_rx_ind);
    }
    else
    {
        os_kprintf("Open device(%s) failed\r\n", device_name);
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Get device of shell.
 *
 * @param           None.
 *
 * @return          Device name of shell.
 ***********************************************************************************************************************
 */
const char *sh_get_device(void)
{
    OS_ASSERT(OS_NULL != gs_shell);
    
    return gs_shell->device->parent.name;
}
#endif /* Not define OS_USING_POSIX */

static os_int32_t sh_get_char(void)
{
#ifdef OS_USING_POSIX
    return getchar();
#else
    char ch;

    OS_ASSERT(OS_NULL != gs_shell);

    ch = 0;
    while (os_device_read(gs_shell->device, -1, &ch, 1) != 1)
    {
        os_sem_wait(&gs_shell->rx_sem, OS_IPC_WAITING_FOREVER);
    }
    
    return (int)ch;   
#endif
}

#ifdef SHELL_USING_HISTORY
static os_bool_t sh_handle_history(shell_ctrl_info_t *shell)
{
#if defined(_WIN32)
    os_int32_t i;
    
    os_kprintf("\r");

    for (i = 0; i <= 60; i++)
    {
        putchar(' ');
    }
    
    os_kprintf("\r");
#else
    os_kprintf("\033[2K\r");
#endif

    os_kprintf("%s%s", sh_get_prompt(), shell->line);
    
    return OS_FALSE;
}

static void sh_push_history(shell_ctrl_info_t *shell)
{
    if (0 != shell->line_position)
    {
        /* Push history */
        if (shell->history_count >= SHELL_HISTORY_LINES)
        {
            /* If current cmd is same as last cmd, don't push */
            if (memcmp(&shell->cmd_history[SHELL_HISTORY_LINES - 1], shell->line, SHELL_CMD_SIZE + 1))
            {
                /* Move history */
                os_int32_t index;
                
                for (index = 0; index < SHELL_HISTORY_LINES - 1; index++)
                {
                    memcpy(&shell->cmd_history[index][0],
                           &shell->cmd_history[index + 1][0],
                           SHELL_CMD_SIZE + 1);
                }
                
                memset(&shell->cmd_history[index][0], 0, SHELL_CMD_SIZE + 1);
                memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

                /* It's the maximum history */
                shell->history_count = SHELL_HISTORY_LINES;
            }
        }
        else
        {
            /* If current cmd is same as last cmd, don't push */
            if ((0 == shell->history_count)
                || memcmp(&shell->cmd_history[shell->history_count - 1], shell->line, SHELL_CMD_SIZE + 1))
            {
                shell->current_history = shell->history_count;
                memset(&shell->cmd_history[shell->history_count][0], 0, SHELL_CMD_SIZE + 1);
                memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

                /* increase count and set current history position */
                shell->history_count++;
            }
        }
    }
    
    shell->current_history = shell->history_count;
}
#endif /* SHELL_USING_HISTORY */

#ifdef SHELL_USING_AUTH
static os_err_t sh_set_password(const char *password)
{
    os_ubase_t level;
    os_size_t  pw_len;

    pw_len = strlen(password);

    if ((pw_len < SHELL_PASSWORD_MIN) || (pw_len > SHELL_PASSWORD_MAX))
    {
        os_kprintf("Invalid password length(%u) of shell\r\n", pw_len);
        return OS_EINVAL;
    }
    
    level = os_hw_interrupt_disable();
    strncpy(gs_shell->password, password, SHELL_PASSWORD_MAX);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static const char *sh_get_password(void)
{
    return gs_shell->password;
}

static void sh_wait_auth(void)
{
    int       ch;
    os_bool_t input_shell;
    char      password[SHELL_PASSWORD_MAX];
    os_size_t cur_pos;

    memset(password, 0, sizeof(password));
    input_shell = OS_FALSE;
    cur_pos     = 0;

    /* Password not set */
    if (!strlen(sh_get_password()))
    {
        return;
    }
    
    while (1)
    {
        os_kprintf("Password for login: ");
        
        while (!input_shell)
        {
            while (1)
            {
                /* read one character from device */
                ch = sh_get_char();
                if (ch < 0)
                {
                    continue;
                }

                if ((ch >= ' ') && (ch <= '~') && (cur_pos < SHELL_PASSWORD_MAX))
                {
                    /* Change the printable characters to '*' */
                    os_kprintf("*");
                    password[cur_pos++] = ch;
                }
                else if ((ch == '\b') && (cur_pos > 0))
                {
                    /* Backspace */
                    password[cur_pos] = '\0';
                    cur_pos--;
                    os_kprintf("\b \b");
                }
                else if ((ch == '\r') || (ch == '\n'))
                {
                    os_kprintf("\r\n");
                    input_shell = OS_TRUE;
                    break;
                }
            }
        }
        
        if (!strncmp(gs_shell->password, password, SHELL_PASSWORD_MAX))
        {
            return;
        }
        else
        {
            /* Authentication failed, delay 2S for retry */
            os_task_mdelay(200);
            
            os_kprintf("Sorry, try again.\r\n");
            
            cur_pos = 0;
            input_shell = OS_FALSE;
            memset(password, 0, sizeof(password));
        }
    }
}
#endif /* SHELL_USING_AUTH */

static void sh_task_entry(void *parameter)
{
    os_int32_t ch;

#ifndef OS_USING_POSIX
    /* set console device as shell device */
    if (gs_shell->device == OS_NULL)
    {
        os_device_t *console;

        console = os_console_get_device();
        if (console)
        {
            sh_set_device(console->parent.name);
        }
    }
#endif

#ifdef SHELL_USING_AUTH
    const char *password;
    os_err_t    ret;

    /* Set the default password when the password isn't setting */
    password = sh_get_password();
    if (!strlen(password) && strlen(SHELL_DEFAULT_PASSWORD))
    {
        ret = sh_set_password(SHELL_DEFAULT_PASSWORD);
        if (OS_EOK != ret)
        {
            os_kprintf("Shell password set failed.\r\n");
        }
    }
    
    /* Waiting authenticate success */
    sh_wait_auth();
#endif /* SHELL_USING_AUTH */

    os_kprintf(sh_get_prompt());

    while (1)
    {
        ch = sh_get_char();
        if (ch < 0)
        {
            continue;
        }

        /*
         * handle control key
         * up key  : 0x1b 0x5b 0x41
         * down key: 0x1b 0x5b 0x42
         * right key:0x1b 0x5b 0x43
         * left key: 0x1b 0x5b 0x44
         */
        if (ch == 0x1b)
        {
            gs_shell->stat = SHELL_WAIT_SPEC_KEY;
            continue;
        }
        else if (gs_shell->stat == SHELL_WAIT_SPEC_KEY)
        {
            if (ch == 0x5b)
            {
                gs_shell->stat = SHELL_WAIT_FUNC_KEY;
                continue;
            }

            gs_shell->stat = SHELL_WAIT_NORMAL;
        }
        else if (gs_shell->stat == SHELL_WAIT_FUNC_KEY)
        {
            gs_shell->stat = SHELL_WAIT_NORMAL;

            if (ch == 0x41)     /* Up key */
            {
#ifdef SHELL_USING_HISTORY
                /* Prev history */
                if (gs_shell->current_history > 0)
                {
                    gs_shell->current_history--;
                }
                else
                {
                    gs_shell->current_history = 0;
                    continue;
                }

                /* Copy the history command */
                memcpy(gs_shell->line,
                       &gs_shell->cmd_history[gs_shell->current_history][0],
                       SHELL_CMD_SIZE + 1);
                gs_shell->line_curpos   = strlen(gs_shell->line);
                gs_shell->line_position = strlen(gs_shell->line);

                sh_handle_history(gs_shell);
#endif
                continue;
            }
            else if (ch == 0x42)    /* Down key */
            {
#ifdef SHELL_USING_HISTORY
                /* Next history */
                if (gs_shell->current_history < gs_shell->history_count - 1)
                {
                    gs_shell->current_history++;
                }
                else
                {
                    /* Set to the end of history */
                    if (gs_shell->history_count != 0)
                    {
                        gs_shell->current_history = gs_shell->history_count - 1;
                    }
                    else
                    {
                        continue;
                    }
                }

                memcpy(gs_shell->line,
                       &gs_shell->cmd_history[gs_shell->current_history][0],
                       SHELL_CMD_SIZE + 1);
                gs_shell->line_curpos   = strlen(gs_shell->line);
                gs_shell->line_position = strlen(gs_shell->line);

                sh_handle_history(gs_shell);
#endif
                continue;
            }
            else if (ch == 0x44) /* Left key */
            {
                if (gs_shell->line_curpos)
                {
                    os_kprintf("\b");
                    gs_shell->line_curpos--;
                }

                continue;
            }
            else if (ch == 0x43) /* Right key */
            {
                if (gs_shell->line_curpos < gs_shell->line_position)
                {
                    os_kprintf("%c", gs_shell->line[gs_shell->line_curpos]);
                    gs_shell->line_curpos++;
                }

                continue;
            }
        }

        /* Received null or error */
        if ((ch == '\0') || (ch == 0xFF))
        {
            continue;
        }
        /* Handle tab key */
        else if (ch == '\t')
        {     
            sh_auto_complete(&gs_shell->line[0], sizeof(gs_shell->line));

            /* Re-calculate position */
            gs_shell->line_curpos   = strlen(gs_shell->line);
            gs_shell->line_position = strlen(gs_shell->line);
            
            continue;
        }
        /* Handle backspace key */
        else if ((ch == 0x7f) || (ch == 0x08))
        {
            /* Note that shell->line_curpos >= 0 */
            if (gs_shell->line_curpos == 0)
            {
                continue;
            }
            
            gs_shell->line_position--;
            gs_shell->line_curpos--;

            if (gs_shell->line_position > gs_shell->line_curpos)
            {
                int i;

                memmove(&gs_shell->line[gs_shell->line_curpos],
                        &gs_shell->line[gs_shell->line_curpos + 1],
                        gs_shell->line_position - gs_shell->line_curpos);
                gs_shell->line[gs_shell->line_position] = '\0';

                os_kprintf("\b%s  \b", &gs_shell->line[gs_shell->line_curpos]);

                /* Move the cursor to the origin position */
                for (i = gs_shell->line_curpos; i <= gs_shell->line_position; i++)
                {
                    os_kprintf("\b");
                }
            }
            else
            {
                os_kprintf("\b \b");
                gs_shell->line[gs_shell->line_position] = '\0';
            }

            continue;
        }

        /* Handle end of line, break */
        if ((ch == '\r') || (ch == '\n'))
        {
#ifdef SHELL_USING_HISTORY
            sh_push_history(gs_shell);
#endif

            if (gs_shell->enable_echo)
            {
                os_kprintf("\r\n");
            }

            sh_exec(gs_shell->line, gs_shell->line_position);
            
            os_kprintf(sh_get_prompt());
            memset(gs_shell->line, 0, sizeof(gs_shell->line));
            gs_shell->line_curpos   = 0;
            gs_shell->line_position = 0;
            
            continue;
        }

        /* It's a large line, discard it */
        if (gs_shell->line_position >= SHELL_CMD_SIZE)
        {
            continue;
        }
        
        /* Normal character */  
        if (gs_shell->line_curpos < gs_shell->line_position)
        {
            os_int32_t i;

            memmove(&gs_shell->line[gs_shell->line_curpos + 1],
                    &gs_shell->line[gs_shell->line_curpos],
                    gs_shell->line_position - gs_shell->line_curpos);

            gs_shell->line[gs_shell->line_curpos] = ch;
            
            if (gs_shell->enable_echo)
            {
                os_kprintf("%s", &gs_shell->line[gs_shell->line_curpos]);
            }
            
            /* Move the cursor to new position */
            for (i = gs_shell->line_curpos; i < gs_shell->line_position; i++)
            {
                os_kprintf("\b");
            }
        }
        else
        {
            gs_shell->line[gs_shell->line_position] = ch;
            if (gs_shell->enable_echo)
            {
                os_kprintf("%c", ch);
            }
        }

        ch = 0;
        gs_shell->line_position ++;
        gs_shell->line_curpos++;
    } /* End of getting char from device */
}

#if defined(__ICCARM__) || defined(__ICCRX__)   /* for IAR compiler */
#pragma section="FSymTab"
#elif defined(__ADSPBLACKFIN__)                 /* for VisaulDSP++ Compiler*/
extern "asm" int __fsymtab_start;
extern "asm" int __fsymtab_end;
#elif defined(_MSC_VER)                         /* for MSC Compiler*/
#pragma section("FSymTab$a", read)
const char __fsym_begin_name[] = "__start";
const char __fsym_begin_desc[] = "begin of shell";
__declspec(allocate("FSymTab$a")) const sh_syscall_t __fsym_begin =
{
    __fsym_begin_name,
    __fsym_begin_desc,
    OS_NULL
};

#pragma section("FSymTab$z", read)
const char __fsym_end_name[] = "__end";
const char __fsym_end_desc[] = "end of shell";
__declspec(allocate("FSymTab$z")) const sh_syscall_t __fsym_end =
{
    __fsym_end_name,
    __fsym_end_desc,
    OS_NULL
};
#endif

static os_err_t sh_system_init(void)
{
    os_err_t ret;

    (void)ret;
#if defined(__CC_ARM) || defined(__CLANG_ARM)                   /* ARM C Compiler */
    extern const int FSymTab$$Base;
    extern const int FSymTab$$Limit;
    sh_symbol_table_init(&FSymTab$$Base, &FSymTab$$Limit);
#elif defined(__ICCARM__) || defined(__ICCRX__)                /* for IAR Compiler */
    sh_symbol_table_init(__section_begin("FSymTab"), __section_end("FSymTab"));
    
#elif defined(__GNUC__) || defined(__TI_COMPILER_VERSION__)    /* GNU GCC Compiler and TI CCS */
    extern const int __fsymtab_start;
    extern const int __fsymtab_end;
    sh_symbol_table_init(&__fsymtab_start, &__fsymtab_end);
    
#elif defined(__ADSPBLACKFIN__) /* for VisualDSP++ Compiler */
    sh_symbol_table_init(&__fsymtab_start, &__fsymtab_end);
    
#elif defined(_MSC_VER)
    unsigned int *ptr_begin, *ptr_end;

    if (gs_shell)
    {
        os_kprintf("Shell already init.\r\n");
        return OS_EOK;
    }

    ptr_begin = (unsigned int *)&__fsym_begin;
    ptr_begin += (sizeof(struct sh_syscall) / sizeof(unsigned int));
    while (*ptr_begin == 0)
    {
        ptr_begin++;
    }
    
    ptr_end = (unsigned int *) &__fsym_end;
    ptr_end--;
    while (*ptr_end == 0)
    {
        ptr_end--;
    }
    
    sh_symbol_table_init(ptr_begin, ptr_end);
#endif

#ifdef OS_USING_HEAP
    /* create or set shell structure */
    gs_shell = (shell_ctrl_info_t *)os_calloc(1, sizeof(shell_ctrl_info_t));
    if (OS_NULL == gs_shell)
    {
        os_kprintf("No memory for shell\r\n");
        return OS_ENOMEM;
    }
    
    gs_shell_task = os_task_create(SHELL_TASK_NAME,
                                   sh_task_entry,
                                   OS_NULL,
                                   SHELL_TASK_STACK_SIZE,
                                   SHELL_TASK_PRIORITY,
                                   10);
    if (!gs_shell_task)
    {
        os_kprintf("Create shell task failed\r\n");
        
        os_free(gs_shell);
        gs_shell = OS_NULL;
        
        return OS_ERROR;
    }
#else
    gs_shell      = &_gs_shell;
    gs_shell_task = &_gs_shell_task;
    
    ret = os_task_init(gs_shell_task,
                       SHELL_TASK_NAME,
                       sh_task_entry,
                       OS_NULL,
                       gs_shell_task_stack,
                       SHELL_TASK_STACK_SIZE,
                       SHELL_TASK_PRIORITY,
                       10);
    OS_ASSERT(OS_EOK == ret);
#endif /* OS_USING_HEAP */

    memset(gs_shell, 0, sizeof(shell_ctrl_info_t));

    ret = os_sem_init(&gs_shell->rx_sem, "shrx", 0, OS_IPC_FLAG_FIFO);
    OS_ASSERT(OS_EOK == ret);

    gs_shell->stat = SHELL_WAIT_NORMAL;

#ifndef SHELL_ECHO_DISABLE_DEFAULT
    gs_shell->enable_echo = OS_TRUE;
#else
    gs_shell->enable_echo = OS_FALSE;
#endif

    gs_shell->enable_prompt = OS_TRUE;

    os_task_startup(gs_shell_task);
    
    return OS_EOK;
}
OS_APP_INIT(sh_system_init);

#endif /* OS_USING_SHELL */

