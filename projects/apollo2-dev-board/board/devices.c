

#if defined(BSP_USING_UART0)
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart0", huart0);
#endif

#if defined(BSP_USING_UART1)
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);
#endif

