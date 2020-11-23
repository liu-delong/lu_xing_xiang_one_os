#include "gd32f4xx_usart.h"


void gd_eval_com_init(uint32_t com)
{
	uint32_t com_id = 0U;
	
	com_id = com;

	/* enable GPIO clock */
	rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);

	/* enable USART clock */
	rcu_periph_clock_enable(COM_CLK[com_id]);

	/* connect port to USARTx_Tx */
	gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);

	/* connect port to USARTx_Rx */
	gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);

	/* USART configure */
	usart_deinit(com);
	usart_baudrate_set(com, 115200U);
	usart_receive_config(com, USART_RECEIVE_ENABLE);
	usart_transmit_config(com, USART_TRANSMIT_ENABLE);
	usart_enable(com);
}

static void GD_USART0_UART_Init(void)
{


	nvic_irq_enable(USART0_IRQn, 0, 0);
	gd_eval_com_init(EVAL_COM0);
	usart_interrupt_enable(USART0, USART_INT_RBNE);
	return;
}

int hardware_init(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  //HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  systick_config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  //MX_GPIO_Init();
  GD_USART0_UART_Init();
  //MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  return 0;
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

