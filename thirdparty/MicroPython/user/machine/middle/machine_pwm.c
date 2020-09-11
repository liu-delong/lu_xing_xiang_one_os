
#include "py/runtime.h"

#if (MICROPY_PY_MACHINE_PWM)
#include "usr_pwm.h"
#include "py/obj.h"
#include <string.h>
#include "py/builtin.h"



typedef struct _machine_hard_pwm_obj_t {
    mp_obj_base_t base;
	  int duty;
	  uint8_t channal;
	  int freq;
	  device_info_t * pwm_device;
} machine_hard_pwm_obj_t;

mp_obj_t machine_hard_pwm_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
		int  ret = 0;
		char pwm_dev_name[10]={0};
		snprintf(pwm_dev_name, sizeof(pwm_dev_name), "pwm_tim%d", mp_obj_get_int(all_args[0]));
		device_info_t * pwm_device = mpycall_device_find(pwm_dev_name);
		if (pwm_device == NULL || pwm_device->ops == NULL)
		{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) doesn't exist", pwm_dev_name));
		}

		machine_hard_pwm_obj_t *self = m_new_obj(machine_hard_pwm_obj_t);
		self->base.type = &mp_module_pwm;
		self->pwm_device = pwm_device;
		self->channal = mp_obj_get_int(all_args[1]);
		self->freq = mp_obj_get_int(all_args[2]);
		self->duty = mp_obj_get_int(all_args[3]);
		
		struct pwm_config pwm_args = {0};
		pwm_args.channel = self->channal;
		pwm_args.frequency = self->freq;
		pwm_args.duty = self->duty;

		int period = 1e9 / pwm_args.frequency;
		int pulse = period * ((float)(pwm_args.duty) / 100);
		//os_kprintf("freq:%d, period:%d; duty:%d, pluse:%d\n",pwm_args.frequency,period,pwm_args.duty,pulse);
		ret = pwm_device->ops->open(pwm_device, OS_DEVICE_FLAG_RDWR);
		if (0 != ret)
		{
				nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) can not open", pwm_dev_name));
		}
		
		ret = pwm_device->ops->ioctl(pwm_device, USR_PWM_CMD_SET_PERIOD, &period);
		if (0 != ret)
		{
				nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) can not set freq , please check if freq  is valid", pwm_dev_name));
		}
		
		ret = pwm_device->ops->write(pwm_device, pwm_args.channel, &pulse,sizeof(pulse));
		if (0 != ret)
		{
				nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) can not set  duty, please check if  duty is valid", pwm_dev_name));
		}
		
		ret = pwm_device->ops->ioctl(pwm_device, USR_PWM_CMD_ENABLE, &pwm_args.channel);
		if (0 != ret)
		{
				nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) can not enable", pwm_dev_name));
		}

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_pwm_init(mp_obj_t self_in, mp_obj_t freq, mp_obj_t duty) {

	int  ret = 0;
	mp_obj_base_t *self = MP_OBJ_TO_PTR(self_in);
	machine_hard_pwm_obj_t* pwm_obj = (machine_hard_pwm_obj_t*)self;
	
	struct pwm_config pwm_args = {0};
	device_info_t *device = ((device_info_t *)(pwm_obj->pwm_device));

	pwm_args.channel = pwm_obj->channal;
	pwm_args.frequency = mp_obj_get_int(freq);
	pwm_args.duty = mp_obj_get_int(duty);

	int period = 1e9 / pwm_args.frequency;
	int pulse =	period * ((float)(pwm_args.duty) / 100);
	//os_kprintf("freq:%d, period:%d; duty:%d, pluse:%d\n",pwm_args.frequency,period,pwm_args.duty,pulse);
	//os_kprintf("channel:%d\n",pwm_args.channel);
	ret = device->ops->open(device, OS_DEVICE_FLAG_RDWR);
	if (0 != ret)
	{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pwm can not open"));
	}
	ret = device->ops->ioctl(device, USR_PWM_CMD_SET_PERIOD, &period);
	if (0 != ret)
	{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "freq = %d can not init\n", pwm_args.frequency, pwm_args.frequency));
	}
	
	ret = device->ops->write(device, pwm_args.channel, &pulse,sizeof(pulse));
	if (0 != ret)
	{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "duty %d = can not init\n", pwm_args.frequency, pwm_args.duty));
	}
	ret = device->ops->ioctl(device, USR_PWM_CMD_ENABLE, &pwm_args.channel);
	if (0 != ret)
	{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pwm can not enable"));
	}
	pwm_obj->freq = pwm_args.frequency;
	pwm_obj->duty = pwm_args.duty;
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(machine_pwm_init_obj, machine_pwm_init);

