/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2020,RT-Thread Development Team
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        resource_monitor.c
 *
 * @brief       This file include cpu usage calculation and task load monitor.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-03   armink          the first version
 * 2020-07-28   OneOS Team      adapt the code to OneOS and modify bugs
 ***********************************************************************************************************************
 */
#include <oneos_config.h>

#ifdef OS_USING_SHELL

#include <shell.h>
#include <os_task.h>
#include <os_clock.h>
#include <os_hw.h>
#include <os_assert.h>
#include <os_errno.h>
#include <board.h>
#include <os_module.h>
#include <os_stddef.h>

#ifdef OS_USING_CPU_USAGE

#define CPU_USAGE_CALC_MS         1000  /* unit is ms */
#define CPU_USAGE_LOOP            100
#define CPU_USAGE_SHELL_DELAY_MS  (CPU_USAGE_CALC_MS + (CPU_USAGE_CALC_MS >> 1))

typedef struct cpu_usage
{
    volatile os_uint32_t total_count;
    volatile os_uint8_t  major;
    volatile os_uint8_t  minor;
    volatile os_bool_t   is_started;
}cpu_usage_t;

static cpu_usage_t gs_cpu_usage = {0, 0, 0, OS_FALSE};

static os_mutex_t gs_cpu_mutex;

extern os_err_t sh_list_task(os_int32_t argc, char **argv);

/**
 ***********************************************************************************************************************
 * @brief           This function start calculate the cpu uage.
 *
 * @param[in]       None.
 *
 * @return          None
 ***********************************************************************************************************************
 */
static void cpu_usage_start(void)
{
    os_base_t level;
    os_mutex_lock(&gs_cpu_mutex, OS_IPC_WAITING_FOREVER);

    level = os_hw_interrupt_disable();
    gs_cpu_usage.is_started = OS_TRUE;
    gs_cpu_usage.major = 100;
    gs_cpu_usage.minor = 0;
    os_hw_interrupt_enable(level);
}

/**
 ***********************************************************************************************************************
 * @brief           This function stop calculate the cpu uage.
 *
 * @param[in]       None.
 *
 * @return          None
 ***********************************************************************************************************************
 */
static void cpu_usage_stop(void)
{
    gs_cpu_usage.is_started = OS_FALSE;
    os_mutex_unlock(&gs_cpu_mutex);
}

/**
 ***********************************************************************************************************************
 * @brief           Initialization of stask monitor.
 *
 * @param[in]       parameter           parameter of timeout handler of task monitor.
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
static os_err_t cpu_usage_init(void)
{
    os_mutex_init(&gs_cpu_mutex, "cpu_usage", OS_IPC_FLAG_FIFO, OS_FALSE);
    return OS_EOK;
}
OS_PREV_INIT(cpu_usage_init);

/**
 ***********************************************************************************************************************
 * @brief           Show the overall memory usage.
 *
 * @param[in]       argc                argment count
 * @param[in]       argv                argment list
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
OS_WEAK os_err_t sh_list_mem(os_int32_t argc, char **argv)
{
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function get the cpu uage.
 *
 * @param[out]      major            Pointer to integral part of cpu usage.
 * @param[out]      minor            Pointer to fractional part of cpu usage.
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
os_err_t cpu_usage_get(os_uint8_t *major, os_uint8_t *minor)
{
    os_base_t level;

    OS_ASSERT(major != OS_NULL);
    OS_ASSERT(minor != OS_NULL);

    cpu_usage_start();

    if (0 == gs_cpu_usage.total_count)
    {
        os_task_mdelay(CPU_USAGE_SHELL_DELAY_MS << 1);
    }
    else
    {
        os_task_mdelay(CPU_USAGE_SHELL_DELAY_MS);
    }

    level = os_hw_interrupt_disable();
    *major = gs_cpu_usage.major;
    *minor = gs_cpu_usage.minor;
    os_hw_interrupt_enable(level);

    cpu_usage_stop();

    return OS_EOK;
}

EXPORT_SYMBOL(cpu_usage_get);

/**
 ***********************************************************************************************************************
 * @brief           Show the cpu usage.
 *
 * @param[in]       argc                argment count
 * @param[in]       argv                argment list
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
static os_err_t sh_list_cpu(os_int32_t argc, char **argv)
{   
    os_uint8_t cpu_major = 0;
    os_uint8_t cpu_minor = 0;

    cpu_usage_get(&cpu_major, &cpu_minor);
    
    os_kprintf("=============================     CPU Usage     ==============================\r\n");

    os_kprintf("CPU Usage : %3d.%02d%\r\n", cpu_major, cpu_minor);
    os_kprintf("System Freq : %d MHz\r\n", SystemCoreClock / 1000000);      

    sh_list_mem(0, OS_NULL); 

    os_kprintf("=============================   Current Task Info   ==========================\r\n");
    sh_list_task(0, OS_NULL);
    os_kprintf("------------------------------------------------------------------------------\r\n");
    return OS_EOK;
}

SH_CMD_EXPORT(list_cpu, sh_list_cpu, "list cpu usage");

/**
 ***********************************************************************************************************************
 * @brief           This function calculate the cpu uage.
 *
 * @param[in]       None.
 *
 * @return          None
 ***********************************************************************************************************************
 */
void cpu_usage_calc(void)
{
    os_tick_t            tick;
    os_base_t            level;
    os_uint32_t          count;
    volatile os_uint32_t loop;
    register os_tick_t   calc_tick;

    if (!gs_cpu_usage.is_started)
    {
        return;
    }

    calc_tick = os_tick_from_ms(CPU_USAGE_CALC_MS);

    if (0 == gs_cpu_usage.total_count)
    {
        /* get total count */
        os_enter_critical();

        tick = os_tick_get();
        while (os_tick_get() - tick < calc_tick)
        {
            gs_cpu_usage.total_count++;

            loop = 0;
            while (loop < CPU_USAGE_LOOP)
            {
                loop++;
            }
        }

        os_exit_critical();
    }

    count = 0;

    /* get CPU usage */
    tick = os_tick_get();
    while (os_tick_get() - tick < calc_tick)
    {
        count++;

        loop = 0;
        while (loop < CPU_USAGE_LOOP)
        {
            loop++;
        }
    }

    /* calculate major and minor */
    if (count < gs_cpu_usage.total_count)
    {
        count = gs_cpu_usage.total_count - count;

        level = os_hw_interrupt_disable();
        gs_cpu_usage.major = (count * 100) / gs_cpu_usage.total_count;
        gs_cpu_usage.minor = (((count * 100) % gs_cpu_usage.total_count) * 100) / gs_cpu_usage.total_count;
        os_hw_interrupt_enable(level);
    }
    else
    {
        gs_cpu_usage.total_count = count;

        level = os_hw_interrupt_disable();
        gs_cpu_usage.major = 0;
        gs_cpu_usage.minor = 0;
        os_hw_interrupt_enable(level);
    }
}
EXPORT_SYMBOL(cpu_usage_calc);

#endif

#ifdef OS_USING_TASK_MONITOR

#include <os_task.h>
#include <string.h>
#include <os_stddef.h>

#define TASK_MONITOR_MAX_TASK      5
#define TASK_MONITOR_MAX_RECORD    5
#define TASK_MONITOR_TIMER_MS      2000   /* unit is ms */

typedef struct task_monitor_info
{
    char       name[OS_NAME_MAX + 1];
    os_uint8_t current_priority;
}task_monitor_info_t;

typedef struct task_monitor
{
    task_monitor_info_t  info[TASK_MONITOR_MAX_TASK];
    os_uint8_t           num;
    os_tick_t            tick;
}task_monitor_t;

static struct os_timer     gs_monitor_timer;
static volatile os_uint8_t gs_monitor_tail = 0;
static volatile os_bool_t  gs_monitor_is_full = OS_FALSE;
static task_monitor_t      gs_monitor[TASK_MONITOR_MAX_RECORD];

