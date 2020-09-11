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
 * @file        serial.c
 *
 * @brief       This file provides functions for serial.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_memory.h>
#include <os_util.h>
#include <os_assert.h>
#include <os_irq.h>
#include <drv_cfg.h>
#include <os_dbg.h>

#define DBG_TAG "serial"

#ifdef OS_USING_POSIX
#include <vfs_posix.h>
#include <vfs_poll.h>

#ifdef OS_USING_POSIX_TERMIOS
#include <posix_termios.h>
#endif

/* It's possible the 'getc/putc' is defined by stdio.h in gcc/newlib. */
#ifdef getc
#undef getc
#endif

#ifdef putc
#undef putc
#endif

static os_err_t serial_fops_rx_ind(struct os_device *dev, os_size_t size)
{
    os_waitqueue_wakeup(&(dev->wait_queue), (void *)POLLIN);

    return OS_EOK;
}

static os_err_t serial_fops_tx_complete(struct os_device *dev, void *buffer)
{
    os_waitqueue_wakeup(&(dev->wait_queue), (void *)POLLOUT);

    return OS_EOK;
}

/* fops for serial */
static int serial_fops_open(struct vfs_fd *fd)
{
    os_err_t    ret = 0;
    os_uint16_t flags = 0;
    struct os_device *device;

    device = (struct os_device *)fd->data;
    OS_ASSERT(device != OS_NULL);

    switch (fd->flags & O_ACCMODE)
    {
    case O_RDONLY:
        LOG_D(DBG_TAG, "fops open: O_RDONLY!");
        flags = OS_DEVICE_FLAG_RDONLY;
        break;
    case O_WRONLY:
        LOG_D(DBG_TAG, "fops open: O_WRONLY!");
        flags = OS_DEVICE_FLAG_WRONLY;
        break;
    case O_RDWR:
        LOG_D(DBG_TAG, "fops open: O_RDWR!");
        flags = OS_DEVICE_FLAG_RDWR;
        break;
    default:
        LOG_E(DBG_TAG, "fops open: unknown mode - %d!", fd->flags & O_ACCMODE);
        break;
    }

    if (flags & OS_DEVICE_FLAG_WRONLY)
        os_device_set_tx_complete(device, serial_fops_tx_complete);
    if (flags & OS_DEVICE_FLAG_RDONLY)
        os_device_set_rx_indicate(device, serial_fops_rx_ind);
    ret = os_device_open(device, flags | OS_SERIAL_FLAG_TX_NOPOLL | OS_SERIAL_FLAG_RX_NOPOLL);
    if (ret == OS_EOK)
        return 0;

    return ret;
}

static int serial_fops_close(struct vfs_fd *fd)
{
    struct os_device *device;

    device = (struct os_device *)fd->data;

    os_device_close(device);

    return 0;
}

static int serial_fops_ioctl(struct vfs_fd *fd, int cmd, void *args)
{
    struct os_device *device;

    device = (struct os_device *)fd->data;
    switch (cmd)
    {
    case FIONREAD:
        break;
    case FIONWRITE:
        break;
    }

    return os_device_control(device, cmd, args);
}

static int serial_fops_read(struct vfs_fd *fd, void *buf, size_t count)
{
    int size = 0;
    struct os_device *device;

    device = (struct os_device *)fd->data;

    do
    {
        size = os_device_read(device, -1, buf, count);
        if (size <= 0)
        {
            if (fd->flags & O_NONBLOCK)
            {
                size = -EAGAIN;
                break;
            }

            os_waitqueue_wait(&(device->wait_queue), 0, OS_TICK_MAX);
        }
    } while (size <= 0);

    return size;
}

static int serial_fops_write(struct vfs_fd *fd, const void *buf, size_t count)
{
    struct os_device *device;

    device = (struct os_device *)fd->data;
    return os_device_write(device, -1, buf, count);
}

static int serial_fops_poll(struct vfs_fd *fd, struct os_pollreq *req)
{
    int mask  = 0;
    int flags = 0;

    os_base_t level;

    struct os_device        *device;
    struct os_serial_device *serial;

    device = (struct os_device *)fd->data;
    OS_ASSERT(device != OS_NULL);

    serial = (struct os_serial_device *)device;

    flags = fd->flags & O_ACCMODE;

    /* POLLIN */
    if ((req->_key & POLLIN) && (flags == O_RDONLY || flags == O_RDWR))
    {
        os_poll_add(&(device->wait_queue), req);

        level = os_hw_interrupt_disable();

        if (serial->rx_fifo != OS_NULL)
        {
            if (rb_ring_buff_data_len(&serial->rx_fifo->rbuff) != 0)
                mask |= POLLIN;
        }
        
        os_hw_interrupt_enable(level);
    }

    /* POLLOUT */
    if ((req->_key & POLLOUT) && (flags == O_WRONLY || flags == O_RDWR))
    {
        os_poll_add(&(device->wait_queue), req);

        level = os_hw_interrupt_disable();

        if (serial->tx_fifo != OS_NULL)
        {
            if (rb_ring_buff_space_len(&serial->tx_fifo->rbuff) != 0)
                mask |= POLLOUT;
        }
        
        os_hw_interrupt_enable(level);
    }

    return mask;
}

const static struct vfs_file_ops _serial_fops =
{
    serial_fops_open,
    serial_fops_close,
    serial_fops_ioctl,
    serial_fops_read,
    serial_fops_write,
    OS_NULL, /* Flush */
    OS_NULL, /* Lseek */
    OS_NULL, /* Getdents */
    serial_fops_poll,
};
#endif

#define OS_SERIAL_RX_TIMER_STATUS_NONE   0
#define OS_SERIAL_RX_TIMER_STATUS_ON     1
#define OS_SERIAL_RX_TIMER_STATUS_OFF    2

static void _serial_start_recv(struct os_serial_device *serial)
{
    if ((serial->ops->recv_state(serial) & OS_SERIAL_FLAG_RX_IDLE) == 0)
        return;

    serial->rx_count = 0;
    serial->ops->start_recv(serial, serial->rx_fifo->line_buff, serial->config.rx_bufsz);
    serial->rx_timer_status = OS_SERIAL_RX_TIMER_STATUS_ON;
}

static void _serial_rx_timer(void *parameter)
{
    os_base_t level;
    
    struct os_serial_device *serial;

    OS_ASSERT(parameter);
    serial  = (struct os_serial_device *)parameter;

    level = os_hw_interrupt_disable();

    if (serial->rx_timer_status != OS_SERIAL_RX_TIMER_STATUS_ON)
    {
        os_hw_interrupt_enable(level);
        return;
    }

    int state = serial->ops->recv_state(serial);

    if ((state & OS_SERIAL_FLAG_RX_IDLE) || (serial->rx_count == state && state != 0))
    {
        serial->ops->stop_recv(serial);

        state &= 0xffff;
        
        if (state == 0)
        {
            _serial_start_recv(serial);
        }
        else
        {
            os_hw_serial_isr_rxdone(serial, state);
        }
    }
    else
    {
        serial->rx_count = state;
    }
    
    os_hw_interrupt_enable(level);
}

static int _serial_rx(struct os_serial_device *serial, os_uint8_t *data, int length)
{
    int size;
    os_base_t level;

    if (length <= 0)
        return 0;

    OS_ASSERT(serial != OS_NULL);

    level = os_hw_interrupt_disable();

    OS_ASSERT(serial->rx_fifo != OS_NULL);

    size = rb_ring_buff_get(&serial->rx_fifo->rbuff, data, length);
    if (size != 0)
    {
        _serial_start_recv(serial);
    }

    os_hw_interrupt_enable(level);
    return size;
}

static int _serial_tx(struct os_serial_device *serial, const os_uint8_t *data, int length)
{
    int count;
    os_base_t level;
    struct os_serial_tx_fifo *tx_fifo;

    level = os_hw_interrupt_disable();

    tx_fifo = serial->tx_fifo;

    length = rb_ring_buff_put(&tx_fifo->rbuff, data, length);

    if (length == 0)
        goto end;
    
    if (tx_fifo->line_buff_count != 0)
        goto end;

    count = rb_ring_buff_get(&tx_fifo->rbuff, tx_fifo->line_buff, serial->config.tx_bufsz);

    tx_fifo->line_buff_count = serial->ops->start_send(serial, tx_fifo->line_buff, count);
    OS_ASSERT(tx_fifo->line_buff_count > 0);

    if (tx_fifo->line_buff_count < count)
    {
        rb_ring_buff_put(&tx_fifo->rbuff, tx_fifo->line_buff + tx_fifo->line_buff_count, count - tx_fifo->line_buff_count);
    }

end:
    OS_ASSERT(tx_fifo->line_buff_count != 0);
    os_hw_interrupt_enable(level);
    return length;
}

static int _serial_poll_rx(struct os_serial_device *serial, os_uint8_t *data, int length)
{
    int count = 0;
    int size  = 0;

    OS_ASSERT(serial != OS_NULL);

    while (size < length)
    {
        count = serial->ops->poll_recv(serial, data + size, 1);
        if (count <= 0)
            break;

        OS_ASSERT(count == 1);

        size += count;

        if (data[size - 1] == '\n')
            break;
    }

    return size;
}

static int _serial_poll_tx(struct os_serial_device *serial, const os_uint8_t *data, int length)
{
    int  count;
    
    OS_ASSERT(serial != OS_NULL);

    int send_index = 0;

    if (serial->parent.open_flag & OS_DEVICE_FLAG_STREAM)
    {
        os_uint8_t temp = '\r';
        
        while (send_index < length)
        {
            if (data[send_index] == '\n')
            {
                count = serial->ops->poll_send(serial, &temp, 1);
                
                if (count != 1)
                {
                    break;
                }
            }
        
            count = serial->ops->poll_send(serial, data + send_index, 1);

            if (count != 1)
            {
                break;
            }
            
            send_index += count;
        }
    }
    else
    {
        while (send_index < length)
        {
            count = serial->ops->poll_send(serial, data + send_index, length - send_index);

            if (count <= 0)
            {
                break;
            }
            
            send_index += count;
        }
    }

    return send_index;
}

/* OneOS Device Interface */

static os_err_t os_serial_init(struct os_device *dev)
{
    os_err_t result = OS_EOK;
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    serial = (struct os_serial_device *)dev;

    /* Initialize rx/tx */
    serial->rx_fifo = OS_NULL;
    serial->tx_fifo = OS_NULL;

    /* Apply configuration */
    if (serial->ops->configure)
        result = serial->ops->configure(serial, &serial->config);

    return result;
}

static os_err_t os_serial_open(struct os_device *dev, os_uint16_t oflag)
{
    os_uint16_t stream_flag = 0;
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    serial = (struct os_serial_device *)dev;

    LOG_D(DBG_TAG, "open serial device: 0x%08x with open flag: 0x%04x", dev, oflag);

    /* Keep steam flag */
    if ((oflag & OS_DEVICE_FLAG_STREAM) || (dev->open_flag & OS_DEVICE_FLAG_STREAM))
        stream_flag = OS_DEVICE_FLAG_STREAM;

    /* Get open flags */
    dev->open_flag = oflag & 0xff;

    /* rx buff */
    if (serial->ops->start_recv == OS_NULL)
    {
        oflag &= ~OS_SERIAL_FLAG_RX_NOPOLL;
    }
    else if (oflag & OS_SERIAL_FLAG_RX_NOPOLL && serial->rx_fifo == OS_NULL)
    {
        serial->rx_fifo = (struct os_serial_rx_fifo *)os_calloc(1, sizeof(struct os_serial_rx_fifo) + serial->config.rx_bufsz * 2);
        OS_ASSERT(serial->rx_fifo != OS_NULL);
        serial->rx_fifo->line_buff = (os_uint8_t *)(serial->rx_fifo + 1);            
        rb_ring_buff_init(&serial->rx_fifo->rbuff, serial->rx_fifo->line_buff + serial->config.rx_bufsz, serial->config.rx_bufsz);

        if (serial->rx_timer_status == OS_SERIAL_RX_TIMER_STATUS_NONE)
        {
            serial->rx_timer_status = OS_SERIAL_RX_TIMER_STATUS_OFF;
            os_timer_init(&serial->rx_timer, 
                      serial->parent.parent.name, 
                      _serial_rx_timer, 
                      serial, 
                      ((OS_TICK_PER_SECOND / 100) != 0) ? (OS_TICK_PER_SECOND / 100) : 1,
                      OS_TIMER_FLAG_PERIODIC);

            os_timer_start(&serial->rx_timer);
        }
        
        _serial_start_recv(serial);
    }

    /* tx buff */
    if (serial->ops->start_send == OS_NULL)
    {
        oflag &= ~OS_SERIAL_FLAG_TX_NOPOLL;
    }
    else if (oflag & OS_SERIAL_FLAG_TX_NOPOLL && serial->tx_fifo == OS_NULL)
    {
        struct os_serial_tx_fifo *tx_fifo;

        tx_fifo = (struct os_serial_tx_fifo *)os_calloc(1, sizeof(struct os_serial_tx_fifo) + serial->config.tx_bufsz * 2);
        OS_ASSERT(tx_fifo != OS_NULL);

        tx_fifo->line_buff = (os_uint8_t *)(tx_fifo + 1);
        tx_fifo->line_buff_count = 0;
        rb_ring_buff_init(&tx_fifo->rbuff, tx_fifo->line_buff + serial->config.tx_bufsz, serial->config.tx_bufsz);
        
        serial->tx_fifo = tx_fifo;
    }

    serial->flag = oflag & OS_SERIAL_FLAG_MASK;

    /* Set stream flag */
    dev->open_flag |= stream_flag;

    return OS_EOK;
}

static os_err_t os_serial_close(struct os_device *dev)
{
    os_base_t level;
    
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    serial = (struct os_serial_device *)dev;

    level = os_hw_interrupt_disable();

    OS_ASSERT(dev->ref_count == 0);

    if (serial->ops->stop_send)
        serial->ops->stop_send(serial);

    if (serial->ops->stop_recv)
        serial->ops->stop_recv(serial);

    os_device_set_rx_indicate(dev, OS_NULL);
    os_device_set_tx_complete(dev, OS_NULL);

    if (serial->rx_timer_status != OS_SERIAL_RX_TIMER_STATUS_NONE)
    {
        os_timer_deinit(&serial->rx_timer);
        serial->rx_timer_status = OS_SERIAL_RX_TIMER_STATUS_NONE;
    }

    /* rx fifo */
    if (serial->rx_fifo != OS_NULL)
    {
        os_free(serial->rx_fifo);
        serial->rx_fifo = OS_NULL;
    }

    /* tx fifo */
    if (serial->tx_fifo != OS_NULL)
    {
        os_free(serial->tx_fifo);
        serial->tx_fifo = OS_NULL;
    }

    os_hw_interrupt_enable(level);
    return OS_EOK;
}

static os_size_t os_serial_read(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    if (size == 0)
        return 0;

    serial = (struct os_serial_device *)dev;

    if (serial->flag & OS_SERIAL_FLAG_RX_NOPOLL)
    {
        return _serial_rx(serial, (os_uint8_t *)buffer, size);
    }
    else
    {
        return _serial_poll_rx(serial, (os_uint8_t *)buffer, size);
    }
}

static os_size_t os_serial_write(struct os_device *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    if (size == 0)
        return 0;

    serial = (struct os_serial_device *)dev;

    if (!(serial->flag & OS_SERIAL_FLAG_TX_NOPOLL)
#if defined(OS_USING_DEVICE) && defined(OS_USING_CONSOLE)
    || os_console_get_device() == dev
#endif
    || os_interrupt_get_nest() != 0)
    {
        return _serial_poll_tx(serial, (const os_uint8_t *)buffer, size);
    }
    else
    {
        return _serial_tx(serial, (const os_uint8_t *)buffer, size);
    }
}

#ifdef OS_USING_POSIX_TERMIOS
struct speed_baudrate_item
{
    speed_t speed;
    int     baudrate;
};

const static struct speed_baudrate_item _tbl[] =
{
    {B2400, BAUD_RATE_2400},
    {B4800, BAUD_RATE_4800},
    {B9600, BAUD_RATE_9600},
    {B19200, BAUD_RATE_19200},
    {B38400, BAUD_RATE_38400},
    {B57600, BAUD_RATE_57600},
    {B115200, BAUD_RATE_115200},
    {B230400, BAUD_RATE_230400},
    {B460800, BAUD_RATE_460800},
    {B921600, BAUD_RATE_921600},
    {B2000000, BAUD_RATE_2000000},
    {B3000000, BAUD_RATE_3000000},
};

static speed_t _get_speed(int baudrate)
{
    int index;

    for (index = 0; index < sizeof(_tbl) / sizeof(_tbl[0]); index++)
    {
        if (_tbl[index].baudrate == baudrate)
            return _tbl[index].speed;
    }

    return B0;
}

static int _get_baudrate(speed_t speed)
{
    int index;

    for (index = 0; index < sizeof(_tbl) / sizeof(_tbl[0]); index++)
    {
        if (_tbl[index].speed == speed)
            return _tbl[index].baudrate;
    }

    return 0;
}

static void _tc_flush(struct os_serial_device *serial, int queue)
{
    os_base_t level;
    int ch = -1;

    struct os_device *        device  = OS_NULL;

    OS_ASSERT(serial != OS_NULL);

    device  = &(serial->parent);

    switch (queue)
    {
    case TCIFLUSH:
    case TCIOFLUSH:

        OS_ASSERT(serial->rx_fifo != OS_NULL);

        if (device->open_flag & OS_SERIAL_FLAG_RX_NOPOLL)
        {
            OS_ASSERT(serial->rx_fifo != OS_NULL);
            level = os_hw_interrupt_disable();
            rb_ring_buff_reset(serial->rx_fifo->rbuff);
            os_hw_interrupt_enable(level);
        }
        else
        {
            while (1)
            {
                ch = serial->ops->getc(serial);
                if (ch == -1)
                    break;
            }
        }

        break;

    case TCOFLUSH:
        break;
    }
}

#endif

static os_err_t os_serial_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t ret = OS_EOK;
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    serial = (struct os_serial_device *)dev;

    switch (cmd)
    {
    case OS_DEVICE_CTRL_SUSPEND:
        /* Suspend device */
        dev->flag |= OS_DEVICE_FLAG_SUSPENDED;
        break;

    case OS_DEVICE_CTRL_RESUME:
        /* Resume device */
        dev->flag &= ~OS_DEVICE_FLAG_SUSPENDED;
        break;

    case OS_DEVICE_CTRL_CONFIG:
        if (args)
        {
            struct serial_configure *pconfig = (struct serial_configure *)args;
            if ((pconfig->rx_bufsz != serial->config.rx_bufsz || pconfig->tx_bufsz != serial->config.tx_bufsz) &&
                serial->parent.ref_count)
            {
                /*Can not change buffer size*/
                return OS_EBUSY;
            }
            /* Set serial configure */
            serial->config = *pconfig;
            if (serial->parent.ref_count)
            {
                /* Serial device has been opened, to configure it */
                return serial->ops->configure(serial, (struct serial_configure *)args);
            }
        }
        break;

#ifdef OS_USING_POSIX_TERMIOS
    case TCGETA:
    {
        struct termios *tio = (struct termios *)args;
        if (tio == OS_NULL)
            return OS_EINVAL;

        tio->c_iflag = 0;
        tio->c_oflag = 0;
        tio->c_lflag = 0;

        /* Update oflag for console device */
        if (os_console_get_device() == dev)
            tio->c_oflag = OPOST | ONLCR;

        /* Set cflag */
        tio->c_cflag = 0;
        if (serial->config.data_bits == DATA_BITS_5)
            tio->c_cflag = CS5;
        else if (serial->config.data_bits == DATA_BITS_6)
            tio->c_cflag = CS6;
        else if (serial->config.data_bits == DATA_BITS_7)
            tio->c_cflag = CS7;
        else if (serial->config.data_bits == DATA_BITS_8)
            tio->c_cflag = CS8;

        if (serial->config.stop_bits == STOP_BITS_2)
            tio->c_cflag |= CSTOPB;

        if (serial->config.parity == PARITY_EVEN)
            tio->c_cflag |= PARENB;
        else if (serial->config.parity == PARITY_ODD)
            tio->c_cflag |= (PARODD | PARENB);

        cfsetospeed(tio, _get_speed(serial->config.baud_rate));
    }
    break;

    case TCSETAW:
    case TCSETAF:
    case TCSETA:
    {
        int baudrate;
        struct serial_configure config;

        struct termios *tio = (struct termios *)args;
        if (tio == OS_NULL)
            return OS_EINVAL;

        config = serial->config;

        baudrate         = _get_baudrate(cfgetospeed(tio));
        config.baud_rate = baudrate;

        switch (tio->c_cflag & CSIZE)
        {
        case CS5:
            config.data_bits = DATA_BITS_5;
            break;
        case CS6:
            config.data_bits = DATA_BITS_6;
            break;
        case CS7:
            config.data_bits = DATA_BITS_7;
            break;
        default:
            config.data_bits = DATA_BITS_8;
            break;
        }

        if (tio->c_cflag & CSTOPB)
            config.stop_bits = STOP_BITS_2;
        else
            config.stop_bits = STOP_BITS_1;

        if (tio->c_cflag & PARENB)
        {
            if (tio->c_cflag & PARODD)
                config.parity = PARITY_ODD;
            else
                config.parity = PARITY_EVEN;
        }
        else
            config.parity = PARITY_NONE;

        serial->ops->configure(serial, &config);
    }
    break;
    case TCFLSH:
    {
        int queue = (int)args;

        _tc_flush(serial, queue);
    }

    break;
    case TCXONC:
        break;
#endif
#ifdef OS_USING_POSIX
    case FIONREAD:
    {
        os_size_t recved = 0;
        os_base_t level;

        level  = os_hw_interrupt_disable();
        recved = rb_ring_buff_data_len(&serial->rx_fifo->rbuff);
        os_hw_interrupt_enable(level);

        *(os_size_t *)args = recved;
    }
    break;
#endif
    default:
        /* Control device */
        ret = OS_ENOSYS;
        break;
    }

    return ret;
}

