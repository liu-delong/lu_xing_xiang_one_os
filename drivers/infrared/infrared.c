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
 * @file        infrared.c
 *
 * @brief       infrared
 *
 * @details     infrared
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_hw.h>
#include <os_device.h>
#include <board.h>
#include <os_clock.h>
#include <os_irq.h>
#include <stdio.h>
#include <timer/clocksource.h>
#include <infrared/infrared.h>

#define DBG_EXT_TAG "infrared"
#define DBG_EXT_LVL DBG_EXT_DEBUG
#include <os_dbg_ext.h>

#define INFRARED_TX_ACTIVE   PIN_HIGH
#define INFRARED_TX_DEACTIVE PIN_LOW

#define INFRARED_TIMEOUT_NSEC       (100 * NSEC_PER_MSEC)

#define INFRARED_RX_STATUS_IDLE    (0)
#define INFRARED_RX_STATUS_SYNC    (1)
#define INFRARED_RX_STATUS_ADDR    (2)
#define INFRARED_RX_STATUS_DATA    (3)
#define INFRARED_RX_STATUS_REPEAT0 (4)
#define INFRARED_RX_STATUS_REPEAT1 (5)

static os_timer_t infrared_timer;

static os_list_node_t infrared_list = {&infrared_list, &infrared_list};

static void infrared_udelay(os_uint32_t us)
{
    os_clocksource_ndelay(us * 1000);
}

static void infrared_rx_idle(struct os_infrared_device *infrared, int rx_pin_status, os_uint64_t now)
{
    os_int32_t usec = (now - infrared->rx_time_stamp) / NSEC_PER_USEC;

    LOG_EXT_D("%d.%09d, %d, %d", (os_int32_t)(now / NSEC_PER_SEC), (os_int32_t)(now % NSEC_PER_SEC), usec, rx_pin_status);

    if (rx_pin_status == PIN_LOW)
    {
        /* 3.6ms repeat */
        if (usec > 35000 && usec < 45000)
        {
            infrared->rx_status = INFRARED_RX_STATUS_REPEAT0;
        }
        else
        {
            infrared->rx_addr        = 0;
            infrared->rx_addr_offset = 0;
            infrared->rx_data        = 0;
            infrared->rx_data_offset = 0;
            infrared->rx_status      = INFRARED_RX_STATUS_SYNC;
        }
    }
}

static void infrared_rx_sync(struct os_infrared_device *infrared, int rx_pin_status, os_uint64_t now)
{
#ifdef OS_DEBUG
    os_int32_t usec = (now - infrared->rx_time_stamp) / NSEC_PER_USEC;

    LOG_EXT_D("%d.%09d, %d, %d", (os_int32_t)(now / NSEC_PER_SEC), (os_int32_t)(now % NSEC_PER_SEC), usec, rx_pin_status);
#endif

    if (rx_pin_status == PIN_LOW)
    {
        infrared->rx_status = INFRARED_RX_STATUS_ADDR;
    }
}

static void infrared_rx_addr(struct os_infrared_device *infrared, int rx_pin_status, os_uint64_t now)
{
    os_int32_t usec = (now - infrared->rx_time_stamp) / NSEC_PER_USEC;

    LOG_EXT_D("%d.%09d, %d, %d", (os_int32_t)(now / NSEC_PER_SEC), (os_int32_t)(now % NSEC_PER_SEC), usec, rx_pin_status);

    if (rx_pin_status == PIN_HIGH)
    {
        return;
    }

    infrared->rx_addr >>= 1;

    if (usec > 1200 && usec < 2000)
    {
        infrared->rx_addr |= 1 << 15;
    }

    if (++infrared->rx_addr_offset < 16)
    {
        return;
    }

    infrared->rx_status = INFRARED_RX_STATUS_DATA;
}

static void infrared_rx_data(struct os_infrared_device *infrared, int rx_pin_status, os_uint64_t now)
{
    os_int32_t usec = (now - infrared->rx_time_stamp) / NSEC_PER_USEC;

    LOG_EXT_D("%d.%09d, %d, %d", (os_int32_t)(now / NSEC_PER_SEC), (os_int32_t)(now % NSEC_PER_SEC), usec, rx_pin_status);

    if (rx_pin_status == PIN_HIGH)
    {
        return;
    }

    infrared->rx_data >>= 1;

    if (usec > 1200 && usec < 2000)
    {
        infrared->rx_data |= 1 << 15;
    }

    if (++infrared->rx_data_offset < 16)
    {
        return;
    }

    if ((infrared->rx_addr >> 8 == (~infrared->rx_addr & 0xff)) &&
        (infrared->rx_data >> 8 == (~infrared->rx_data & 0xff)))
    {
        os_sem_post(&infrared->rx_sem);
    }

    infrared->rx_status = INFRARED_RX_STATUS_IDLE;
}

