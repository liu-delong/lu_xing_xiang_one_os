extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SPI_HandleTypeDef hspi1;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi1", hspi1);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern UART_HandleTypeDef huart3;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart3", huart3);

