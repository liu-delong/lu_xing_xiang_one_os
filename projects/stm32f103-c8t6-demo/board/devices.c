extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern UART_HandleTypeDef huart2;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", huart2);

extern UART_HandleTypeDef huart3;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart3", huart3);

extern DMA_HandleTypeDef hdma_usart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_rx", hdma_usart1_rx);

extern DMA_HandleTypeDef hdma_usart2_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart2_rx", hdma_usart2_rx);

extern DMA_HandleTypeDef hdma_usart3_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart3_rx", hdma_usart3_rx);

