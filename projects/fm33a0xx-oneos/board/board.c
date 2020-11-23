/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * \@file        board.c
 *
 * \@brief       Initializes the CPU, System clocks, and Peripheral device
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"

#include "push_button.h"
#include "drv_gpio.h"
#include "drv_uart.h"


//Ĭ�Ͽ����󲿷�����ʱ�ӣ��û��ɰ�����رղ���Ҫ��ʱ��
//ʱ�ӿ����رնԹ���Ӱ�첻��
void Init_RCC_PERIPH_clk(void)
{
    //PERCLKCON1
    RCC_PERCLK_SetableEx( DCUCLK, 		ENABLE );		//debug controʱ��ʹ�ܣ������
    RCC_PERCLK_SetableEx( EXTI2CLK, 	ENABLE );		//EXTI�ⲿ�����жϲ���ʱ�ӣ�IO�����˲�ʱ��ʹ��
    RCC_PERCLK_SetableEx( EXTI1CLK, 	ENABLE );		//EXTI�ⲿ�����жϲ���ʱ�ӣ�IO�����˲�ʱ��ʹ��
    RCC_PERCLK_SetableEx( EXTI0CLK, 	ENABLE );		//EXTI�ⲿ�����жϲ���ʱ�ӣ�IO�����˲�ʱ��ʹ��
    RCC_PERCLK_SetableEx( PDCCLK, 		ENABLE );		//IO����ʱ�ӼĴ���ʹ��
    RCC_PERCLK_SetableEx( ANACCLK, 		ENABLE );		//ģ���·����ʱ��ʹ��
    RCC_PERCLK_SetableEx( IWDTCLK, 		ENABLE );		//IWDT����ʱ��ʹ��
    RCC_PERCLK_SetableEx( SCUCLK, 		ENABLE );		//system controlʱ��ʹ�ܣ������
    RCC_PERCLK_SetableEx( PMUCLK, 		ENABLE );		//��Դ����ģ��ʱ��ʹ��
    RCC_PERCLK_SetableEx( RTCCLK, 		ENABLE );		//RTC����ʱ��ʹ��
    RCC_PERCLK_SetableEx( LPTFCLK, 		ENABLE );		//LPTIM����ʱ��ʹ��
    RCC_PERCLK_SetableEx( LPTRCLK, 		ENABLE );		//LPTIM����ʱ��ʹ��

    //PERCLKCON2 SETTING
    RCC_PERCLK_SetableEx( ADCCLK, 		ENABLE );		//ADCʱ��ʹ��
    RCC_PERCLK_SetableEx( WWDTCLK, 		ENABLE );		//WWDTʱ��ʹ��
    RCC_PERCLK_SetableEx( RAMBISTCLK, 	DISABLE );		//RAMBISTʱ��ʹ�ܣ�����ر�
    RCC_PERCLK_SetableEx( FLSEPCLK, 	DISABLE );		//Flash��д������ʱ��ʹ�ܣ�����͹�
    RCC_PERCLK_SetableEx( DMACLK, 		ENABLE );		//DMAʱ��ʹ��
    RCC_PERCLK_SetableEx( LCDCLK, 		ENABLE );		//LCDʱ��ʹ��
    RCC_PERCLK_SetableEx( AESCLK, 		ENABLE );		//AESʱ��ʹ��
    RCC_PERCLK_SetableEx( TRNGCLK, 		ENABLE );		//TRNGʱ��ʹ��
    RCC_PERCLK_SetableEx( CRCCLK, 		ENABLE );		//CRCʱ��ʹ��

    //PERCLKCON3 SETTING
    RCC_PERCLK_SetableEx( I2CCLK, 		ENABLE );		//I2Cʱ��ʹ��
    RCC_PERCLK_SetableEx( U7816CLK1, 	ENABLE );		//78161ʱ��ʹ��
    RCC_PERCLK_SetableEx( U7816CLK0, 	ENABLE );		//78160ʱ��ʹ��
    RCC_PERCLK_SetableEx( UARTCOMCLK, 	ENABLE );		//UART0~5����Ĵ���ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART5CLK, 	ENABLE );		//UART5ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART4CLK, 	ENABLE );		//UART4ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART3CLK, 	ENABLE );		//UART3ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART2CLK, 	ENABLE );		//UART2ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART1CLK, 	ENABLE );		//UART1ʱ��ʹ��
    RCC_PERCLK_SetableEx( UART0CLK, 	ENABLE );		//UART0ʱ��ʹ��
    RCC_PERCLK_SetableEx( HSPICLK, 		ENABLE );		//HSPIʱ��ʹ��
    RCC_PERCLK_SetableEx( SPI2CLK, 		ENABLE );		//SPI2ʱ��ʹ��
    RCC_PERCLK_SetableEx( SPI1CLK, 		ENABLE );		//SPI1ʱ��ʹ��

    //PERCLKCON4 SETTING
    RCC_PERCLK_SetableEx( ET4CLK, 		ENABLE );		//ET4ʱ��ʹ��
    RCC_PERCLK_SetableEx( ET3CLK, 		ENABLE );		//ET3ʱ��ʹ��
    RCC_PERCLK_SetableEx( ET2CLK, 		ENABLE );		//ET2ʱ��ʹ��
    RCC_PERCLK_SetableEx( ET1CLK, 		ENABLE );		//ET1ʱ��ʹ��
    RCC_PERCLK_SetableEx( BT2CLK, 		ENABLE );		//BT2ʱ��ʹ��
    RCC_PERCLK_SetableEx( BT1CLK, 		ENABLE );		//BT1ʱ��ʹ��
}

