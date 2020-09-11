/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * modified from Copyright (c) 2018 SummerGift <zhangyuan@rt-thread.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "middle/include/machine_i2c.h"
#include "usr_i2c.h"

#ifdef MICROPY_PY_MACHINE_I2C

STATIC const mp_obj_type_t machine_hard_i2c_type;



typedef struct middle_i2c_config
{
    int baudrate;
		bool dma;
} middle_i2c_config_t;


STATIC void machine_hard_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print,"I2C(%s)\n",
            self->i2c_bus->owner.name);
    return;
}

int machine_hard_i2c_readfrom(mp_obj_base_t *self_in, uint16_t addr, uint8_t *dest, size_t len, bool stop) {
    machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    //return i2c_master_recv(self->i2c_bus, addr, 0, dest, len);
		return ((device_info_t *)(self->i2c_bus))->ops->read(self->i2c_bus, addr, dest, len);
}

int machine_hard_i2c_writeto(mp_obj_base_t *self_in, uint16_t addr, const uint8_t *src, size_t len, bool stop) {
    uint8_t buf[1] = {0};
    machine_hard_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (len == 0){
        len = 1;
        if (src == NULL){
            src = buf;
        }
        //return !i2c_master_send(self->i2c_bus, addr, 0, src, len);
				return ((device_info_t *)(self->i2c_bus))->ops->write(self->i2c_bus, addr, (void*)src, len);
    } else if (src == NULL){
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "buf must not NULL"));
    }
    //return i2c_master_send(self->i2c_bus, addr, 0, src, len);
		return ((device_info_t *)(self->i2c_bus))->ops->write(self->i2c_bus, addr, (void*)src, len);
}

/******************************************************************************/
/* MicroPython bindings for machine API                                       */

STATIC mp_obj_t machine_uart_init_helper(machine_hard_i2c_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 9600} },
        { MP_QSTR_dma, MP_ARG_BOOL, {.u_bool = false} },
    };
		
		struct {
        mp_arg_val_t baudrate, dma;
    } args;
		
		mp_arg_parse_all(n_args, pos_args, kw_args,
			MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);
		
		middle_i2c_config_t config;
		config.baudrate = args.baudrate.u_int;
		config.dma = args.dma.u_bool;
		device_info_t *i2c = self->i2c_bus;
		
		i2c->ops->ioctl(i2c,0,&config);
	return (mp_obj_t) self;
}

mp_obj_t machine_hard_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    char iic_device[IIC_NAME_MAX];

    snprintf(iic_device, sizeof(iic_device), "i2c%d", mp_obj_get_int(all_args[0]));
    //struct i2c_bus_device *i2c_bus = _i2c_open(mp_obj_get_int(all_args[0]));
		void * i2c_bus = mpycall_device_find(iic_device);
    if (i2c_bus == NULL) {
        //printf("can't find %s device\r\n", iic_device);
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "I2C(%s) doesn't exist", iic_device));
    }
		((device_info_t *)(i2c_bus))->ops->open(i2c_bus, 0);

		//mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

		
    // create new hard I2C object
    machine_hard_i2c_obj_t *self = m_new_obj(machine_hard_i2c_obj_t);
    self->base.type = &machine_hard_i2c_type;
    self->i2c_bus = i2c_bus;
		
		if(n_args > 1 || n_kw > 0){
			mp_map_t kw_args;
			mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
			machine_uart_init_helper(self, n_args - 1, all_args + 1, &kw_args);
		}
		
    return (mp_obj_t) self;
}

STATIC const mp_machine_i2c_p_t machine_hard_i2c_p = {
    .start = NULL,
    .stop = NULL,
    .read = NULL,
    .write = NULL,
    .readfrom = machine_hard_i2c_readfrom,
    .writeto = machine_hard_i2c_writeto,
};

STATIC const mp_obj_type_t machine_hard_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_I2C,
    .print = machine_hard_i2c_print,
    .make_new = machine_hard_i2c_make_new,
    .protocol = &machine_hard_i2c_p,
    .locals_dict = (mp_obj_dict_t*)&mp_machine_soft_i2c_locals_dict,
};

#endif // MICROPY_PY_MACHINE_I2C
