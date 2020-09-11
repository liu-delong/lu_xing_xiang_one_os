/**
  ******************************************************************************
  * @file    fm33a0xx_gpio.h
  * @author  FM33a0xx Application Team
  * @version V3.01
  * @date    
  * @brief   This file contains all the functions prototypes for the GPIO firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33A0XX_GPIO_H
#define __FM33A0XX_GPIO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33A0XX.h"
 
/** @addtogroup FM33a0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup GPIO
  * @{
  */
	 
/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  GPIO Configuration Mode enumeration 
  */
typedef enum
{ 
    GPIO_FCR_IN		= 0x00, /*!< GPIO ���� */
    GPIO_FCR_OUT	= 0x01, /*!< GPIO ��� */
    GPIO_FCR_DIG	= 0x02, /*!< GPIO �������⹦�� */
    GPIO_FCR_ANA	= 0x03  /*!< GPIO ģ�⹦�� */
}GPIO_FCR_TypeDef;

/** 
  * @brief  GPIO Output type enumeration 
  */  
typedef enum
{ 
    GPIO_OD_Dis = 0x00,
    GPIO_OD_En = 0x01
}GPIO_ODEN_TypeDef;

/** 
  * @brief  GPIO Configuration PullUp enumeration 
  */ 
typedef enum
{ 
    GPIO_PU_Dis = 0x00,
    GPIO_PU_En  = 0x01 
}GPIO_PUEN_TypeDef;

/** 
  * @brief  GPIO Configuration PullUp enumeration 
  */ 
typedef enum
{ 
    GPIO_IN_Dis		= 0x00,
    GPIO_IN_En		= 0x01 
}GPIO_INEN_TypeDef;

/** 
  * @brief   GPIO Init structure definition  
  */ 
typedef struct
{
    uint32_t					Pin;     	//PINѡ��
    GPIO_FCR_TypeDef			PxFCR;		//IO����ѡ��
    GPIO_ODEN_TypeDef			PxODEN;		//��©���ʹ�ܿ���
    GPIO_PUEN_TypeDef			PxPUEN;		//��������ʹ�ܿ���
    GPIO_INEN_TypeDef			PxINEN;		//����ʹ�ܿ���
}GPIO_InitTypeDef;

/** 
  * @brief  GPIO Bit SET and Bit RESET enumeration 
  */ 
typedef enum
{ 
    Bit_RESET = 0,
    Bit_SET
}BitAction;
#define IS_GPIO_BIT_ACTION(ACTION) (((ACTION) == Bit_RESET) || ((ACTION) == Bit_SET))

typedef enum
{ 
    EXTI_RISING,
    EXTI_FALLING,
    EXTI_BOTH,
    EXTI_DISABLE,
	
}GPIOExtiEdge;
#define IS_GPIO_INTERRUPT_TRIGER_EDGE(EDGE) (((EDGE) == GPIO_RISING) || ((EDGE) == GPIO_FALLING)|| ((EDGE) == GPIO_BOTH)|| ((EDGE) == GPIO_DISABLE))


/* Exported constants --------------------------------------------------------*/

/** @defgroup GPIO_Exported_Constants
  * @{
  */
	 
/**
* @}
*/ 

/** @defgroup GPIO_Exported_Types
  * @{
  */

#define IS_GPIO_ALL_PERIPH(PERIPH) (((PERIPH) == GPIOA) || \
                                    ((PERIPH) == GPIOB) || \
                                    ((PERIPH) == GPIOC) || \
                                    ((PERIPH) == GPIOD) || \
                                    ((PERIPH) == GPIOE) || \
                                    ((PERIPH) == GPIOF) || \
                                    ((PERIPH) == GPIOG))
                                     
/** @defgroup GPIO_pins_define 
  * @{
  */ 
