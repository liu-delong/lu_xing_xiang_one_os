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
#include "usr_lightsensor_ap3216.h"



typedef enum sta_result_light
{
    light_ok                      = 0,  ///< 无错，操作成功
    light_error                   = 1,  ///< 非特定错误
    light_timeout                 = 3,  ///< 超时错误
    light_framing                 = 9,  ///< 数据帧错误
	light_nnknow_error			  = 0x7F,
    light_no_response             = 0xFF
}sta_lightsensor_t;


static sta_lightsensor_t light_read (float *light)
{

	float light_t = 0;
	int count = 0;
	struct os_sensor_data sensor_data = {0};

	os_device_t *li_sensor = os_device_find("li_ap3216c");
	if(li_sensor == NULL)
	{
		return light_error;
	}

	struct os_sensor_info sensor_info;
    os_device_control(li_sensor, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
	for (int i = 0; i < 10; i++)
    {
        os_device_read(li_sensor, 0, &sensor_data, sizeof(struct os_sensor_data));

		if (sensor_info.unit == OS_SENSOR_UNIT_MLUX) 
		{
			//os_kprintf("lightsensor_read light MLUX [%d]: %d\n",i, sensor_data.data.light);
			count++;
			light_t += sensor_data.data.light/1000;
        }
		else
		{
			//os_kprintf("invalid unit\n");
		}
    }
    *light = light_t/count;

    return light_ok;
}

static sta_lightsensor_t proximity_read (int *proximity)
{

	int proximity_t = 0;
	int count = 0;
	struct os_sensor_data sensor_data = {0};

	os_device_t *pr_sensor = os_device_find("pr_ap3216c");
	if(pr_sensor == NULL)
	{
		return light_error;
	}

	struct os_sensor_info sensor_info;
    os_device_control(pr_sensor, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
	for (int i = 0; i < 10; i++)
    {
        os_device_read(pr_sensor, 0, &sensor_data, sizeof(struct os_sensor_data));

		if (sensor_info.unit == OS_SENSOR_UNIT_RAW) 
		{
			//os_kprintf("lightsensor_read light MLUX [%d]: %d\n",i, sensor_data.data.raw);
			count++;
			proximity_t += sensor_data.data.raw;
        }
		else
		{
			//os_kprintf("invalid unit\n");
		}
    }
	
    *proximity = (proximity_t/count);

    return light_ok;
}

/**
 * @brief Function for handling data from light sensor.
 *
 * @param[in] p_data          light in Celsius degrees read from sensor.
 */
static sta_lightsensor_t lightsensor_read (float *light, int *proximity)
{
	sta_lightsensor_t ret;
    ret =light_read(light);
	if(ret != light_ok)
		return ret;
	ret = proximity_read(proximity);
	if(ret != light_ok)
		return ret;
    return light_ok;
}

int  mpycall_lightsensor_init(void *dev, uint16_t oflag)
{
    device_info_t * dev_lightsensor = (device_info_t  *)dev;
	if (NULL == dev_lightsensor)
    {
        mp_raise_ValueError("dev_lightsensor is NULL!\n");
    }

	os_device_t *li_sensor = os_device_find("li_ap3216c");
	if(li_sensor == NULL)
	{
		os_kprintf("li_ap3216c not find\r\n");
		return light_error;
	}

	os_device_t *pr_sensor = os_device_find("pr_ap3216c");
	if(pr_sensor == NULL)
	{
		os_kprintf("pr_ap3216c not find\r\n");
		return light_error;
	}
	
	os_err_t ret = os_device_open(li_sensor, OS_DEVICE_FLAG_RDWR);
	if(ret != 0)
	{
		os_kprintf("device open fail\r\n");
		return light_error;
	}

	ret = os_device_open(pr_sensor, OS_DEVICE_FLAG_RDWR);
	if(ret != 0)
	{
		os_kprintf("device open fail\r\n");
		return light_error;
	}
	dev_lightsensor->open_flag = LIGHTSENSOR_INIT_FLAG;
    return light_ok;
}

int mpycall_lightsensor_deinit(void *dev)
{
    device_info_t * dev_lightsensor = (device_info_t  *)dev;
    if (NULL == dev_lightsensor)
    {
        mp_raise_ValueError("dev_lightsensor is NULL!\n");
    }

	os_device_t *li_sensor = os_device_find("li_ap3216c");
	if(li_sensor == NULL)
	{
		return light_error;
	}

	os_device_t *pr_sensor = os_device_find("pr_ap3216c");
	if(pr_sensor == NULL)
	{
		return light_error;
	}
	os_device_close(li_sensor);
	os_device_close(pr_sensor);

    dev_lightsensor->open_flag = LIGHTSENSOR_DEINIT_FLAG;

    return light_ok;
}


int mpycall_lightsensor_ioctl(void *dev, int cmd, void *arg)
{
    device_info_t *device = (device_info_t *)dev;
    if (NULL == device)
    {
        mp_raise_ValueError("device is NULL \n");
        return -1;
    }

	if(device->open_flag != LIGHTSENSOR_INIT_FLAG)
	{
		mp_raise_ValueError("device is not open \n");
        return -1;
    }

    switch(cmd)
    {
        case  IOCTL_lightsensor_READ:
            return lightsensor_read(&(((struct lightsensordata *)arg)->light), &(((struct lightsensordata *)arg)->proximitys));
        default:
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return light_error;
    }
}



int mpycall_lightsensor_register(void)
{
	device_info_t * lightsensor = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == lightsensor)
	{
		os_kprintf("mpycall_oled_register malloc mem failed!");
		return -1;
	}
    memset(lightsensor, 0, sizeof(device_info_t));
	

	lightsensor->owner.name = "lightsensor";
	lightsensor->owner.type = DEV_LIGHT;
	
	lightsensor->ops = (struct operate *)os_malloc(sizeof(struct operate));
	
	if(NULL == lightsensor->ops)
	{
		os_kprintf("lightsensor ops malloc mem failed!"); 
		return -1;
	}
    memset(lightsensor->ops, 0, sizeof(struct operate));
	
	lightsensor->ops->open = mpycall_lightsensor_init;
	lightsensor->ops->ioctl = mpycall_lightsensor_ioctl;
    lightsensor->ops->close = mpycall_lightsensor_deinit;
    

	mpycall_device_add(lightsensor);

	return 0;
}

OS_CMPOENT_INIT(mpycall_lightsensor_register);




