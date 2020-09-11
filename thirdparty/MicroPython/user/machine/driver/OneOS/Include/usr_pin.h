#ifndef _USR_PIN_H
#define _USR_PIN_H
#include "py/obj.h"
#include "model_device.h"
#include "os_device.h"
#include <pin.h>

#define NAME_MAX    8

//用户实现Pin操作时需要对应以下的Mode
#define PIN_LOW                 0x00
#define PIN_HIGH                0x01

#define PIN_MODE_OUTPUT         0x00
#define PIN_MODE_INPUT          0x01
#define PIN_MODE_INPUT_PULLUP   0x02
#define PIN_MODE_INPUT_PULLDOWN 0x03
#define PIN_MODE_OUTPUT_OD      0x04

#define PIN_IRQ_MODE_RISING             0x00
#define PIN_IRQ_MODE_FALLING            0x01
#define PIN_IRQ_MODE_RISING_FALLING     0x02
#define PIN_IRQ_MODE_HIGH_LEVEL         0x03
#define PIN_IRQ_MODE_LOW_LEVEL          0x04

#define PIN_IRQ_DISABLE                 0x00
#define PIN_IRQ_ENABLE                  0x01

#define PIN_IRQ_PIN_NONE                -1

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    char name[NAME_MAX];
    uint32_t pin;
		uint32_t mode;
		uint32_t irq;
		mp_obj_t pin_isr_cb;
	  void *device;
} machine_pin_obj_t;


#define _pin_read	mpycall_pin_read
#define _pin_write	mpycall_pin_write
#define _pin_mode	mpycall_pin_ctrl

/*
int _pin_read(uint32_t pin);
void _pin_write(uint32_t pin, int value);
void _pin_mode(uint32_t pin, int mode);
*/

extern const mp_obj_type_t machine_pin_type;

typedef struct mpy_pin_index
{
    int index;
    //void (*rcc)(void);
    //GPIO_TypeDef *gpio;
    uint32_t pin;
}Pin_index;


/**
*********************************************************************************************************
*                                      获取gpio的序列号
*
* @description: 这个函数用来获取gpio的序列号。
*
* @param      : device:         设备。
*
*				mesg:			gpio的信息，如['A', 13]
* @returns    : gpio 的序列号，（操作系统层的序列号）
*********************************************************************************************************
*/
int mp_pin_get_num(void *device, void *mesg);

#endif