#define GPIO_Pin_0                 ((uint32_t)0x00000001)  /* Pin 0 selected */
#define GPIO_Pin_1                 ((uint32_t)0x00000002)  /* Pin 1 selected */
#define GPIO_Pin_2                 ((uint32_t)0x00000004)  /* Pin 2 selected */
#define GPIO_Pin_3                 ((uint32_t)0x00000008)  /* Pin 3 selected */
#define GPIO_Pin_4                 ((uint32_t)0x00000010)  /* Pin 4 selected */
#define GPIO_Pin_5                 ((uint32_t)0x00000020)  /* Pin 5 selected */
#define GPIO_Pin_6                 ((uint32_t)0x00000040)  /* Pin 6 selected */
#define GPIO_Pin_7                 ((uint32_t)0x00000080)  /* Pin 7 selected */
#define GPIO_Pin_8                 ((uint32_t)0x00000100)  /* Pin 8 selected */
#define GPIO_Pin_9                 ((uint32_t)0x00000200)  /* Pin 9 selected */
#define GPIO_Pin_10                ((uint32_t)0x00000400)  /* Pin 10 selected */
#define GPIO_Pin_11                ((uint32_t)0x00000800)  /* Pin 11 selected */
#define GPIO_Pin_12                ((uint32_t)0x00001000)  /* Pin 12 selected */
#define GPIO_Pin_13                ((uint32_t)0x00002000)  /* Pin 13 selected */
#define GPIO_Pin_14                ((uint32_t)0x00004000)  /* Pin 14 selected */
#define GPIO_Pin_15                ((uint32_t)0x00008000)  /* Pin 15 selected */
#define GPIO_Pin_All               ((uint16_t)0xFFFF)  /*!< All pins selected */

#define IS_GPIO_PIN(PIN) ((((PIN) & (uint16_t)0x00) == 0x00) && ((PIN) != (uint16_t)0x00))
#define IS_GET_GPIO_PIN(PIN) (((PIN) == GPIO_Pin_0) || \
                              ((PIN) == GPIO_Pin_1) || \
                              ((PIN) == GPIO_Pin_2) || \
                              ((PIN) == GPIO_Pin_3) || \
                              ((PIN) == GPIO_Pin_4) || \
                              ((PIN) == GPIO_Pin_5) || \
                              ((PIN) == GPIO_Pin_6) || \
                              ((PIN) == GPIO_Pin_7) || \
                              ((PIN) == GPIO_Pin_8) || \
                              ((PIN) == GPIO_Pin_9) || \
                              ((PIN) == GPIO_Pin_10) || \
                              ((PIN) == GPIO_Pin_11) || \
                              ((PIN) == GPIO_Pin_12) || \
                              ((PIN) == GPIO_Pin_13) || \
                              ((PIN) == GPIO_Pin_14) || \
                              ((PIN) == GPIO_Pin_15))



/* Exported macro ------------------------------------------------------------*/

/* �����˲�����PIN�궨�� */
#define		IODF_PF3		23	//BIT23
#define		IODF_PF2		22	//BIT22
#define		IODF_PF1		21	//BIT21
#define		IODF_PE9		20	//BIT20
#define		IODF_PE8		19	//BIT19
#define		IODF_PE7		18	//BIT18
#define		IODF_PE6		17	//BIT17
#define		IODF_PE2		16	//BIT16
#define		IODF_PD3		15	//BIT15
#define		IODF_PD2		14	//BIT14
#define		IODF_PD1		13	//BIT13
#define		IODF_PD0		12	//BIT12
#define		IODF_PC15		11	//BIT11
#define		IODF_PC14		10	//BIT10
#define		IODF_PC13		9	//BIT9
#define		IODF_PC12		8	//BIT8
#define		IODF_PB7		7	//BIT7
#define		IODF_PB6		6	//BIT6
#define		IODF_PB5		5	//BIT5
#define		IODF_PB4		4	//BIT4
#define		IODF_PA11		3	//BIT3
#define		IODF_PA10		2	//BIT2
#define		IODF_PA9		1	//BIT1
#define		IODF_PA8		0	//BIT0

/* WKUP����ʹ��PIN�궨�� */
#define		PINWKEN_PD6		BIT7
#define		PINWKEN_PE9		BIT6
#define		PINWKEN_PE2		BIT5
#define		PINWKEN_PA13	BIT4
#define		PINWKEN_PG7		BIT3
#define		PINWKEN_PC13	BIT2
#define		PINWKEN_PB0		BIT1
#define		PINWKEN_PF5		BIT0

//Macro_START
//2018-04-02-13-21-16
//FM33A0XX_Driver_Gen_V1.4

#define	GPIOx_INEN_INEN_Pos	0	/* PortX����ʹ�ܼĴ��� */
#define	GPIOx_INEN_INEN_Msk	(0xffffU << GPIOx_INEN_INEN_Pos)

#define	GPIOx_PUEN_PUEN_Pos	0	/* PortX����ʹ�ܼĴ��� */
#define	GPIOx_PUEN_PUEN_Msk	(0xffffU << GPIOx_PUEN_PUEN_Pos)

