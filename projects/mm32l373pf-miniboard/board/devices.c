
#include "bsp.h"
#include "board.h"

#if defined(OS_USING_RTC)
extern RTC_HandleTypeDef rtc;
OS_HAL_DEVICE_DEFINE("RTC_Type", "rtc", rtc);
#endif

#if defined(BSP_USING_UART1)
extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);
#endif

#if defined(BSP_USING_UART2)
extern UART_HandleTypeDef huart2;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", huart2);
#endif


