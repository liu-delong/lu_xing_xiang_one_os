#include "usr_adc.h"

#if MICROPY_PY_MACHINE_ADC
#include "adc.h"
#include "usr_misc.h"
#include "os_device.h"
#include "py/runtime.h"




static int adc_open(void *device, uint16_t oflag)
{
	if(device == NULL){
		mp_raise_ValueError("Couldn't find dac! \n");
		return -1;
	}

	os_device_t *adc_device = os_device_find(((device_info_t *)device)->owner.name);
	if(adc_device == OS_NULL){
		mp_raise_ValueError("Couldn't find dac! \n");
		return -1;
	}
	//os_kprintf("name = %s, channel=%d\n", adc_device->name);
	if(os_device_open(adc_device, OS_DEVICE_OFLAG_RDWR) != OS_EOK)
	{
		return -1;
	}
	((device_info_t *)device)->open_flag = MP_ADC_INIT_FLAG;
	return 0;
}

static int adc_read(void *device, uint32_t channel, void *buf, uint32_t bufsize)
{

	os_device_t *adc_device = os_device_find(((device_info_t *)device)->owner.name);
	
	if(adc_device == NULL)
	{
		return -1;
	}
	//os_adc_read((os_adc_device_t *)adc_device, channel, buf);
	os_device_read(adc_device, channel, buf, bufsize);
	return 0;
}

static int adc_close(void *device)
{
	os_device_t *adc_device = os_device_find(((device_info_t *)device)->owner.name);
	
	if (adc_device == NULL){
		return MP_ADC_OP_ERROR;
	}
	((device_info_t *)device)->open_flag = MP_ADC_DEINIT_FLAG;
	return os_device_close(adc_device);
}

STATIC struct operate adc_ops = {
	.open = adc_open,
	.read = adc_read,
	.write = NULL,
	.ioctl = NULL,
	.close = adc_close,
};

int mpycall_adc_register(void)
{
	device_info_t  *pos, *adc = mp_misc_find_similar_device(MICROPYTHON_MACHINE_ADC_PRENAME);
	if (!adc){
		return -1;
	}
	
	DEV_LIST_LOOP(pos, adc, get_list_head())
	{
		pos->owner.type = DEV_BUS;
		pos->ops = &adc_ops;
		
	}
	return 0;
}

OS_DEVICE_INIT(mpycall_adc_register);
#endif

