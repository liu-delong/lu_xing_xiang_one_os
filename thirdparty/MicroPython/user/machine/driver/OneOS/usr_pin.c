#include "usr_pin.h"
#include <stdlib.h>

#include "py/runtime.h"



#ifdef MICROPY_PY_MACHINE_PIN

#include <drv_gpio.h>

os_base_t gpio_list[] ={
	#if defined(GPIOA_BASE)
	GPIOA_BASE,
	#endif
	#if defined(GPIOB_BASE)
	GPIOB_BASE,
	#endif
	#if defined(GPIOC_BASE)
	GPIOC_BASE,
	#endif
	#if defined(GPIOD_BASE)
	GPIOD_BASE,
	#endif
	#if defined(GPIOE_BASE)
	GPIOE_BASE,
	#endif
	#if defined(GPIOF_BASE)
	GPIOF_BASE,
	#endif
	#if defined(GPIOG_BASE)
	GPIOG_BASE,
	#endif
	#if defined(GPIOH_BASE)
	GPIOH_BASE,
	#endif
};


static int pin_read(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	return os_pin_read((os_base_t)offset);
}

static int pin_write(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	os_pin_write((os_base_t)offset, *((os_base_t*)buf));
	
	return 0;
}

static int pin_ctrl(void *device, int cmd, void* arg)
{
	os_pin_mode(*((os_base_t*)arg), (os_base_t)cmd);
	
	return 0;
}

static int pin_close(void *device)
{
	return 0;
}


/**
*********************************************************************************************************
*                                      获取gpio的序列号
*
* @description: 这个函数用来获取gpio的序列号。
*
* @param      : device:         设备。
*
*				mesg:			gpio的信息，如['A', 13]
* @returns    : gpio 的序列号，（操作系统层的序列号）
*********************************************************************************************************
*/
int mp_pin_get_num(void *device, void *mesg)
{
	char *data = (char *)mesg;
	int group_index = data[0];
	if (group_index > 'H' || group_index < 'A'){
		mp_raise_ValueError("[pin_get_num] parameters is wrong! \n");
		return -1;
	} else {
		group_index -= 'A';
	}
	int index = data[1];
	
	return (os_base_t)((16 * ((gpio_list[group_index] - (os_base_t)GPIOA_BASE) / (0x0400UL))) + index);
}

int mpycall_pin_register(void)
{
	device_info_t * pin;
	pin = (device_info_t *)malloc(sizeof(device_info_t));
	
	if(NULL == pin)
	{
		printf("mpycall_pin_register malloc mem failed!");
		return -1;
	}
	

	pin->owner.name = "pin";
	pin->owner.type = DEV_BUS;
	
	pin->ops = (struct operate *)malloc(sizeof(struct operate));
	
	if(NULL == pin->ops)
	{
		printf("mpycall_pin_register malloc mem failed!");
		return -1;
	}
	
	pin->ops->read = pin_read;
	pin->ops->write = pin_write;
	pin->ops->ioctl = pin_ctrl;	
	pin->ops->close = pin_close;
	pin->ops->suspend = mp_pin_get_num;
	mpycall_device_add(pin);
	return 0;
}

OS_DEVICE_INIT(mpycall_pin_register);

#endif // MICROPY_PY_MACHINE_PIN


