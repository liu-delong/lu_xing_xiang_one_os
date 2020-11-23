
#include <board.h>
#include "am_mcu_apollo.h"
#include <string.h>
#include <stdlib.h>
#include "th_i2c.h"
#include "cmcc_lowpowerconf.h"
#include "am_bsp_gpio.h"
#include "am_hal_adc.h"
#include "lpmgr_lower.h"

#define	NB_ON_OFF_PIN_NUM	33
#define	NB_RESET_PIN_NUM	2
#define	NB_NETLIGHT_PIN_NUM	46
#define	NB_OPT_ON			1
#define	NB_OPT_OFF			0

void lpmgr_low_set(void);


/****************************************************************************
* Function: nbModulePowerOnOFFInit
* Description:≥ı ºªØƒ£◊Èø™πÿpin
* Param: none
* retval:none
*****************************************************************************/
void nbModulePowerOnOFFInit(void)
{
	// Configure the pin as a push-pull GPIO output.
	am_hal_gpio_pin_config(NB_ON_OFF_PIN_NUM, AM_HAL_PIN_OUTPUT | AM_HAL_GPIO_PULLUP);
	am_hal_gpio_out_enable_bit_set(NB_ON_OFF_PIN_NUM);
	am_hal_gpio_out_bit_clear(NB_ON_OFF_PIN_NUM);
}

/****************************************************************************
* Function: nbModulePowerOnOff
* Description:ø™πÿƒ£◊ÈµÁ‘¥
* Param: opt:0,πÿNB; >0∆‰À¸:ø™NB
* retval:none
*****************************************************************************/
void nbModulePowerOnOff(os_uint16_t opt)
{
	if(opt == NB_OPT_OFF)
	{
		am_hal_gpio_out_bit_clear(NB_ON_OFF_PIN_NUM);
	}
	else
	{
		am_hal_gpio_out_bit_set(NB_ON_OFF_PIN_NUM);
	}
	//os_task_msleep(5);
}


/****************************************************************************
* Function: netLightMonitorEnable
* Description:
* Param: none
* retval:none
*****************************************************************************/
void netLightMonitorEnable(os_uint8_t opt)
{
	if(opt)
	{
		//nbModuleNetLightPinInit();
		am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(NB_NETLIGHT_PIN_NUM));
		am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(NB_NETLIGHT_PIN_NUM));
	}
	else
	{
		am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(NB_NETLIGHT_PIN_NUM));
		am_hal_gpio_pin_config(NB_NETLIGHT_PIN_NUM, AM_HAL_PIN_DISABLE);
	}
}


void am_bsp_low_power_init(void)
{
    //
    // Enable internal buck converters.
    //
    am_hal_mcuctrl_bucks_enable();

    //
    // Turn off the voltage comparator as this is enabled on reset.
    //
    am_hal_vcomp_disable();

    //
    // Run the RTC off the LFRC.
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC);

    //
    // Stop the XTAL.
    //
    am_hal_clkgen_osc_stop(AM_HAL_CLKGEN_OSC_XT);

    //
    // Disable the RTC.
    //
    am_hal_rtc_osc_disable();

    //
    // Disable the bandgap.
    //
    am_hal_mcuctrl_bandgap_disable();
}

