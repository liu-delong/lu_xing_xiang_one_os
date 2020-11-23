
#include <drv_cfg.h>
#include <os_clock.h>
#include <stdio.h>
#include <shell.h>
#include <lpmgr/lpmgr.h>

#include "lpmgr_lower.h"
#include "os_stddef.h"
#include "th_i2c.h"

#ifdef OS_USING_LPMGR

void lpmgr_dev_set(os_uint8_t event, os_uint8_t mode, void * data)
{
    
    if (SYS_ENTER_SLEEP == event)
    {
        //os_kprintf("[%s]-[%d], notify call[SYS_ENTER_SLEEP], tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());
        lpmgr_low_set();
    }
    else if (SYS_EXIT_SLEEP == event)
    {
        lpmgr_low_resume();
        os_kprintf("[%s]-[%d], notify call[SYS_EXIT_SLEEP], tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());
    }
}


void lpmgr_dev_init(void)
{
    os_lpmgr_notify_set(lpmgr_dev_set, NULL);
    os_kprintf("[%s]-[%d], notify set, tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());
}


//////////////////////////////////
static int adc_suspend(struct os_device *device, os_uint8_t mode)
{
    os_kprintf("[%s]-[%d], adc_suspend, tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());

    if (mode >= SYS_SLEEP_MODE_MAX)
    {
        os_kprintf("demo_suspend invalide mode %d\r\n", mode);
        return -1;
    }

//    ADCTurnOFF();

    return 0;
}

static void adc_resume(struct os_device *device, os_uint8_t mode)
{
    os_kprintf("[%s]-[%d], adc_resume, tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());
    if (mode >= SYS_SLEEP_MODE_MAX)
    {
        os_kprintf("demo_resume invalide mode %d\r\n", mode);
        return;
    }
    
//    ADCTurnOn();
}

static int adc_frequency_change(const struct os_device *device, os_uint8_t mode)
{
    os_kprintf("demo_frequency_change: %d MHz \r\n", mode + 1);

    return 0;
}

static struct os_lpmgr_device_ops adc_lpmgr_ops =
{
    adc_suspend,
    adc_resume,
    adc_frequency_change,
};

#define ADC_DEV_NAME "adc"
int lpmgr_adc(void)
{
    struct os_device *device = os_device_find("soft_i2c1");

    OS_ASSERT(device);

    os_lpmgr_device_register(device, &adc_lpmgr_ops);
    os_kprintf("[%s]-[%d], lpmgr_device_register, tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());
    
    return 0;
}

OS_DEVICE_INIT(lpmgr_adc);

#endif /* OS_USING_LPMGR */



