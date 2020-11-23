#ifndef __BSP_H_
#define __BSP_H_

#include "os_types.h"
#include "board.h"

//#define HW_INIT_UART

typedef struct __UART_HandleTypeDef
{
    UART_TypeDef *dev;
    IRQn_Type irq;
    os_uint8_t *buff;
}UART_HandleTypeDef;


typedef struct __RTC_TypeDef 
{
	RTC_TypeDef *dev;
	IRQn_Type irq;
}RTC_HandleTypeDef;

#if defined(OS_USING_RTC)
extern RTC_HandleTypeDef rtc;
#endif

#if defined(BSP_USING_UART1)
extern UART_HandleTypeDef huart1;
#endif

#if defined(BSP_USING_UART2)
extern UART_HandleTypeDef huart2;
#endif

int hardware_init(void);
#endif /* __BSP_H_ */

