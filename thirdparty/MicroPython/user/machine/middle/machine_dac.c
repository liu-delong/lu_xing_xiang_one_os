#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "machine_dac.h"
#include "usr_dac.h"

uint16_t channel=0;


typedef struct _device_dac_obj_t {
    mp_obj_base_t base;
    device_info_t *dac_device;
}device_dac_obj_t;


mp_obj_t mp_dac_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
	char dac_name[8] = {0};
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
	snprintf(dac_name, sizeof(dac_name)-1, "dac%d", mp_obj_get_int(args[0]));
    device_info_t * dac = mpycall_device_find(dac_name);
    if (NULL == dac){
        mp_raise_ValueError("dac can not find!\n");
    }
    device_dac_obj_t *self = m_new_obj(device_dac_obj_t);
    self->base.type = &machine_dac_type;
    self->dac_device = dac;

	self->dac_device->id =  mp_obj_get_int(args[0]);
	
    return (mp_obj_t) self;
}

STATIC mp_obj_t  dac_init(mp_obj_t self_in, mp_obj_t int_in)
{
	device_dac_obj_t *self = self_in;
    device_info_t *dac_device = ((device_dac_obj_t*) self)->dac_device;
	channel = mp_obj_get_int(int_in);
    dac_device->ops->open(dac_device, channel);
	self->dac_device->other = &channel;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(dac_init_obj,dac_init);


STATIC mp_obj_t dac_open(size_t n_args, const mp_obj_t *args)
{
	device_dac_obj_t *self = MP_OBJ_TO_PTR(args[0]);
	
    device_info_t *dac_device = ((device_dac_obj_t*) self)->dac_device;
	
	if (n_args == 3){
		self->dac_device->id = mp_obj_get_int(args[1]);
		channel = mp_obj_get_int(args[2]);
		char dac_name[8] = {0};
		snprintf(dac_name, sizeof(dac_name)-1, "dac%d", mp_obj_get_int(args[1]));
		dac_device = mpycall_device_find(dac_name);
	} else{
		channel = mp_obj_get_int(args[1]);
	}
	self->dac_device->other = &channel;
    dac_device->ops->open(dac_device, channel);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(dac_open_obj,2, 3, dac_open);


STATIC mp_obj_t dac_write(size_t n_args, const mp_obj_t *args)
{
	device_dac_obj_t *self = MP_OBJ_TO_PTR(args[0]);
	
    device_info_t *dac_device = ((device_dac_obj_t*) self)->dac_device;

	if (dac_device == NULL ||dac_device->ops->write == NULL || dac_device->open_flag == MP_DAC_DEINIT_FLAG){
		 mp_raise_ValueError("dac is closed!\n");
		return mp_const_none;
	}
	
	uint32_t  value;
	if (n_args == 3){
		channel = mp_obj_get_int(args[1]);
		value = mp_obj_get_int(args[2]);
	} else{
		channel = *(uint32_t *)self->dac_device->other;
		value = mp_obj_get_int(args[1]);
	}
	
    dac_device->ops->write(dac_device, channel, &value, 1);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(dac_write_obj, 2, 3, dac_write);

STATIC mp_obj_t dac_off(mp_obj_t self_in)
{
    device_info_t *dac_device = ((device_dac_obj_t*) self_in)->dac_device;

    dac_device->ops->close(dac_device);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(dac_off_obj, dac_off);



STATIC const mp_rom_map_elem_t mp_module_dac_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&dac_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_open), 	 MP_ROM_PTR(&dac_open_obj) },
	{ MP_ROM_QSTR(MP_QSTR_write), 	 MP_ROM_PTR(&dac_write_obj) },
	{ MP_ROM_QSTR(MP_QSTR_close),  MP_ROM_PTR(&dac_off_obj) },
};

STATIC MP_DEFINE_CONST_DICT(device_dac_locals_dict, mp_module_dac_globals_table);

const mp_obj_type_t machine_dac_type =
{
    { &mp_type_type },
    .name = MP_QSTR_DAC,
    .make_new = mp_dac_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_dac_locals_dict,
};


