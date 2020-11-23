
#include <board.h>
#include <os_hw.h>
#include <os_device.h>
#include <drv_common.h>
#include <drv_cfg.h>
#include <stdio.h>
#include <shell.h>
#include <lpmgr/lpmgr.h>
#include "os_stddef.h"
#include <os_event.h>
#include <os_clock.h>
#ifdef OS_USING_LPMGR

/* 
 * example 1:
 * enter sleep mode, Calculate sleep time according to all tasks of the system
 * The user task needs to call os_lpmgr_request(SYS_SLEEP_MODE_IDLE); and os_lpmgr_release(SYS_SLEEP_MODE_IDLE);
 */
void request_sleep(int argc, char *argv[])
{
    if (argc != 2)
    {
        os_kprintf("usage: request_sleep <sleep_mode>\n");
        os_kprintf("example: request_sleep 2\n");
        return;
    }
    
    os_lpmgr_request(atoi(argv[1])); /* SYS_SLEEP_MODE_LIGHT */
}


/* OS_BOARD_INIT(timer_app_init); // Called when the system is initialized, the system calculates the sleep time according to the task   */
SH_CMD_EXPORT(request_sleep, request_sleep, "request_sleep <sleep_mode>");


/* 
 * example 2:
 * Setting the timer time is also the time for regular sleep; 
 * Note: To make the timer time effective, the time of the next priority task calculated 
 * automatically by the system is greater than this timer time
 */ 
static os_timer_t *timer1;

static void requlst_entry(void *parameter)
{
    os_lpmgr_request(SYS_SLEEP_MODE_IDLE);
    os_kprintf("user task, start current tick: %d\n",os_tick_get());
    os_lpmgr_release(SYS_SLEEP_MODE_IDLE);
}

void requst_wakeup(int mode, int timeout)
{
    timer1 = os_timer_create("requst_wake_up",  requlst_entry, OS_NULL,
                             OS_TICK_PER_SECOND * timeout, OS_TIMER_FLAG_PERIODIC);
    if (timer1 == OS_NULL)
    {
        os_kprintf("[%s]-[%d], os_timer_create err!\r\n", __FILE__, __LINE__);
        return;
    }
    
    os_timer_start(timer1);
    os_lpmgr_request(mode);

}

void requst_wake_up(int argc, char *argv[])
{
    os_uint8_t timeout;
    os_uint8_t mode;

    if (argc != 3)
    {
        os_kprintf("usage: requst_wake_up <mode> <timeout_tick>\r\n");
        os_kprintf("example: requst_wake_up 2 5\r\n");
        return;
    }

    mode = atoi(argv[1]);
    timeout = atoi(argv[2]);
    os_kprintf("[%s]-[%d], mode[%d], timeout[%d]\r\n", __FILE__, __LINE__, mode, timeout);
    requst_wakeup(mode, timeout);
}

//OS_BOARD_INIT(requst_wake_up);

SH_CMD_EXPORT(requst_wake_up, requst_wake_up, "requst_wake_up");



/* 
 * example 3:
 * Wake up sleep mode by external interrupt, continue to sleep after processing some tasks
 * 
 */ 
#define WAKEUP_EVENT_BUTTON                 (1 << 0)
#define LED_PIN                                   led_table[0].pin
#define WAKEUP_PIN                               key_table[0].pin
#define WAKEUP_APP_STACK_SIZE        512

typedef struct _tag_sleep_info_s
{
    os_event_t *wakeup_event;
    os_uint8_t mode;
}sleep_info_s;

static sleep_info_s sleep_info;

static void wakeup_callback(void *args)
{
    os_kprintf("wake up[%s]-[%d], pin[%d], tick[%d]\r\n", __FILE__, __LINE__, (os_uint32_t)args, os_tick_get());
    os_event_send(sleep_info.wakeup_event, WAKEUP_EVENT_BUTTON);
    os_pin_irq_enable(WAKEUP_PIN, PIN_IRQ_ENABLE);
}

static void wakeup_iqr_init(void)
{
    os_pin_mode(WAKEUP_PIN, PIN_MODE_INPUT_PULLUP);
    os_pin_attach_irq(WAKEUP_PIN, PIN_IRQ_MODE_FALLING, wakeup_callback, (void *)WAKEUP_PIN);
    os_pin_irq_enable(WAKEUP_PIN, PIN_IRQ_ENABLE);
}

static void led_work(os_uint32_t cnt)
{
    os_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    
    do
    {
        os_pin_write(LED_PIN, PIN_LOW);
        os_task_mdelay(500);
        os_pin_write(LED_PIN, PIN_HIGH);
        os_task_mdelay(500);
    }while(cnt-- > 0);
}

static void wakeup_app_entry(void *parameter)
{
    sleep_info_s *sleep_info;
    OS_ASSERT(parameter != OS_NULL);

    sleep_info = (sleep_info_s *)parameter;
    
    wakeup_iqr_init();
    os_kprintf("request sleep [%s]-[%d], mode[%d]\r\n", __FILE__, __LINE__, sleep_info->mode);
    os_lpmgr_request(sleep_info->mode);


    while (1)
    {
        if (os_event_recv(sleep_info->wakeup_event, WAKEUP_EVENT_BUTTON,
                          OS_EVENT_OPTION_AND | OS_EVENT_OPTION_CLEAR,
                          OS_IPC_WAITING_FOREVER, OS_NULL) == OS_EOK)
        {
            os_lpmgr_request(SYS_SLEEP_MODE_NONE);
            os_kprintf("wake up, enter user code[%s]-[%d], tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());
            
            led_work(6);
            
            os_kprintf("wake up, exit user code[%s]-[%d], tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());
            os_lpmgr_release(SYS_SLEEP_MODE_NONE);
        }
    }
}

static int wakeup_iqr(int argc, char *argv[])
{
    os_task_t *tid;

    if (argc != 2)
    {
        os_kprintf("usage: wakeup_iqr <mode>\r\n");
        os_kprintf("example: wakeup_iqr 2\r\n");
        return -1;
    }

    sleep_info.mode = atoi(argv[1]);

    sleep_info.wakeup_event = os_event_create("wakeup_iqr", OS_IPC_FLAG_FIFO);
    OS_ASSERT(sleep_info.wakeup_event != OS_NULL);

    tid = os_task_create("wakeup_app", wakeup_app_entry, (void *)&sleep_info,
                           WAKEUP_APP_STACK_SIZE, 5, 8);
    OS_ASSERT(tid != OS_NULL);

    os_task_startup(tid);

    return 0;
}

SH_CMD_EXPORT(wakeup_iqr, wakeup_iqr, "wakeup_iqr");



#endif /* OS_USING_LPMGR */


