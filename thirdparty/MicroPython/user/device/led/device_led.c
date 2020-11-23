#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "usr_led.h"

/***********************led color************************/
#define  LED_RED                 ((uint32_t)0x00000000)
#define  LED_GREEN               ((uint32_t)0x00000001)
#define  LED_BLUE                ((uint32_t)0x00000002)



const mp_obj_type_t mp_led_type ;

typedef struct _device_touchkey_obj_t {
    mp_obj_base_t base;
	  int  led_color; 
    device_info_t *led_device_t;
}device_led_obj_t;


mp_obj_t mp_led_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 1, 1, false);        
	  device_info_t *led_device = mpycall_device_find("LED");

    if (led_device == NULL)
    {
         nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "LED doesn't exist", mp_obj_get_int(args[0])));
    }

    device_led_obj_t *self = m_new_obj(device_led_obj_t);
    self->base.type = &mp_led_type;
    self->led_device_t = led_device;

    //构造函数就要配置好led的管脚，所以此构造函数带参数，解析参数
    if (n_args != 1)
    {
        //这个参数检查貌似用不到，mp_arg_check_num已经对参数个数和参数类型做了检查
        mp_raise_ValueError("args can be void or mode only!\n");
    }
		self->led_color = mp_obj_get_int(args[0]);
    
    return (mp_obj_t) self;
}

STATIC mp_obj_t led_init(mp_obj_t self_in)
{
    device_info_t *led_device = ((device_led_obj_t*) self_in)->led_device_t;

    led_device->ops->open(led_device, ((device_led_obj_t *)self_in)->led_color);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(led_init_obj, led_init);


STATIC mp_obj_t led_on(mp_obj_t self_in)
{
    device_info_t *led_device = ((device_led_obj_t*) self_in)->led_device_t;

    led_device->ops->ioctl(led_device, IOCTL_LED_ON, &(((device_led_obj_t*) self_in)->led_color));

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(led_on_obj, led_on);

STATIC mp_obj_t led_off(mp_obj_t self_in)
{
    device_info_t *led_device = ((device_led_obj_t*) self_in)->led_device_t;

    led_device->ops->ioctl(led_device, IOCTL_LED_OFF, &(((device_led_obj_t*) self_in)->led_color));

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(led_off_obj, led_off);


STATIC const mp_rom_map_elem_t mp_module_led_globals_table[] =
{
	  { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&led_init_obj) },
		{ MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&led_off_obj) },
		{ MP_ROM_QSTR(MP_QSTR_LED_RED), MP_ROM_INT(LED_RED) },
		{ MP_ROM_QSTR(MP_QSTR_LED_BLUE), MP_ROM_INT(LED_BLUE) },
		{ MP_ROM_QSTR(MP_QSTR_LED_GREEN), MP_ROM_INT(LED_GREEN) },
};

STATIC MP_DEFINE_CONST_DICT(device_led_locals_dict, mp_module_led_globals_table);

const mp_obj_type_t mp_led_type =
{
    { &mp_type_type },
    .name = MP_QSTR_LED,
    .make_new = mp_led_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_led_locals_dict,
};


