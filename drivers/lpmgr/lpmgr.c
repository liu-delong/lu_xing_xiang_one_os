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
 * @file        lpmgr.c
 *
 * @brief       this file implements Low power management related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <stdlib.h>
#include <os_hw.h>
#include <os_clock.h>
#include <os_timer.h>
#include <os_memory.h>
#include <os_assert.h>
#include <os_errno.h>
#include <lpmgr.h>

#ifdef OS_USING_LPMGR
static struct lpmgr        gs_lpmgr;
static os_uint8_t          gs_lpmgr_default_sleep = SYS_DEFAULT_SLEEP_MODE;
static struct lpmgr_notify gs_lpmgr_notify;
static os_uint8_t          gs_lpmgr_init_flag = 0;

#define LPMGR_TICKLESS_THRESH (2)

OS_WEAK os_uint32_t lpmgr_enter_critical(os_uint8_t sleep_mode)
{
    return os_hw_interrupt_disable();
}

OS_WEAK void lpmgr_exit_critical(os_uint32_t ctx, os_uint8_t sleep_mode)
{
    os_hw_interrupt_enable(ctx);
}

static int lpmgr_device_suspend(os_uint8_t mode)
{
    int index, ret = OS_EOK;

    for (index = 0; index < gs_lpmgr.device_number; index++)
    {
        if (gs_lpmgr.lp_device[index].ops->suspend != OS_NULL)
        {
            ret = gs_lpmgr.lp_device[index].ops->suspend(gs_lpmgr.lp_device[index].device, mode);
            if (ret != OS_EOK)
                break;
        }
    }

    return ret;
}

static void lpmgr_device_resume(os_uint8_t mode)
{
    int index;

    for (index = 0; index < gs_lpmgr.device_number; index++)
    {
        if (gs_lpmgr.lp_device[index].ops->resume != OS_NULL)
        {
            gs_lpmgr.lp_device[index].ops->resume(gs_lpmgr.lp_device[index].device, mode);
        }
    }
}

static void lpmgr_device_frequency_change(os_uint8_t mode)
{
    os_uint32_t index;

    /* make the frequency change */
    for (index = 0; index < gs_lpmgr.device_number; index++)
    {
        if (gs_lpmgr.lp_device[index].ops->frequency_change != OS_NULL)
            gs_lpmgr.lp_device[index].ops->frequency_change(gs_lpmgr.lp_device[index].device, mode);
    }
}

static void lpmgr_frequency_scaling(struct lpmgr *lpm)
{
    os_base_t level;

    if (lpm->flags & LPMGR_FREQUENCY_PENDING)
    {
        level = os_hw_interrupt_disable();
        /* change system runing mode */
        lpm->ops->run(lpm, lpm->run_mode);
        /* changer device frequency */
        lpmgr_device_frequency_change(lpm->run_mode);
        lpm->flags &= ~LPMGR_FREQUENCY_PENDING;
        os_hw_interrupt_enable(level);
    }
}

static os_uint8_t lpmgr_select_sleep_mode(struct lpmgr *lpm)
{
    int        index;
    os_uint8_t mode;

    mode = gs_lpmgr_default_sleep;
    for (index = SYS_SLEEP_MODE_NONE; index < SYS_SLEEP_MODE_MAX; index++)
    {
        if (lpm->modes[index])
        {
            mode = index;
            break;
        }
    }
    lpm->sleep_mode = mode;

    return mode;
}

extern os_tick_t os_timer_next_timeout_tick(void);
extern void      os_timer_check(void);

