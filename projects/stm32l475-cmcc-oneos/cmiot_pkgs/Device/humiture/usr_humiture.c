#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"

#include <os_device.h>
#include <sensors/sensor.h>
#include "user_humiture.h"


os_device_t *G_HUMI = NULL;
os_device_t *G_TEMP = NULL;



static int temperature_init (void)
{

	G_HUMI = os_device_find("humi_aht10");
	G_TEMP = os_device_find("temp_aht10");
	
	if(G_HUMI == OS_NULL || G_TEMP == OS_NULL){
		mp_raise_ValueError("Couldn't find humiture device(humi_aht10 | temp_aht10)! \n");
		return -1;
	}
	
    os_device_open(G_HUMI, OS_DEVICE_FLAG_RDWR);
    os_device_open(G_TEMP, OS_DEVICE_FLAG_RDWR);
	 
	
    return 0;
}


static int temperature_read(float *temperature)
{
	if (G_TEMP == NULL) return -1;
	struct os_sensor_data sensor_data = {0};
	struct os_sensor_info sensor_info;
	
    os_device_control(G_TEMP, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
	os_device_read(G_TEMP, 0, &sensor_data, sizeof(struct os_sensor_data));
   
    os_device_read(G_TEMP, 0, &sensor_data, sizeof(struct os_sensor_data));

	if (sensor_info.unit == OS_SENSOR_UNIT_MDCELSIUS) {
		*temperature = sensor_data.data.temp/ 1000;
    } else if (sensor_info.unit == OS_SENSOR_UNIT_DCELSIUS) {
		*temperature = sensor_data.data.temp;
    } else {
        os_kprintf("invalid unit\r\n");
    }
    
	return 0;
}

static int humidity_read(float *humidity)
{
	if (G_HUMI == NULL) return -1;
	struct os_sensor_data sensor_data = {0};

	struct os_sensor_info sensor_info;
    os_device_control(G_HUMI, OS_SENSOR_CTRL_GET_INFO, &sensor_info);

    os_device_read(G_HUMI, 0, &sensor_data, sizeof(struct os_sensor_data));

	if (sensor_info.unit == OS_SENSOR_UNIT_MPERMILLAGE)  {
		*humidity = sensor_data.data.humi /1000;
    } else if (sensor_info.unit == OS_SENSOR_UNIT_PERMILLAGE) {
		*humidity = sensor_data.data.humi;
    } else {
        os_kprintf("invalid unit\r\n");
    }
   
	return 0;
}


int  mpycall_humiture_init(void *dev, uint16_t oflag)
{
    device_info_t * dev_humiture = (device_info_t  *)dev;
    temperature_init();
    dev_humiture->open_flag = HUMITURE_INIT_FLAG;

    return 0;
}


int mpycall_humiture_deinit(void *dev)
{
    device_info_t * dev_oled = (device_info_t  *)dev;
    if (NULL == dev_oled)
    {
        mp_raise_ValueError("dev_oled is NULL!\n");
    }
	os_device_close(G_HUMI);
    os_device_close(G_TEMP);
    dev_oled->open_flag = HUMITURE_DEINIT_FLAG;

    return 0;
}


int mpycall_humiture_ioctl(void *dev, int cmd, void *arg)
{
//    device_info_t *device = (device_info_t *)dev;
//    if (NULL == device)
//    {
//        mp_raise_ValueError("device is NULL \n");
//        return -1;
//    }
	struct humituredata *humiture = arg;
    switch(cmd)
    {
        case  IOCTL_HUMITURE_READ:
		{
			temperature_read(&humiture->temperature);
			humidity_read(&humiture->humidity);
            return 0; 
		}
        default:
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return -1;
    }
}



int mpycall_humiture_register(void)
{
	device_info_t * humiture = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == humiture)
	{
		os_kprintf("mpycall_oled_register malloc mem failed!");
		return -1;
	}
    memset(humiture, 0, sizeof(device_info_t));
	

	humiture->owner.name = "humiture";
	humiture->owner.type = DEV_HUMITURE;
	
	humiture->ops = (struct operate *)os_malloc(sizeof(struct operate));
	
	if(NULL == humiture->ops)
	{
		os_kprintf("mpycall_i2c_register malloc mem failed!"); 
		return -1;
	}
    memset(humiture->ops, 0, sizeof(struct operate));
	
	humiture->ops->open = mpycall_humiture_init;
	humiture->ops->ioctl = mpycall_humiture_ioctl;
    humiture->ops->close = mpycall_humiture_deinit;
    
	mpycall_device_add(humiture);

	return 0;
}

OS_CMPOENT_INIT(mpycall_humiture_register);