static void infrared_rx_repeat0(struct os_infrared_device *infrared, int rx_pin_status, os_uint64_t now)
{
    os_int32_t usec = (now - infrared->rx_time_stamp) / NSEC_PER_USEC;

    LOG_EXT_D("%d.%09d, %d, %d", (os_int32_t)(now / NSEC_PER_SEC), (os_int32_t)(now % NSEC_PER_SEC), usec, rx_pin_status);

    if (rx_pin_status == PIN_HIGH)
    {
        if (usec < 8000)
        {
            infrared->rx_status = INFRARED_RX_STATUS_IDLE;
        }
        else
        {
            if ((infrared->rx_addr >> 8 == (~infrared->rx_addr & 0xff)) &&
                (infrared->rx_data >> 8 == (~infrared->rx_data & 0xff)))
            {
                os_sem_post(&infrared->rx_sem);
            }
        }
    }
    else
    {
        infrared->rx_status = INFRARED_RX_STATUS_REPEAT1;
    }
}

static void infrared_rx_repeat1(struct os_infrared_device *infrared, int rx_pin_status, os_uint64_t now)
{
    os_int32_t usec = (now - infrared->rx_time_stamp) / NSEC_PER_USEC;

    LOG_EXT_D("%d.%09d, %d, %d", (os_int32_t)(now / NSEC_PER_SEC), (os_int32_t)(now % NSEC_PER_SEC), usec, rx_pin_status);

    if (rx_pin_status == PIN_HIGH)
    {
    }
    else
    {
        if (usec < 94000)
        {
            infrared->rx_status = INFRARED_RX_STATUS_IDLE;
        }
        else if (usec > 100000)
        {
            infrared_rx_idle(infrared, rx_pin_status, now);
        }
        else
        {
            infrared->rx_status = INFRARED_RX_STATUS_REPEAT0;
        }
    }
}

static void infrared_rx_pin_callback(void *param)
{
    struct os_infrared_device *infrared = param;

    int rx_pin_status;
    
    os_uint64_t now;

    rx_pin_status = os_pin_read(BSP_USING_RMT_CTL_ATK_RX_PIN);
    now = os_clocksource_gettime();

    switch (infrared->rx_status)
    {
    case INFRARED_RX_STATUS_IDLE:
        infrared_rx_idle(infrared, rx_pin_status, now);
        break;
    case INFRARED_RX_STATUS_SYNC:
        infrared_rx_sync(infrared, rx_pin_status, now);
        break;
    case INFRARED_RX_STATUS_ADDR:
        infrared_rx_addr(infrared, rx_pin_status, now);
        break;
    case INFRARED_RX_STATUS_DATA:
        infrared_rx_data(infrared, rx_pin_status, now);
        break;
    case INFRARED_RX_STATUS_REPEAT0:
        infrared_rx_repeat0(infrared, rx_pin_status, now);
        break;
    case INFRARED_RX_STATUS_REPEAT1:
        infrared_rx_repeat1(infrared, rx_pin_status, now);
        break;
    default:
        os_kprintf("infrared rx(%d) invalid\r\n", infrared->rx_status);
        break;
    }

    infrared->rx_time_stamp = now;
}

static void infrared_rx_timer_callback(void *parameter)
{
    os_base_t   level;
    os_uint64_t now;
    
    struct os_infrared_device *infrared;

    now = os_clocksource_gettime();

    level = os_hw_interrupt_disable();

    os_list_for_each_entry(infrared, &infrared_list, struct os_infrared_device, list)
    {
        if (infrared->rx_pin != OS_INFRARED_INVALIDE_PIN
        && infrared->rx_status != INFRARED_RX_STATUS_IDLE && infrared->rx_status != INFRARED_RX_STATUS_REPEAT1
        && (now - infrared->rx_time_stamp) > INFRARED_TIMEOUT_NSEC)
        {
            LOG_EXT_D("%d.%09d, infrared rx(%d) timeout", now / NSEC_PER_SEC, now % NSEC_PER_SEC, infrared->rx_status);
            infrared->rx_status = INFRARED_RX_STATUS_IDLE;
        }
    }

    os_hw_interrupt_enable(level);
}

