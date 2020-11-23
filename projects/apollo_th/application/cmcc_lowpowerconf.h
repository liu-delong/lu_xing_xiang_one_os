#ifndef CMCC_LOWPOWERCONF_H__
#define CMCC_LOWPOWERCONF_H__

// unused gpio, disabled.
#define AM_BSP_GPIO_PIN00                00
#define AM_BSP_GPIO_PIN01                01
#define AM_BSP_GPIO_PIN02                02		//NB_RESET
#define AM_BSP_GPIO_PIN03                03		//KEY
#define AM_BSP_GPIO_PIN04                04
#define AM_BSP_GPIO_PIN05				 05		//SCL0
#define AM_BSP_GPIO_PIN06				 06		//SDA0
#define AM_BSP_GPIO_PIN07				 07
#define AM_BSP_GPIO_PIN08				 08		//SCL1
#define AM_BSP_GPIO_PIN09				 09		//SDA1
#define AM_BSP_GPIO_PIN10				 10
#define AM_BSP_GPIO_PIN11 				 11
#define AM_BSP_GPIO_PIN12 				 12		//ADC_VBAT_CHECK
#define AM_BSP_GPIO_PIN13				 13
#define AM_BSP_GPIO_PIN14				 14
#define AM_BSP_GPIO_PIN15				 15
#define AM_BSP_GPIO_PIN16				 16
#define AM_BSP_GPIO_PIN17				 17
#define AM_BSP_GPIO_PIN18				 18
#define AM_BSP_GPIO_PIN19				 19
#define AM_BSP_GPIO_PIN20				 20		//SWDCK
#define AM_BSP_GPIO_PIN21				 21		//SWDIO
#define AM_BSP_GPIO_PIN22				 22		//sim_UART_TX
#define AM_BSP_GPIO_PIN23				 23		//sim_UART_TX
#define AM_BSP_GPIO_PIN24				 24
#define AM_BSP_GPIO_PIN25				 25
#define AM_BSP_GPIO_PIN26				 26
#define AM_BSP_GPIO_PIN27				 27
#define AM_BSP_GPIO_PIN28				 28
#define AM_BSP_GPIO_PIN29				 29
#define AM_BSP_GPIO_PIN30				 30
#define AM_BSP_GPIO_PIN31				 31
#define AM_BSP_GPIO_PIN32				 32
#define AM_BSP_GPIO_PIN33				 33
#define AM_BSP_GPIO_PIN34				 34
#define AM_BSP_GPIO_PIN35				 35		//UART_TX
#define AM_BSP_GPIO_PIN36				 36		//UART_TX
#define AM_BSP_GPIO_PIN37				 37
#define AM_BSP_GPIO_PIN38				 38
#define AM_BSP_GPIO_PIN39                39
#define AM_BSP_GPIO_PIN40                40
#define AM_BSP_GPIO_PIN41                41		//SWO
#define AM_BSP_GPIO_PIN42                42
#define AM_BSP_GPIO_PIN43                43
#define AM_BSP_GPIO_PIN44                44
#define AM_BSP_GPIO_PIN45                45
#define AM_BSP_GPIO_PIN46                46
#define AM_BSP_GPIO_PIN47                47
#define AM_BSP_GPIO_PIN48                48
#define AM_BSP_GPIO_PIN49                49

//*****************************************************************************
//
// TEST UART pins.
// Pin 22/23
//
//*****************************************************************************
#define AM_BSP_GPIO_TEST_TX            22
#define AM_BSP_GPIO_TEST_RX            23
#define AM_BSP_GPIO_CFG_TEST_TX        AM_HAL_PIN_22_GPIO
#define AM_BSP_GPIO_CFG_TEST_RX        AM_HAL_PIN_23_GPIO
//*****************************************************************************
//
// Key pins.
//
//*****************************************************************************
#define AM_BSP_GPIO_KEY                3                // am_bsp_gpio.h defined is IOM1_CS
#define AM_BSP_GPIO_CFG_KEY           AM_HAL_PIN_3_GPIO


//*****************************************************************************
//
// ADC pins.
//
//*****************************************************************************
#define AM_BSP_GPIO_ADC0                12                // am_bsp_gpio.h defined is IOM1_CS
#define AM_BSP_GPIO_CFG_ADC0            AM_HAL_PIN_12_GPIO

//*****************************************************************************
//
// UART NB  pins.
// Pin 14/15
// Pin 33/34
//
//*****************************************************************************
#define AM_BSP_GPIO_UART_TX_NB          35
#define AM_BSP_GPIO_CFG_UART_TX_NB      AM_HAL_PIN_35_UARTTX
#define AM_BSP_GPIO_UART_RX_NB          36
#define AM_BSP_GPIO_CFG_UART_RX_NB      AM_HAL_PIN_36_UARTRX
#define AM_BSP_GPIO_RESET_NB            2
#define AM_BSP_GPIO_CFG_RESET_NB        AM_HAL_PIN_2_GPIO

//*****************************************************************************
//
// SWD pins.
// Pin 20/21             It's defined in am_bsp_gpio.h
//*****************************************************************************
//#define AM_BSP_GPIO_SWDCK                20
//#define AM_BSP_GPIO_CFG_SWDCK            AM_HAL_PIN_20_GPIO
//#define AM_BSP_GPIO_SWDIO                21
//#define AM_BSP_GPIO_CFG_SWDIO            AM_HAL_PIN_21_GPIO

#define AM_HAL_PIN45_DISABLE      (AM_HAL_GPIO_FUNC(2))

#define PIN_CONFIG_DISABLE 1

//#ifdef PIN_CONFIG_DISABLE
//#undef PIN_CONFIG_DISABLE
//#endif

#define am_bsp_pin_config(name)                                              \
    am_hal_gpio_pin_config(AM_BSP_GPIO_ ## name, AM_HAL_PIN_3STATE);

//#define am_bsp_pin_disable2(name)                                              \
//    am_hal_gpio_pin_config(AM_BSP_GPIO_ ## name, AM_HAL_PIN_DISABLE );

#define am_bsp_pin_confinput(name)                                              \
    am_hal_gpio_pin_config(AM_BSP_GPIO_ ## name, AM_HAL_PIN_INPUT);

#define am_bsp_pin_confoutput(name)                                              \
    am_hal_gpio_pin_config(AM_BSP_GPIO_ ## name, AM_HAL_PIN45_DISABLE);

void UnusedPinDis(void);
/* 进入深度睡眠前dosomething */

void preDeepSleep(void);

/* 退去深度睡眠后dosomething */
void postDeepSleep(void);

void ADCTurnOn(void);
void ADCTurnOFF(void);

void uartTurnOn(void);
void uartTurnOff(void);

void uartStatusSwitch(int newStatus);
#endif /* end of the file */
