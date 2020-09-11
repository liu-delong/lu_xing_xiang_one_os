#include <stdio.h>
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"

#include <os_device.h>
#include <drv_cfg.h>
#include <drv_gpio.h>



#if (MICROPY_PY_DAC)

#include "usr_dac.h"
#include "usr_misc.h"

/**
 ********************************************************************************************************
 *                                      dac 初始化
 *
 * @description: 初始化DAC模块
 *
 * @param 	   : device 		设备指针
 *
 *			     channel 		dac 通道
 *
 * @return     : 成功或失败
 * 
 * @note	   : (device_info_t *)dev 中的ref_count成员没有被使用
 ********************************************************************************************************
*/
static int dac_init(void *device, uint16_t channel)
{
	
	if(device == NULL){
		mp_raise_ValueError("Couldn't find dac! \n");
		return -1;
	}

	os_device_t *dac_device = os_device_find(((device_info_t *)device)->owner.name);
	if(dac_device == OS_NULL){
		mp_raise_ValueError("Couldn't find dac! \n");
		return -1;
	}
	os_kprintf("name = %s, channel=%d\n", dac_device->parent.name, channel);
	os_dac_enable((os_dac_device_t *)dac_device, channel);
	((device_info_t *)device)->open_flag = MP_DAC_INIT_FLAG;
	return 0;
}


static int dac_close(void *device)
{
	if(device == NULL){
		mp_raise_ValueError("Couldn't find dac! \n");
		return -1;
	}

    uint16_t channel = *(uint16_t*)(((device_info_t *)device)->other);
	os_device_t *dac_device = os_device_find(((device_info_t *)device)->owner.name);
	if(dac_device == OS_NULL){
		mp_raise_ValueError("Couldn't find dac! \n");
		return -1;
	 }
	 os_dac_disable((os_dac_device_t *)dac_device, channel);
	((device_info_t *)device)->open_flag = MP_DAC_DEINIT_FLAG;
	return 0;
}


static int dac_write(void *device, uint32_t channel, void *buf, uint32_t bufsize)
{
	os_device_t *dac_device = os_device_find(((device_info_t *)device)->owner.name);
	if(dac_device == OS_NULL){
		mp_raise_ValueError("Couldn't find dac! \n");
		return -1;
	}

	if (!channel){
		channel = *(uint16_t*)(((device_info_t *)device)->other);
	}
	
	os_kprintf("channel:%d, voltage:%d(mv)\n", channel, *(uint32_t *)buf);
	return os_dac_write((os_dac_device_t *)dac_device, channel, *(uint32_t *)buf);
}

STATIC struct operate dac_ops = {
	.open = dac_init,
	.read = NULL,
	.write = dac_write,
	.ioctl = NULL,
	.close = dac_close,
};


int mpycall_dac_register(void)
{
	device_info_t  *pos, *dac = mp_misc_find_similar_device("dac");
	if (!dac){
		return -1;
	}

	//mpycall_device_add_list(dac);
	
	DEV_LIST_LOOP(pos, dac, get_list_head())
	{
		pos->owner.type = DEV_BUS;
		pos->ops = &dac_ops;
		
	}
	return 0;
}

OS_CMPOENT_INIT(mpycall_dac_register);

#endif


