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



#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "py/stream.h"

#if MICROPY_PY_MACHINE_WDT
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "machine_wdt.h"
#include "usr_wdt.h"



typedef struct _machine_wdt_obj_t {
    mp_obj_base_t base;
    mp_wdt_device_handler *wdt_device;
}machine_wdt_obj_t;

STATIC void machine_wdt_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_wdt_device_handler *wdt_device = ((machine_wdt_obj_t*) self_in)->wdt_device;
    
    middle_wdt_print(wdt_device, print);
}

STATIC mp_obj_t machine_wdt_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);

    //char wdt_dev_name[MP_WDT_NAME_MAX];
    //snprintf(wdt_dev_name, sizeof(wdt_dev_name), "wdt%d", mp_obj_get_int(args[0]));

   // mp_wdt_device_handler *wdt_device = middle_wdt_find_device(wdt_dev_name);
    
	
	device_info_t * wdt = mpycall_device_find("wdt");
    if (NULL == wdt){
        mp_raise_ValueError("wdt can not find!\n");
    }
    machine_wdt_obj_t *self = m_new_obj(machine_wdt_obj_t);
    self->base.type = &machine_wdt_type;
    self->wdt_device = wdt;
    
    return (mp_obj_t) self;
}

STATIC mp_obj_t  wdt_init(mp_obj_t self_in)
{
    mp_wdt_device_handler *wdt_device = ((machine_wdt_obj_t*) self_in)->wdt_device;

    wdt_device->ops->open(wdt_device, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(wdt_init_obj, wdt_init);


STATIC mp_obj_t machine_wdt_deinit(mp_obj_t self_in) {

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wdt_deinit_obj, machine_wdt_deinit);

STATIC mp_obj_t wdt_start(mp_obj_t self_in)
{
    mp_wdt_device_handler *wdt_device = ((machine_wdt_obj_t*) self_in)->wdt_device;

    wdt_device->ops->ioctl(wdt_device, IOCTL_WTD_START, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(wdt_start_obj, wdt_start);

STATIC mp_obj_t machine_wdt_feed(mp_obj_t self_in) {
	mp_wdt_device_handler *wdt_device = ((machine_wdt_obj_t*) self_in)->wdt_device;

    wdt_device->ops->ioctl(wdt_device, IOCTL_WTD_KEEPALIVE, 0);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wdt_feed_obj, machine_wdt_feed);

STATIC const mp_rom_map_elem_t machine_wdt_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_init), 	MP_ROM_PTR(&wdt_init_obj) 	},
	{ MP_ROM_QSTR(MP_QSTR_start), 	MP_ROM_PTR(&wdt_start_obj) 	},
    { MP_ROM_QSTR(MP_QSTR_deinit), 	MP_ROM_PTR(&wdt_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_feed), 	MP_ROM_PTR(&wdt_feed_obj) 	},
};
STATIC MP_DEFINE_CONST_DICT(machine_wdt_locals_dict, machine_wdt_locals_dict_table);

const mp_obj_type_t machine_wdt_type = {
    { &mp_type_type },
    .name = MP_QSTR_WDT,
    .print = machine_wdt_print,
    .make_new = machine_wdt_make_new,
    .getiter = mp_identity_getiter,
    .iternext = mp_stream_unbuffered_iter,
    .locals_dict = (mp_obj_dict_t*)&machine_wdt_locals_dict,
};

#endif // MICROPYTHON_USING_MACHINE_WDT