void UnusedPinDis(void)
{
#if defined (PIN_CONFIG_DISABLE)

	am_bsp_pin_disable(PIN00);
	am_bsp_pin_disable(PIN01);
//	am_bsp_pin_disable(PIN02);
//	am_bsp_pin_disable(PIN03);
	am_bsp_pin_disable(PIN04);
//	am_bsp_pin_disable(PIN05);
//	am_bsp_pin_disable(PIN06);
	am_bsp_pin_disable(PIN07);
//	am_bsp_pin_disable(PIN08);
//	am_bsp_pin_disable(PIN09);
	am_bsp_pin_disable(PIN10);
	am_bsp_pin_disable(PIN11);
//	am_bsp_pin_disable(PIN12);
	am_bsp_pin_disable(PIN13);
	am_bsp_pin_disable(PIN14);
	am_bsp_pin_disable(PIN15);
	am_bsp_pin_disable(PIN16);
	am_bsp_pin_disable(PIN17);
	am_bsp_pin_disable(PIN18);
	am_bsp_pin_disable(PIN19);
//	am_bsp_pin_disable(PIN20);
//	am_bsp_pin_disable(PIN21);
//	am_bsp_pin_disable(PIN22);
//	am_bsp_pin_disable(PIN23);
	am_bsp_pin_disable(PIN24);
	am_bsp_pin_disable(PIN25);
	am_bsp_pin_disable(PIN26);
	am_bsp_pin_disable(PIN27);
	am_bsp_pin_disable(PIN28);
	am_bsp_pin_disable(PIN29);
	am_bsp_pin_disable(PIN30);
	am_bsp_pin_disable(PIN31);
	am_bsp_pin_disable(PIN32);
	am_bsp_pin_disable(PIN33);
	am_bsp_pin_disable(PIN34);
//	am_bsp_pin_disable(PIN35);
//	am_bsp_pin_disable(PIN36);
	am_bsp_pin_disable(PIN37);
	am_bsp_pin_disable(PIN38);
	am_bsp_pin_disable(PIN39);
	am_bsp_pin_disable(PIN40);
//	am_bsp_pin_disable(PIN41);
	am_bsp_pin_disable(PIN42);
	am_bsp_pin_disable(PIN43);
	am_bsp_pin_disable(PIN44);
	am_bsp_pin_disable(PIN45);
	am_bsp_pin_disable(PIN46);
	am_bsp_pin_disable(PIN47);
	am_bsp_pin_disable(PIN48);
	am_bsp_pin_disable(PIN49);
#else
	am_bsp_pin_config(PIN00);
	am_bsp_pin_config(PIN01);
//	am_bsp_pin_config(PIN02);
//	am_bsp_pin_config(PIN03);
	am_bsp_pin_config(PIN04);
//	am_bsp_pin_config(PIN05);
//	am_bsp_pin_config(PIN06);
	am_bsp_pin_config(PIN07);
//	am_bsp_pin_config(PIN08);
//	am_bsp_pin_config(PIN09);
	am_bsp_pin_config(PIN10);
	am_bsp_pin_config(PIN11);
//	am_bsp_pin_config(PIN12);
	am_bsp_pin_config(PIN13);
	am_bsp_pin_config(PIN14);
	am_bsp_pin_config(PIN15);
	am_bsp_pin_config(PIN16);
	am_bsp_pin_config(PIN17);
	am_bsp_pin_config(PIN18);
	am_bsp_pin_config(PIN19);
//	am_bsp_pin_config(PIN20);
//	am_bsp_pin_config(PIN21);
	am_bsp_pin_config(PIN22);
	am_bsp_pin_config(PIN23);
	am_bsp_pin_config(PIN24);
	am_bsp_pin_config(PIN25);
	am_bsp_pin_config(PIN26);
	am_bsp_pin_config(PIN27);
	am_bsp_pin_config(PIN28);
	am_bsp_pin_config(PIN29);
	am_bsp_pin_config(PIN30);
	am_bsp_pin_config(PIN31);
	am_bsp_pin_config(PIN32);
	am_bsp_pin_config(PIN33);
	am_bsp_pin_config(PIN34);
//	am_bsp_pin_config(PIN35);
//	am_bsp_pin_config(PIN36);
	am_bsp_pin_config(PIN37);
	am_bsp_pin_config(PIN38);
	am_bsp_pin_config(PIN39);
	am_bsp_pin_config(PIN40);
//	am_bsp_pin_config(PIN41);
	am_bsp_pin_config(PIN42);
	am_bsp_pin_config(PIN43);
	am_bsp_pin_config(PIN44);
	am_bsp_pin_config(PIN45);
	am_bsp_pin_config(PIN46);
	am_bsp_pin_config(PIN47);
	am_bsp_pin_config(PIN48);
	am_bsp_pin_config(PIN49);
#endif
}




#define AM_BSP_UART_PRINT_INST          0


