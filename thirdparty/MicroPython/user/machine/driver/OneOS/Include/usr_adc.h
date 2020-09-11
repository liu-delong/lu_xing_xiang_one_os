#ifndef _USR_ADC_H
#define _USR_ADC_H
#include "py/obj.h"
#include "model_device.h"
#include "mpconfigport.h"


#define _adc_open	mpycall_adc_open
#define _adc_close	mpycall_adc_close
#define _adc_read	mpycall_adc_read
#define _adc_ioctl	mpycall_adc_ctrl

enum{
	SET_BASEV = 0,
	SET_RESOLUTION
};

extern uint32_t resolution;
/*
float _adc_read(uint32_t pin);
int _adc_open(uint32_t pin, uint32_t mode);
int _adc_close(uint32_t pin);
int _adc_ioctl(uint32_t pin, uint32_t ctl);
*/

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