STATIC mp_obj_t machine_pwm_deinit(mp_obj_t self_in) {

	int  ret = 0;
	mp_obj_base_t *self = MP_OBJ_TO_PTR(self_in);
	machine_hard_pwm_obj_t* pwm_obj = (machine_hard_pwm_obj_t*)self;
	device_info_t *device = ((device_info_t *)(pwm_obj->pwm_device));
	
	struct pwm_config pwm_args = {0};
	pwm_args.channel = ((machine_hard_pwm_obj_t*)self)->channal;

	ret = device->ops->ioctl(device, USR_PWM_CMD_DISABLE, &pwm_args.channel);
	if (0 != ret)
	{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "channel = %d can not deinit\n", ((machine_hard_pwm_obj_t*)self)->channal));
	}
	
	ret = device->ops->close(device);
	if (0 != ret)
	{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "channel = %d can not close\n", ((machine_hard_pwm_obj_t*)self)->channal));
	}
	pwm_obj->duty = 0;
	pwm_obj->freq = 0;
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(machine_pwm_deinit_obj, machine_pwm_deinit);

STATIC mp_obj_t machine_pwm_freq(size_t n_args, const mp_obj_t *args) {

	int  ret = 0;
	mp_obj_base_t *self = MP_OBJ_TO_PTR(args[0]);
	machine_hard_pwm_obj_t* pwm_obj = (machine_hard_pwm_obj_t*)self;
	
	if(n_args > 2)
	{
		mp_raise_ValueError("args error\n");
	}
	device_info_t *device = ((device_info_t *)(pwm_obj->pwm_device));

	if(n_args == 2)
	{
		
		int freq = mp_obj_get_int(args[1]);
		int period = 1e9 / freq;

		//os_kprintf("freq:%d, period:%d; duty:%d, pluse:%d\n",freq,period,pwm_obj->duty,period * (pwm_obj->duty) / 100);
		ret = ((device_info_t *)(device))->ops->ioctl(device, USR_PWM_CMD_SET_PERIOD, &period);
		if (0 != ret)
		{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pwm can not set freq\n"));
		}
		pwm_obj->freq = freq;

		//int duty = pwm_obj->duty;
		int pulse = period * ((float)(pwm_obj->duty) / 100);

		ret = ((device_info_t *)(device))->ops->write(device, pwm_obj->channal, &pulse, sizeof(pulse));
		if (0 != ret)
		{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pwm can not set duty\n"));
		}
		
	}
	else
	{
		return MP_OBJ_NEW_SMALL_INT(pwm_obj->freq);
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_freq_obj, 1, 2, machine_pwm_freq);


STATIC mp_obj_t machine_pwm_duty(size_t n_args, const mp_obj_t *args) {

	int  ret = 0;
	mp_obj_base_t *self = MP_OBJ_TO_PTR(args[0]);
	machine_hard_pwm_obj_t* pwm_obj = (machine_hard_pwm_obj_t*)self;
	
	if(n_args > 2)
	{
		mp_raise_ValueError("args error\n");
	}
	device_info_t *device = (device_info_t *)(pwm_obj->pwm_device);

	
	int period = 1e9 / pwm_obj->freq;
	int pulse;

	if(n_args == 2)
	{
		int duty = mp_obj_get_int(args[1]);
		pulse = period * ((float)(duty) / 100);

		//os_kprintf("freq:%d, period:%d; duty:%d, pluse:%d\n",pwm_obj->freq,period,duty,pulse);
		ret = ((device_info_t *)(device))->ops->write(device, pwm_obj->channal, &pulse, sizeof(pulse));
		if (0 != ret)
		{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pwm can not set duty\n"));
		}
		pwm_obj->duty = duty;
	}
	else
	{
		return MP_OBJ_NEW_SMALL_INT(pwm_obj->duty);
	}

	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_duty_obj, 1, 2, machine_pwm_duty);

STATIC const mp_rom_map_elem_t mp_module_pwm_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pwm_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_pwm_deinit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_freq), MP_ROM_PTR(&machine_pwm_freq_obj) },
	{ MP_ROM_QSTR(MP_QSTR_duty), MP_ROM_PTR(&machine_pwm_duty_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_pwm_globals, mp_module_pwm_globals_table);


const mp_obj_type_t mp_module_pwm = {
    { &mp_type_type },
		.name = MP_QSTR_PWM,
	  .make_new = machine_hard_pwm_make_new,
    .locals_dict = (mp_obj_dict_t*)&mp_module_pwm_globals,
};

#endif // MICROPY_PY_MACHINE_PWM
