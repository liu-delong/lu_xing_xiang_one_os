#include <stdio.h>
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "usr_i2c.h"

#ifdef MICROPY_PY_I2C
#include "usr_misc.h"


static int i2c_open(void *device, uint16_t oflag)
{
	os_device_t *i2c_device = os_device_find(((device_info_t *)device)->owner.name);
	return os_device_open(i2c_device, OS_DEVICE_OFLAG_RDWR | OS_DEVICE_FLAG_INT_RX );
}

static int i2c_read(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	os_device_t *i2c_device = os_device_find(((device_info_t *)device)->owner.name);
	
	if(NULL == i2c_device)
	{
		printf("i2c_read find name error\n");
		return -1;
	}
	
	return os_device_read(i2c_device, offset, buf, bufsize);
}

static int i2c_write(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	os_device_t *i2c_device = os_device_find(((device_info_t *)device)->owner.name);
	

	if(NULL == i2c_device)
	{
		printf("i2c_write find name error\n");
		return -1;
	}
	
	return os_device_write(i2c_device, offset, buf, bufsize);

}

static int i2c_ctrl(void *device, int cmd, void* arg)
{
	os_device_t *i2c_device = os_device_find(((device_info_t *)device)->owner.name);
	if(NULL == i2c_device)
	{
		printf("i2c_ctrl find name error\n");
		return -1;
	}

	return 0;
}

static int i2c_close(void *device)
{
	os_device_t *i2c_device = os_device_find(((device_info_t *)device)->owner.name);
	
	if(NULL == i2c_device)
	{
		printf("i2c_close find name error\n");
		return -1;
	}

	return os_device_close(i2c_device);
}

STATIC struct operate i2c_ops = {
	.open = i2c_open,
	.read = i2c_read,
	.write = i2c_write,
	.ioctl = i2c_ctrl,
	.close = i2c_close,
};

int mpycall_i2c_register(void)
{
	device_info_t  *pos, *i2c = mp_misc_find_similar_device("i2c");
	if (!i2c){
		return -1;
	}

	//mpycall_device_add_list(i2c);
	
	DEV_LIST_LOOP(pos, i2c, get_list_head())
	{
		pos->owner.type = DEV_BUS;
		pos->ops = &i2c_ops;	
	}
	
	return 0;
}
OS_DEVICE_INIT(mpycall_i2c_register);


void test_devmodel_i2c(void)
{
	os_uint8_t cmd[32] = {0};
	os_uint8_t recvbuf[32] = {0};
	mpycall_device_listall();
	
	device_info_t * i2c1 = mpycall_device_find("i2c1");
	cmd[0] = 0x00;
	cmd[1] = 'a';
	cmd[2] = 'b';
	
	i2c1->ops->open(i2c1, 0);
	i2c1->ops->write(i2c1, 0x50<<1, cmd, 3);
	os_task_mdelay(5);
	
	i2c1->ops->write(i2c1, 0x50<<1, cmd, 1);
	i2c1->ops->read(i2c1, ((0x50<<1) + 1), recvbuf, 2);
	printf("i2c read e2prom: %s\n", recvbuf);
	
}

#ifdef FINSH_USING_MSH
#include <shell.h>
SH_CMD_EXPORT(test_devmodel_i2c, i2c1 read write e2prom);
#endif
#endif

