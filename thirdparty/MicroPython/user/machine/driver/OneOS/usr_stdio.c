#include "stdio.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/stackctrl.h"
#include "lib/mp-readline/readline.h"
#include "lib/utils/pyexec.h"
#include "py/mphal.h"

#include <os_device.h>
#include "os_hw.h"
#include "lib/utils/interrupt_char.h"
#include "os_errno.h"
#include "ring_buff.h"
#include "os_util.h"
#include "usr_misc.h"
#include "shell.h"

#define UART_FIFO_SIZE 256

static  rb_ring_buff_t *rx_fifo = NULL;
static os_err_t (*odev_rx_ind)(os_device_t *dev, os_size_t size) = NULL;

static os_err_t getchar_rx_ind(os_device_t *dev, os_size_t size) {
    unsigned char ch;
    os_size_t i;
    os_base_t int_lvl;

    for (i = 0; i < size; i++) {
        /* read a char */
        if (os_device_read(dev, 0, &ch, 1)) {
            if (ch == mp_interrupt_char) {
                mp_keyboard_interrupt();
            } else {
                int_lvl = os_hw_interrupt_disable();
                rb_ring_buff_put_force(rx_fifo, &ch, 1);
                os_hw_interrupt_enable(int_lvl);
            }
        }
    }
    return OS_EOK;
}

/**
 *********************************************************************************************************
 *                                      oneos终端串口初始化
 *
 * @description: 设置oneos终端串口的初始化，与操作系统强相关
 *
 * @param 	   : void
 *
 * @return     : void
 *
 * @note       : 没有对应的头文件函数声明，用时需先extern
 *
 * @example    : extern void oneos_getchar_init(void);
 *               oneos_getchar_init();
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
        odev_rx_ind = console->rx_indicate;
        os_device_set_rx_indicate(console, getchar_rx_ind);
    }
    os_hw_interrupt_enable(int_lvl);

}
/**
 *********************************************************************************************************
 *                                      oneos终端串口去初始化
 *
 * @description: 设置oneos终端串口的去初始化，与操作系统强相关
 *
 * @param 	   : void
 *
 * @return     : void
 *
 * @note       : 没有对应的头文件函数声明，用时需先extern
 *
 * @example    : extern void oneos_getchar_deinit(void);
 *               oneos_getchar_deinit();
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
        os_device_set_rx_indicate(console, odev_rx_ind);
    }
    os_hw_interrupt_enable(int_lvl);
}
/**
 *********************************************************************************************************
 *                                      oneos终端串口获取一个字符
 *
 * @description: 串口获取一个字符，与操作系统强相关
 *
 * @param 	   : void
 *
 * @return     : void
 *
 * @note       : 无
 *
 * @example    : 无
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
   /*  char ch = 0xFF;
    int recvlen = 0;
    static int i = 0;
    //const portTickType xDelay = pdMS_TO_TICKS(1);

    while (1) {
		  recvlen = huart2.RxXferSize - huart2.RxXferCount;
		  //__disable_irq();
		  if(recvlen != i)
		  {
			  //HAL_UART_Transmit(&huart2, &recvfifo[i++], 1, 10);
			  ch = recvfifo[i++];
			  if(i >= huart2.RxXferSize)
			  {
				  i = 0;
			  }
		  }

		  if(huart2.RxXferCount == 0)
		  {
			  //vPortEnterCritical();
			  //extern void MX_USART2_UART_Init(void);
			  //MX_USART2_UART_Init();
			  //HAL_UART_Receive_IT(&huart2,recvfifo,sizeof(recvfifo));
			  //vPortExitCritical();
		  }

        if (ch != (char)0xFF) {
            break;
        }
        MICROPY_EVENT_POLL_HOOK;
        //vTaskDelay(xDelay);
    } */
    //return ch;
}

void mp_hal_stdout_tx_strn(const char *str, size_t len)
{
	   os_device_t *console;

    console = os_console_get_device();
    if (console) {
        os_device_write(console, 0, str, len);
    }
	//HAL_UART_Transmit(&huart2, str, len, 100);
}

void mp_hal_stdout_tx_strn_stream(const char *str, size_t len) {
	//HAL_UART_Transmit(&huart2, str, len, 100);
	os_kprintf("%.*s", len, str);
}

OS_BOARD_INIT(Init_listhead);


int run_mpy(int argc, char **argv)
{
	char *file;
	int length = 0;
    if (argc != 2)
    {
		os_kprintf("parameter is wrong\n");
        return 0;
    }
	file = argv[1];
	length = strlen(file);
	if (strncmp(file+length-3,".py", 3) !=0){
		os_kprintf("file is wrong\n");
		return -1;
	}
	Mpy_Task(file);
	return 0;
}

SH_CMD_EXPORT(mpy, run_mpy, "run py file");

SH_CMD_EXPORT(Mpy_Task, Mpy_Task, "Run MicroPython");
