#include <stdio.h>
#include <string.h>
#include <os_memory.h>
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
#include "usr_key.h"
#include "drv_gpio.h"
#include "pin.h"

typedef void (*fun_t)(void *args);

static int key_init (int n)
{
    if(n > 4)
    {
        mp_raise_ValueError("the key num is wrong, please check!\n");
    }

    os_pin_mode(key_table[n].pin, key_table[n].mode);

    return 0;
}

int key_callback(void *pin_callback, int n,  device_key_obj_t *key_self)
{
	  //os_pin_attach_irq(key_table[n].pin, key_table[n].irq_mode, pin_callback, (void *)key_table[n].pin);
	  os_pin_attach_irq(key_table[n].pin, key_table[n].irq_mode, (fun_t)pin_callback, (void *)key_self);
	  os_pin_irq_enable(key_table[n].pin, PIN_IRQ_ENABLE);

	  return 0;
}
int mpycall_key_callback( device_key_func_obj_t *key, int n)
{
		key_callback(key->machine_key_isr_handler , n, key->key_self);
	  return 0;
}

int mpycall_key_init(void *dev, uint16_t oflag)
{
	  device_info_t *device = (device_info_t *)dev;
		int key_id = 0; 
		if (NULL == device)
		{
				mp_raise_ValueError("device is NULL \n");
				return -1;
		}
		key_id = (int)device->id;
		key_init(key_id);
	  return 0;
}

int mpycall_key_ioctl(void *dev, int cmd, void *arg)
{
		device_info_t *device = (device_info_t *)dev;
	  device_key_func_obj_t *key = NULL;
		int key_id = 0; 
		if (NULL == device)
		{
				mp_raise_ValueError("device is NULL \n");
				return -1;
		}
		//key_id = (int)(device->id);
		#if 1
		key = (device_key_func_obj_t *)arg;
		key_id = key->key_self->key_device_t->id;
		#endif
		
		
		switch(cmd)
    {
        case  IOCTL_KEY_INIT:
            return key_init(key_id);
        case  IOCTL_KEY_CALLBACK:
					  return mpycall_key_callback(key, key_id);
        default:
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return -1;
    }
}

int mpycall_key_register(void)
{
	device_info_t * key = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == key)
	{
		os_kprintf("mpycall_key_register malloc mem failed!");
		return -1;
	}
    memset(key, 0, sizeof(device_info_t));
	

	key->owner.name = "KEY";
	key->owner.type = DEV_KEY;
	
	key->ops = (struct operate *)os_malloc(sizeof(struct operate));
	
	if(NULL == key->ops)
	{
		os_kprintf("mpycall_key_register malloc mem failed!"); 
		return -1;
	}
    memset(key->ops, 0, sizeof(struct operate));
	
	key->ops->open = mpycall_key_init;
	key->ops->ioctl = mpycall_key_ioctl;
    
	mpycall_device_add(key);

	return 0;
}

OS_CMPOENT_INIT(mpycall_key_register);







