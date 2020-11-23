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
#include "machine_spi.h"
#include "usr_spi.h"

#if  (MICROPY_PY_MACHINE_SPI && MICROPY_PY_MACHINE_PIN)


STATIC const mp_obj_type_t machine_hard_spi_type;

typedef struct _machine_hard_spi_obj_t {
    mp_obj_base_t base;
    mp_spi_device_handler* spi_device;
} machine_hard_spi_obj_t;

STATIC void machine_hard_spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    //machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t*)self_in;
    //mp_printf(print,"SPI(device port : %s)",self->spi_device->parent.parent.name);
}

mp_obj_t machine_hard_spi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    char spi_dev_name[PY_SPI_NAME_MAX];

    snprintf(spi_dev_name, sizeof(spi_dev_name), "spi%d", mp_obj_get_int(all_args[0]));


    //mp_spi_device_handler* spi_device = middle_spi_open(mp_obj_get_int(all_args[0]));
		mp_spi_device_handler* spi_device = mpycall_device_find(spi_dev_name);
		if (spi_device == NULL)
		{
					nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "SPI(%s) doesn't exist", spi_dev_name));
		}

    // create new hard SPI object
    machine_hard_spi_obj_t *self = m_new_obj(machine_hard_spi_obj_t);
    self->base.type = &machine_hard_spi_type;
    self->spi_device = spi_device;
		
    return (mp_obj_t) self;
}

//SPI.init( baudrate=100000, polarity=0, phase=0, bits=8, firstbit=SPI.MSB/LSB )
#if 0
STATIC void machine_hard_spi_init(mp_obj_base_t *self_in, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t*)self_in;
    int mode = 0;
    int baudrate = mp_obj_get_int(pos_args[0]);
    int polarity = mp_obj_get_int(pos_args[1]);
    int phase = mp_obj_get_int(pos_args[2]);
    int bits = mp_obj_get_int(pos_args[3]);
    int firstbit = mp_obj_get_int(pos_args[4]);
    int ret;
	
	
		enum { ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit};
	  static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 500000} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
		mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

		
	
		int b_buf[2];
		b_buf[0] = baudrate;
		b_buf[1] = bits;
		
    if(!polarity && !phase)
    {
        mode = MP_SPI_MODE_0;
    }

    if(!polarity && phase)
    {
        mode = MP_SPI_MODE_1;
    }

    if(polarity && !phase)
    {
        mode = MP_SPI_MODE_2;
    }

    if(polarity && phase)
    {
        mode = MP_SPI_MODE_3;
    }

    if(firstbit == 0)
    {
        mode |= MP_SPI_MSB;
    } else {
        mode |= MP_SPI_LSB;
    }

    /* config spi */
    {
        //middle_spi_set_config(self->spi_device, bits, mode, baudrate);
			ret = ((mp_spi_device_handler*)(self->spi_device))->ops->ioctl(self->spi_device, mode, b_buf);
    }
		
		if(ret < 0)
		{
			mp_raise_OSError(-1);
		}
}
#endif
STATIC void machine_hard_spi_init(mp_obj_base_t *self_in, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
	

    enum { ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 500000} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // create new object
    //machine_hard_spi_obj_t *self = m_new_obj(machine_hard_spi_obj_t);
    //self->base.type = &machine_hard_spi_type;

		
		
		machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t*)self_in;
		
		int mode = 0;
    int baudrate = args[ARG_baudrate].u_int;;
    int polarity = args[ARG_polarity].u_int;
    int phase = args[ARG_phase].u_int;
    int bits = args[ARG_bits].u_int;
    int firstbit = args[ARG_firstbit].u_int;
    int ret;
	
    // set parameters
//    self->spi.delay_half = baudrate_to_delay_half(args[ARG_baudrate].u_int);
//    self->spi.polarity = args[ARG_polarity].u_int;
//    self->spi.phase = args[ARG_phase].u_int;
//    if (args[ARG_bits].u_int != 8) {
//        mp_raise_ValueError("bits must be 8");
//    }
//    if (args[ARG_firstbit].u_int != MICROPY_PY_MACHINE_SPI_MSB) {
//        mp_raise_ValueError("firstbit must be MSB");
//    }
//    if (args[ARG_sck].u_obj == MP_OBJ_NULL
//        || args[ARG_mosi].u_obj == MP_OBJ_NULL
//        || args[ARG_miso].u_obj == MP_OBJ_NULL) {
//        mp_raise_ValueError("must specify all of sck/mosi/miso");
//    }
//    self->spi.sck = mp_hal_get_pin_obj(args[ARG_sck].u_obj);
//    self->spi.mosi = mp_hal_get_pin_obj(args[ARG_mosi].u_obj);
//    self->spi.miso = mp_hal_get_pin_obj(args[ARG_miso].u_obj);

//    // configure bus
//    mp_soft_spi_ioctl(&self->spi, MP_SPI_IOCTL_INIT);

		int b_buf[2];
		b_buf[0] = baudrate;
		b_buf[1] = bits;
		
    if(!polarity && !phase)
    {
        mode = MP_SPI_MODE_0;
    }

    if(!polarity && phase)
    {
        mode = MP_SPI_MODE_1;
    }

    if(polarity && !phase)
    {
        mode = MP_SPI_MODE_2;
    }

    if(polarity && phase)
    {
        mode = MP_SPI_MODE_3;
    }

    if(firstbit == 0)
    {
        mode |= MP_SPI_MSB;
    } else {
        mode |= MP_SPI_LSB;
    }

    /* config spi */
    {
        //middle_spi_set_config(self->spi_device, bits, mode, baudrate);
			ret = ((mp_spi_device_handler*)(self->spi_device))->ops->ioctl(self->spi_device, mode, b_buf);
    }
		
		if(ret < 0)
		{
			mp_raise_OSError(-1);
		}
}

//STATIC void machine_hard_spi_deinit(mp_obj_base_t *self_in) {
//		machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t*)self_in;
//		((mp_spi_device_handler*)(self->spi_device))->ops->close(self->spi_device);
//    return;
//}

STATIC void machine_hard_spi_transfer(mp_obj_base_t *self_in, size_t len, const uint8_t *src, uint8_t *dest) {
    machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t*)self_in;

		//middle_spi_send_then_recv(self->spi_device, src, dest, len);
		
		((mp_spi_device_handler*)(self->spi_device))->ops->write(self->spi_device, 0, (void*)src, len);
	  ((mp_spi_device_handler*)(self->spi_device))->ops->read(self->spi_device, 0, (void*)dest, len);
}

STATIC const mp_machine_spi_p_t machine_hard_spi_p = {
    .init = machine_hard_spi_init,
    .deinit = NULL,
    .transfer = machine_hard_spi_transfer,
};

STATIC const mp_obj_type_t machine_hard_spi_type = {
    { &mp_type_type },
    .name = MP_QSTR_SPI,
    .print = machine_hard_spi_print,
    .make_new = machine_hard_spi_make_new,
    .protocol = &machine_hard_spi_p,
    .locals_dict = (mp_obj_t)&mp_machine_spi_locals_dict,
};

#endif // MICROPY_PY_MACHINE_SPI

