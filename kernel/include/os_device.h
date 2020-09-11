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
 * @file        os_device.h
 *
 * @brief       Header file for device interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-27   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __OS_DEVICE_H__
#define __OS_DEVICE_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_object.h>

#ifdef OS_USING_DEVICE

#ifdef __cplusplus
extern "C" {
#endif

enum os_device_type
{
    OS_DEVICE_TYPE_CHAR = 0,        /* character device. */
    OS_DEVICE_TYPE_BLOCK,           /* block device. */
    OS_DEVICE_TYPE_NETIF,           /* Net interface. */
    OS_DEVICE_TYPE_MTD,             /* Memory device. */
    OS_DEVICE_TYPE_CAN,             /* CAN device. */
    OS_DEVICE_TYPE_RTC,             /* RTC device. */
    OS_DEVICE_TYPE_SOUND,           /* Sound device. */
    OS_DEVICE_TYPE_GRAPHIC,         /* Graphic device. */
    OS_DEVICE_TYPE_I2CBUS,          /* I2C bus device. */
    OS_DEVICE_TYPE_USBDEVICE,       /* USB slave device. */
    OS_DEVICE_TYPE_USBHOST,         /* USB host bus. */
    OS_DEVICE_TYPE_SPIBUS,          /* SPI bus device. */
    OS_DEVICE_TYPE_SPIDEVICE,       /* SPI device. */
    OS_DEVICE_TYPE_SDIO,            /* SDIO bus device. */
    OS_DEVICE_TYPE_PM,              /* PM pseudo device. */
    OS_DEVICE_TYPE_PIPE,            /* Pipe device. */
    OS_DEVICE_TYPE_PORTAL,          /* Portal device. */
    OS_DEVICE_TYPE_CLOCKSOURCE,     /* ClockSource device. */
    OS_DEVICE_TYPE_CLOCKEVENT,      /* ClockEvent device. */
    OS_DEVICE_TYPE_MISCELLANEOUS,   /* Miscellaneous device. */
    OS_DEVICE_TYPE_SENSOR,          /* Sensor device. */
    OS_DEVICE_TYPE_TOUCH,           /* Touch device. */
    OS_DEVICE_TYPE_INFRARED,        /* Infrared device. */
    OS_DEVICE_TYPE_UNKNOWN          /* Unknown device. */
};

#define OS_DEVICE_FLAG_DEACTIVATE       0x000       /* Device is not not initialized. */

#define OS_DEVICE_FLAG_RDONLY           0x001       /* Read only. */
#define OS_DEVICE_FLAG_WRONLY           0x002       /* Write only. */
#define OS_DEVICE_FLAG_RDWR             0x003       /* Read and write. */

#define OS_DEVICE_FLAG_REMOVABLE        0x004       /* Removable device. */
#define OS_DEVICE_FLAG_STANDALONE       0x008       /* Standalone device. */
#define OS_DEVICE_FLAG_ACTIVATED        0x010       /* Device is activated. */
#define OS_DEVICE_FLAG_SUSPENDED        0x020       /* Device is suspended. */
#define OS_DEVICE_FLAG_STREAM           0x040       /* Stream mode. */
#define OS_DEVICE_FLAG_RAW              0x080       /* Raw mode. */

#define OS_DEVICE_FLAG_INT_RX           0x100       /* INT mode on Rx. */
#define OS_DEVICE_FLAG_DMA_RX           0x200       /* DMA mode on Rx. */
#define OS_DEVICE_FLAG_INT_TX           0x400       /* INT mode on Tx. */
#define OS_DEVICE_FLAG_DMA_TX           0x800       /* DMA mode on Tx. */

#define OS_DEVICE_OFLAG_CLOSE           0x000       /* Device is closed. */
#define OS_DEVICE_OFLAG_RDONLY          0x001       /* Read only access. */
#define OS_DEVICE_OFLAG_WRONLY          0x002       /* Write only access. */
#define OS_DEVICE_OFLAG_RDWR            0x003       /* Read and write. */
#define OS_DEVICE_OFLAG_OPEN            0x008       /* Device is opened. */
#define OS_DEVICE_OFLAG_MASK            0xf0f       /* Mask of open flag. */

/* general device commands */
#define OS_DEVICE_CTRL_RESUME           0x01        /* Resume device. */
#define OS_DEVICE_CTRL_SUSPEND          0x02        /* Suspend device. */
#define OS_DEVICE_CTRL_CONFIG           0x03        /* Configure device. */

#define OS_DEVICE_CTRL_SET_INT          0x10        /* Set interrupt. */
#define OS_DEVICE_CTRL_CLR_INT          0x11        /* Clear interrupt. */
#define OS_DEVICE_CTRL_GET_INT          0x12        /* Get interrupt status. */

/* special device commands */
#define OS_DEVICE_CTRL_CHAR_STREAM      0x10        /* Stream mode on char device. */
#define OS_DEVICE_CTRL_BLK_GETGEOME     0x10        /* Get geometry information.   */
#define OS_DEVICE_CTRL_BLK_SYNC         0x11        /* Flush data to block device. */
#define OS_DEVICE_CTRL_BLK_ERASE        0x12        /* Erase block on block device. */
#define OS_DEVICE_CTRL_BLK_AUTOREFRESH  0x13        /* Block device : enter/exit auto refresh mode. */
#define OS_DEVICE_CTRL_NETIF_GETMAC     0x10        /* Get mac address. */
#define OS_DEVICE_CTRL_MTD_FORMAT       0x10        /* Format a MTD device. */
#define OS_DEVICE_CTRL_RTC_GET_TIME     0x10        /* Get time. */
#define OS_DEVICE_CTRL_RTC_SET_TIME     0x11        /* Set time. */
#define OS_DEVICE_CTRL_RTC_GET_ALARM    0x12        /* Get alarm. */
#define OS_DEVICE_CTRL_RTC_SET_ALARM    0x13        /* Set alarm. */

typedef struct os_device os_device_t;

struct os_device_ops
{
    os_err_t  (*init)   (os_device_t *dev);
    os_err_t  (*open)   (os_device_t *dev, os_uint16_t oflag);
    os_err_t  (*close)  (os_device_t *dev);
    os_size_t (*read)   (os_device_t *dev, os_off_t pos, void *buffer, os_size_t size);
    os_size_t (*write)  (os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size);
    os_err_t  (*control)(os_device_t *dev, os_int32_t cmd, void *args);
};

#if defined(OS_USING_POSIX)
#include <extend/os_waitqueue.h> /* For wqueue_init. */
#endif

struct os_device
{
    os_object_t           parent;       /* Inherit from os_object. */

    enum os_device_type   type;         /* Device type. */
    os_uint16_t           flag;         /* Device flag. */
    os_uint16_t           open_flag;    /* Device open flag. */

    os_uint8_t            ref_count;    /* Reference count. */
    os_uint8_t            device_id;    /* 0 - 255 */

    /* Device call back. */
    os_err_t (*rx_indicate)(os_device_t *dev, os_size_t size);
    os_err_t (*tx_complete)(os_device_t *dev, void *buffer);

#ifdef OS_USING_DEVICE_OPS
    const struct os_device_ops *ops;
#else
    /* Common device interface. */
    os_err_t  (*init)   (os_device_t *dev);
    os_err_t  (*open)   (os_device_t *dev, os_uint16_t oflag);
    os_err_t  (*close)  (os_device_t *dev);
    os_size_t (*read)   (os_device_t *dev, os_off_t pos, void *buffer, os_size_t size);
    os_size_t (*write)  (os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size);
    os_err_t  (*control)(os_device_t *dev, os_int32_t cmd, void *args);
#endif

#if defined(OS_USING_POSIX)
    const struct vfs_file_ops *fops;
    struct os_waitqueue wait_queue;
#endif

    /* device private data. */
    void *user_data;        
};

struct os_device_blk_geometry
{
    os_uint32_t sector_count;           /* Count of sectors. */
    os_uint32_t bytes_per_sector;       /* Number of bytes per sector. */
    os_uint32_t block_size;             /* Number of bytes to erase one block. */
};

struct os_device_blk_sectors
{
    os_uint32_t sector_begin;           /* Begin sector. */
    os_uint32_t sector_end;             /* End sector.   */
};

#define OS_DEVICE_CTRL_CURSOR_SET_POSITION  0x10
#define OS_DEVICE_CTRL_CURSOR_SET_TYPE      0x11

extern os_err_t     os_device_register(os_device_t *dev, const char *name, os_uint16_t flags);
extern os_err_t     os_device_unregister(os_device_t *dev);
extern os_device_t *os_device_find(const char *name);
extern os_err_t     os_device_init(os_device_t *dev);
extern os_err_t     os_device_open(os_device_t *dev, os_uint16_t oflag);
extern os_err_t     os_device_close(os_device_t *dev);
extern os_size_t    os_device_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size);
extern os_size_t    os_device_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size);
extern os_err_t     os_device_control(os_device_t *dev, int cmd, void *arg);
extern os_err_t     os_device_set_rx_indicate(os_device_t *dev, os_err_t (*rx_ind)(os_device_t *dev, os_size_t size));
extern os_err_t     os_device_set_tx_complete(os_device_t *dev, os_err_t (*tx_done)(os_device_t *dev, void *buffer));

#if defined(OS_USING_DEVICE) && defined(OS_USING_CONSOLE)
extern os_device_t *os_console_set_device(const char *name);
extern os_device_t *os_console_get_device(void);
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif

