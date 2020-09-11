/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "py/runtime.h"

//#include "serial/serial.h"


#if (MICROPY_PY_MACHINE_UART)
#include <stdio.h>
#include <string.h>
#include "usr_misc.h"
#include "usr_uart.h"

int uart_timeout = 1;

void middle_uart_print(mp_uart_device_handler * uart_device, const mp_print_t *print)
{
      mp_printf(print, "uart( device port : %s", uart_device->owner.name);
}

int uart_open(void *device ,uint16_t oflag)
{
	os_device_t *uart_device = os_device_find(((device_info_t *)device)->owner.name);
	
	os_device_open((os_device_t *)uart_device, OS_DEVICE_FLAG_STREAM | OS_DEVICE_FLAG_INT_RX );
	struct  serial_configure config = OS_SERIAL_CONFIG_DEFAULT; /* ???? */

	os_err_t err = os_device_control(uart_device, OS_DEVICE_CTRL_CONFIG, &config);
	if (err != OS_EOK)
	{
		printf("config error!\n");
		return -1;
	}
	((device_info_t *)device)->open_flag = UART_INIT_FLAG;
	return 0;
}

int uart_read(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	os_device_t *uart_device = os_device_find(((device_info_t *)device)->owner.name);
	
	int num = uart_timeout;
	int readlen = bufsize;
	if(NULL == uart_device)
	{
		printf("uart_read find name error\n");
		return -1;
	}
	unsigned char *ptr = buf;

	do
	{
		if(os_device_read(uart_device, 0, ptr++, 1) == 0)
		{
			os_task_mdelay(1);
			num--;
			if(num == 0)
			{
				break;
			}
		}
		else
		{
			bufsize--;
		}
		
	}while(bufsize);
	
	return (readlen - bufsize);
}

int uart_write(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	os_device_t *uart_device = os_device_find(((device_info_t *)device)->owner.name);
	
	if(NULL == uart_device)
	{
		printf("uart_write find name error\n");
		return -1;
	}
	
	return os_device_write(uart_device, -1, buf, bufsize);
}

int uart_ctrl(void *device, int cmd, void* arg)
{
	os_device_t *uart_device = os_device_find(((device_info_t *)device)->owner.name);

	struct  serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
	uart_timeout = ((middle_uart_config_t *)(arg))->timeout;
	
	config.baud_rate = ((middle_uart_config_t *)(arg))->baud_rate;
	config.stop_bits = ((middle_uart_config_t *)(arg))->stop_bits;
	config.parity = ((middle_uart_config_t *)(arg))->parity;
	config.data_bits = ((middle_uart_config_t *)(arg))->data_bits;
	//printf("%s baudrate: %d\n", uart_device->parent.name, config.baud_rate);
	os_device_control(uart_device, OS_DEVICE_CTRL_CONFIG, &config);
	
	return 0;
}

int uart_close(void *device)
{
	int result;
	os_device_t *uart_device = os_device_find(((device_info_t *)device)->owner.name);
	if(NULL == uart_device)
	{
		printf("uart_close find name error\n");
		return -1;
	}
	result = os_device_close(uart_device);
	if(OS_EOK == result)
	{
		((device_info_t *)device)->open_flag = UART_DEINIT_FLAG;
	}
	return result;
}

STATIC struct operate uart_ops = {
	.open = uart_open,
	.read = uart_read,
	.write = uart_write,
	.ioctl = uart_ctrl,
	.close = uart_close,
};

int mpycall_uart_register(void)
{

	device_info_t  *pos, *uart = mp_misc_find_similar_device("uart");
	if (!uart){
		return -1;
	}
	
	DEV_LIST_LOOP(pos, uart, get_list_head())
	{
		pos->owner.type = DEV_BUS;
		pos->ops = &uart_ops;
		
	}
	return 0;
}

OS_DEVICE_INIT(mpycall_uart_register);


#endif // MICROPY_PY_MACHINE_UART

