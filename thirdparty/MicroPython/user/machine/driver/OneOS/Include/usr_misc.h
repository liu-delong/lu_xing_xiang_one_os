#ifndef __USR_MISC_H
#define __USR_MISC_H

#include <os_memory.h>
#include <stdint.h>
#include <model_device.h>

#define usr_malloc(sz)      os_malloc(sz)


typedef enum mp_heap{
	MP_HEAP_ALLOC,
	MP_HEAP_RAM_ADDR
}mp_heap_flag_t;

void usr_getchar_init(void);
void usr_getchar_deinit(void);

/**
*********************************************************************************************************
* @brief                                 分配堆空间
*
* @description: 调用此函数，为MicroPython分配堆空间。
*
* @param	  :	size_or_addr	需要分配的空间大小，或者RAM区域地址
*
*				flag			分配堆空间的方式
*								MP_HEAP_ALLOC: 调用malloc分配空间
*								MP_HEAP_RAM_ADDR：RAM地址设置方式，直接指定RAM区域为堆MicroPython
*
* @return	  :	MicroPython堆空间地址
*********************************************************************************************************
*/
void * mp_heap_malloc(size_t size_or_addr, mp_heap_flag_t flag);

/**
*********************************************************************************************************
* @brief                                 释放堆空间
*
* @description: 调用此函数，为MicroPython释放堆空间。
*
* @param	  :	addr		需要分配的空间大小，或者RAM区域地址
*
*				flag		目标堆空间当时创建的方式
*							MP_HEAP_ALLOC: 调用malloc分配空间
*							MP_HEAP_RAM_ADDR：RAM地址设置方式，直接指定RAM区域为堆MicroPython
*
* @return	  :	MicroPython堆空间地址
*********************************************************************************************************
*/
void mp_heap_free(void * addr, mp_heap_flag_t flag);


/**
*********************************************************************************************************
* @brief                                 字节数据转化为字符串
*
* @description: 调用此函数，将字节数据转化为字符串。
*
* @param	  :	data		字节数据
*
*				buf			缓存区，存放结果
*
*				flag		补位标志，高位补0， 当目标为个位数时，此标志起作用
*
* @return	  :	转化结果
*
* @example    : byte2char(2, buf, 0)  >>> "2" 不补位
*				byte2char(2, buf, 1)  >>> "02" 补位
*********************************************************************************************************
*/
inline static char *byte2char(uint8_t data, char *buf, uint8_t flag)
{
	uint8_t h_4bit = data >> 4, l_4bit = data &0x0f;
	if (!flag && data < 10){
		buf[0]  = (l_4bit > 9)? (l_4bit - 10 + 'A'): (l_4bit + '0'); 
		return buf;
	}
	buf[0]  = (h_4bit > 9)? (h_4bit - 10 + 'A'): (h_4bit + '0'); 
	buf[1]  = (l_4bit > 9)? (l_4bit - 10 + 'A'): (l_4bit + '0'); 
	return buf;
}


/**
*********************************************************************************************************
* @brief                                 释放整个链表
*
* @description: 调用此函数，释放整个链表。
*
* @param	  :	head		链表的头节点
*
* @return	  :	无 
*********************************************************************************************************
*/
void mp_free_list(mpy_os_list_t *head);


/**
*********************************************************************************************************
* @brief                                 查找操作系统底层同类设备
*
* @description: 调用此函数，查找操作系统底层同类设备。
*
* @param	  :	prename		前缀名，用以判断同类	
*
* @return	  :	设备链表
*
* @note		  : 设备链表是双向环形链表
*********************************************************************************************************
*/
device_info_t * mp_misc_find_similar_device(char *prename);


#endif
