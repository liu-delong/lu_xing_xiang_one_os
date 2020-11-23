
#include "py/mperrno.h"

#if (MICROPY_PY_MACHINE_PWM)

#include "usr_misc.h"
#include "usr_pwm.h"


static int pwm_open(void *device, uint16_t oflag)
{
	int ret = OS_EOK;
	os_device_t *pwm_device = os_device_find(((device_info_t *)device)->owner.name);
	
	
	if(NULL == pwm_device)
	{
		printf("pwm find name error\n");
		return -MP_ENXIO;
	}

	ret = os_device_open(pwm_device, oflag);
	if (ret != OS_EOK)
	{
		ret = -MP_EPERM;
	}
	return ret;
}

static int pwm_close(void *device)
{
	int ret = OS_EOK;
	os_device_t *pwm_device = os_device_find(((device_info_t *)device)->owner.name);
	
	
	if(NULL == pwm_device)
	{
		printf("pwm find name error\n");
		return -MP_ENXIO;
	}

	ret = os_device_close(pwm_device);
	if (ret != OS_EOK)
	{
		ret = -MP_EPERM;
	}
	return ret;
}

/**
 *********************************************************************************************************
 *                                      pwm的ioctl函数
 *
 * @description: 调用此函数，可以实现打开和关闭pwm,可以实现周期和占空比的获取和设置
 *
 * @param 	  : device      通用设备结构体
 *
 *               cmd	        操作参数
 *
 *               arg        通过arg将所有需要的参数进行打包传入底层pwm驱动函数
 *
 *
 * @return     : 0           成功
 *
 *             ：-1或者其他负值  失败
 *
 * @note       : 这个函数是与machine_pwm.c对接的函数，因为micropython中对pwm的设置是频率和占空比数值，而设置到pwm驱动层
 *
 *               对应的是周期和占空比，均以ns为单位，所以此函数对此作了换算。
 *
 * @example    : 对内函数，无example
 *********************************************************************************************************
*/
static int pwm_ctrl(void *device, int cmd, void* arg)
{
	int ret = OS_EOK;
	os_device_t *pwm_device = os_device_find(((device_info_t *)device)->owner.name);
	
	
	if(NULL == pwm_device)
	{
		printf("pwm find name error\n");
		return -MP_ENXIO;
	}

	ret = os_device_control(pwm_device, cmd, arg);
	if (ret != OS_EOK)
	{
		ret = -MP_EPERM;
	}
	return ret;
}

//设置占空比
static int pwm_write(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	int ret = OS_EOK;
	os_device_t *pwm_device = os_device_find(((device_info_t *)device)->owner.name);
	
	
	if(NULL == pwm_device)
	{
		printf("pwm find name error\n");
		return -MP_ENXIO;
	}

	ret = os_device_write(pwm_device, offset, buf, bufsize);
	if (ret != OS_EOK)
	{
		ret = -MP_EPERM;
	}
	
	return ret;
}

STATIC struct operate pwm_ops = {
	.open = pwm_open,
	.read = NULL,
	.write = pwm_write,
	.ioctl = pwm_ctrl,
	.close = pwm_close,
};


int mpycall_pwm_register(void)
{
	device_info_t  *pos, *pwm = mp_misc_find_similar_device(MICROPYTHON_MACHINE_PWM_PRENAME);
	if (!pwm){
		return -1;
	}
	
	DEV_LIST_LOOP(pos, pwm, get_list_head())
	{
		pos->owner.type = DEV_BUS;		
		pos->ops = &pwm_ops;
	}

	return 0;
}

OS_DEVICE_INIT(mpycall_pwm_register);
#endif  // MICROPY_PY_MACHINE_PWM
