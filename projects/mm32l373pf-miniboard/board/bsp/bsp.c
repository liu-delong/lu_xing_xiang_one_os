#include "bsp.h"
#include "board.h"

#if defined(BSP_USING_UART1)
UART_HandleTypeDef huart1 =
{
    UART1,
    UART1_IRQn,
};
#endif


#if defined(BSP_USING_UART2)
UART_HandleTypeDef huart2 =
{
    UART2,
    UART2_IRQn,
};
#endif

#if defined(OS_USING_RTC)
RTC_HandleTypeDef rtc = 
{
	RTC,
	RTC_IRQn
};
#endif
static void bsp_systick_init(void)
{
    SystemInit();
    SysTick_Config(SystemCoreClock / OS_TICK_PER_SECOND);
    SysTick->CTRL |= 0x00000004UL;
}

#if defined(HW_INIT_UART)
static void bsp_uart_init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	UART_InitTypeDef UART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
#if defined(BSP_USING_UART1)		
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_UART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 	
	
	NVIC_InitStructure.NVIC_IRQChannel = UART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
  NVIC_Init(&NVIC_InitStructure);	
	
	UART_InitStructure.UART_BaudRate = BAUD_RATE_115200;
  UART_InitStructure.UART_WordLength = UART_WordLength_8b;
  UART_InitStructure.UART_StopBits = UART_StopBits_1;
  UART_InitStructure.UART_Parity = UART_Parity_No;
  UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
  UART_InitStructure.UART_Mode = UART_Mode_Rx | UART_Mode_Tx;
	
	UART_Init(UART1, &UART_InitStructure);
	UART_ITConfig(UART1, UART_IT_RXIEN, ENABLE);	
	UART_Cmd(UART1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);		
#endif
	
#if defined(BSP_USING_UART2)	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 	
	
	NVIC_InitStructure.NVIC_IRQChannel = UART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
  NVIC_Init(&NVIC_InitStructure);	
	
	UART_InitStructure.UART_BaudRate = BAUD_RATE_115200;
  UART_InitStructure.UART_WordLength = UART_WordLength_8b;
  UART_InitStructure.UART_StopBits = UART_StopBits_1;
  UART_InitStructure.UART_Parity = UART_Parity_No;
  UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
  UART_InitStructure.UART_Mode = UART_Mode_Rx | UART_Mode_Tx;
	
	UART_Init(UART2, &UART_InitStructure);
	UART_ITConfig(UART2, UART_IT_RXIEN, ENABLE);	
	UART_Cmd(UART2, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);		
#endif
}
#endif
int hardware_init(void)
{
	bsp_systick_init();
	
#if defined(HW_INIT_UART)
	//bsp_uart_init();
#endif
	
	return 0;
}


