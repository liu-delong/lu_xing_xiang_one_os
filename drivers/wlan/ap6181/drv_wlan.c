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
 * @file        drv_wlan.c
 *
 * @brief       The driver file for wlan.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_device.h>
#include <wlan_mgnt.h>
#include <wlan_prot.h>
#include <wlan_cfg.h>
#include <fal.h>
#include <os_memory.h>
#include <string.h>
#include <os_clock.h>
#include <drv_wlan.h>
#include "drv_gpio.h"

#define DRV_EXT_TAG "WLAN"
#include <drv_log.h>

#define WIFI_INIT_WAIT_TIME         (os_tick_from_ms(100))

extern int  wifi_hw_init(void);
extern void wwd_thread_notify_irq(void);

static os_uint32_t init_flag = 0;

struct os_wlan_device *bcm_hw_wlan_dev_alloc(void)
{
    struct os_wlan_device *wlan;

    wlan = os_malloc(sizeof(struct os_wlan_device));

    return wlan;
}

#ifdef OS_USING_LPMGR
void wiced_platform_keep_awake(void)
{
    os_lpmgr_request(SYS_SLEEP_MODE_NONE);
}

void wiced_platform_let_sleep(void)
{
    os_lpmgr_release(SYS_SLEEP_MODE_NONE);
}
#endif

int os_hw_wlan_get_initialize_status(void)
{
    return init_flag;
}

int os_hw_wlan_wait_init_done(os_uint32_t time_ms)
{
    os_uint32_t time_cnt = 0;

    /* wait wifi low level initialize complete */
    while (time_cnt <= (time_ms / 100))
    {
        time_cnt++;
        os_task_mdelay(100);
        if (os_hw_wlan_get_initialize_status() == 1)
        {
            break;
        }
    }

    if (time_cnt > (time_ms / 100))
    {
        return OS_ETIMEOUT;
    }

    return OS_EOK;
}

static void _wiced_irq_handler(void *param)
{
    wwd_thread_notify_irq();
}

static int ap6181_early_init(void)
{
    /* WIFI_REG_ON */
    os_pin_mode(BSP_AP6181_REG_ON_PIN, PIN_MODE_OUTPUT);
    os_pin_write(BSP_AP6181_REG_ON_PIN, PIN_LOW);
    os_hw_us_delay(2000);
    os_pin_write(BSP_AP6181_REG_ON_PIN, PIN_HIGH);

    return 0;
}

OS_BOARD_INIT(ap6181_early_init);

static int ap6181_init(void)
{
    /* set wifi irq handle, must be initialized first */
    os_pin_mode(BSP_AP6181_IRQ_PIN, PIN_MODE_INPUT_PULLUP);
    os_pin_attach_irq(BSP_AP6181_IRQ_PIN, PIN_IRQ_MODE_RISING_FALLING, _wiced_irq_handler, OS_NULL);
    os_pin_irq_enable(BSP_AP6181_IRQ_PIN, PIN_IRQ_ENABLE);
    
    /* initialize low level wifi(ap6181) library */
    wifi_hw_init();

    /* waiting for sdio bus stability */
    os_task_delay(WIFI_INIT_WAIT_TIME);

    /* set wifi work mode */
    os_wlan_set_mode(OS_WLAN_DEVICE_STA_NAME, OS_WLAN_STATION);

    init_flag = 1;

    return 0;
}

OS_APP_INIT(ap6181_init);

