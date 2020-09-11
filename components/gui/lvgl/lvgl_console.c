#include <board.h>
#include <drv_cfg.h>
#include <lvgl/lvgl.h>

static lv_obj_t *label_console = NULL;
static struct os_serial_device serial_gui;

static os_err_t gui_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    return OS_EOK;
}

static int gui_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;

    for (i = 0; i < size; i++)
    {
        char fb[2] = {buff[i], 0};
        lv_label_ins_text(label_console, LV_LABEL_POS_LAST, fb);
    }
    
    return size;
}

static int gui_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    return -1;
}

static const struct os_uart_ops gui_uart_ops =
{
    .configure  = gui_configure,
    .poll_send  = gui_poll_send,
    .poll_recv  = gui_poll_recv,
};

static int os_gui_console_init(void)
{
    label_console = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_pos(label_console, 0, 60);
    lv_obj_set_size(label_console, 240, 260);
    lv_label_set_text(label_console, "");

    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    serial_gui.ops    = &gui_uart_ops;
    serial_gui.config = config;

    os_hw_serial_register(&serial_gui, "gui_console",
                          OS_DEVICE_FLAG_RDWR, NULL);
    return 0;
}
OS_APP_INIT(os_gui_console_init);
