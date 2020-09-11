
#include "py/mperrno.h"

#if MICROPY_PY_MACHINE_PM
#include "usr_pm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os_device.h"

int pm_ctrl(void *device, int cmd, void* arg)
{
	os_device_t *pm_device = os_device_find(((device_info_t *)device)->owner.name);
	//int* data = arg;
	switch(cmd){
	
		case MP_PM_SLEEP_MODE_NONE:
			break;
		
		case MP_PM_SLEEP_MODE_IDLE:
			break;
		
		case MP_PM_SLEEP_MODE_LIGHT:
			
			break;
		
		case MP_PM_SLEEP_MODE_DEEP:
			break;
		
		case MP_PM_SLEEP_MODE_STANDBY:
			#ifdef OS_USING_LPMGR
			os_lpmgr_request(SYS_SLEEP_MODE_STANDBY);
			#endif
			break;
		
		case MP_PM_SLEEP_MODE_SHUTDOWN:
			#ifdef OS_USING_LPMGR
			os_lpmgr_request(SYS_SLEEP_MODE_SHUTDOWN);
			#endif
			break;
		
		default:
			break;
	}

	return 0;
}

int mpycall_pm_register(void)
{
	device_info_t * pm;
	pm = (device_info_t *)malloc(sizeof(device_info_t));
	
	if(NULL == pm)
	{
		printf("mpycall_pm_register malloc mem failed!");
		return -1;
	}
	

	pm->owner.name = "pm";
	pm->owner.type = DEV_BUS;
	
	pm->ops = (struct operate *)malloc(sizeof(struct operate));
	
	if(NULL == pm->ops)
	{
		printf("mpycall_pm_register malloc mem failed!");
		return -1;
	}
	
	pm->ops->ioctl = pm_ctrl;
	
	mpycall_device_add(pm);
	
	return 0;
}

OS_DEVICE_INIT(mpycall_pm_register);

#endif //MICROPY_PY_MACHINE_PM