static int infrared_timer_init(void)
{
    os_timer_init(&infrared_timer, 
                  "infrared", 
                  infrared_rx_timer_callback, 
                  OS_NULL, 
                  ((OS_TICK_PER_SECOND / 10) != 0) ? (OS_TICK_PER_SECOND / 10) : 1,
                  OS_TIMER_FLAG_PERIODIC);

    os_timer_start(&infrared_timer);

    return OS_EOK;
}

static int infrared_recv_init(struct os_infrared_device *infrared)
{
    char buff[32];

    snprintf(buff, sizeof(buff) - 1, "%s_rx", device_name(&infrared->parent));

    os_sem_init(&infrared->rx_sem, buff, 0, OS_IPC_FLAG_FIFO);

    /* pin */
    os_pin_mode(infrared->rx_pin, PIN_MODE_INPUT_PULLUP);
    os_pin_attach_irq(infrared->rx_pin, PIN_IRQ_MODE_RISING_FALLING, infrared_rx_pin_callback, infrared);
    os_pin_irq_enable(infrared->rx_pin, PIN_IRQ_ENABLE);

    LOG_EXT_I("infrared start receive");

    return 0;
}

static int infrared_recv_deinit(struct os_infrared_device *infrared)
{
    os_pin_irq_enable(infrared->rx_pin, PIN_IRQ_DISABLE);
    os_pin_detach_irq(infrared->rx_pin);
    os_pin_mode(infrared->rx_pin, PIN_MODE_INPUT);
    os_sem_deinit(&infrared->rx_sem);
    return 0;
}

#define EMISSION_PIN(infrared, value) os_pin_write(infrared->tx_pin, value);

static void irk_on_us(struct os_infrared_device *infrared, os_uint32_t us)
{
    os_uint32_t count;
    os_uint64_t end;

    end = os_clocksource_gettime() + us * NSEC_PER_USEC;

    count = us / 24;

    for (os_uint32_t i = 0; i < count; i++)
    {
        EMISSION_PIN(infrared, INFRARED_TX_ACTIVE);
        infrared_udelay(6);
        EMISSION_PIN(infrared, INFRARED_TX_DEACTIVE);
        infrared_udelay(18);

        if (os_clocksource_gettime() > end)
            break;
    }
}

static void irk_off_us(struct os_infrared_device *infrared, os_uint32_t us)
{
    infrared_udelay(us);
}

static int infrared_send_init(struct os_infrared_device *infrared)
{
    char buff[32];

    snprintf(buff, sizeof(buff) - 1, "%s_tx", device_name(&infrared->parent));

    os_sem_init(&infrared->tx_sem, buff, 1, OS_IPC_FLAG_FIFO);

    os_pin_mode(infrared->tx_pin, PIN_MODE_OUTPUT);

    return 0;
}

static int infrared_send_deinit(struct os_infrared_device *infrared)
{
    os_sem_deinit(&infrared->tx_sem);
    os_pin_mode(infrared->tx_pin, PIN_MODE_INPUT);
    return 0;
}

void infrared_send(struct os_infrared_device *infrared, os_uint8_t addrcode, os_uint8_t keycode, int times)
{
    int         index;
    os_uint32_t data;

    data = ((~keycode & 0xFF) << 24) + ((keycode & 0xFF) << 16) + ((~addrcode & 0xff) << 8) + addrcode;

    irk_on_us(infrared, 9000);
    irk_off_us(infrared, 4500);

    for (index = 0; index < 32; index++)
    {
        irk_on_us(infrared, 560);

        if (((data >> index) & 0x00000001) == 0)
        {
            irk_off_us(infrared, 550);
        }
        else
        {
            irk_off_us(infrared, 1680);
        }
    }

    irk_on_us(infrared, 560);
    irk_off_us(infrared, 40000);

    for (index = 0; index < times; index++)
    {
        irk_on_us(infrared, 9000);
        irk_off_us(infrared, 2250);
        irk_on_us(infrared, 560);
        irk_off_us(infrared, 96000);
    }
}

