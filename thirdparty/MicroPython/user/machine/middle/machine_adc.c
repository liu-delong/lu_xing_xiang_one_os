#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include "usr_adc.h"
#include "stdio.h"

#if MICROPY_PY_MACHINE_ADC

typedef struct _machine_hard_adc_obj_t {
    mp_obj_base_t base;
    void* adc_device;
	  int resolution;
	  uint8_t channal;
} machine_hard_adc_obj_t;

/*
STATIC void machine_adc_obj_init_helper(machine_hard_adc_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_scl, ARG_sda, ARG_freq, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_resolution, MP_ARG_REQUIRED | MP_ARG_INT },
    };
		
		 struct {
        mp_arg_val_t resolution;
    } args;
    //mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);
//    self->scl = mp_hal_get_pin_obj(args[ARG_scl].u_obj);
//    self->sda = mp_hal_get_pin_obj(args[ARG_sda].u_obj);
//    self->us_timeout = args[ARG_timeout].u_int;
//    mp_hal_i2c_init(self, args[ARG_freq].u_int);
		self->resolution = args.resolution.u_int;
		
}
*/

mp_obj_t machine_hard_adc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    char adc_dev_name[6];

    snprintf(adc_dev_name, sizeof(adc_dev_name), "adc%d", mp_obj_get_int(all_args[0]));


    //mp_spi_device_handler* spi_device = middle_spi_open(mp_obj_get_int(all_args[0]));
		void* adc_device = mpycall_device_find(adc_dev_name);
		if (adc_device == NULL)
		{
					nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) doesn't exist", adc_dev_name));
		}
		

    // create new hard SPI object
    machine_hard_adc_obj_t *self = m_new_obj(machine_hard_adc_obj_t);
    self->base.type = &mp_module_adc;
    self->adc_device = adc_device;
		self->channal = mp_obj_get_int(all_args[1]);
		((device_info_t *)(adc_device))->ops->open(adc_device, self->channal);
//		mp_map_t kw_args;
//    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
//		machine_adc_obj_init_helper(self, n_args, all_args, &kw_args);
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_adc_read(mp_obj_t self_in) {

//	if(n_args < 1 || n_args > 2)
//	{
//		mp_raise_ValueError("args error\n");
//	}

//	//float data = _adc_read(args[1]);
//	int data = _adc_read(mp_obj_get_int(args[0]));
//	mp_float_t b = (mp_float_t)data / resolution;
//	return mp_obj_new_float(b);
	mp_obj_base_t *self = MP_OBJ_TO_PTR(self_in);
	int vol;
	device_info_t *device = ((device_info_t *)(((machine_hard_adc_obj_t*)self)->adc_device));
	
	device->ops->read(device, ((machine_hard_adc_obj_t*)self)->channal, &vol, 1);
	
	return mp_obj_new_int(vol);
}
MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_read_obj, machine_adc_read);

//STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_adc_read_obj, 1, 2, machine_adc_read);

STATIC mp_obj_t machine_adc_open(size_t n_args, const mp_obj_t *args) {

//	if(n_args < 2 || n_args > 3)
//	{
//		mp_raise_ValueError("args error\n");
//	}
//	int data = _adc_open(mp_obj_get_int(args[0]),mp_obj_get_int(args[1]));

//	if(data < 0)
//	{
//		mp_raise_ValueError("adc open error\n");
//	}
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_adc_open_obj, 2, 3, machine_adc_open);


STATIC mp_obj_t machine_adc_init(size_t n_args, const mp_obj_t *args) {

//	if(n_args < 2 || n_args > 3)
//	{
//		mp_raise_ValueError("args error\n");
//	}
//	//int data = _adc_init(args[1], args[2]);
//int data = 0;
//	if(data < 0)
//	{
//		mp_raise_ValueError("adc init error\n");
//	}
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_adc_init_obj, 2, 3, machine_adc_init);


STATIC mp_obj_t machine_adc_ioctl(size_t n_args, const mp_obj_t *args) {

//	if(n_args < 2 || n_args > 3)
//	{
//		mp_raise_ValueError("args error\n");
//	}
//	int data = _adc_ioctl(mp_obj_get_int(args[0]), mp_obj_get_int(args[1]), mp_obj_get_int(args[2]));

//	if(data < 0)
//	{
//		mp_raise_ValueError("adc ioctl error\n");
//	}
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_adc_ioctl_obj, 2, 3, machine_adc_ioctl);

STATIC mp_obj_t machine_adc_close(size_t n_args, const mp_obj_t *args) {

//	if(n_args < 1 || n_args > 2)
//	{
//		mp_raise_ValueError("args error\n");
//	}
//	int data = _adc_close(mp_obj_get_int(args[0]), mp_obj_get_int(args[1]));
//	if(data < 0)
//	{
//		mp_raise_ValueError("adc close error\n");
//	}
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_adc_close_obj, 1, 2, machine_adc_close);



STATIC const mp_rom_map_elem_t mp_module_adc_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_adc_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&machine_adc_open_obj) },
	{ MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&machine_adc_close_obj) },
	{ MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&machine_adc_ioctl_obj) },
	{ MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_adc_read_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_adc_globals, mp_module_adc_globals_table);

const mp_obj_type_t mp_module_adc = {
    { &mp_type_type },
		.name = MP_QSTR_ADC,
	  .make_new = machine_hard_adc_make_new,
    .locals_dict = (mp_obj_dict_t*)&mp_module_adc_globals,
};
#endif

