/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : stm32h7xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
extern DMA_HandleTypeDef hdma_uart5_rx;

extern DMA_HandleTypeDef hdma_usart1_rx;

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */
 
/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
* @brief I2C MSP Initialization
* This function configures the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hi2c->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB9     ------> I2C1_SDA
    PB8     ------> I2C1_SCL
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }

}

/**
* @brief I2C MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if(hi2c->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB9     ------> I2C1_SDA
    PB8     ------> I2C1_SCL
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }

}

/**
* @brief QSPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hqspi: QSPI handle pointer
* @retval None
*/
void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hqspi->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspInit 0 */

  /* USER CODE END QUADSPI_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**QUADSPI GPIO Configuration
    PG6     ------> QUADSPI_BK1_NCS
    PF6     ------> QUADSPI_BK1_IO3
    PF7     ------> QUADSPI_BK1_IO2
    PF8     ------> QUADSPI_BK1_IO0
    PF9     ------> QUADSPI_BK1_IO1
    PB2     ------> QUADSPI_CLK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }

}

/**
* @brief QSPI MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hqspi: QSPI handle pointer
* @retval None
*/
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi)
{
  if(hqspi->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();

    /**QUADSPI GPIO Configuration
    PG6     ------> QUADSPI_BK1_NCS
    PF6     ------> QUADSPI_BK1_IO3
    PF7     ------> QUADSPI_BK1_IO2
    PF8     ------> QUADSPI_BK1_IO0
    PF9     ------> QUADSPI_BK1_IO1
    PB2     ------> QUADSPI_CLK
    */
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);

  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }

}

/**
* @brief RTC MSP Initialization
* This function configures the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }

}

/**
* @brief RTC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }

}

/**
* @brief TIM_Base MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspInit 0 */

  /* USER CODE END TIM1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();
    /* TIM1 interrupt Init */
    HAL_NVIC_SetPriority(TIM1_UP_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
  /* USER CODE BEGIN TIM1_MspInit 1 */

  /* USER CODE END TIM1_MspInit 1 */
  }

}

