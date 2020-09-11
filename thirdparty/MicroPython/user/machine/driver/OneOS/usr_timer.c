#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/stackctrl.h"
#include "lib/mp-readline/readline.h"
#include "lib/utils/pyexec.h"
#include "py/mphal.h"
#include "os_timer.h"
#include "os_clock.h"
#include "usr_timer.h"

#if 1
mp_uint_t mp_hal_ticks_us(void) {
/* 	const portTickType xDelay = pdMS_TO_TICKS(1);

    return  xTaskGetTickCount() * xDelay * 1000; //1ms 跑多少tick */
	return os_tick_get() * 1000000UL / OS_TICK_PER_SECOND;
}

mp_uint_t mp_hal_ticks_ms(void) {
/* 	const portTickType xDelay = pdMS_TO_TICKS(1);
    return xTaskGetTickCount() * xDelay; //1s */
	return os_tick_get() * 1000 / OS_TICK_PER_SECOND;
}

mp_uint_t mp_hal_ticks_cpu(void) {
    //return xTaskGetTickCount();
	return os_tick_get();
}

void mp_hal_delay_us(mp_uint_t us) {
/* 	const portTickType xDelay = pdMS_TO_TICKS(us)/1000;
	vTaskDelay(xDelay); */
	  os_tick_t t0 = os_tick_get(), t1, dt;
    uint64_t dtick = us * OS_TICK_PER_SECOND / 1000000L;
    while (1) {
        t1 = os_tick_get();
        dt = t1 - t0;
        if (dt >= dtick) {
            break;
        }
        mp_handle_pending();
    }
}


void mp_hal_delay_ms(mp_uint_t ms) {
/* 	const portTickType xDelay = pdMS_TO_TICKS(ms);
	vTaskDelay(xDelay); */
	   os_tick_t t0 = os_tick_get(), t1, dt;
    uint64_t dtick = ms * OS_TICK_PER_SECOND / 1000L;
    while (1) {
        t1 = os_tick_get();
        dt = t1 - t0;
        if (dt >= dtick) {
            break;
        }
        MICROPY_EVENT_POLL_HOOK;
        os_task_sleep(1);
    }
}
#endif
