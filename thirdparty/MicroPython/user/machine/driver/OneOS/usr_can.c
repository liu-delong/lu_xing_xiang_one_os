/*
 * This file is paos of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial poosions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PAosICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TOos OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "py/runtime.h"

#if (MICROPY_PY_MACHINE_CAN)
#include "can.h"
#include "bus_can.h"
#include "usr_misc.h"
#include <stdio.h>
#include <string.h>
#include <os_device.h>


static int rx_count = 0;
static int tx_count = 0;


int can_open(void *device, uint16_t oflag)
{
	os_device_t *can_device = os_device_find(((device_info_t *)device)->owner.name);
	if(os_device_open(can_device, OS_DEVICE_OFLAG_RDWR) != OS_EOK)
	{
		return MP_CAN_OP_ERROR;
	}
	((device_info_t *)device)->open_flag = MP_CAN_INIT_FLAG;
	return MP_CAN_OP_OK;
}

int can_close(void *device)
{
	os_device_t *can_device = os_device_find(((device_info_t *)device)->owner.name);
	
	if (can_device == NULL){
		return MP_CAN_OP_ERROR;
	}
	((device_info_t *)device)->open_flag = MP_CAN_DEINIT_FLAG;
	return os_device_close(can_device);
}

static uint32_t Byte2HexStr(uint8_t *buf, uint32_t bufsize, char *dest)
{	
	int i= 0;
	for (; i < bufsize; i++){
		byte2char(buf[i], dest+i*2, 1);
	}
	return i*2;
}


static os_err_t can_rx_call(os_device_t *dev, os_size_t size)
{
    rx_count++;
    return MP_CAN_OP_OK;
}


static int can_read(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	device_info_t * dev = device;
	can_msg_t *res_msg = (can_msg_t *)buf;
	struct os_can_msg msg = {0}; 
	uint8_t id_buf[4] = {0};
	os_device_t *can_device = os_device_find(dev->owner.name);
	if(NULL == can_device)
	{
		os_kprintf("can_read find name error\n");
		return MP_CAN_OP_ERROR;
	}
	os_device_set_rx_indicate(can_device, can_rx_call);
	if (os_device_read(can_device, offset, &msg, sizeof(msg)) > 0){
		id_buf[3] = msg.id & 0xff;
		id_buf[2] = (msg.id >> 8) & 0xff;
		id_buf[1] = (msg.id >> 16) & 0xff;
		id_buf[0] = (msg.id >> 24) & 0xff;

		Byte2HexStr(id_buf, 4, res_msg->id);
		res_msg->ide = msg.ide;
		res_msg->rtr = msg.rtr;
		res_msg->len = Byte2HexStr(msg.data, msg.len, res_msg->data);
		return res_msg->len;
	}
	return MP_CAN_OP_OK;
}

static int CharNum2intNum(uint8_t data)
{
	if(data <=  '9'){
		return  data - '0';
	} else if (data >=  'A' && data <= 'F'){
		return data - 'A' + 10;
	} else if  (data >=  'a' && data <= 'f'){	 
		return data - 'a'+ 10;
	}
	return MP_CAN_OP_ERROR;
}

static int HexStr2Byte(char *buf, uint32_t bufsize, uint8_t *dest)
{
	uint8_t j=0;
	int h_4bit, l_4bit;
	if (strlen((char *)buf) != bufsize || bufsize%2 != 0){
		return MP_CAN_OP_ERROR;
	}
	for (int i=0; i < bufsize; i+=2){
		h_4bit = CharNum2intNum(buf[i]);
		l_4bit = CharNum2intNum(buf[i+1]);
		if (h_4bit == MP_CAN_OP_ERROR || l_4bit == MP_CAN_OP_ERROR){
			return 0;
		}
		dest[j] = h_4bit << 4 |  l_4bit;
		j++;
	}
	return j;
}


static struct os_can_msg *check_data(uint32_t *buf, uint32_t bufsize, struct os_can_msg *msg)
{
	int len ;
	uint8_t dest[30];
	if (bufsize <7 || bufsize > 30|| strlen((char *)buf) != bufsize ||  bufsize%2 != 0){
		os_kprintf("the parameter is invalid!\n");
		return NULL;
	}
	
	len = HexStr2Byte((char*)buf, bufsize, dest);
	if (!len){
		os_kprintf("[check_data] Failed to convert data! \n");
		return NULL;
	}
	msg->id  = dest[0] << 24 | dest[1] << 16 | dest[2] << 8 | dest[3];
	if (dest[4] > 1 || dest[5] > 1){
		os_kprintf("the parameter is invalid! id is 4 bytes of data, ide is wrong or rtr is wrong!\n"); 
		return NULL;
	} else {
		msg->ide = dest[4];//OS_CAN_STDID; // id 扩展帧
		msg->rtr = dest[5];//OS_CAN_DTR;   // 表示数据帧的有无
	}
	msg->len = dest[6];
	if (len > 7){
		memcpy(msg->data, dest+7, msg->len);
	}		
	return msg;
}

static os_err_t can_tx_done(os_device_t *uart, void *buffer)
{
    tx_count++;
    return MP_CAN_OP_OK;
}

static int can_write(void *device, uint32_t offset, void *buf, uint32_t bufsize)
{
	struct os_can_msg msg = {0};
	os_device_t *can_device = os_device_find(((device_info_t *)device)->owner.name);
	
	if(NULL == can_device)
	{
		os_kprintf("can_write find name error\n");
		return MP_CAN_OP_ERROR;
	}
	if (! check_data(buf, bufsize, &msg)){
		os_kprintf("can_write failed! \n");
		return MP_CAN_OP_ERROR;
	}
	os_device_set_tx_complete(can_device, can_tx_done);
	return os_device_write(can_device, offset, &msg, sizeof(msg));
}



int can_ioctl(void *device, int cmd, void* arg)
{
	os_device_t *can_device  =  os_device_find(((device_info_t *)device)->owner.name);
	if (can_device == NULL){
		os_kprintf("can_ioctl find device error\n");
		return MP_CAN_OP_ERROR;
	}
	
	switch(cmd){
		case MP_CAN_IOCTL_PARAM:{
			os_device_control(can_device, OS_CAN_CMD_SET_BAUD, (void *)((mp_can_obj_t *)arg)->baud);
			os_device_control(can_device, OS_CAN_CMD_SET_MODE, (void *)(((mp_can_obj_t *)arg)->mode));
			break;
		}
		case OS_CAN_CMD_SET_PRIV:
			os_device_control(can_device, OS_CAN_CMD_SET_PRIV, (void *)(uint32_t)((mp_can_obj_t *)arg)->prescale);
			break;
		case OS_CAN_CMD_SET_BAUD:
			os_device_control(can_device, OS_CAN_CMD_SET_BAUD, (void *)((mp_can_obj_t *)arg)->baud);
			break;
		case OS_CAN_CMD_SET_MODE:
			os_device_control(can_device, OS_CAN_CMD_SET_MODE, (void *)(((mp_can_obj_t *)arg)->mode));
			break;
		case OS_CAN_CMD_SET_FILTER:
			os_device_control(can_device, OS_CAN_CMD_SET_FILTER, arg);
			break;
		case OS_CAN_CMD_GET_STATUS:
			os_device_control(can_device, OS_CAN_CMD_GET_STATUS, arg);
			break;	
	}
	
	return 0;
}

STATIC struct operate can_ops = {
	.open = can_open,
	.read = can_read,
	.write = can_write,
	.ioctl = can_ioctl,
	.close = can_close,
};
	
int mpycall_can_register(void)
{
	device_info_t  *pos, *can = mp_misc_find_similar_device("can");
	if (!can){
		return -1;
	}
	
	DEV_LIST_LOOP(pos, can, get_list_head())
	{
		pos->owner.type = DEV_BUS;
		pos->ops = &can_ops;
		
	}
	return 0;
}

OS_DEVICE_INIT(mpycall_can_register);


#endif // MICROPY_PY_MACHINE_CAN

