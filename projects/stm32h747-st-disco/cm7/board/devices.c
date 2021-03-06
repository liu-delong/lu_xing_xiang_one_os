extern IWDG_HandleTypeDef hiwdg1;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg1", hiwdg1);

extern QSPI_HandleTypeDef hqspi;
OS_HAL_DEVICE_DEFINE("QSPI_HandleTypeDef", "qspi", hqspi);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern DMA_HandleTypeDef hdma_usart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_rx", hdma_usart1_rx);

