#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "user_beep.h"



const mp_obj_type_t mp_beep_type;

typedef struct _device_beep_obj_t {
    mp_obj_base_t base;
    device_info_t *beep_device;
}device_beep_obj_t;


mp_obj_t mp_beep_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    device_info_t * beep = mpycall_device_find("beep");
    if (NULL == beep){
        mp_raise_ValueError("beep can not find!\n");
    }
    device_beep_obj_t *self = m_new_obj(device_beep_obj_t);
    self->base.type = &mp_beep_type;
    self->beep_device = beep;
    
    return (mp_obj_t) self;
}

STATIC mp_obj_t  beep_init(mp_obj_t self_in)
{
    device_info_t *beep_device = ((device_beep_obj_t*) self_in)->beep_device;

    beep_device->ops->open(beep_device, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(beep_init_obj, beep_init);


STATIC mp_obj_t beep_on(mp_obj_t self_in)
{
    device_info_t *beep_device = ((device_beep_obj_t*) self_in)->beep_device;

    beep_device->ops->ioctl(beep_device, IOCTL_BEEP_ON, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(beep_on_obj, beep_on);

STATIC mp_obj_t beep_off(mp_obj_t self_in)
{
    device_info_t *beep_device = ((device_beep_obj_t*) self_in)->beep_device;

    beep_device->ops->ioctl(beep_device, IOCTL_BEEP_OFF, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(beep_off_obj, beep_off);



STATIC const mp_rom_map_elem_t mp_module_beep_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&beep_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_on), 	 MP_ROM_PTR(&beep_on_obj) },
	{ MP_ROM_QSTR(MP_QSTR_off),  MP_ROM_PTR(&beep_off_obj) },
};

STATIC MP_DEFINE_CONST_DICT(device_beep_locals_dict, mp_module_beep_globals_table);

const mp_obj_type_t mp_beep_type =
{
    { &mp_type_type },
    .name = MP_QSTR_BEEP,
    .make_new = mp_beep_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_beep_locals_dict,
};


