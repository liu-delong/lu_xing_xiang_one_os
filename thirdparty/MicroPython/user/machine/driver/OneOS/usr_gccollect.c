/*
 * gccollect.c
 *
 *  Created on: 2019年5月23日
 *      Author: admin
 */
/* get sp for gc_collect_root
 * *
 * */
#include <stdio.h>
#include "py/mpstate.h"
#include "py/gc.h"
#include "os_task.h"
#include "usr_misc.h"

//只会释放tail不会释放head,若要释放head需要调用gc_free即free
void gc_collect(void) {
/* 	 gc_dump_info();
    gc_collect_start();

#if MICROPY_PY_THREAD
    // trace root pointers from any threads
    mp_thread_gc_others();
#else
    // gc the main thread stack
    // 单线程模式下，获取当前线程的使用情况
    // 查找线程栈中 地址范围在heap中的数据并标记
    extern osThreadId defaultTaskHandle;
    extern void *stack_top;


    gc_collect_root((void*)(((TCB_t *)defaultTaskHandle)->pxTopOfStack), ((mp_uint_t)((void *)MP_STATE_THREAD(stack_top) - (void*)(((TCB_t *)defaultTaskHandle)->pxTopOfStack))) / 4);
#endif

    gc_collect_end();
    gc_dump_info(); */
	
	gc_dump_info();
    gc_collect_start();

#if MICROPY_PY_THREAD
    // trace root pointers from any threads
    mp_thread_gc_others();
#else
		gc_collect_root(os_task_self()->sp, ((mp_uint_t)((void *)MP_STATE_THREAD(stack_top) - os_task_self()->sp)) / 4);
#endif

    gc_collect_end();
    gc_dump_info();
}

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
void * mp_heap_malloc(size_t size_or_addr, mp_heap_flag_t flag)
{
	if (flag == MP_HEAP_ALLOC){
		return usr_malloc(size_or_addr);
	} 
	return (void *)size_or_addr;
}

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
void mp_heap_free(void * addr, mp_heap_flag_t flag)
{
	if (flag == MP_HEAP_ALLOC){
		free(addr);
	} 
	return ;
}
