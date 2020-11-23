#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "usr_lightsensor_ap3216.h"


/****************************采样类型*******************************/
#define LIGHT                    		  (0x1)
#define PROXIMITYS                        (0x2)



const mp_obj_type_t mp_lightsensor_type;

typedef struct _device_lightsensor_obj_t {
    mp_obj_base_t base;
    device_info_t *lightsensor_device;
}device_lightsensor_obj_t;

mp_obj_t mp_lightsensor_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    device_info_t * lightsensor = mpycall_device_find("lightsensor");
    if (NULL == lightsensor)
    {
        mp_raise_ValueError("lightsensor can not find!\n");
    }
    device_lightsensor_obj_t *self = m_new_obj(device_lightsensor_obj_t);
    self->base.type = &mp_lightsensor_type;
    self->lightsensor_device = lightsensor;
    return (mp_obj_t) self;
}

STATIC mp_obj_t mp_lightsensor_init(mp_obj_t self)
{
    int ret = 0;
    device_lightsensor_obj_t *lightsensor = (device_lightsensor_obj_t *)self;
    if (NULL != lightsensor->lightsensor_device->ops && NULL != lightsensor->lightsensor_device->ops->ioctl)
    {
        ret = lightsensor->lightsensor_device->ops->open(lightsensor->lightsensor_device, 1);
        if (0 != ret)
        {
            mp_raise_ValueError("lightsensor_init failed!\n");
        }
    }
    else
    {
        printf("lightsensor init function!\n");
		mp_raise_ValueError("lightsensor_init failed!\n");
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mplightsensor_init_obj, mp_lightsensor_init);

STATIC mp_obj_t mp_lightsensor_deinit (mp_obj_t self)
{
    int ret = 0;
    device_lightsensor_obj_t *lightsensor = (device_lightsensor_obj_t *)self;
    if (NULL != lightsensor->lightsensor_device->ops && NULL != lightsensor->lightsensor_device->ops->ioctl)
    {
        ret = lightsensor->lightsensor_device->ops->close(lightsensor->lightsensor_device);
        if (0 != ret)
        {
            mp_raise_ValueError("lightsensor_deinit failed!\n");
        }
    }
    else
    {
        printf("lightsensor deinit function!\n");
		mp_raise_ValueError("lightsensor_deinit failed!\n");
    }
    
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mplightsensor_deinit_obj, mp_lightsensor_deinit);


STATIC mp_obj_t mp_lightsensor_read(size_t n_args, const mp_obj_t *args)
{
    int ret = 0;
    struct lightsensordata value = {0};
    device_lightsensor_obj_t *lightsensor = (device_lightsensor_obj_t *)args[0];
    if (NULL != lightsensor->lightsensor_device->ops && NULL != lightsensor->lightsensor_device->ops->ioctl)
	{
		ret = lightsensor->lightsensor_device->ops->ioctl(lightsensor->lightsensor_device, IOCTL_lightsensor_READ, &value);
		if (0 != ret) 
		{
			printf("mp_lightsensor_read failed!\n");
			return mp_const_none;
		}
		if (1 == n_args)
		{
			mp_obj_module_t *o = mp_obj_new_dict(2);
			mp_obj_dict_store(o, MP_OBJ_NEW_QSTR(MP_QSTR_LIGHT), mp_obj_new_float(value.light));
			mp_obj_dict_store(o, MP_OBJ_NEW_QSTR(MP_QSTR_PROXIMITYS), mp_obj_new_int(value.proximitys));
			return MP_OBJ_FROM_PTR(o);

		} 
		else if (2 == n_args) 
		{
			mp_int_t data = mp_obj_get_int(args[1]);
			if ((LIGHT != data) && (PROXIMITYS != data))
			{
				mp_raise_ValueError("lightsensor_deinit failed!\n");
			}

			if (LIGHT == data) 
			{
				return mp_obj_new_float(value.light);
			} 
			else 
			{
				return mp_obj_new_int(value.proximitys);
			}
		} 
	}
	else 
	{
		mp_raise_ValueError("lightsensor read n_args error!\n");
	}
	return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mplightsensor_read_obj, 1, 2, mp_lightsensor_read);



STATIC const mp_rom_map_elem_t mp_module_lightsensor_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), 	MP_ROM_PTR(&mplightsensor_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), 	MP_ROM_PTR(&mplightsensor_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), 	MP_ROM_PTR(&mplightsensor_read_obj) },

	{ MP_ROM_QSTR(MP_QSTR_LIGHT), MP_ROM_INT(LIGHT) },
    { MP_ROM_QSTR(MP_QSTR_PROXIMITYS), MP_ROM_INT(PROXIMITYS) },
};

STATIC MP_DEFINE_CONST_DICT(device_lightsensor_locals_dict, mp_module_lightsensor_globals_table);

const mp_obj_type_t mp_lightsensor_type =
{
    { &mp_type_type },
    .name = MP_QSTR_Lightsensor,
    .make_new = mp_lightsensor_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_lightsensor_locals_dict,
};


