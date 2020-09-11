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
#include "py/mphal.h"
#include "py/mperrno.h"



#if MICROPY_PY_MACHINE_WDT
#include <stdio.h>
#include <string.h>
#include <os_device.h>
#include <os_memory.h>
#include "usr_wdt.h"

#define WDT_NAME 	"iwdg"

os_device_t *G_wdt = OS_NULL;



void middle_wdt_print(mp_wdt_device_handler * wdt_device, const mp_print_t *print)
{
    mp_printf(print, "wdt device port name: iwdg");
}


static int wdt_init (void)
{
    G_wdt = os_device_find(WDT_NAME);
	if(G_wdt == OS_NULL){
		mp_raise_ValueError("Couldn't find watchdog! \n");
		return -1;
	}
	
    if (os_device_init(G_wdt) != OS_EOK)
    {
        mp_raise_ValueError("initialize iwdg failed!\n");
        return OS_ERROR;
    }

    return 0;
}

static int wdt_start(void)
{
	if(G_wdt == OS_NULL){
		mp_raise_ValueError("Couldn't find watchdog! Please initialize it\n");
		return -1;
	}
	
    if ( os_device_control(G_wdt, OS_DEVICE_CTRL_WDT_START, OS_NULL) != OS_EOK)
    {
        mp_raise_ValueError("start iwdg failed!\n");
        return -1;
    }
	return 0;
}

static int wdt_refresh(void)
{
	if(G_wdt == OS_NULL){
		mp_raise_ValueError("Couldn't find watchdog! Please initialize it\n");
		return -1;
	}
	
    if ( os_device_control(G_wdt, OS_DEVICE_CTRL_WDT_START, OS_NULL) != OS_EOK)
    {
        mp_raise_ValueError("start iwdg failed!\n");
        return -1;
    }
	return 0;
}

int mpycall_wdt_init(void *dev, uint16_t oflag)
{
	wdt_init();
	return 0;
}

int mpycall_wdt_ioctl(void *dev, int cmd, void *arg)
{
    device_info_t *device = (device_info_t *)dev;
    if (NULL == device)
    {
        mp_raise_ValueError("device is NULL \n");
        return -1;
    }

    switch(cmd)
    {
        case  IOCTL_WTD_INIT:
            return wdt_init();
        case  IOCTL_WTD_START:
            return wdt_start();
        case  IOCTL_WTD_KEEPALIVE:
            return wdt_refresh();
        default:
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return -1;
    }
}



int mpycall_wdt_register(void)
{
	device_info_t * wdt = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == wdt)
	{
		os_kprintf("[mpycall_wdt_register] Failed to malloc memory!"); 
		return -1;
	}
    memset(wdt, 0, sizeof(device_info_t));
	

	wdt->owner.name = "wdt";
	wdt->owner.type = DEV_WDT;
	
	wdt->ops = (struct operate *)os_malloc(sizeof(struct operate));
	
	if(NULL == wdt->ops)
	{
		os_kprintf("[mpycall_wdt_register] Failed to malloc memory!"); 
		return -1;
	}
    memset(wdt->ops, 0, sizeof(struct operate));
	
	wdt->ops->open =  mpycall_wdt_init;
	wdt->ops->ioctl = mpycall_wdt_ioctl;

	mpycall_device_add(wdt);

	return 0;
}

OS_CMPOENT_INIT(mpycall_wdt_register);



#endif