static os_err_t _infrared_init(os_device_t *dev)
{
    os_base_t                  level;
    struct os_infrared_device *infrared = (struct os_infrared_device *)dev;

    if (infrared->tx_pin != OS_INFRARED_INVALIDE_PIN)
    {
        infrared_send_init(infrared);
    }

    if (infrared->rx_pin != OS_INFRARED_INVALIDE_PIN)
    {
        infrared_recv_init(infrared);
    }

    level = os_hw_interrupt_disable();
    os_list_add_tail(&infrared_list, &infrared->list);
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t _infrared_open(os_device_t *dev, os_uint16_t oflag)
{
    return OS_EOK;
}

static os_err_t _infrared_close(os_device_t *dev)
{
    os_base_t                  level;
    struct os_infrared_device *infrared;

    infrared = (struct os_infrared_device *)dev;

    level = os_hw_interrupt_disable();

    if (infrared->tx_pin != OS_INFRARED_INVALIDE_PIN)
    {
        infrared_send_deinit(infrared);
    }

    if (infrared->rx_pin != OS_INFRARED_INVALIDE_PIN)
    {
        infrared_recv_deinit(infrared);
    }

    os_list_del(&infrared->list);

    dev->flag &= ~OS_DEVICE_FLAG_ACTIVATED;
    
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_size_t _infrared_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct os_infrared_info *  info;
    struct os_infrared_device *infrared = (struct os_infrared_device *)dev;

    OS_ASSERT(buffer);
    OS_ASSERT(sizeof(struct os_infrared_info) == size);

    info = (struct os_infrared_info *)buffer;

    os_sem_wait(&infrared->rx_sem, OS_IPC_WAITING_FOREVER);

    info->addr = infrared->rx_addr;
    info->data = infrared->rx_data;

    return size;
}

static os_size_t _infrared_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    struct os_infrared_info *  info;
    struct os_infrared_device *infrared = (struct os_infrared_device *)dev;

    OS_ASSERT(buffer);
    OS_ASSERT(sizeof(struct os_infrared_info) == size);

    info = (struct os_infrared_info *)buffer;

    os_sem_wait(&infrared->tx_sem, OS_IPC_WAITING_FOREVER);

    infrared_send(infrared, info->addr, info->data, info->times);

    os_sem_post(&infrared->tx_sem);

    return size;
}

static os_err_t _infrared_control(os_device_t *dev, int cmd, void *args)
{
    return OS_EOK;
}

const static struct os_device_ops infrared_ops =
{
    _infrared_init,
    _infrared_open,
    _infrared_close,
    _infrared_read,
    _infrared_write,
    _infrared_control
};

os_err_t os_infrared_register_device(const char *name, struct os_infrared_device *device)
{
    os_device_t *dev;

    dev = OS_DEVICE(device);
    OS_ASSERT(dev != OS_NULL);

    /* set device class and generic device interface */
    dev->type = OS_DEVICE_TYPE_INFRARED;
    dev->ops = &infrared_ops;
    dev->cb_table[OS_DEVICE_CB_TYPE_RX].cb = OS_NULL;
    dev->cb_table[OS_DEVICE_CB_TYPE_TX].cb = OS_NULL;

    return os_device_register(dev, name, OS_DEVICE_FLAG_RDWR);
}

static int os_hw_infrared_init(void)
{
    return infrared_timer_init();
}

OS_DEVICE_INIT(os_hw_infrared_init);

#ifdef OS_USING_SHELL

#include <shell.h>

static int list_infrared(void)
{
    os_base_t   level;
    const char *tx_str;
    const char *rx_str;
    
    struct os_infrared_device *infrared;

    level = os_hw_interrupt_disable();

    os_kprintf("infrared\r\n");

    os_list_for_each_entry(infrared, &infrared_list, struct os_infrared_device, list)
    {
        if (infrared->tx_pin != OS_INFRARED_INVALIDE_PIN)
        {
            tx_str = "tx";
        }
        else
        {
            tx_str = "  ";
        }
        

        if (infrared->rx_pin != OS_INFRARED_INVALIDE_PIN)
        {
            rx_str = "rx";
        }
        else
        {
            rx_str = "  ";
        }
    
        os_kprintf("%12s    %s      %s\r\n", device_name(&infrared->parent), tx_str, rx_str);
    }

    os_hw_interrupt_enable(level);

    return 0;
}
SH_CMD_EXPORT(list_infrared, list_infrared, "list_infrared");

#endif

