/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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
#ifndef MICROPY_INCLUDED_EXTMOD_MACHINE_CAN_H
#define MICROPY_INCLUDED_EXTMOD_MACHINE_CAN_H

#include "py/obj.h"
#include "py/mphal.h"
#include "bus_can.h"

// SPI protocol
typedef struct _mp_machine_can_p_t {
    void (*init)(mp_obj_base_t *obj, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);
    void (*deinit)(mp_obj_base_t *obj); // can be NULL
    void (*transfer)(mp_obj_base_t *obj, uint32_t offset, const uint8_t *buf, uint32_t bufsize);
} mp_machine_can_p_t;

typedef struct _mp_machine_can_obj_t {
    mp_obj_base_t base;
    mp_can_obj_t cfg;
	device_info_t *can;
} mp_machine_can_obj_t;



extern const mp_obj_type_t mp_machine_can_type;

#endif // MICROPY_INCLUDED_EXTMOD_MACHINE_CAN_H