/****************************************************************************
* Function: uartTurnOff
* Description: ÊâìÂºÄ‰∏≤Âè£
* Param: pvParametersÔºövoid
* retval: void
*****************************************************************************/
void uartTurnOff(void)
{
    os_int32_t i32Module = AM_BSP_UART_PRINT_INST;

	am_hal_uart_int_disable(0, AM_HAL_UART_INT_TX|AM_HAL_UART_INT_RX|AM_HAL_UART_INT_RX_TMOUT);
	am_hal_interrupt_disable(AM_HAL_INTERRUPT_UART);

    // Power on the selected UART
    am_hal_uart_pwrctrl_disable(i32Module);

    // Start the UART interface, apply the desired configuration settings, and enable the FIFOs.
    am_hal_uart_clock_disable(i32Module);

    // Disable the UART before configuring it.
    am_hal_uart_disable(i32Module);

	// Make sure the UART RX and TX pins are enabled.
    am_bsp_pin_disable(COM_UART_TX);
    am_bsp_pin_disable(COM_UART_RX);
}


int nbModuleHardwareReset(int opt)
{
	uint8_t counter = 2;
	nbModulePowerOnOff(NB_OPT_ON);

	do
	{	/**reset opt*/
		am_hal_gpio_out_bit_clear(NB_RESET_PIN_NUM);
		//nbDelayms(10);
		os_task_mdelay(100);
		am_hal_gpio_out_bit_set(NB_RESET_PIN_NUM);
		//nbDelayms(200);
		os_task_mdelay(500);
		am_hal_gpio_out_bit_clear(NB_RESET_PIN_NUM);
		//nbDelayms(200);

		if(opt == 0)
		{
			os_task_mdelay(5000);
			return 0;   //Â∑•ÂéÇÊ®°ÂºèÁõ¥Êé•ËøîÂõûÔºå‰ª•‰æøËÄ¶ÂêàÊµãËØïÊó∂ËÉΩÁõ¥Êé•ËøõÂÖ•Â∑•ÂéÇÊ®°Âºè„ÄÇ
		}
		else
		{
			os_task_mdelay(5000);
		}
		/**reset opt*/
		
	}while(counter--);
	return 0;
}

void lpmgr_low_set(void)
{
    am_bsp_low_power_init();
    UnusedPinDis();
    netLightMonitorEnable(0);
    nbModulePowerOnOFFInit();
    nbModulePowerOnOff(NB_OPT_OFF);
    
    uartTurnOff();
}


void lpmgr_low_resume(void)
{
    int ret;

    netLightMonitorEnable(1);
    nbModulePowerOnOFFInit();
    nbModulePowerOnOff(NB_OPT_ON);
    ret = SHT3X_I2C_init();
    if (ret != 0)
    {
        os_kprintf("[%s]-[%d], SHT3X_I2C_init failed, ret[%d]\r\n", __FILE__, __LINE__, ret);
        return;
    }
}


void lpmgr_hard_init(void)
{
    int ret;

    ret = SHT3X_I2C_init();
    if (ret != 0)
    {
        os_kprintf("[%s]-[%d], SHT3X_I2C_init failed, ret[%d]\r\n", __FILE__, __LINE__, ret);
        return;
    }

    nbModulePowerOnOFFInit();
    nbModulePowerOnOff(NB_OPT_ON);
    
}

#include <shell.h>

LPMGR_TIME_S lpmgr_time = {10, 10};

LPMGR_TIME_S *lpmgr_time_get(void)
{
    return &lpmgr_time;
}

void lpmgr_time_set(int run_time, int sleep_time)
{
    LPMGR_TIME_S *time;

    time = lpmgr_time_get();

    time->run_time = run_time;
    time->sleep_time = sleep_time;

    os_kprintf("[%s]-[%d], lpmgr set time, run_time[%d], sleep_time[%d]\r\n", 
        __FILE__, __LINE__, time->run_time, time->sleep_time);
}


void lower_time_set(int argc, char *argv[])
{
    if (3 != argc)
    {
        os_kprintf("lower_time_set run_time(s) sleep_time(s)\r\n");
        os_kprintf("example: lower_time_set 10 10\r\n");
        return;
    }

    lpmgr_time_set(atoi(argv[1]), atoi(argv[2]));
}

SH_CMD_EXPORT(lower_time_set, lower_time_set, "lower_time_set");


