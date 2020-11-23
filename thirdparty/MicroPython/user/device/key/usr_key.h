#include "py/obj.h"
#include "model_device.h"
#include "os_device.h"

#define IOCTL_KEY_INIT         (0)
#define IOCTL_KEY_CALLBACK     (1)

typedef struct _device_key_obj_t {
    mp_obj_base_t base;
    device_info_t *key_device_t;
	  mp_obj_t pin_isr_cb;
}device_key_obj_t;


typedef struct _device_key_func_obj_t {
    device_key_obj_t *key_self;
	  mp_obj_t machine_key_isr_handler;
}device_key_func_obj_t;