static void lpmgr_change_sleep_mode(struct lpmgr *lpm, os_uint8_t mode)
{
    os_tick_t timeout_tick, delta_tick;
    os_base_t level;
    int       ret = OS_EOK;
    static os_uint8_t last_mode = SYS_SLEEP_MODE_MAX;

    if (last_mode == mode)
    {
        return;
    }

    last_mode = mode;

    if (mode == SYS_SLEEP_MODE_NONE)
    {
        lpm->sleep_mode = mode;
        lpm->ops->sleep(lpm, SYS_SLEEP_MODE_NONE);
    }
    else
    {
        level = lpmgr_enter_critical(mode);

        /* Notify app will enter sleep mode */
        if (gs_lpmgr_notify.notify)
            gs_lpmgr_notify.notify(SYS_ENTER_SLEEP, mode, gs_lpmgr_notify.data);

        /* Suspend all peripheral device */
        ret = lpmgr_device_suspend(mode);
        if (ret != OS_EOK)
        {
            lpmgr_device_resume(mode);
            if (gs_lpmgr_notify.notify)
                gs_lpmgr_notify.notify(SYS_EXIT_SLEEP, mode, gs_lpmgr_notify.data);
            lpmgr_exit_critical(level, mode);

            return;
        }

        /* Tickless*/
        if (lpm->timer_mask & (0x01 << mode))
        {
            timeout_tick = os_timer_next_timeout_tick();
            if (timeout_tick == OS_TICK_MAX)
            {
                if (lpm->ops->timer_start)
                {
                    lpm->ops->timer_start(lpm, OS_TICK_MAX);
                }
            }
            else
            {
                timeout_tick = timeout_tick - os_tick_get();
                /* timeout_tick = 20*100; // test sleep 20s */
                if (timeout_tick < LPMGR_TICKLESS_THRESH)
                {
                    mode = SYS_SLEEP_MODE_IDLE;
                }
                else
                {
                    lpm->ops->timer_start(lpm, timeout_tick);
                }
            }
        }

        /* enter lower power state */
        lpm->ops->sleep(lpm, mode);

        /* wake up from lower power state*/
        if (lpm->timer_mask & (0x01 << mode))
        {
            delta_tick = lpm->ops->timer_get_tick(lpm);
            lpm->ops->timer_stop(lpm);
            if (delta_tick)
            {
                os_tick_set(os_tick_get() + delta_tick);
                os_timer_check();
            }
        }

        /* resume all device */
        lpmgr_device_resume(lpm->sleep_mode);

        if (gs_lpmgr_notify.notify)
            gs_lpmgr_notify.notify(SYS_EXIT_SLEEP, mode, gs_lpmgr_notify.data);

        lpmgr_exit_critical(level, mode);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           enter corresponding power mode
 *
 * @param[in]       no param
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_low_power_manager(void)
{
    os_uint8_t mode;

    if (gs_lpmgr_init_flag == 0)
        return;

    /* CPU frequency scaling according to the runing mode settings */
    lpmgr_frequency_scaling(&gs_lpmgr);

    /* Low Power Mode Processing */
    mode = lpmgr_select_sleep_mode(&gs_lpmgr);
    lpmgr_change_sleep_mode(&gs_lpmgr, mode);
}

/**
 ***********************************************************************************************************************
 * @brief           Upper application or device driver requests the system stall in corresponding power mode
 *
 * @param[in]       mode                the parameter of run mode or sleep mode
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_request(os_uint8_t mode)
{
    os_base_t     level;
    struct lpmgr *lpm;

    if (gs_lpmgr_init_flag == 0)
        return;

    if (mode > (SYS_SLEEP_MODE_MAX - 1))
        return;

    level = os_hw_interrupt_disable();
    lpm   = &gs_lpmgr;
    if (lpm->modes[mode] < 255)
        lpm->modes[mode]++;
    os_hw_interrupt_enable(level);
}

/**
 ***********************************************************************************************************************
 * @brief           Upper application or device driver releases the system stall in corresponding power mode
 *
 * @param[in]       mode                the parameter of run mode or sleep mode
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_release(os_uint8_t mode)
{
    os_ubase_t    level;
    struct lpmgr *lpm;

    if (gs_lpmgr_init_flag == 0)
        return;

    if (mode > (SYS_SLEEP_MODE_MAX - 1))
        return;

    level = os_hw_interrupt_disable();
    lpm   = &gs_lpmgr;
    if (lpm->modes[mode] > 0)
        lpm->modes[mode]--;
    os_hw_interrupt_enable(level);
}

/**
 ***********************************************************************************************************************
 * @brief           Register a device with PM feature
 *
 * @param[in]       device              pointer of os device with lpmgr feature
 * @param[in]       ops                 pointer of lpmgr device operation function set
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_device_register(struct os_device *device, const struct os_lpmgr_device_ops *ops)
{
    os_base_t               level;
    struct os_lpmgr_device *device_lpm;

    level = os_hw_interrupt_disable();

    device_lpm = (struct os_lpmgr_device *)os_realloc(gs_lpmgr.lp_device,
                                                      (gs_lpmgr.device_number + 1) * sizeof(struct os_lpmgr_device));
    if (device_lpm != OS_NULL)
    {
        gs_lpmgr.lp_device                                = device_lpm;
        gs_lpmgr.lp_device[gs_lpmgr.device_number].device = device;
        gs_lpmgr.lp_device[gs_lpmgr.device_number].ops    = ops;
        gs_lpmgr.device_number += 1;
    }

    os_hw_interrupt_enable(level);
}

/**
 ***********************************************************************************************************************
 * @brief           Unregister device from low power manager
 *
 * @param[in]       device              pointer of device with lpmgr feature
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_device_unregister(struct os_device *device)
{
    os_ubase_t  level;
    os_uint32_t index;

    level = os_hw_interrupt_disable();

    for (index = 0; index < gs_lpmgr.device_number; index++)
    {
        if (gs_lpmgr.lp_device[index].device == device)
        {
            /* remove current entry */
            for (; index < gs_lpmgr.device_number - 1; index++)
            {
                gs_lpmgr.lp_device[index] = gs_lpmgr.lp_device[index + 1]; /* copy and move */
            }

            gs_lpmgr.lp_device[gs_lpmgr.device_number - 1].device = OS_NULL;
            gs_lpmgr.lp_device[gs_lpmgr.device_number - 1].ops    = OS_NULL;

            gs_lpmgr.device_number -= 1;
            /* break out and not touch memory */
            break;
        }
    }

    os_hw_interrupt_enable(level);
}

