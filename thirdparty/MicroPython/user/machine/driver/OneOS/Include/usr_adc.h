#ifndef _USR_ADC_H
#define _USR_ADC_H
#include "py/obj.h"
#include "model_device.h"
#include "mpconfigport.h"

#define MP_ADC_DEINIT_FLAG		0
#define MP_ADC_INIT_FLAG		1

#define MP_ADC_OP_ERROR 		-1


enum{
	SET_BASEV = 0,
	SET_RESOLUTION
};

extern uint32_t resolution;


//int _adc_init(uint32_t pin, uint32_t mode);

typedef struct _mp_machine_adc_p_t {
	float (*read)(uint32_t pin);
	int (*open)(uint32_t pin, uint32_t mode);
	int (*close)(uint32_t pin);
	int (*ioctl)(uint32_t pin, uint32_t ctl);
	int (*init)(uint32_t pin, uint32_t mode);
} mp_machine_adc_p_t;

extern const mp_obj_type_t mp_module_adc;
#endif
