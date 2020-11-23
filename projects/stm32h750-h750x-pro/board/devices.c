extern I2C_HandleTypeDef hi2c1;
struct stm32_i2c_info i2c1_info = {.instance = &hi2c1, .scl = 0x18, .sda = 0x19};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c1", i2c1_info);

extern IWDG_HandleTypeDef hiwdg1;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg1", hiwdg1);

extern QSPI_HandleTypeDef hqspi;
OS_HAL_DEVICE_DEFINE("QSPI_HandleTypeDef", "qspi", hqspi);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern UART_HandleTypeDef huart5;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart5", huart5);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern DMA_HandleTypeDef hdma_uart5_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_uart5_rx", hdma_uart5_rx);

extern DMA_HandleTypeDef hdma_usart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_rx", hdma_usart1_rx);

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
OS_HAL_DEVICE_DEFINE("PCD_HandleTypeDef", "pcd_USB_OTG_HS", hpcd_USB_OTG_HS);

extern SDRAM_HandleTypeDef hsdram1;
OS_HAL_DEVICE_DEFINE("SDRAM_HandleTypeDef", "sdram1", hsdram1);

