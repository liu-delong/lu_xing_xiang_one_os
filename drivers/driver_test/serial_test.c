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
 * @file        serial_test.c
 *
 * @brief       The test file for serial.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_clock.h>
#include <os_memory.h>
#include <os_mailbox.h>
#include <stdint.h>
#include <stdlib.h>
#include <shell.h>
#include <serial/serial.h>

static int32_t  rx_thread_status = 0;
static int32_t  rx_done_cnt      = 0;
static int32_t  rx_total_cnt     = 0;
static uint16_t rx_crc           = 0;
static os_mailbox_t rx_mb;
static os_uint32_t  mb_pool[4];

static os_err_t rx_done(os_device_t *uart, struct os_device_cb_info *info)
{
    rx_done_cnt++;
    os_mb_send(&rx_mb, (os_uint32_t)info->size, OS_IPC_WAITING_NO);
    return 0;
}

static void rx_thread(void *parameter)
{
    int            rx_cnt;
    unsigned char *rx_buff = os_malloc(OS_SERIAL_RX_BUFSZ);
    os_uint32_t    size;
    os_device_t   *uart = (os_device_t *)parameter;

    OS_ASSERT(rx_buff);

    while (rx_thread_status == 0)
    {
        if (os_mb_recv(&rx_mb, &size, OS_TICK_PER_SECOND) != OS_EOK)
            continue;

        while (size)
        {
            rx_cnt = os_device_read(uart, 0, rx_buff, min(OS_SERIAL_RX_BUFSZ, size));
            if (rx_cnt == 0)
                break;

            rx_total_cnt += rx_cnt;

            os_kprintf("<%d>rx cnt:%d\r\n", os_tick_get(), rx_cnt);
            hex_dump(rx_buff, rx_cnt);
            rx_crc = crc16(rx_crc, rx_buff, rx_cnt);
            size -= rx_cnt;
        }

        size = 0;

        os_kprintf("<%d>rx done(%d) total:%d\r\n", os_tick_get(), rx_done_cnt, rx_total_cnt);
    }

    /* Last pack */
    rx_total_cnt += os_device_read(uart, 0, rx_buff, OS_SERIAL_RX_BUFSZ);

    os_free(rx_buff);

    rx_thread_status = 2;
}

static int tx_complete;
static int tx_done_cnt;

static os_err_t tx_done(os_device_t *uart, struct os_device_cb_info *info)
{
    os_kprintf("<%d>tx done %d\r\n", os_tick_get(), tx_done_cnt++);
    tx_complete = 1;
    return 0;
}

static unsigned char tx_buff[OS_SERIAL_RX_BUFSZ];

static int serial_int_tx_test(int argc, char *argv[])
{
    int           ret, i, j, loops = 1;
    uint32_t      tx_index, rand_tx_cnt;
    os_device_t  *uart;
    uint16_t      tx_crc = 0;

    if (argc != 2 && argc != 3)
    {
        os_kprintf("usage: serial_int_tx_test <dev> [times]\r\n");
        os_kprintf("       serial_int_tx_test uart1 (default 1 times)\r\n");
        os_kprintf("       serial_int_tx_test uart1 10000\r\n");
        return -1;
    }

    os_mb_init(&rx_mb, "rx_mb", mb_pool, sizeof(mb_pool) / sizeof(mb_pool[0]), OS_IPC_FLAG_FIFO);

    uart = os_device_find(argv[1]);
    OS_ASSERT(uart);

    if (argc == 3)
    {
        loops = strtol(argv[2], OS_NULL, 0);
    }

    struct os_device_cb_info cb_info;
    
    cb_info.type = OS_DEVICE_CB_TYPE_TX;
    cb_info.cb   = tx_done;
    os_device_control(uart, IOC_SET_CB, &cb_info);

    cb_info.type = OS_DEVICE_CB_TYPE_RX;
    cb_info.cb   = rx_done;
    os_device_control(uart, IOC_SET_CB, &cb_info);

    os_device_open(uart, OS_SERIAL_FLAG_RX_NOPOLL | OS_SERIAL_FLAG_TX_NOPOLL | OS_DEVICE_OFLAG_RDWR);

    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_2000000;
    ret = os_device_control(uart, OS_DEVICE_CTRL_CONFIG, &config);
    if (ret != 0)
    {
        os_kprintf("serial baud 115200\r\n");
        config.baud_rate = BAUD_RATE_115200;
        ret = os_device_control(uart, OS_DEVICE_CTRL_CONFIG, &config);
        if (ret != 0)
        {
            os_kprintf("serial control fail %d\r\n", ret);
            return -1;
        }
    }

    rx_thread_status = 0;
    rx_done_cnt      = 0;
    rx_total_cnt     = 0;
    rx_crc           = 0;

    os_task_t *task = os_task_create("rx_thread", rx_thread, uart, 512, 4, 5);
    OS_ASSERT(task);
    os_task_startup(task);

    tx_done_cnt = 0;

    for (i = 0; i < loops; i++)
    {
        tx_index = 0;

        for (j = 0; j < sizeof(tx_buff); j++)
        {
            tx_buff[j] = rand();
        }
        os_task_msleep(50);

        tx_crc = crc16(tx_crc, tx_buff, sizeof(tx_buff));

        while (tx_index < sizeof(tx_buff))
        {
            tx_complete = 0;

            rand_tx_cnt = (sizeof(tx_buff) - tx_index) * (rand() & 0xff) / 256 + 1;
            rand_tx_cnt = min(sizeof(tx_buff) - tx_index, rand_tx_cnt);
            tx_index += os_device_write(uart, 0, tx_buff + tx_index, rand_tx_cnt);

            while (tx_complete == 0)
            {
                os_kprintf("<%d>uart tx busy %d/%d, %d/%d\r\n", os_tick_get(), i, loops, tx_index, sizeof(tx_buff));

                os_task_yield();
            }
        }
    }

    /* Wait rx thread exit */
    os_task_msleep(300);
    rx_thread_status = 1;
    while (rx_thread_status != 2)
    {
        os_kprintf("wait rx thread exit..\r\n");
        os_task_msleep(300);
    }
    
    cb_info.type = OS_DEVICE_CB_TYPE_TX;
    cb_info.cb   = OS_NULL;
    os_device_control(uart, IOC_SET_CB, &cb_info);

    cb_info.type = OS_DEVICE_CB_TYPE_RX;
    cb_info.cb   = OS_NULL;
    os_device_control(uart, IOC_SET_CB, &cb_info);

    os_device_close(uart);
    os_mb_deinit(&rx_mb);

    os_kprintf("\r\n");
    os_kprintf("stat:\r\n");
    os_kprintf("    tx done %d\r\n", tx_done_cnt);
    os_kprintf("    tx size %d\r\n", sizeof(tx_buff) * loops);
    os_kprintf("\r\n");
    os_kprintf("    rx done %d\r\n", rx_done_cnt);
    os_kprintf("    rx size %d\r\n", rx_total_cnt);
    os_kprintf("\r\n");
    os_kprintf("    %s tx_crc:%04x, rx_crc:%04x\r\n", (tx_crc == rx_crc) ? "success" : "failed", tx_crc, rx_crc);

    return 0;
}
SH_CMD_EXPORT(serial_int_tx_test, serial_int_tx_test, "serial_int_tx_test");

