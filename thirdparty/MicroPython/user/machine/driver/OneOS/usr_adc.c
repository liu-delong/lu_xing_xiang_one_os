#include "usr_adc.h"
#include <stdio.h>
#include "os_device.h"
#include "os_errno.h"
#include "stdlib.h"
#ifdef BSP_USING_ADC
#include "adc.h"


static char *adc_name[] =
{
#ifdef BSP_USING_ADC1
    "adc1",
#endif
#ifdef BSP_USING_ADC2
	  "adc2",
#endif
#ifdef BSP_USING_ADC3
    "adc3",
#endif
};

#if 0
uint32_t basevolt = 3300;//mv
uint32_t resolution = 4096; //·Ö±æÂÊ
/*
int _adc_init(uint32_t pin, uint32_t mode)
{
	mp_hal_stdout_tx_strn("_adc_init", strlen("_adc_init"));
	return 0;
}
*/
int mpycall_adc_open(uint32_t num, uint32_t channal)
{
	char name[6] = {0};
	os_device_t adc_device;
	sprintf(name, "adc%d", (int)num);
	
	adc_device = os_device_find(name);
	
	adc_device->control(adc_device, RT_ADC_CMD_ENABLE, (void*)channal);
	
	//init adc
/* 	extern ADC_HandleTypeDef hadc1;
	if(HAL_ADC_Start(&hadc1) != HAL_OK)
		mp_hal_stdout_tx_strn("HAL_ADC_Start failed", strlen("HAL_ADC_Start failed"));

	mp_hal_stdout_tx_strn("_adc_open\n", strlen("_adc_open\n"));
 */
	return 0;
}

int mpycall_adc_close(uint32_t num, uint32_t channal)
{
	char name[6] = {0};
	os_device_t adc_device;
	sprintf(name, "adc%d", (int)num);
	
	adc_device = os_device_find(name);
	
	adc_device->control(adc_device, RT_ADC_CMD_DISABLE, (void*)channal);
	return 0;
}

#if 1
int mpycall_adc_read(uint32_t num)
{
	//int v;
	//char prtbuf[32];

/* 	if(pin == 25)
	{
		extern ADC_HandleTypeDef hadc1;
		if(HAL_ADC_Start(&hadc1) != HAL_OK)
			mp_hal_stdout_tx_strn("_adc_read failed", strlen("_adc_read failed"));
		//int volt = HAL_ADC_GetValue(&hadc1)*3300 >> 12;
		return (HAL_ADC_GetValue(&hadc1) * basevolt);
	}
	else
	{
		return -1;
	} */
	//v = (((float)volt)/4096)*3300;
	//sprintf(prtbuf,"v:%f",v);
	//mp_hal_stdout_tx_strn(prtbuf,strlen(prtbuf));
	
	char name[6] = {0};
	os_uint32_t volt;
	os_device_t adc_device;
	sprintf(name, "adc%d", (int)num);
	
	adc_device = os_device_find(name);
	
	adc_device->read(adc_device, 0, &volt, 1);
	
	return volt;
}
#endif




int mpycall_adc_ctrl(uint32_t pin, uint32_t ctl, int value)
{
//	mp_hal_stdout_tx_strn("_adc_ioctl", strlen("_adc_ioctl"));
//	if(ctl == SET_BASEV)
//	{
//		basevolt = value;
//	}
//	else if(ctl == SET_RESOLUTION)
//	{
//		resolution = value;
//	}
	return 0;
}

#endif

int adc_open(void *device, uint16_t oflag)
{
	int result = OS_EOK;
	os_adc_device_t adc_device = (os_adc_device_t)os_device_find(((device_info_t *)device)->owner.name);
	
	result = os_adc_open(&adc_device->parent, oflag);
	if(OS_EOK != result)
	{
		printf("adc enable failed!\n");
		return -1;
	}
	
	return 0;
}

int adc_read(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	int value = 0;
	os_adc_device_t adc_device = (os_adc_device_t)os_device_find(((device_info_t *)device)->owner.name);
	
	if(adc_device == NULL)
		return -1;
	
	value = os_adc_read(&adc_device->parent, offset, (os_int32_t *)&bufsize);
	((int*)buf)[0] = value;
	
	return 0;
}

int mpycall_adc_register(void)
{
	int adc_num = sizeof(adc_name)/sizeof(adc_name[0]);
	device_info_t * adc[adc_num];
	for (int i = 0; i < adc_num; i++)
	{
		adc[i] = (device_info_t *)malloc(sizeof(device_info_t));
	
		if(NULL == adc[i])
		{
			printf("mpycall_adc_register malloc mem failed!");
			return -1;
		}
	

		adc[i]->owner.name = adc_name[i];
		adc[i]->owner.type = DEV_BUS;
	
		adc[i]->ops = (struct operate *)malloc(sizeof(struct operate));
		
		if(NULL == adc[i]->ops)
		{
			printf("mpycall_adc_register malloc mem failed!");
			return -1;
		}
		
		adc[i]->ops->open = adc_open;
		adc[i]->ops->read = adc_read;

		
		mpycall_device_add(adc[i]);
	}
	return 0;
}

OS_DEVICE_INIT(mpycall_adc_register);
#endif