void Init_PLL(void)
{
    RCC_PLL_InitTypeDef PLL_InitStruct;

    PLL_InitStruct.PLLDB = 499;	//pll��Ƶ�� = PLLDB + 1
    PLL_InitStruct.PLLINSEL = RCC_PLLCON_PLLINSEL_XTLF;	//PLLʱ��Դѡ��XTLF
    PLL_InitStruct.PLLOSEL = RCC_PLLCON_PLLOSEL_MUL1;	//Ĭ��ѡ��1�������������PLLDB��1023ʱ����ʹ��2�����ʵ�ָ��ߵı�Ƶ
    PLL_InitStruct.PLLEN = DISABLE;	//Ĭ�Ϲر�PLL

    RCC_PLL_Init(&PLL_InitStruct);
    RCC_PLLCON_PLLEN_Setable(DISABLE);//�ر�PLL
}

//ϵͳʱ������
//ʹ��RCHF����ʱ��,define_all.h ��SYSCLKdef�����ϵͳʱ��Ƶ��
void Init_SysClk(void)
{
    RCC_RCHF_InitTypeDef RCHF_InitStruct;
    RCC_SYSCLK_InitTypeDef SYSCLK_InitStruct;

    RCHF_InitStruct.FSEL = SYSCLKdef;//define_all.h ��SYSCLKdef�����ϵͳʱ��Ƶ��
    RCHF_InitStruct.RCHFEN = ENABLE;//��RCHF

    RCC_RCHF_Init(&RCHF_InitStruct);

    SYSCLK_InitStruct.SYSCLKSEL = RCC_SYSCLKSEL_SYSCLKSEL_RCHF;	//ѡ��RCHF����ʱ��
    SYSCLK_InitStruct.AHBPRES = RCC_SYSCLKSEL_AHBPRES_DIV1;		//AHB����Ƶ
    SYSCLK_InitStruct.APBPRES = RCC_SYSCLKSEL_APBPRES_DIV1;		//APB����Ƶ
    SYSCLK_InitStruct.EXTICKSEL = RCC_SYSCLKSEL_EXTICKSEL_AHBCLK;	//EXTI,�����˲�ʱ��ʹ��AHBʱ��
    SYSCLK_InitStruct.SLP_ENEXTI = ENABLE;//����ģʽʹ���ⲿ�жϲ���
    SYSCLK_InitStruct.LPM_RCLP_OFF = DISABLE;//����ģʽ�¿���RCLP

    RCC_SysClk_Init(&SYSCLK_InitStruct);
}