/**
* @brief TIM_Base MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspDeInit 0 */

  /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();

    /* TIM1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(TIM1_UP_IRQn);
  /* USER CODE BEGIN TIM1_MspDeInit 1 */

  /* USER CODE END TIM1_MspDeInit 1 */
  }

}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(huart->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspInit 0 */

  /* USER CODE END UART5_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_UART5_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**UART5 GPIO Configuration
    PB5     ------> UART5_RX
    PB6     ------> UART5_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_UART5;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* UART5 DMA Init */
    /* UART5_RX Init */
    hdma_uart5_rx.Instance = DMA1_Stream1;
    hdma_uart5_rx.Init.Request = DMA_REQUEST_UART5_RX;
    hdma_uart5_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart5_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart5_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart5_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart5_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart5_rx.Init.Mode = DMA_NORMAL;
    hdma_uart5_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_uart5_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_uart5_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_uart5_rx);

    /* UART5 interrupt Init */
    HAL_NVIC_SetPriority(UART5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspInit 1 */

  /* USER CODE END UART5_MspInit 1 */
  }
  else if(huart->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA10     ------> USART1_RX
    PA9     ------> USART1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA1_Stream0;
    hdma_usart1_rx.Init.Request = DMA_REQUEST_USART1_RX;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_NORMAL;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart1_rx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }

}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspDeInit 0 */

  /* USER CODE END UART5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART5_CLK_DISABLE();

    /**UART5 GPIO Configuration
    PB5     ------> UART5_RX
    PB6     ------> UART5_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5|GPIO_PIN_6);

    /* UART5 DMA DeInit */
    HAL_DMA_DeInit(huart->hdmarx);

    /* UART5 interrupt DeInit */
    HAL_NVIC_DisableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspDeInit 1 */

  /* USER CODE END UART5_MspDeInit 1 */
  }
  else if(huart->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA10     ------> USART1_RX
    PA9     ------> USART1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10|GPIO_PIN_9);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(huart->hdmarx);

    /* USART1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }

}

/**
* @brief PCD MSP Initialization
* This function configures the hardware resources used in this example
* @param hpcd: PCD handle pointer
* @retval None
*/
void HAL_PCD_MspInit(PCD_HandleTypeDef* hpcd)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hpcd->Instance==USB_OTG_HS)
  {
  /* USER CODE BEGIN USB_OTG_HS_MspInit 0 */

  /* USER CODE END USB_OTG_HS_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USB_OTG_HS GPIO Configuration
    PB15     ------> USB_OTG_HS_DP
    PB14     ------> USB_OTG_HS_DM
    */
    GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF12_OTG2_FS;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_USB_OTG_HS_CLK_ENABLE();
    /* USB_OTG_HS interrupt Init */
    HAL_NVIC_SetPriority(OTG_HS_EP1_OUT_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(OTG_HS_EP1_OUT_IRQn);
    HAL_NVIC_SetPriority(OTG_HS_EP1_IN_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(OTG_HS_EP1_IN_IRQn);
    HAL_NVIC_SetPriority(OTG_HS_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
  /* USER CODE BEGIN USB_OTG_HS_MspInit 1 */

  /* USER CODE END USB_OTG_HS_MspInit 1 */
  }

}

/**
* @brief PCD MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hpcd: PCD handle pointer
* @retval None
*/
void HAL_PCD_MspDeInit(PCD_HandleTypeDef* hpcd)
{
  if(hpcd->Instance==USB_OTG_HS)
  {
  /* USER CODE BEGIN USB_OTG_HS_MspDeInit 0 */

  /* USER CODE END USB_OTG_HS_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USB_OTG_HS_CLK_DISABLE();

    /**USB_OTG_HS GPIO Configuration
    PB15     ------> USB_OTG_HS_DP
    PB14     ------> USB_OTG_HS_DM
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15|GPIO_PIN_14);

    /* USB_OTG_HS interrupt DeInit */
    HAL_NVIC_DisableIRQ(OTG_HS_EP1_OUT_IRQn);
    HAL_NVIC_DisableIRQ(OTG_HS_EP1_IN_IRQn);
    HAL_NVIC_DisableIRQ(OTG_HS_IRQn);
  /* USER CODE BEGIN USB_OTG_HS_MspDeInit 1 */

  /* USER CODE END USB_OTG_HS_MspDeInit 1 */
  }

}

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit(void){
  /* USER CODE BEGIN FMC_MspInit 0 */

  /* USER CODE END FMC_MspInit 0 */
  GPIO_InitTypeDef GPIO_InitStruct ={0};
  if (FMC_Initialized) {
    return;
  }
  FMC_Initialized = 1;

  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_ENABLE();

  /** FMC GPIO Configuration
  PI6   ------> FMC_D28
  PI5   ------> FMC_NBL3
  PI4   ------> FMC_NBL2
  PI1   ------> FMC_D25
  PI0   ------> FMC_D24
  PI7   ------> FMC_D29
  PE1   ------> FMC_NBL1
  PI2   ------> FMC_D26
  PH15   ------> FMC_D23
  PH14   ------> FMC_D22
  PE0   ------> FMC_NBL0
  PI3   ------> FMC_D27
  PG15   ------> FMC_SDNCAS
  PD0   ------> FMC_D2
  PH13   ------> FMC_D21
  PI9   ------> FMC_D30
  PD1   ------> FMC_D3
  PI10   ------> FMC_D31
  PG8   ------> FMC_SDCLK
  PF2   ------> FMC_A2
  PF1   ------> FMC_A1
  PF0   ------> FMC_A0
  PG5   ------> FMC_BA1
  PF3   ------> FMC_A3
  PG4   ------> FMC_BA0
  PG2   ------> FMC_A12
  PF5   ------> FMC_A5
  PF4   ------> FMC_A4
  PC0   ------> FMC_SDNWE
  PE10   ------> FMC_D7
  PF13   ------> FMC_A7
  PF14   ------> FMC_A8
  PE9   ------> FMC_D6
  PE11   ------> FMC_D8
  PH10   ------> FMC_D18
  PH11   ------> FMC_D19
  PD15   ------> FMC_D1
  PD14   ------> FMC_D0
  PF12   ------> FMC_A6
  PF15   ------> FMC_A9
  PE12   ------> FMC_D9
  PE15   ------> FMC_D12
  PH9   ------> FMC_D17
  PH12   ------> FMC_D20
  PF11   ------> FMC_SDNRAS
  PG0   ------> FMC_A10
  PE8   ------> FMC_D5
  PE13   ------> FMC_D10
  PH6   ------> FMC_SDNE1
  PH8   ------> FMC_D16
  PD10   ------> FMC_D15
  PD9   ------> FMC_D14
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE14   ------> FMC_D11
  PH7   ------> FMC_SDCKE1
  PD8   ------> FMC_D13
  */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_1
                          |GPIO_PIN_0|GPIO_PIN_7|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_9
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_8
                          |GPIO_PIN_13|GPIO_PIN_7|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_13|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_9|GPIO_PIN_12|GPIO_PIN_6
                          |GPIO_PIN_8|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_8|GPIO_PIN_5|GPIO_PIN_4
                          |GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_15|GPIO_PIN_14
                          |GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_3
                          |GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN FMC_MspInit 1 */

  /* USER CODE END FMC_MspInit 1 */
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef* hsdram){
  /* USER CODE BEGIN SDRAM_MspInit 0 */

  /* USER CODE END SDRAM_MspInit 0 */
  HAL_FMC_MspInit();
  /* USER CODE BEGIN SDRAM_MspInit 1 */

  /* USER CODE END SDRAM_MspInit 1 */
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit(void){
  /* USER CODE BEGIN FMC_MspDeInit 0 */

  /* USER CODE END FMC_MspDeInit 0 */
  if (FMC_DeInitialized) {
    return;
  }
  FMC_DeInitialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_DISABLE();

  /** FMC GPIO Configuration
  PI6   ------> FMC_D28
  PI5   ------> FMC_NBL3
  PI4   ------> FMC_NBL2
  PI1   ------> FMC_D25
  PI0   ------> FMC_D24
  PI7   ------> FMC_D29
  PE1   ------> FMC_NBL1
  PI2   ------> FMC_D26
  PH15   ------> FMC_D23
  PH14   ------> FMC_D22
  PE0   ------> FMC_NBL0
  PI3   ------> FMC_D27
  PG15   ------> FMC_SDNCAS
  PD0   ------> FMC_D2
  PH13   ------> FMC_D21
  PI9   ------> FMC_D30
  PD1   ------> FMC_D3
  PI10   ------> FMC_D31
  PG8   ------> FMC_SDCLK
  PF2   ------> FMC_A2
  PF1   ------> FMC_A1
  PF0   ------> FMC_A0
  PG5   ------> FMC_BA1
  PF3   ------> FMC_A3
  PG4   ------> FMC_BA0
  PG2   ------> FMC_A12
  PF5   ------> FMC_A5
  PF4   ------> FMC_A4
  PC0   ------> FMC_SDNWE
  PE10   ------> FMC_D7
  PF13   ------> FMC_A7
  PF14   ------> FMC_A8
  PE9   ------> FMC_D6
  PE11   ------> FMC_D8
  PH10   ------> FMC_D18
  PH11   ------> FMC_D19
  PD15   ------> FMC_D1
  PD14   ------> FMC_D0
  PF12   ------> FMC_A6
  PF15   ------> FMC_A9
  PE12   ------> FMC_D9
  PE15   ------> FMC_D12
  PH9   ------> FMC_D17
  PH12   ------> FMC_D20
  PF11   ------> FMC_SDNRAS
  PG0   ------> FMC_A10
  PE8   ------> FMC_D5
  PE13   ------> FMC_D10
  PH6   ------> FMC_SDNE1
  PH8   ------> FMC_D16
  PD10   ------> FMC_D15
  PD9   ------> FMC_D14
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE14   ------> FMC_D11
  PH7   ------> FMC_SDCKE1
  PD8   ------> FMC_D13
  */
  HAL_GPIO_DeInit(GPIOI, GPIO_PIN_6|GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_1
                          |GPIO_PIN_0|GPIO_PIN_7|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_9|GPIO_PIN_10);

  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_9
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_8
                          |GPIO_PIN_13|GPIO_PIN_7|GPIO_PIN_14);

  HAL_GPIO_DeInit(GPIOH, GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_13|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_9|GPIO_PIN_12|GPIO_PIN_6
                          |GPIO_PIN_8|GPIO_PIN_7);

  HAL_GPIO_DeInit(GPIOG, GPIO_PIN_15|GPIO_PIN_8|GPIO_PIN_5|GPIO_PIN_4
                          |GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_1);

  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_15|GPIO_PIN_14
                          |GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_8);

  HAL_GPIO_DeInit(GPIOF, GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_3
                          |GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_11);

  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);

  /* USER CODE BEGIN FMC_MspDeInit 1 */

  /* USER CODE END FMC_MspDeInit 1 */
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef* hsdram){
  /* USER CODE BEGIN SDRAM_MspDeInit 0 */

  /* USER CODE END SDRAM_MspDeInit 0 */
  HAL_FMC_MspDeInit();
  /* USER CODE BEGIN SDRAM_MspDeInit 1 */

  /* USER CODE END SDRAM_MspDeInit 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
