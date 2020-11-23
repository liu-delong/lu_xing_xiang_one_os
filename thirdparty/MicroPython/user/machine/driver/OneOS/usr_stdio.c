#include "stdio.h"
#include "py/runtime.h"
#include <os_device.h>
#include "os_hw.h"
#include "lib/utils/interrupt_char.h"

#include "ring_buff.h"
#include "os_util.h"
#include "string.h"



#define UART_FIFO_SIZE 256

static  rb_ring_buff_t *rx_fifo = NULL;

os_err_t (*odev_rx_ind)(os_device_t *dev, struct os_device_cb_info *info);

static os_err_t getchar_rx_ind(os_device_t *dev, struct os_device_cb_info *info) {
    unsigned char ch;
    os_size_t i;
    os_base_t int_lvl;

    for (i = 0; i < info->size; i++) {
        /* read a char */
        if (os_device_read(dev, 0, &ch, 1)) {
//            if (ch == mp_interrupt_char) {
//                mp_keyboard_interrupt();
//            } else {
                int_lvl = os_hw_interrupt_disable();
                rb_ring_buff_put_force(rx_fifo, &ch, 1);
                os_hw_interrupt_enable(int_lvl);
            //}
        }
    }
    return OS_EOK;
}

/**
 *********************************************************************************************************
 *                                      initialize shell channel
 *
 * @description: This function initialize shell channel for micropython.
 *
 * @param 	   : void
 *
 * @return     : void
 *
 * @note       : The function use micropython shell channel to receive data from commond line replace OneOS
 *				 system shell channel.
 *********************************************************************************************************
*/
void usr_getchar_init(void) {
    os_base_t int_lvl;
    os_device_t *console;

    /* create RX FIFO */
    rx_fifo = rb_ring_buff_create(UART_FIFO_SIZE);
    /* created must success */
    assert(rx_fifo);

    int_lvl = os_hw_interrupt_disable();
    console = os_console_get_device();
    if (console) {
        /* backup RX indicate */
		struct os_device_cb_info cb_info = 
        {
            .type = OS_DEVICE_CB_TYPE_RX,
            .cb   = getchar_rx_ind,
        };
		odev_rx_ind = console->cb_table[OS_DEVICE_CB_TYPE_RX].cb;
        os_device_control(console, IOC_SET_CB, &cb_info);
    }
    os_hw_interrupt_enable(int_lvl);

}
/**
 *********************************************************************************************************
 *                                      close shell channel
 *
 * @description: This function close micropython shell channel, recover the shell channel of OneOS system.
 *
 * @param 	   : void
 *
 * @return     : void
 *********************************************************************************************************
*/
void usr_getchar_deinit(void) {
    os_base_t int_lvl;
    os_device_t *console;

    rb_ring_buff_destroy(rx_fifo);

    int_lvl = os_hw_interrupt_disable();
    console = os_console_get_device();
    if (console && odev_rx_ind) {
        /* restore RX indicate */
		struct os_device_cb_info cb_info = 
        {
            .type = OS_DEVICE_CB_TYPE_RX,
            .cb   = odev_rx_ind,
        };
		os_device_control(console, IOC_SET_CB, &cb_info);
    }
    os_hw_interrupt_enable(int_lvl);
}
/**
 *********************************************************************************************************
 *                                      oneos终端串口获取一个字�?
 *
 * @description: 串口获取一个字符，与操作系统强相关
 *
 * @param 	   : void
 *
 * @return     : void
 *
 * @note       : �?
 *
 * @example    : �?
 *********************************************************************************************************
*/
int oneos_getchar(void) {
    unsigned char ch;
    os_base_t int_lvl;

    int_lvl = os_hw_interrupt_disable();
    if (!rb_ring_buff_get_char(rx_fifo, &ch)) {
        ch = 0xFF;
    }
    os_hw_interrupt_enable(int_lvl);

    return ch;

}

int mp_hal_stdin_rx_chr(void) {
	
	char ch;
    while (1) {
        ch = oneos_getchar();
        if (ch != (char)0xFF) {
            break;
        }
        MICROPY_EVENT_POLL_HOOK;
        os_task_mdelay(1);
    }
    return ch;
}

void mp_hal_stdout_tx_strn(const char *str, size_t len)
{
	   os_device_t *console;

    console = os_console_get_device();
    if (console) {
        os_device_write(console, 0, str, len);
    }
}

void mp_hal_stdout_tx_strn_stream(const char *str, size_t len) {
	os_kprintf("%.*s", len, str);
}



