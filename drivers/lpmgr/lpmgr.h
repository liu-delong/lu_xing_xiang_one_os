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
 * @file        lpmgr.h
 *
 * @brief       this file implements lpmgr related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __LPMGR_H__
#define __LPMGR_H__

#include <os_task.h>
#include <os_device.h>

#ifndef LPMGR_CUSTOM_CONFIG

enum
{
    /* sleep modes */
    SYS_SLEEP_MODE_NONE = 0,
    SYS_SLEEP_MODE_IDLE,
    SYS_SLEEP_MODE_LIGHT,
    SYS_SLEEP_MODE_DEEP,
    SYS_SLEEP_MODE_STANDBY,
    SYS_SLEEP_MODE_SHUTDOWN,
    SYS_SLEEP_MODE_MAX,
};

enum
{
    /* run modes*/
    SYS_RUN_MODE_HIGH_SPEED = 0,
    SYS_RUN_MODE_NORMAL_SPEED,
    SYS_RUN_MODE_MEDIUM_SPEED,
    SYS_RUN_MODE_LOW_SPEED,
    SYS_RUN_MODE_MAX,
};

enum
{
    LPMGR_FREQUENCY_PENDING = 0x01,
};

#define SYS_DEFAULT_SLEEP_MODE SYS_SLEEP_MODE_IDLE
#define SYS_DEFAULT_RUN_MODE   SYS_RUN_MODE_NORMAL_SPEED

#define SYS_SLEEP_MODE_NAMES    \
{                               \
    "None Mode",                \
    "Idle Mode",                \
    "LightSleep Mode",          \
    "DeepSleep Mode",           \
    "Standby Mode",             \
    "Shutdown Mode",            \
}

#define SYS_RUN_MODE_NAMES      \
{                               \
    "High Speed",               \
    "Normal Speed",             \
    "Medium Speed",             \
    "Low Mode",                 \
}

#else /* LPMGR_CUSTOM_CONFIG */

#include <lpmgr_cfg.h>

#endif /* LPMGR_CUSTOM_CONFIG */

#define LPMGR_DEVICE_CTRL_REQUEST 0x01
#define LPMGR_DEVICE_CTRL_RELEASE 0x00

struct lpmgr;

struct os_lpmgr_ops
{
    void (*sleep)(struct lpmgr *lpm, os_uint8_t mode);
    void (*run)(struct lpmgr *lpm, os_uint8_t mode);
    void (*timer_start)(struct lpmgr *lpm, os_uint32_t timeout);
    void (*timer_stop)(struct lpmgr *lpm);
    os_tick_t (*timer_get_tick)(struct lpmgr *lpm);
};

struct os_lpmgr_device_ops
{
    int (*suspend)(const struct os_device *device, os_uint8_t mode);
    void (*resume)(const struct os_device *device, os_uint8_t mode);
    int (*frequency_change)(const struct os_device *device, os_uint8_t mode);
};

struct os_lpmgr_device
{
    const struct os_device           *device;
    const struct os_lpmgr_device_ops *ops;
};

struct lpmgr
{
    struct os_device parent;

    /* modes */
    os_uint8_t modes[SYS_SLEEP_MODE_MAX];
    os_uint8_t sleep_mode; /* current sleep mode */
    os_uint8_t run_mode;   /* current running mode */

    /* the list of device, which has low power manager feature */
    os_uint8_t              device_number;
    struct os_lpmgr_device *lp_device;

    /* if the mode has timer, the corresponding bit is 1*/
    os_uint8_t timer_mask;
    os_uint8_t flags;

    const struct os_lpmgr_ops *ops;
};

enum
{
    SYS_ENTER_SLEEP = 0,
    SYS_EXIT_SLEEP,
};

struct lpmgr_notify
{
    void (*notify)(os_uint8_t event, os_uint8_t mode, void *data);
    void *data;
};

void os_lpmgr_request(os_uint8_t sleep_mode);
void os_lpmgr_release(os_uint8_t sleep_mode);
int  os_lpmgr_run_enter(os_uint8_t run_mode);

void os_lpmgr_device_register(struct os_device *device, const struct os_lpmgr_device_ops *ops);
void os_lpmgr_device_unregister(struct os_device *device);

void os_lpmgr_notify_set(void (*notify)(os_uint8_t event, os_uint8_t mode, void *data), void *data);
void os_lpmgr_default_set(os_uint8_t sleep_mode);

void os_lpmgr_init(const struct os_lpmgr_ops *ops, os_uint8_t timer_mask, void *user_data);

#endif /* __LPMGR_H__ */
