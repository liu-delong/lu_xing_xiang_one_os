#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>

#include "user_audio.h"

#define CHALLEN			0
#define SAMPLERATE		1
#define VOLUME			3

const mp_obj_type_t mp_audio_type;

typedef struct _device_audio_obj_t {
    mp_obj_base_t base;
	audio_dev_t	  audio;
}device_audio_obj_t;

mp_obj_t mp_audio_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    device_info_t * audio = mpycall_device_find("audio");
    if (NULL == audio){
        mp_raise_ValueError("audio can not find!\n");
    }
    device_audio_obj_t *self = m_new_obj(device_audio_obj_t);
    self->base.type = &mp_audio_type;
    self->audio.device = audio;
    return (mp_obj_t) self;
}


STATIC mp_obj_t audio_init_helper(device_audio_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args, int flag) {
	
	static const mp_arg_t allowed_args[] = {
		{ MP_QSTR_volume, 	  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 35	  } },
        { MP_QSTR_samplerate, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 44100} },
        { MP_QSTR_channel,	  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1	  } },
        { MP_QSTR_bits, 	  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0	  } },
    };
	  	
    // parse args
    struct args_{
        mp_arg_val_t volume, samplerate, channel, bits;
    } args;
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);
	
	int cmd = 0;
	
    // set the audio configuration values
	if (args.volume.u_int != self->audio.volume || flag == AUDIO_INIT_FLAG){
		self->audio.volume = args.volume.u_int;		//args[ARG_volume].u_int;
		cmd = 1;
	} 
	if (args.samplerate.u_int != self->audio.samplerate || flag == AUDIO_INIT_FLAG) {
		self->audio.samplerate = args.samplerate.u_int;	//args[ARG_samplerate].u_int;
	} 
	if (args.channel.u_int != self->audio.channels || flag == AUDIO_INIT_FLAG){
		self->audio.channels = 	args.channel.u_int;		//args[ARG_channel].u_int;
	} 
	if (args.bits.u_int != self->audio.samplebits || flag == AUDIO_INIT_FLAG){
		self->audio.samplebits = args.bits.u_int;		//args[ARG_bits].u_int;
	}
	
	cmd = (cmd != 0)?AUDIO_WRITE_VOLUME_CMD:AUDIO_WRITE_PARAM_CMD;
	
    if (NULL != self->audio.device->ops && NULL != self->audio.device->ops->ioctl){
		
		if (0 != self->audio.device->ops->ioctl(&self->audio, cmd, 0)){
			mp_raise_ValueError("Set parameters failed!\n");
			return mp_const_none;
		}
	}
   return mp_obj_new_int(0);
}


STATIC mp_obj_t mp_audio_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    int ret = 0;
    device_audio_obj_t *self = (device_audio_obj_t *)args[0];
	if (NULL != self->audio.device->ops && NULL != self->audio.device->ops->open){
		ret = self->audio.device->ops->open(&self->audio, 0);
		if (0 != ret){
			mp_raise_ValueError("audio_init failed!\n");
			return mp_const_none;
		}
	}
    
    return audio_init_helper(self, n_args-1, args+1, kw_args, AUDIO_INIT_FLAG);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mp_audio_init_obj, 1, mp_audio_init);

STATIC mp_obj_t mp_audio_deinit (mp_obj_t self)
{
    int ret = 0;
    device_audio_obj_t *audio = (device_audio_obj_t *)self;
    if (NULL != audio->audio.device->ops && NULL != audio->audio.device->ops->ioctl)
    {
        ret = audio->audio.device->ops->close(&audio->audio);
        if (0 != ret){
            mp_raise_ValueError("audio_deinit failed!\n");
			return mp_const_none;
        }
    } else{
        printf("audio deinit function!\n");
    }
    
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mp_audio_deinit_obj, mp_audio_deinit);


STATIC mp_obj_t mp_audio_set_parameter(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
	device_audio_obj_t *self = (device_audio_obj_t *)args[0];
	if (self->audio.device->open_flag == AUDIO_DEINIT_FLAG){
		mp_raise_ValueError("write is poweroff! Please open it!\n");
		return mp_const_none;
	}
	return audio_init_helper(args[0], n_args - 1, args + 1, kw_args, AUDIO_WRITE_FLAG);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mp_audio_set_parameter_obj, 1, mp_audio_set_parameter);



STATIC mp_obj_t mp_audio_player(mp_obj_t self_in, mp_obj_t wr_buf)
{
	device_audio_obj_t *self = (device_audio_obj_t *)MP_OBJ_TO_PTR(self_in);
	
	if (self->audio.device->open_flag == AUDIO_DEINIT_FLAG){
		mp_raise_ValueError("audio is poweroff! Please open it!\n");
		return mp_const_none;
	}
	
	if (NULL != self->audio.device->ops && NULL != self->audio.device->ops->write){
		mp_buffer_info_t bufinfo;
		mp_get_buffer_raise(wr_buf, &bufinfo, MP_BUFFER_READ);

		if (0 != self->audio.device->ops->write(&self->audio, 0, bufinfo.buf, bufinfo.len)) {
            mp_raise_ValueError("mp_audio_set_parameter failed!\n");
			return mp_const_none;
        }
		return mp_obj_new_int(0);
	}else {
		mp_raise_ValueError("Error parameters!\n");
	}
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mp_audio_player_obj, mp_audio_player);


STATIC mp_obj_t player_continue(mp_obj_t self_in)
{
    device_audio_obj_t *self = (device_audio_obj_t *)MP_OBJ_TO_PTR(self_in);

    self->audio.device->ops->ioctl(&self->audio, AUDIO_PLAYER_CONTINUE_CMD, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(player_continue_obj, player_continue);

STATIC mp_obj_t player_stop(mp_obj_t self_in)
{
    device_audio_obj_t *self = (device_audio_obj_t *)MP_OBJ_TO_PTR(self_in);

    self->audio.device->ops->ioctl(&self->audio, AUDIO_PLAYER_STOP_CMD, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(player_stop_obj, player_stop);

STATIC const mp_rom_map_elem_t mp_module_audio_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), 	MP_ROM_PTR(&mp_audio_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), 	MP_ROM_PTR(&mp_audio_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), 	MP_ROM_PTR(&mp_audio_set_parameter_obj) },
	{ MP_ROM_QSTR(MP_QSTR_player), 	MP_ROM_PTR(&mp_audio_player_obj) },
	{ MP_ROM_QSTR(MP_QSTR_stop), 	MP_ROM_PTR(&player_stop_obj) },
	{ MP_ROM_QSTR(MP_QSTR_start), 	MP_ROM_PTR(&player_continue_obj) },

};

STATIC MP_DEFINE_CONST_DICT(device_audio_locals_dict, mp_module_audio_globals_table);

const mp_obj_type_t mp_audio_type =
{
    { &mp_type_type },
    .name = MP_QSTR_AUDIO,
    .make_new = mp_audio_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_audio_locals_dict,
};