#define	GPIOx_ODEN_ODEN_Pos	0	/* PortX��©ʹ�ܼĴ��� */
#define	GPIOx_ODEN_ODEN_Msk	(0xffffU << GPIOx_ODEN_ODEN_Pos)

#define	GPIOx_FCR_FCR_Pos	0	/* PortX����ѡ��Ĵ��� */
#define	GPIOx_FCR_FCR_Msk	(0xffffffffU << GPIOx_FCR_FCR_Pos)

#define	GPIOx_DO_DO_Pos	0	/* PortX������ݼĴ��� */
#define	GPIOx_DO_DO_Msk	(0xffffU << GPIOx_DO_DO_Pos)

#define	GPIOx_DSET_DSET_Pos	0	/* PortX���������λ�Ĵ��� */
#define	GPIOx_DSET_DSET_Msk	(0xffffU << GPIOx_DSET_DSET_Pos)

#define	GPIOx_DRESET_DRESET_Pos	0	/* PortX������ݸ�λ�Ĵ��� */
#define	GPIOx_DRESET_DRESET_Msk	(0xffffU << GPIOx_DRESET_DRESET_Pos)

#define	GPIOx_DIN_DIN_Pos	0	/* PortX�������ݼĴ��� */
#define	GPIOx_DIN_DIN_Msk	(0xffffU << GPIOx_DIN_DIN_Pos)

#define	GPIO_EXTI0_SEL_EXTI0_SEL_Pos	0	/* �ⲿ�����ж�ѡ��Ĵ���0 */
#define	GPIO_EXTI0_SEL_EXTI0_SEL_Msk	(0xffffffffU << GPIO_EXTI0_SEL_EXTI0_SEL_Pos)

#define	GPIO_EXTI1_SEL_EXTI1_SEL_Pos	0	/* �ⲿ�����ж�ѡ��Ĵ���1 */
#define	GPIO_EXTI1_SEL_EXTI1_SEL_Msk	(0xffffffffU << GPIO_EXTI1_SEL_EXTI1_SEL_Pos)

#define	GPIO_EXTI2_SEL_EXTI2_SEL_Pos	0	/* �ⲿ�����ж�ѡ��Ĵ���2 */
#define	GPIO_EXTI2_SEL_EXTI2_SEL_Msk	(0xffffffffU << GPIO_EXTI2_SEL_EXTI2_SEL_Pos)

#define	GPIO_EXTI0IF_EXTI0IF_Pos	0	/* �ⲿ�����жϱ�־�Ĵ���0 */
#define	GPIO_EXTI0IF_EXTI0IF_Msk	(0xffU << GPIO_EXTI0IF_EXTI0IF_Pos)

#define	GPIO_EXTI1IF_EXTI1IF_Pos	0	/* �ⲿ�����жϱ�־�Ĵ���1 */
#define	GPIO_EXTI1IF_EXTI1IF_Msk	(0xffU << GPIO_EXTI1IF_EXTI1IF_Pos)

#define	GPIO_EXTI2IF_EXTI2IF_Pos	0	/* �ⲿ�����жϱ�־�Ĵ���2 */
#define	GPIO_EXTI2IF_EXTI2IF_Msk	(0xffU << GPIO_EXTI2IF_EXTI2IF_Pos)

#define	GPIO_FOUTSEL_FOUTSEL_Pos	0	/* FOUT���Ƶ��ѡ���ź� */
#define	GPIO_FOUTSEL_FOUTSEL_Msk	(0xfU << GPIO_FOUTSEL_FOUTSEL_Pos)
#define	GPIO_FOUTSEL_FOUTSEL_XTLF	(0x0U << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 0000: XTLF */
#define	GPIO_FOUTSEL_FOUTSEL_RCLP	(0x1U << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 0001: RCLP */
#define	GPIO_FOUTSEL_FOUTSEL_RCHFD64	(0x2U << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 0010: RCHF/64 */
#define	GPIO_FOUTSEL_FOUTSEL_LSCLK	(0x3U << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 0011: LSCLK */
#define	GPIO_FOUTSEL_FOUTSEL_AHBCLKD64	(0x4U << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 0100: AHBCLK/64 */
#define	GPIO_FOUTSEL_FOUTSEL_RTCTM	(0x5U << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 0101: RTCTM */
#define	GPIO_FOUTSEL_FOUTSEL_PLLOD64	(0x6U << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 0110: PLLO/64 */
#define	GPIO_FOUTSEL_FOUTSEL_RTCCLK64HZ	(0x7U << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 0111: RTCCLK64Hz */
#define	GPIO_FOUTSEL_FOUTSEL_APBCLKD64	(0x8U << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 1000: APBCLK/64 */
#define	GPIO_FOUTSEL_FOUTSEL_LCDBOOSTERCLK	(0x9U << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 1001: LCD Booster Clock */
#define	GPIO_FOUTSEL_FOUTSEL_LVMOS	(0xaU << GPIO_FOUTSEL_FOUTSEL_Pos)	/* 1010: LVMOS-Monitor-RCOSC/64 */

