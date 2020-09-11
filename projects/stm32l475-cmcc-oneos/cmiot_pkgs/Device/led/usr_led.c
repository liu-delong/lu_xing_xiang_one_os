#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"

#include <os_device.h>
#include <drv_cfg.h>
#include <os_clock.h>
#include <stdio.h>
#include <sensors/sensor.h>
#include "usr_led.h"
#include "drv_gpio.h"

static int led_init (int n)
{
    if(n > 2)
    {
        mp_raise_ValueError("the led num is wrong, please check!\n");
    }
    os_pin_mode(led_table[n].pin, PIN_MODE_OUTPUT);
		os_pin_write(led_table[n].pin, !led_table[n].active_level);
    return 0;
}

static int led_on (int n)
{
     os_pin_write(led_table[n].pin, led_table[n].active_level);
     return 0;
}

static int led_off (int n)
{
     os_pin_write(led_table[n].pin, !led_table[n].active_level);
     return 0;
}


int mpycall_led_init(void *dev, uint16_t oflag)
{
		 led_init(oflag);
	   return 0;
}

int mpycall_led_ioctl(void *dev, int cmd, void *arg)
{
    device_info_t *device = (device_info_t *)dev;
	  int led_color = 0; 
    if (NULL == device)
    {
        mp_raise_ValueError("device is NULL \n");
        return -1;
    }
		
		led_color = *((int *)arg);

    switch(cmd)
    {
        case  IOCTL_LED_INIT:
            return led_init(led_color);
        case  IOCTL_LED_ON:
            return led_on(led_color);
        case  IOCTL_LED_OFF:
            return led_off(led_color);
        default:
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return -1;
    }
}



int mpycall_led_register(void)
{
	device_info_t * led = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == led)
	{
		os_kprintf("mpycall_led_register malloc mem failed!");
		return -1;
	}
    memset(led, 0, sizeof(device_info_t));
	

	led->owner.name = "LED";
	led->owner.type = DEV_LED;
	
	led->ops = (struct operate *)os_malloc(sizeof(struct operate));
	
	if(NULL == led->ops)
	{
		os_kprintf("mpycall_led_register malloc mem failed!"); 
		return -1;
	}
  memset(led->ops, 0, sizeof(struct operate));
	
	led->ops->open = mpycall_led_init;
	led->ops->ioctl = mpycall_led_ioctl;
    
	mpycall_device_add(led);

	return 0;
}

OS_CMPOENT_INIT(mpycall_led_register);




