#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "os_device.h"
#include "usr_rtc.h"


#include <rtc.h>

int rtc_ctrl(void *device, int cmd, void* arg)
{
	//os_device_t *rtc_device = os_device_find(((device_info_t *)device)->owner.name);
	
	int* data = arg;
	switch(cmd){
	
		case SET_DATE:
			set_date(data[0], data[1], data[2]);
			break;
		
		case SET_TIME:
			set_time(data[0], data[1], data[2]);
			break;
		#if 0
		case SET_ALARM:
			os_device_control(rtc_device, RT_DEVICE_CTRL_RTC_SET_ALARM, arg);
			break;
		#endif
		case GET_TIME:
		{
			time_t now;
			now = time(OS_NULL);
			char *strtime = ctime(&now);
			//os_kprintf("%s\n", strtime);
			//为什么要带个\n结尾呢？？？
			memcpy(arg,strtime, strlen(strtime)-1);
			break;
		}
		default:
			break;
	}
	
	
	return 0;
}

int mpycall_rtc_register(void)
{
	device_info_t * rtc;
	rtc = (device_info_t *)malloc(sizeof(device_info_t));
	
	if(NULL == rtc)
	{
		printf("mpycall_rtc_register malloc mem failed!");
		return -1;
	}
	

	rtc->owner.name = "rtc";
	rtc->owner.type = DEV_BUS;
	
	rtc->ops = (struct operate *)malloc(sizeof(struct operate));
	
	if(NULL == rtc->ops)
	{
		printf("mpycall_rtc_register malloc mem failed!");
		return -1;
	}
	
//	rtc->ops->open = rtc_open;
//	rtc->ops->read = rtc_read;
//	rtc->ops->write = rtc_write;
	rtc->ops->ioctl = rtc_ctrl;
	
	mpycall_device_add(rtc);
	
	return 0;
}

OS_DEVICE_INIT(mpycall_rtc_register);

