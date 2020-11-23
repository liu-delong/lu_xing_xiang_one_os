#include "board.h"
#include "define_all.h"
#include "os_idle.h"
#include "wdt.h"

#define WDT_DEVICE_NAME "wdt"
static os_device_t *wdg_dev;

static void idle_hook(void)
{
    os_device_control(wdg_dev, OS_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
}

void init_wdt(void)
{
    os_err_t    ret     = OS_EOK;
    char        device_name[OS_NAME_MAX];
    uint32_t timeout = IWDT_IWDTCFG_IWDTOVP_8s;

    strncpy(device_name, WDT_DEVICE_NAME, OS_NAME_MAX);

    wdg_dev = os_device_find(device_name);
    if (!wdg_dev)
    {
        os_kprintf("find %s failed!\n", device_name);
        return;
    }

    ret = os_device_init(wdg_dev);
    if (ret != OS_EOK)
    {
        os_kprintf("initialize %s failed!\n", device_name);
        return;
    }

    ret = os_device_control(wdg_dev, OS_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    if (ret != OS_EOK)
    {
        os_kprintf("set %s timeout failed!\n", device_name);
        return;
    }

    ret = os_device_control(wdg_dev, OS_DEVICE_CTRL_WDT_START, OS_NULL);
    if (ret != OS_EOK)
    {
        os_kprintf("start %s failed!\n", device_name);
        return;
    }

    os_idle_task_set_hook(idle_hook);

}