//Mode:0 ����ģʽ�����п��Ź����������ж�ʱ��
//Mode:1 ����ģʽ�رտ��Ź����������ж�ʱ��
void SCU_Init(os_uint8_t Mode)
{
    if(Mode == 1)//debug
    {
        SCU_MCUDBGCR_DBG_WWDT_STOP_Setable(ENABLE);//����ģʽ�¹ر�WWDT
        SCU_MCUDBGCR_DBG_IWDT_STOP_Setable(ENABLE);//����ģʽ�¹ر�IWDT
    }
    else//release
    {
        SCU_MCUDBGCR_DBG_WWDT_STOP_Setable(DISABLE);//����ģʽ������WWDT
        SCU_MCUDBGCR_DBG_IWDT_STOP_Setable(DISABLE);//����ģʽ������IWDT
    }

    SCU_MCUDBGCR_DBG_ET4_STOP_Setable(DISABLE);//����ģʽ������ET4
    SCU_MCUDBGCR_DBG_ET3_STOP_Setable(DISABLE);//����ģʽ������ET3
    SCU_MCUDBGCR_DBG_ET2_STOP_Setable(DISABLE);//����ģʽ������ET2
    SCU_MCUDBGCR_DBG_ET1_STOP_Setable(DISABLE);//����ģʽ������ET1
    SCU_MCUDBGCR_DBG_BT2_STOP_Setable(DISABLE);//����ģʽ������BT2
    SCU_MCUDBGCR_DBG_BT1_STOP_Setable(DISABLE);//����ģʽ������BT1
}

void Init_SysClk_Gen( void )				//ʱ��ѡ�����
{

    /*ϵͳʱ�ӳ���24M����Ҫ��wait1*/
    if( RCHFCLKCFG > 24 ) FLASH_FLSRDCON_WAIT_Set(FLASH_FLSRDCON_WAIT_1CYCLE);
    else FLASH_FLSRDCON_WAIT_Set(FLASH_FLSRDCON_WAIT_0CYCLE);

    /*PLL����*/
    Init_PLL();	//Ĭ�Ϲر�PLL

    /*ϵͳʱ������*/
    Init_SysClk();	//Ĭ��ʹ��RCHF����ʱ��

    /*����ʱ��ʹ������*/
    Init_RCC_PERIPH_clk();	//Ĭ�Ͽ����󲿷�����ʱ��

    /*DMA����RAM���ȼ�����*/
    RCC_MPRIL_MPRIL_Set(RCC_MPRIL_MPRIL_DMA);	//Ĭ��AHB Master���ȼ�����DMA����


    /*�µ縴λ����*/
    //pdr��bor�����µ縴λ����Ҫ��һ��
    //����Դ��ѹ�����µ縴λ��ѹʱ��оƬ�ᱻ��λס
    //pdr��ѹ��λ��׼���ǹ��ļ��ͣ������޷�������
    //bor��ѹ��λ׼ȷ������Ҫ����2uA����
    ANAC_PDRCON_PDREN_Setable(ENABLE);		//��PDR
    ANAC_BORCON_OFF_BOR_Setable(DISABLE);	//��PDR

    /*������ƼĴ�������*/
#ifdef __DEBUG
    SCU_Init(1);//����ʱ���ж�ʱ��,�رտ��Ź�
#else
    SCU_Init(0);
#endif
}


const struct push_button key_table[] =
{
    {GET_PIN(F, 5),    PIN_MODE_INPUT,      PIN_IRQ_MODE_RISING_FALLING},
    {GET_PIN(D, 6),    PIN_MODE_INPUT,      PIN_IRQ_MODE_RISING_FALLING},
    {GET_PIN(A, 13),     PIN_MODE_INPUT,      PIN_IRQ_MODE_RISING_FALLING},
    {GET_PIN(C, 15),     PIN_MODE_INPUT,      PIN_IRQ_MODE_RISING_FALLING},
};

const int key_table_size = ARRAY_SIZE(key_table);



