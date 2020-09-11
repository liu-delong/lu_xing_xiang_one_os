#ifndef _USR_PWM_H_
#define _USR_PWM_H_
#include "py/obj.h"
#include "model_device.h"
#include "mpconfigport.h"
#include "model_bus.h"
#include "os_device.h"

#include "pwm.h"

extern const mp_obj_type_t mp_module_pwm;

#define USR_PWM_CMD_ENABLE   		OS_PWM_CMD_ENABLE 
#define USR_PWM_CMD_DISABLE  		OS_PWM_CMD_DISABLE
#define USR_PWM_CMD_SET_PERIOD      OS_PWM_CMD_SET_PERIOD



#endif