#ifdef OS_USING_DEVICE_OPS
const static struct os_device_ops serial_ops = 
{
    os_serial_init,
    os_serial_open,
    os_serial_close,
    os_serial_read,
    os_serial_write,
    os_serial_control
};
#endif
int serial_suspend(const struct os_device *device, os_uint8_t mode)
{
    return os_serial_control((struct os_device *)device,OS_DEVICE_CTRL_SUSPEND,NULL);
}

void serial_resume(const struct os_device *device, os_uint8_t mode)
{
     os_serial_control((struct os_device *)device,OS_DEVICE_CTRL_RESUME,NULL);
}


os_err_t os_hw_serial_register(struct os_serial_device *serial, const char *name, os_uint32_t flag, void *data)
{
    os_err_t          ret;
    struct os_device *device;
    OS_ASSERT(serial != OS_NULL);

    device = &(serial->parent);

    device->type        = OS_DEVICE_TYPE_CHAR;
    device->rx_indicate = OS_NULL;
    device->tx_complete = OS_NULL;

#ifdef OS_USING_DEVICE_OPS
    device->ops = &serial_ops;
#else
    device->init    = os_serial_init;
    device->open    = os_serial_open;
    device->close   = os_serial_close;
    device->read    = os_serial_read;
    device->write   = os_serial_write;
    device->control = os_serial_control;
#endif
    device->user_data = data;

    /* Register a character device */
    ret = os_device_register(device, name, flag);

#if defined(OS_USING_POSIX)
    /* Set fops */
    device->fops = &_serial_fops;
#endif

    return ret;
}

void os_hw_serial_isr_rxdone(struct os_serial_device *serial, int count)
{
    os_size_t rx_length;

    OS_ASSERT(serial);
    OS_ASSERT(serial->rx_fifo != OS_NULL);
    OS_ASSERT(count != 0);
    OS_ASSERT(rb_ring_buff_space_len(&serial->rx_fifo->rbuff) != 0);

    serial->rx_timer_status = OS_SERIAL_RX_TIMER_STATUS_OFF;

    rb_ring_buff_put(&serial->rx_fifo->rbuff, serial->rx_fifo->line_buff, count);

    if (serial->parent.rx_indicate != OS_NULL)
    {
        rx_length = rb_ring_buff_data_len(&serial->rx_fifo->rbuff);
        OS_ASSERT(rx_length != 0);
        serial->parent.rx_indicate(&serial->parent, rx_length);
    }

    if (rb_ring_buff_space_len(&serial->rx_fifo->rbuff) != 0)
    {
        _serial_start_recv(serial);
    }
}

void os_hw_serial_isr_txdone(struct os_serial_device *serial)
{
    int count;
    struct os_serial_tx_fifo *tx_fifo;

    tx_fifo = (struct os_serial_tx_fifo *)serial->tx_fifo;

    OS_ASSERT(tx_fifo->line_buff_count != 0);

    tx_fifo->line_buff_count = 0;

    count = rb_ring_buff_get(&tx_fifo->rbuff, tx_fifo->line_buff, serial->config.tx_bufsz);

    if (count == 0)
    {        
        if (serial->parent.tx_complete != OS_NULL)
        {
            serial->parent.tx_complete(&serial->parent, OS_NULL);
        }
    }
    else
    {
        tx_fifo->line_buff_count = serial->ops->start_send(serial, tx_fifo->line_buff, count);
        OS_ASSERT(tx_fifo->line_buff_count > 0);

        if (tx_fifo->line_buff_count < count)
        {
            rb_ring_buff_put(&tx_fifo->rbuff, tx_fifo->line_buff + tx_fifo->line_buff_count, count - tx_fifo->line_buff_count);
        }
    }
}