/**
 ***********************************************************************************************************************
 * @brief           set notification callback for application
 *
 * @param[in]       notify              pointer of notify
 * @param[in]       data                pointer of data
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_notify_set(void (*notify)(os_uint8_t event, os_uint8_t mode, void *data), void *data)
{
    gs_lpmgr_notify.notify = notify;
    gs_lpmgr_notify.data   = data;
}

/**
 ***********************************************************************************************************************
 * @brief           set default sleep mode when no pm_request
 *
 * @param[in]       sleep_mode              lpmgr mode
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_default_set(os_uint8_t sleep_mode)
{
    gs_lpmgr_default_sleep = sleep_mode;
}

static os_size_t lpmgr_device_read(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct lpmgr *lpm;
    os_size_t     length;

    length = 0;
    lpm    = (struct lpmgr *)dev;
    OS_ASSERT(lpm != OS_NULL);

    if (pos < SYS_SLEEP_MODE_MAX)
    {
        int mode;

        mode   = lpm->modes[pos];
        length = os_snprintf(buffer, size, "%d", mode);
    }

    return length;
}

static os_size_t lpmgr_device_write(struct os_device *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    unsigned char request;

    if (size)
    {
        /* get request */
        request = *(unsigned char *)buffer;
        if (request == 0x01)
        {
            os_lpmgr_request(pos);
        }
        else if (request == 0x00)
        {
            os_lpmgr_release(pos);
        }
    }

    return 1;
}

static os_err_t lpmgr_device_control(struct os_device *dev, int cmd, void *args)
{
    os_uint32_t mode;

    switch (cmd)
    {
    case LPMGR_DEVICE_CTRL_REQUEST:
        mode = (os_uint32_t)args;
        os_lpmgr_request(mode);
        break;

    case LPMGR_DEVICE_CTRL_RELEASE:
        mode = (os_uint32_t)args;
        os_lpmgr_release(mode);
        break;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           lpmgr run enter function
 *
 * @param[in]       mode            lpmgr mode
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
int os_lpmgr_run_enter(os_uint8_t mode)
{
    os_base_t     level;
    struct lpmgr *lpm;

    if (gs_lpmgr_init_flag == 0)
        return OS_EIO;

    if (mode >= SYS_RUN_MODE_MAX)
    {
        os_kprintf("invalid mode: %d\n", mode);
        return OS_EINVAL;
    }

    level = os_hw_interrupt_disable();
    lpm   = &gs_lpmgr;
    if (mode < lpm->run_mode)
    {
        /* change system runing mode */
        lpm->ops->run(lpm, mode);
        /* changer device frequency */
        lpmgr_device_frequency_change(mode);
    }
    else
    {
        lpm->flags |= LPMGR_FREQUENCY_PENDING;
    }
    lpm->run_mode = mode;
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

#ifdef OS_USING_DEVICE_OPS
const static struct os_device_ops lpm_ops = {
    OS_NULL,
    OS_NULL,
    OS_NULL,
    lpmgr_device_read,
    lpmgr_device_write,
    lpmgr_device_control,
};
#endif

/**
 ***********************************************************************************************************************
 * @brief           initialize low power manager
 *
 * @param[in]       ops             pointer of lpmgr operation function set
 * @param[in]       timer_mask      indicates which mode has timer feature
 * @param[in]       user_data       user_data
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_init(const struct os_lpmgr_ops *ops, os_uint8_t timer_mask, void *user_data)
{
    struct os_device *device;
    struct lpmgr     *lpm;

    lpm    = &gs_lpmgr;
    device = &(gs_lpmgr.parent);

    device->type        = OS_DEVICE_TYPE_PM;
    device->rx_indicate = OS_NULL;
    device->tx_complete = OS_NULL;

#ifdef OS_USING_DEVICE_OPS
    device->ops = &lpm_ops;
#else
    device->init    = OS_NULL;
    device->open    = OS_NULL;
    device->close   = OS_NULL;
    device->read    = lpmgr_device_read;
    device->write   = lpmgr_device_write;
    device->control = lpmgr_device_control;
#endif
    device->user_data = user_data;

    /* register low power manager device to the system */
    os_device_register(device, "lpm", OS_DEVICE_FLAG_RDWR);

    memset(lpm->modes, 0, sizeof(lpm->modes));
    lpm->sleep_mode = gs_lpmgr_default_sleep;
    lpm->run_mode   = SYS_DEFAULT_RUN_MODE;
    lpm->timer_mask = timer_mask;

    lpm->ops = ops;

    lpm->lp_device     = OS_NULL;
    lpm->device_number = 0;

    gs_lpmgr_init_flag = 1;
}

#ifdef OS_USING_SHELL
#include <shell.h>

static const char *gs_lpmgr_sleep_str[] = SYS_SLEEP_MODE_NAMES;
static const char *gs_lpmgr_run_str[]   = SYS_RUN_MODE_NAMES;

static void lpmgr_release_mode(int argc, char **argv)
{
    int mode = 0;
    if (argc >= 2)
    {
        mode = atoi(argv[1]);
    }

    os_lpmgr_release(mode);
}
SH_CMD_EXPORT(power_release, lpmgr_release_mode, "release power management mode");

static void lpmgr_request_mode(int argc, char **argv)
{
    int mode = 0;
    if (argc >= 2)
    {
        mode = atoi(argv[1]);
    }

    os_lpmgr_request(mode);
}
SH_CMD_EXPORT(power_request, lpmgr_request_mode, "request power management mode");

static void lpmgr_run_mode_switch(int argc, char **argv)
{
    int mode = 0;
    if (argc >= 2)
    {
        mode = atoi(argv[1]);
    }

    os_lpmgr_run_enter(mode);
}
SH_CMD_EXPORT(power_run, lpmgr_run_mode_switch, "switch power management run mode");

static void lpmgr_dump_status(void)
{
    os_uint32_t   index;
    struct lpmgr *lpm;

    lpm = &gs_lpmgr;

    os_kprintf("| Power Management Mode | Counter | Timer |\n");
    os_kprintf("+-----------------------+---------+-------+\n");
    for (index = 0; index < SYS_SLEEP_MODE_MAX; index++)
    {
        int has_timer = 0;
        if (lpm->timer_mask & (1 << index))
            has_timer = 1;

        os_kprintf("| %021s | %7d | %5d |\n", gs_lpmgr_sleep_str[index], lpm->modes[index], has_timer);
    }
    os_kprintf("+-----------------------+---------+-------+\n");

    os_kprintf("lpmgr current sleep mode: %s\n", gs_lpmgr_sleep_str[lpm->sleep_mode]);
    os_kprintf("lpmgr current run mode:   %s\n", gs_lpmgr_run_str[lpm->run_mode]);
}
SH_CMD_EXPORT(power_status, lpmgr_dump_status, "dump power management status");
#endif

#endif /* OS_USING_LPMGR */