/**
 ***********************************************************************************************************************
 * @brief           Timeout handler of task monitor.
 *
 * @param[in]       parameter           parameter of timeout handler of task monitor.
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
 static void task_monitor_timeout_handler(void *parameter)
{
    os_uint32_t  i;
    os_task_t   *task_arr[TASK_MONITOR_MAX_TASK];

    gs_monitor[gs_monitor_tail].num = os_task_get_by_priority_order(task_arr, TASK_MONITOR_MAX_TASK);
    gs_monitor[gs_monitor_tail].tick = os_tick_get();
    
    for (i = 0; i < gs_monitor[gs_monitor_tail].num; i++)
    {
        gs_monitor[gs_monitor_tail].info[i].current_priority = task_arr[i]->current_priority;
        strncpy(gs_monitor[gs_monitor_tail].info[i].name, task_arr[i]->parent.name, OS_NAME_MAX + 1);
    }

    gs_monitor_tail++;
    if (gs_monitor_tail >= TASK_MONITOR_MAX_RECORD) 
    {
        if (!gs_monitor_is_full)
        {
            gs_monitor_is_full = OS_TRUE;
        }

        gs_monitor_tail = 0;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Initialization of stask monitor.
 *
 * @param[in]       parameter           parameter of timeout handler of task monitor.
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
static os_err_t task_monitor_init(void)
{
    memset(gs_monitor, 0, sizeof(gs_monitor));

    os_timer_init(&gs_monitor_timer,
                 "task_monitor",
                  task_monitor_timeout_handler,
                  OS_NULL,
                  os_tick_from_ms(TASK_MONITOR_TIMER_MS),
                  OS_TIMER_FLAG_PERIODIC | OS_TIMER_FLAG_HARD_TIMER);

    os_timer_start(&gs_monitor_timer);
    return OS_EOK;
}
OS_PREV_INIT(task_monitor_init);

/**
 ***********************************************************************************************************************
 * @brief           Show current system running load information.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void sh_task_monitor(void)
{   
    os_base_t      level;
    os_uint8_t     tail;
    os_uint8_t     head;
    os_uint8_t     count;
    os_bool_t      is_full;
    task_monitor_t monitor[TASK_MONITOR_MAX_RECORD];

    level = os_hw_interrupt_disable();
    tail = gs_monitor_tail;
    is_full = gs_monitor_is_full;
    memcpy(monitor, gs_monitor, sizeof(gs_monitor));
    os_hw_interrupt_enable(level);

    if (is_full)
    {
        count = TASK_MONITOR_MAX_RECORD;
        head = tail;
    }
    else
    {
        count = tail;
        head = 0;
    }

    os_kprintf("====================  Task Load Monitor  ====================\r\n");
    os_kprintf("%-*.s | %-*.s | ready task list..\n", OS_NAME_MAX, "Tick", OS_NAME_MAX+5, "Running");

    for (; count > 0; count--)
    {
        char       tmp[30];
        os_uint8_t i = 0;

        os_snprintf(tmp, sizeof(tmp), "%s(%d)", monitor[head].info[i].name, monitor[head].info[i].current_priority);
        os_kprintf("%-15u | %-*.*s | ", monitor[head].tick , OS_NAME_MAX + 5, OS_NAME_MAX + 5, tmp);

        i++;

        for (; i < monitor[head].num; i++)
        {
            os_kprintf("%s(%02d) ",
                       monitor[head].info[i].name,
                       monitor[head].info[i].current_priority);
        }

        os_kprintf("\n");

        head++;
        if (head >= TASK_MONITOR_MAX_RECORD)
        {
            head = 0;
        }
    }

    os_kprintf("-------------------------------------------------------------\r\n");
}

SH_CMD_EXPORT(list_task_monitor, sh_task_monitor, "list current system running load information")

#endif

#endif
