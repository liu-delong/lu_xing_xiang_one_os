extern ADC_HandleTypeDef hadc1;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", hadc1);

extern CAN_HandleTypeDef hcan1;
OS_HAL_DEVICE_DEFINE("CAN_HandleTypeDef", "can1", hcan1);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern LPTIM_HandleTypeDef hlptim1;
OS_HAL_DEVICE_DEFINE("LPTIM_HandleTypeDef", "lptim1", hlptim1);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SPI_HandleTypeDef hspi1;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi1", hspi1);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern TIM_HandleTypeDef htim2;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim2", htim2);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern UART_HandleTypeDef huart2;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", huart2);

extern DMA_HandleTypeDef hdma_usart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_rx", hdma_usart1_rx);

extern DMA_HandleTypeDef hdma_usart2_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart2_rx", hdma_usart2_rx);