#define	GPIO_HDSEL_PG6HDEN_Pos	1	/* PG6ǿ����ʹ�� */
#define	GPIO_HDSEL_PG6HDEN_Msk	(0x1U << GPIO_HDSEL_PG6HDEN_Pos)
	/* 0���ر�ǿ����ģʽ */
	/* 1��ʹ��ǿ����ģʽ */

#define	GPIO_HDSEL_PE2HDEN_Pos	0	/* PE2ǿ����ʹ�� */
#define	GPIO_HDSEL_PE2HDEN_Msk	(0x1U << GPIO_HDSEL_PE2HDEN_Pos)
	/* 0���ر�ǿ����ģʽ */
	/* 1��ʹ��ǿ����ģʽ */

#define	GPIO_ANASEL_PE4ANS_Pos	5	/* PE4ģ�⹦��ѡ�� */
#define	GPIO_ANASEL_PE4ANS_Msk	(0x1U << GPIO_ANASEL_PE4ANS_Pos)
#define	GPIO_ANASEL_PE4ANS_ACMP2_INP1	(0x0U << GPIO_ANASEL_PE4ANS_Pos)	/* 0��ѡ����ΪACMP2_INP1 */
#define	GPIO_ANASEL_PE4ANS_SEG19	(0x1U << GPIO_ANASEL_PE4ANS_Pos)	/* 1��ѡ����ΪSEG19 */

#define	GPIO_ANASEL_PE3ANS_Pos	4	/* PE3ģ�⹦��ѡ�� */
#define	GPIO_ANASEL_PE3ANS_Msk	(0x1U << GPIO_ANASEL_PE3ANS_Pos)
#define	GPIO_ANASEL_PE3ANS_ACMP2_INN1	(0x0U << GPIO_ANASEL_PE3ANS_Pos)	/* 0��ѡ����ΪACMP2_INN1 */
#define	GPIO_ANASEL_PE3ANS_SEG18	(0x1U << GPIO_ANASEL_PE3ANS_Pos)	/* 1��ѡ����ΪSEG18 */

#define	GPIO_ANASEL_PC15ANS_Pos	3	/* PC15ģ�⹦��ѡ�� */
#define	GPIO_ANASEL_PC15ANS_Msk	(0x1U << GPIO_ANASEL_PC15ANS_Pos)
#define	GPIO_ANASEL_PC15ANS_ACMP2_INP0	(0x0U << GPIO_ANASEL_PC15ANS_Pos)	/* 0��ѡ����ΪACMP2_INP0 */
#define	GPIO_ANASEL_PC15ANS_SEG39	(0x1U << GPIO_ANASEL_PC15ANS_Pos)	/* 1��ѡ����ΪSEG39 */

#define	GPIO_ANASEL_PC14ANS_Pos	2	/* PC14ģ�⹦��ѡ�� */
#define	GPIO_ANASEL_PC14ANS_Msk	(0x1U << GPIO_ANASEL_PC14ANS_Pos)
#define	GPIO_ANASEL_PC14ANS_ACMP1_INN0	(0x0U << GPIO_ANASEL_PC14ANS_Pos)	/* 0��ѡ����ΪACMP1_INN0 */
#define	GPIO_ANASEL_PC14ANS_SEG38	(0x1U << GPIO_ANASEL_PC14ANS_Pos)	/* 1��ѡ����ΪSEG38 */

#define	GPIO_ANASEL_PC13ANS_Pos	1	/* PC13ģ�⹦��ѡ�� */
#define	GPIO_ANASEL_PC13ANS_Msk	(0x1U << GPIO_ANASEL_PC13ANS_Pos)
#define	GPIO_ANASEL_PC13ANS_ADC_IN2	(0x0U << GPIO_ANASEL_PC13ANS_Pos)	/* 0��ѡ����ΪADC_IN2 */
#define	GPIO_ANASEL_PC13ANS_SEG37	(0x1U << GPIO_ANASEL_PC13ANS_Pos)	/* 1��ѡ����ΪSEG37 */

