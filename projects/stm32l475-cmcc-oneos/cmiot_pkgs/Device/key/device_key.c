#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "usr_key.h"

//��ģ����Ҫʵ�ְ����Ļص��������ܡ����ǲ���������,������ȱ�ݣ�����������ʱ���ܲ�������
//�Ƿ�Ҫ��Ӱ������жϴ�����ʽ�ӿڣ���ʱδ��ӣ������أ��½��صȣ�
const mp_obj_type_t mp_key_type ;

mp_obj_t mp_key_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);        
	device_info_t *key_device = mpycall_device_find("KEY");

    if (key_device == NULL)
    {
         nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "key doesn't exist", mp_obj_get_int(args[0])));
    }
	
    // create new uart object
    device_key_obj_t *self = m_new_obj(device_key_obj_t);
    self->base.type = &mp_key_type;
    self->key_device_t = key_device;
		
		//���캯����Ҫ���ú�gpio�Ĺܽţ����Դ˹��캯������������������
    if (n_args != 1)
    {
         //����������ò���ò�����mp_arg_check_num�Ѿ��Բ��������Ͳ����������˼��
         mp_raise_ValueError("args can be void or mode only!\n");
    }
    mp_int_t key_id = mp_obj_get_int(args[0]);
		self->key_device_t->id = key_id;
    self->key_device_t->ops->open(key_device, 0);

    return (mp_obj_t) self;
}

STATIC void device_key_isr_handler(void *arg) {

    device_key_obj_t *self = arg;
    mp_sched_schedule(self->pin_isr_cb, MP_OBJ_FROM_PTR(self));
}

STATIC mp_obj_t key_callback(mp_obj_t self, mp_obj_t func_ptr)
{
	  device_key_func_obj_t key_func;
    key_func.key_self = self;
	  key_func.key_self->pin_isr_cb = func_ptr;
	  key_func.machine_key_isr_handler = device_key_isr_handler;
	  ((device_key_obj_t*)self)->key_device_t->ops->ioctl( ((device_key_obj_t *)self)->key_device_t, IOCTL_KEY_CALLBACK, &key_func);
	
	  return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mpkey_callback, key_callback);

STATIC const mp_rom_map_elem_t mp_module_touchkey_globals_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR_callback), MP_ROM_PTR(&mpkey_callback) },
};

STATIC MP_DEFINE_CONST_DICT(device_key_locals_dict, mp_module_touchkey_globals_table);

const mp_obj_type_t mp_key_type =
{
    { &mp_type_type },
    .name = MP_QSTR_key,
    .make_new = mp_key_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_key_locals_dict,
};


