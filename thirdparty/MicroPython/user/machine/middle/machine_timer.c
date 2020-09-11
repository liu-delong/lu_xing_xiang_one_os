/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 ChenYong (chenyong@rt-thread.com)
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "modmachine.h"
#include "mphalport.h"
#include <os_util.h>

#define MICROPYTHON_USING_MACHINE_TIMER

#ifdef MICROPYTHON_USING_MACHINE_TIMER

#include <os_task.h>
#include "machine_timer.h"

#define  MAX_TIMER  17

typedef struct _machine_timer_obj_t {
    mp_obj_base_t base;
    os_timer_t *timer_handler;
    char dev_name[OS_NAME_MAX];
    mp_obj_t timeout_cb;
    int8_t timerid;
    uint32_t timeout;
    os_bool_t is_repeat;
    os_bool_t is_init;
} machine_timer_obj_t;

const mp_obj_type_t machine_timer_type;

STATIC void error_check(bool status, const char *msg) {
    if (!status) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, msg));
    }
}

STATIC void machine_timer_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_timer_obj_t *self = self_in;

    mp_printf(print, "Timer(%p; ", self);

    if (self->timerid >= 0) {
        mp_printf(print, "timer_id=%d, ", self->timerid);
    } else {
        mp_printf(print, "timer_name=%s, ", self->dev_name);
    }
    mp_printf(print, "period=%d, ", self->timeout);
    mp_printf(print, "auto_reload=%d)", self->is_repeat);
}

STATIC mp_obj_t machine_timer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    machine_timer_obj_t *self = m_new_obj(machine_timer_obj_t);

    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, true);

    // check input timer device name or ID
    if (mp_obj_is_qstr(args[0])) {
        //static int device_id = 0;
        self->timerid = -1;
        strncpy(self->dev_name, mp_obj_str_get_str(args[0]), OS_NAME_MAX);
    } else {
        error_check(0, "Input ADC device name or ID error.");
    }

    // initialize timer device
    self->base.type = &machine_timer_type;
    self->timeout = 0;
    self->timeout_cb = OS_NULL;
    self->is_repeat = OS_TRUE;
    self->is_init = OS_FALSE;
    self->timer_handler = OS_NULL;

    // return constant object
    return MP_OBJ_FROM_PTR(self);
}

//static machine_timer_obj_t *timer_self[MAX_TIMER] = {OS_NULL};

STATIC mp_obj_t machine_timer_deinit(mp_obj_t self_in) {
    machine_timer_obj_t *self = self_in;
    //os_err_t result = OS_EOK;

    if (self->is_init == OS_TRUE) {
        os_timer_destroy(self->timer_handler);
        self->is_init = OS_FALSE;
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_timer_deinit_obj, machine_timer_deinit);

void timer_callback_handler(void* parameter) {
    machine_timer_obj_t *self = (machine_timer_obj_t *)parameter;

    mp_sched_schedule(self->timeout_cb, MP_OBJ_FROM_PTR(self));
    return;
}

STATIC mp_obj_t machine_timer_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    machine_timer_obj_t *self = (machine_timer_obj_t *)args[0];
    //os_bool_t result = OS_EOK;
    int mode = 0;

    enum {
        ARG_mode,
        ARG_period,
        ARG_callback,
    };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode,         MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_period,       MP_ARG_INT, {.u_int = 0xffffffff} },
        { MP_QSTR_callback,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

    mp_arg_val_t dargs[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, dargs);

    if (4 == n_args) {
        self->is_repeat = dargs[ARG_mode].u_int;
        self->timeout = dargs[ARG_period].u_int;
        self->timeout_cb = dargs[ARG_callback].u_obj;
    } else {
        mp_raise_ValueError("invalid format");
    }

    error_check(self->timeout > 0, "Set timeout value error");

    if (self->is_init != OS_FALSE)
    {
		mp_raise_ValueError("timer is already init");
    }

    if (self->timeout_cb == OS_NULL) {
		mp_raise_ValueError("invalid callback");
    }

    // set timer mode
	mode = (self->is_repeat == OS_TRUE)? OS_TIMER_FLAG_PERIODIC:OS_TIMER_FLAG_ONE_SHOT;
	self->timer_handler = os_timer_create(self->dev_name,timer_callback_handler,self,self->timeout,OS_TIMER_FLAG_SOFT_TIMER | mode);
	os_timer_start(self->timer_handler);
    self->is_init = OS_TRUE;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_timer_init_obj, 1, machine_timer_init);

STATIC const mp_rom_map_elem_t machine_timer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_timer_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_timer_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_ONE_SHOT), MP_ROM_INT(OS_FALSE) },
    { MP_ROM_QSTR(MP_QSTR_PERIODIC), MP_ROM_INT(OS_TRUE) },
};
STATIC MP_DEFINE_CONST_DICT(machine_timer_locals_dict, machine_timer_locals_dict_table);

const mp_obj_type_t machine_timer_type = {
    { &mp_type_type },
    .name = MP_QSTR_Timer,
    .print = machine_timer_print,
    .make_new = machine_timer_make_new,
    .locals_dict = (mp_obj_t) &machine_timer_locals_dict,
};

#endif // MICROPYTHON_USING_MACHINE_TIMER