#define	GPIO_ANASEL_PC12ANS_Pos	0	/* PC12ģ�⹦��ѡ�� */
#define	GPIO_ANASEL_PC12ANS_Msk	(0x1U << GPIO_ANASEL_PC12ANS_Pos)
#define	GPIO_ANASEL_PC12ANS_ADC_IN1	(0x0U << GPIO_ANASEL_PC12ANS_Pos)	/* 0��ѡ����ΪADC_IN1 */
#define	GPIO_ANASEL_PC12ANS_SEG36	(0x1U << GPIO_ANASEL_PC12ANS_Pos)	/* 1��ѡ����ΪSEG36 */

#define	GPIO_IODF_IODF_Pos	0	/* GPIO���������˲��Ĵ��� */
#define	GPIO_IODF_IODF_Msk	(0xffffffU << GPIO_IODF_IODF_Pos)

#define	GPIO_PINWKEN_PINWKEN_Pos	0	/* WKUP����ʹ�ܼĴ��� */
#define	GPIO_PINWKEN_PINWKEN_Msk	(0xffU << GPIO_PINWKEN_PINWKEN_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void GPIOx_Deinit(GPIOx_Type* GPIOx);

/* PortX������ݼĴ��� ��غ��� */
extern void GPIOx_DO_Write(GPIOx_Type* GPIOx, uint32_t SetValue);
extern uint32_t GPIOx_DO_Read(GPIOx_Type* GPIOx);

/* PortX���������λ�Ĵ��� ��غ��� */
extern void GPIOx_DSET_Write(GPIOx_Type* GPIOx, uint32_t SetValue);

/* PortX������ݸ�λ�Ĵ��� ��غ��� */
extern void GPIOx_DRESET_Write(GPIOx_Type* GPIOx, uint32_t SetValue);

/* PortX�������ݼĴ��� ��غ��� */
extern uint32_t GPIOx_DIN_Read(GPIOx_Type* GPIOx);
extern void GPIO_Deinit(void);

/* FOUT���Ƶ��ѡ���ź� ��غ��� */
extern void GPIO_FOUTSEL_FOUTSEL_Set(uint32_t SetValue);
extern uint32_t GPIO_FOUTSEL_FOUTSEL_Get(void);

/* PG6ǿ����ʹ�� ��غ��� */
extern void GPIO_HDSEL_PG6HDEN_Setable(FunState NewState);
extern FunState GPIO_HDSEL_PG6HDEN_Getable(void);

/* PE2ǿ����ʹ�� ��غ��� */
extern void GPIO_HDSEL_PE2HDEN_Setable(FunState NewState);
extern FunState GPIO_HDSEL_PE2HDEN_Getable(void);

/* PE4ģ�⹦��ѡ�� ��غ��� */
extern void GPIO_ANASEL_PE4ANS_Set(uint32_t SetValue);
extern uint32_t GPIO_ANASEL_PE4ANS_Get(void);

/* PE3ģ�⹦��ѡ�� ��غ��� */
extern void GPIO_ANASEL_PE3ANS_Set(uint32_t SetValue);
extern uint32_t GPIO_ANASEL_PE3ANS_Get(void);

/* PC15ģ�⹦��ѡ�� ��غ��� */
extern void GPIO_ANASEL_PC15ANS_Set(uint32_t SetValue);
extern uint32_t GPIO_ANASEL_PC15ANS_Get(void);

/* PC14ģ�⹦��ѡ�� ��غ��� */
extern void GPIO_ANASEL_PC14ANS_Set(uint32_t SetValue);
extern uint32_t GPIO_ANASEL_PC14ANS_Get(void);

/* PC13ģ�⹦��ѡ�� ��غ��� */
extern void GPIO_ANASEL_PC13ANS_Set(uint32_t SetValue);
extern uint32_t GPIO_ANASEL_PC13ANS_Get(void);

/* PC12ģ�⹦��ѡ�� ��غ��� */
extern void GPIO_ANASEL_PC12ANS_Set(uint32_t SetValue);
extern uint32_t GPIO_ANASEL_PC12ANS_Get(void);
//Announce_End



/* Exported functions --------------------------------------------------------*/ 

extern void GPIO_ALL_Deinit(void);

/* GPIO ��ʼ������ */
extern void GPIO_Init(GPIOx_Type* GPIOx, GPIO_InitTypeDef* para);

/* ��ȡһ��IO�ڵ����ò����ṹ�� 
	ע��һ��ֻ�ܶ�ȡһ��IO������*/
