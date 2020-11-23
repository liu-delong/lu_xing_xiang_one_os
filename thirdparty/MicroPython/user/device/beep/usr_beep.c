#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"

#include <drv_gpio.h>
#include "user_beep.h"



int BEEP_PIN  =	GET_PIN(B, 2);



static int beep_on (void)
{
     os_pin_write(BEEP_PIN, PIN_HIGH);
     return 0;
}

static int beep_off (void)
{
     os_pin_write(BEEP_PIN, PIN_LOW);
     return 0;
}

static int beep_init(void *dev, uint16_t oflag)
{
	os_pin_mode(BEEP_PIN, PIN_MODE_OUTPUT);
	os_pin_write(BEEP_PIN, PIN_LOW);
	return 0;
}

static int beep_ioctl(void *dev, int cmd, void *arg)
{
    device_info_t *device = (device_info_t *)dev;
    if (NULL == device)
    {
        mp_raise_ValueError("device is NULL \n");
        return -1;
    }

    switch(cmd)
    {
        case  IOCTL_BEEP_ON:
            return beep_on();
        case  IOCTL_BEEP_OFF:
            return beep_off();
        default:
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return -1;
    }
}



static int beep_register(void)
{
	device_info_t * beep = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == beep)
	{
		os_kprintf("mpycall_beep_register malloc mem faibeep!");
		return -1;
	}
    memset(beep, 0, sizeof(device_info_t));
	

	beep->owner.name = "beep";
	beep->owner.type = DEV_BEEP;
	
	beep->ops = (struct operate *)os_malloc(sizeof(struct operate));
	
	if(NULL == beep->ops)
	{
		os_kprintf("mpycall_beep_register malloc mem faibeep!"); 
		return -1;
	}
    memset(beep->ops, 0, sizeof(struct operate));
	
	beep->ops->open =  beep_init;
	beep->ops->ioctl = beep_ioctl;

	mpycall_device_add(beep);

	return 0;
}

OS_CMPOENT_INIT(beep_register);