static int serial_test(int argc, char *argv[])
{
    int           ret;
    int           tx_cnt;
    int           rx_cnt;
    unsigned char rx_buff[32];
    os_device_t  *uart;
    const char   *dev_name;

    if (argc != 2)
    {
        os_kprintf("usage: serial_test <dev> \r\n");
        os_kprintf("       serial_test uart2 \r\n");
        return -1;
    }

    dev_name = argv[1];

    uart = os_device_find(dev_name);
    OS_ASSERT(uart);

    /* Open serial device with rx nopoll flag */
    os_device_open(uart, OS_SERIAL_FLAG_RX_NOPOLL | OS_SERIAL_FLAG_TX_NOPOLL | OS_DEVICE_OFLAG_RDWR);

    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_115200;
    ret = os_device_control(uart, OS_DEVICE_CTRL_CONFIG, &config);
    if (ret != 0)
    {
        os_kprintf("serial control fail %d\r\n", ret);
        return -1;
    }

    os_task_msleep(1);

    /* Tx */
    tx_cnt = os_device_write(uart, 0, "Hello World!\n", sizeof("Hello World!\n"));

    /* Wait rx complete */
    os_task_msleep(100);

    /* Rx */
    rx_cnt = os_device_read(uart, 0, rx_buff, sizeof(rx_buff));

    os_kprintf("tx_cnt: %d, rx_cnt: %d\n", tx_cnt, rx_cnt);

    if (rx_cnt == sizeof("Hello World!\n"))
    {
        os_kprintf("rx buff:%s\n", rx_buff);
    }
    else
    {
        os_kprintf("rx failed\n");
    }

    os_device_close(uart);

    return 0;
}
SH_CMD_EXPORT(serial_test, serial_test, "serial_test");

static int serial_rx_test(int argc, char *argv[])
{
    int            ret;
    int            rx_cnt;
    unsigned char *rx_buff;
    os_device_t   *uart;
    const char    *dev_name;

    if (argc != 2)
    {
        os_kprintf("usage: serial_rx_test <dev> \r\n");
        os_kprintf("       serial_rx_test uart2 \r\n");
        return -1;
    }

    dev_name = argv[1];

    os_mb_init(&rx_mb, "rx_test_mb", mb_pool, sizeof(mb_pool) / sizeof(mb_pool[0]), OS_IPC_FLAG_FIFO);

    uart = os_device_find(dev_name);
    OS_ASSERT(uart);

    struct os_device_cb_info cb_info = 
    {
        .type = OS_DEVICE_CB_TYPE_RX,
        .cb   = rx_done,
    };

    os_device_control(uart, IOC_SET_CB, &cb_info);

    os_device_open(uart, OS_SERIAL_FLAG_RX_NOPOLL | OS_SERIAL_FLAG_TX_NOPOLL | OS_DEVICE_OFLAG_RDWR);

    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_115200;
    ret = os_device_control(uart, OS_DEVICE_CTRL_CONFIG, &config);
    if (ret != 0)
    {
        os_kprintf("serial control fail %d\r\n", ret);
        return -1;
    }

    rx_buff = os_malloc(128);
    OS_ASSERT(rx_buff);

    while (1)
    {
        os_uint32_t size;
        
        os_mb_recv(&rx_mb, &size, OS_IPC_WAITING_FOREVER);

        do
        {
            rx_cnt = os_device_read(uart, 0, rx_buff, 128);

            if (rx_cnt == 0)
                break;

            if (rx_cnt < 0)
                goto end;

            os_kprintf("rx_cnt: %d\n", rx_cnt);
            
            hex_dump(rx_buff, rx_cnt);
        } while (1);
    }

end:
    os_free(rx_buff);
    os_device_close(uart);

    return 0;
}
SH_CMD_EXPORT(serial_rx_test, serial_rx_test, "serial_rx_test");


