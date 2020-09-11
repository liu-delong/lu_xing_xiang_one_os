
#ifdef OS_USING_RTC
extern RTC_HandleTypeDef hrtc;
MT_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);
#endif

#ifdef  OS_USING_WDG
extern WDG_HandleTypeDef hwdg;
MT_DEVICE_DEFINE("WDG_HandleTypeDef", "wdg", hwdg);
#endif

#ifdef  OS_USING_PWM
#ifdef OS_USING_PWM0
extern PWM_HandleTypeDef hpwm0;
MT_DEVICE_DEFINE("PWM_HandleTypeDef", "pwm0", hpwm0);
#endif

#ifdef OS_USING_PWM1
extern PWM_HandleTypeDef hpwm1;
MT_DEVICE_DEFINE("PWM_HandleTypeDef", "pwm1", hpwm1);
#endif

#ifdef OS_USING_PWM2
extern PWM_HandleTypeDef hpwm2;
MT_DEVICE_DEFINE("PWM_HandleTypeDef", "pwm2", hpwm2);
#endif

#ifdef OS_USING_PWM3
extern PWM_HandleTypeDef hpwm3;
MT_DEVICE_DEFINE("PWM_HandleTypeDef", "pwm3", hpwm3);
#endif

#endif  /* OS_USING_PWM */

#ifdef OS_USING_ADC
#ifdef OS_USING_ADC0
extern ADC_HandleTypeDef hadc0;
MT_DEVICE_DEFINE("ADC_HandleTypeDef", "adc0", hadc0);
#endif

#ifdef OS_USING_ADC1
extern ADC_HandleTypeDef hadc1;
MT_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", hadc1);
#endif

#ifdef OS_USING_ADC2
extern ADC_HandleTypeDef hadc2;
MT_DEVICE_DEFINE("ADC_HandleTypeDef", "adc2", hadc2);
#endif

#ifdef OS_USING_ADC3
extern ADC_HandleTypeDef hadc3;
MT_DEVICE_DEFINE("ADC_HandleTypeDef", "adc3", hadc3);
#endif

#ifdef OS_USING_ADC4
extern ADC_HandleTypeDef hadc4;
MT_DEVICE_DEFINE("ADC_HandleTypeDef", "adc4", hadc4);
#endif

#endif /* OS_USING_ADC */

#ifdef OS_USING_TIMER_DRIVER
/* tim0, 3,4,5已经使用在其它地方 */
#ifdef OS_USING_HWTIMER1
extern TIM_HandleTypeDef htim1;
MT_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);
#endif

#ifdef OS_USING_HWTIMER2
extern TIM_HandleTypeDef htim2;
MT_DEVICE_DEFINE("TIM_HandleTypeDef", "tim2", htim2);
#endif

/* The library of tim3 mtk2625 is fixed as a free-running mode for counting */
extern TIM_HandleTypeDef htim3;
MT_DEVICE_DEFINE("TIM_HandleTypeDef", "tim3_clock", htim3);


#endif  /* OS_USING_TIMER_DRIVER */

#ifdef BSP_USING_UART
#ifdef BSP_USING_UART0
extern UART_HandleTypeDef huart0;
MT_DEVICE_DEFINE("UART_HandleTypeDef", "uart0", huart0);
#endif

#ifdef BSP_USING_UART1
extern UART_HandleTypeDef huart1;
MT_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);
#endif

#ifdef BSP_USING_UART2
extern UART_HandleTypeDef huart2;
MT_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", huart2);
#endif

#ifdef BSP_USING_UART3
extern UART_HandleTypeDef huart3;
MT_DEVICE_DEFINE("UART_HandleTypeDef", "uart3", huart3);
#endif

#endif /* BSP_USING_UART */
#ifdef OS_USING_SPI

#ifdef BSP_USING_SPI0
extern SPI_HandleTypeDef hspi0;
MT_DEVICE_DEFINE("SPI_HandleTypeDef", "spi0", hspi0);
#endif

#ifdef BSP_USING_SPI1
extern SPI_HandleTypeDef hspi1;
MT_DEVICE_DEFINE("SPI_HandleTypeDef", "spi1", hspi1);
#endif


#endif /* OS_USING_SPI */



