/*
 * This file is paos of the MicroPython project, http://micropython.org/
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
 * all copies or substantial poosions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PAosICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TOos OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include "py/runtime.h"
#include <os_device.h>
#include <os_stddef.h>

#if (MICROPY_PY_MACHINE_SPI)
#include "usr_misc.h"
#include "spi.h"
#include "usr_spi.h"


static int spi_open(void *device, uint16_t oflag)
{
	os_device_t *spi_device = os_device_find(((device_info_t *)device)->owner.name);
	if(!spi_device){
		return -1;
	}
	struct os_spi_configuration cfg;
	cfg.data_width = 8;
	cfg.mode = OS_SPI_MODE_3 | OS_SPI_MSB;
	cfg.max_hz = 40000;
	os_spi_configure((struct os_spi_device *)spi_device, &cfg);
	
	return 0;
	
}

static int spi_close(void *device)
{
	os_device_t *spi_device = os_device_find(((device_info_t *)device)->owner.name);
	
	return os_device_close(spi_device);
}

static int spi_read(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	os_device_t *spi_device = os_device_find(((device_info_t *)device)->owner.name);
	
	if(NULL == spi_device)
	{
		printf("spi_read find name error\n");
		return -1;
	}
	
	return os_spi_transfer((struct os_spi_device *)spi_device, NULL, buf, bufsize);
}

static int spi_write(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	os_device_t *spi_device = os_device_find(((device_info_t *)device)->owner.name);
	

	if(NULL == spi_device)
	{
		os_kprintf("spi_write find name error\n");
		return -1;
	}
	
	return os_spi_transfer((struct os_spi_device *)spi_device, buf, NULL, bufsize);
}

static int spi_ioctl(void *device, int cmd, void* arg)
{
	os_device_t *spi_device  = os_device_find(((device_info_t *)device)->owner.name);
	if (spi_device == NULL)
	{
		os_kprintf("test_spi find device error\n");
		return -1;
	}
	struct os_spi_configuration cfg;
	cfg.data_width = 8;
	cfg.max_hz = 40000;
	
	if((cmd &(~MP_SPI_MASK)) == MP_SPI_MSB)
	{
		cmd &= MP_SPI_MASK;
		if(cmd == MP_SPI_MODE_0)
		{
			cfg.mode = OS_SPI_MODE_0 | OS_SPI_MSB;
		}
		else if(cmd == MP_SPI_MODE_1)
		{
			cfg.mode = OS_SPI_MODE_1 | OS_SPI_MSB;
		}
		else if(cmd == MP_SPI_MODE_2)
		{
			cfg.mode = OS_SPI_MODE_2 | OS_SPI_MSB;
		}
		else if(cmd == MP_SPI_MODE_3)
		{
			cfg.mode = OS_SPI_MODE_2 | OS_SPI_MSB;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		cmd &= MP_SPI_MASK;
		if(cmd == MP_SPI_MODE_0)
		{
			cfg.mode = OS_SPI_MODE_0 | OS_SPI_LSB;
		}
		else if(cmd == MP_SPI_MODE_1)
		{
			cfg.mode = OS_SPI_MODE_1 | OS_SPI_LSB;
		}
		else if(cmd == MP_SPI_MODE_2)
		{
			cfg.mode = OS_SPI_MODE_2 | OS_SPI_LSB;
		}
		else if(cmd == MP_SPI_MODE_3)
		{
			cfg.mode = OS_SPI_MODE_2 | OS_SPI_LSB;
		}
		else
		{
			return -1;
		}
	}
	
	if(arg != NULL)
	{
		cfg.max_hz = ((int*)arg)[0];
		cfg.data_width = ((int*)arg)[1];
	}
	
	os_spi_configure((struct os_spi_device *)spi_device, &cfg);
	
	return 0;
}

STATIC struct operate spi_ops = {
	.open  =  spi_open,
	.read  =  spi_read,
	.write =  spi_write,
	.ioctl =  spi_ioctl,
	.close =  spi_close,
};	


int mpycall_spi_register(void)
{
	device_info_t  *pos, *spi = mp_misc_find_similar_device(MICROPYTHON_MACHINE_SPI_PRENAME);
	if (!spi){
		return -1;
	}

	//mpycall_device_add_list(spi);
	
	DEV_LIST_LOOP(pos, spi, get_list_head())
	{
		pos->owner.type = DEV_BUS;
		pos->ops = &spi_ops;
		
	}
	return 0;
}

OS_DEVICE_INIT(mpycall_spi_register);

#endif