extern void GPIO_Get_InitPara(GPIOx_Type* GPIOx, uint32_t GPIO_Pin, GPIO_InitTypeDef* para);

/* ��ȡGPIOx�������ݼĴ��� */
extern uint32_t GPIO_ReadInputData(GPIOx_Type* GPIOx);

/* ��ȡGPIOx�������ݼĴ���bit */
extern uint8_t GPIO_ReadInputDataBit(GPIOx_Type* GPIOx, uint32_t GPIO_Pin);

/* GPIOx���������1 */
extern void GPIO_SetBits(GPIOx_Type* GPIOx, uint32_t GPIO_Pin);

/* GPIOx���������0 */
extern void GPIO_ResetBits(GPIOx_Type* GPIOx, uint32_t GPIO_Pin);

/* GPIOx��������÷�ת */
extern void GPIO_ToggleBits(GPIOx_Type* GPIOx, uint32_t GPIO_Pin);

/* ��ȡGPIOx������ݼĴ��� */
extern uint32_t GPIO_ReadOutputData(GPIOx_Type* GPIOx);

/* ��ȡGPIOx������ݼĴ���bit */
extern uint8_t GPIO_ReadOutputDataBit(GPIOx_Type* GPIOx, uint32_t GPIO_Pin);

/* GPIO���������˲����� */
extern void GPIO_IODF_SetableEx(uint32_t DFPinDef, FunState NewState);

/* WKUP����ʹ�� */
extern void GPIO_PINWKEN_SetableEx(uint32_t NWKPinDef, FunState NewState);

/*************************************************************************
 �������ƣ�GPIO_EXTI_Select_Pin
 ����˵����GPIO EXTI �ⲿ����ѡ��
 ���������GPIOx �˿� GPIO_Pinpin �˿������� 
 �����������
 ���ز����w��
 *************************************************************************/
extern void GPIO_EXTI_Select_Pin(GPIOx_Type* GPIOx,uint32_t GPIO_Pin);

/*************************************************************************
 �������ƣ�GPIO_EXTI_Select_Edge
 ����˵����GPIO EXTI_Select_Edge �ⲿ�жϱ���ѡ��
 ���������GPIOx �˿� ��GPIO_Pin �˿������� ,edge_select ������
 �����������
 ���ز����w��
 *************************************************************************/
extern void GPIO_EXTI_Select_Edge(GPIOx_Type* GPIOx,uint32_t GPIO_Pin,GPIOExtiEdge edge_select);

/*************************************************************************
 �������ƣ�GPIO_EXTI_EXTIxIF_ClrEx
 ����˵����GPIO_EXTI_EXTIxIF_ClrEx ����жϱ�־
 ���������GPIOx �˿� ��GPIO_Pin �˿������� 
 �����������
 ���ز����w��
 *************************************************************************/
extern void GPIO_EXTI_EXTIxIF_ClrEx(GPIOx_Type* GPIOx,uint32_t GPIO_Pin);

/*************************************************************************
 �������ƣ�GPIO_EXTI_EXTIxIF_ChkEx
 ����˵����GPIO_EXTI_EXTIxIF_ChkEx ��ȡ�жϱ�־״̬
 ���������GPIOx �˿� ��GPIO_Pin �˿������� 
 �����������
 ���ز����wFlagStatus �жϱ�־״̬
 *************************************************************************/
extern FlagStatus GPIO_EXTI_EXTIxIF_ChkEx(GPIOx_Type* GPIOx,uint32_t GPIO_Pin);

/*************************************************************************
 �������ƣ�GPIO_EXTI_Init
 ����˵����GPIO_EXTI_Init �ⲿ�жϳ�ʼ��
 ���������port �˿� ��pin �˿������� ,edge ������
 �����������
 ���ز����w��
 *************************************************************************/
extern void GPIO_EXTI_Init(GPIOx_Type* GPIOx,uint32_t GPIO_Pin,GPIOExtiEdge edge_select);

/*************************************************************************
 �������ƣ�GPIO_EXTI_Close
 ����˵����GPIO_EXTI_Close �ⲿ�жϹر�
 ���������port �˿� ��pin �˿������� 
 �����������
 ���ز����w��
 *************************************************************************/
extern void GPIO_EXTI_Close(GPIOx_Type* GPIOx,uint32_t GPIO_Pin);
	
#ifdef __cplusplus
}
#endif

#endif /* __FM33A0XX_GPIO_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



