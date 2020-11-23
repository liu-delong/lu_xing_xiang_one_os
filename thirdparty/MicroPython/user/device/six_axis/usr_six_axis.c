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
#include "user_six_axis.h"

static int six_axis_init()
{
	 char *gyro_sensor_name = MICROPYTHON_DEVICE_GGYROSCOPE_NAME;
	 char *acce_sensor_name = MICROPYTHON_DEVICE_ACCELEROMETER_NAME;
	
	 os_device_t *gyro_sensor = os_device_find(gyro_sensor_name);
	 OS_ASSERT(gyro_sensor != NULL);
	 os_device_open(gyro_sensor, OS_DEVICE_FLAG_RDWR);
	 os_device_t *acce_sensor = os_device_find(acce_sensor_name);
	 OS_ASSERT(acce_sensor != NULL);
	 os_device_open(acce_sensor, OS_DEVICE_FLAG_RDWR);
	
	 return 0;
}

static int six_axis_deinit()
{
	 char *gyro_sensor_name = "gyro_icm20602";
	 char *acce_sensor_name = "acce_icm20602";
	
	 os_device_t *gyro_sensor = os_device_find(gyro_sensor_name);
	 OS_ASSERT(gyro_sensor != NULL);
	 os_device_close(gyro_sensor);
	 os_device_t *acce_sensor = os_device_find(acce_sensor_name);
	 OS_ASSERT(acce_sensor != NULL);
	 os_device_close(acce_sensor);
	
	 return 0;
}


static int gyro_sensor_read(struct os_sensor_data *sensor_data)
{
	 char *gyro_sensor_name = "gyro_icm20602";
	
	 os_device_t *gyro_sensor = os_device_find(gyro_sensor_name);
   os_device_read(gyro_sensor, 0, sensor_data, sizeof(struct os_sensor_data));
	 
	 return 0;

}

static int acce_sensor_read(struct os_sensor_data *sensor_data)
{
	 char *acce_sensor_name = "acce_icm20602";
	
	 os_device_t *acce_sensor = os_device_find(acce_sensor_name);
   os_device_read(acce_sensor, 0, sensor_data, sizeof(struct os_sensor_data));
	 
	 return 0;
}


int  mpycall_six_axis_init(void *dev, uint16_t oflag)
{
	device_info_t * dev_six_axis = (device_info_t  *)dev;
	six_axis_init();
	   
    dev_six_axis->open_flag = SIX_AXIS_INIT_FLAG;
    return 0;
}

int mpycall_six_axis_deinit(void *dev)
{
	device_info_t * dev_six_axis = (device_info_t  *)dev;
   six_axis_deinit();
   dev_six_axis->open_flag = SIX_AXIS_DEINIT_FLAG;
   return 0;
}



int mpycall_six_axis_ioctl(void *dev, int cmd, void *arg)
{
    device_info_t *device = (device_info_t *)dev;
    if (NULL == device)
    {
        mp_raise_ValueError("device is NULL \n");
        return -1;
    }
		
		struct os_sensor_data *sensor_data = (struct os_sensor_data *)arg;

    switch(cmd)
    {
			#if 0
        case  IOCTL_GYRO_READ:
            return gyro_sensor_read(sensor_data);
				case  IOCTL_ACCE_READ:
					  return acce_sensor_read(sensor_data);
			#endif
				case  IOCTL_GYRO_ACCE_READ:
					  gyro_sensor_read(&sensor_data[0]);
				    acce_sensor_read(&sensor_data[1]);
					  return 0;
        default:
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return -1;
    }
}



int mpycall_six_axis_register(void)
{
	device_info_t * six_axis = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == six_axis)
	{
		os_kprintf("mpycall_six_axis_register malloc mem failed!");
		return -1;
	}
    memset(six_axis, 0, sizeof(device_info_t));
	

	six_axis->owner.name = "six_axis";
	six_axis->owner.type = DEV_SIX_AXIS;
	
	six_axis->ops = (struct operate *)os_malloc(sizeof(struct operate));
	
	if(NULL == six_axis->ops)
	{
		os_kprintf("mpycall_six_axis_register malloc mem failed!"); 
		return -1;
	}
    memset(six_axis->ops, 0, sizeof(struct operate));
	
	six_axis->ops->open = mpycall_six_axis_init;
	six_axis->ops->ioctl = mpycall_six_axis_ioctl;
  six_axis->ops->close = mpycall_six_axis_deinit;
    
//    if (0 != mpy_inline_humiture_bus(humiture))
//    {
//        os_kprintf("i2c1 can not found!\n");
//        return -1;
//    }
	mpycall_device_add(six_axis);

	return 0;
}

OS_CMPOENT_INIT(mpycall_six_axis_register);




