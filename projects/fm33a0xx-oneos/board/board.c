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


//默认开启大部分外设时钟，用户可按需求关闭不需要的时钟
//时钟开启关闭对功耗影响不大
void Init_RCC_PERIPH_clk(void)
{
    //PERCLKCON1
    RCC_PERCLK_SetableEx( DCUCLK, 		ENABLE );		//debug contro时钟使能，建议打开
    RCC_PERCLK_SetableEx( EXTI2CLK, 	ENABLE );		//EXTI外部引脚中断采样时钟，IO数字滤波时钟使能
    RCC_PERCLK_SetableEx( EXTI1CLK, 	ENABLE );		//EXTI外部引脚中断采样时钟，IO数字滤波时钟使能
    RCC_PERCLK_SetableEx( EXTI0CLK, 	ENABLE );		//EXTI外部引脚中断采样时钟，IO数字滤波时钟使能
    RCC_PERCLK_SetableEx( PDCCLK, 		ENABLE );		//IO控制时钟寄存器使能
    RCC_PERCLK_SetableEx( ANACCLK, 		ENABLE );		//模拟电路总线时钟使能
    RCC_PERCLK_SetableEx( IWDTCLK, 		ENABLE );		//IWDT总线时钟使能
    RCC_PERCLK_SetableEx( SCUCLK, 		ENABLE );		//system control时钟使能，建议打开
    RCC_PERCLK_SetableEx( PMUCLK, 		ENABLE );		//电源管理模块时钟使能
    RCC_PERCLK_SetableEx( RTCCLK, 		ENABLE );		//RTC总线时钟使能
    RCC_PERCLK_SetableEx( LPTFCLK, 		ENABLE );		//LPTIM功能时钟使能
    RCC_PERCLK_SetableEx( LPTRCLK, 		ENABLE );		//LPTIM总线时钟使能

    //PERCLKCON2 SETTING
    RCC_PERCLK_SetableEx( ADCCLK, 		ENABLE );		//ADC时钟使能
    RCC_PERCLK_SetableEx( WWDTCLK, 		ENABLE );		//WWDT时钟使能
    RCC_PERCLK_SetableEx( RAMBISTCLK, 	DISABLE );		//RAMBIST时钟使能，建议关闭
    RCC_PERCLK_SetableEx( FLSEPCLK, 	DISABLE );		//Flash擦写控制器时钟使能，用完就关
    RCC_PERCLK_SetableEx( DMACLK, 		ENABLE );		//DMA时钟使能
    RCC_PERCLK_SetableEx( LCDCLK, 		ENABLE );		//LCD时钟使能
    RCC_PERCLK_SetableEx( AESCLK, 		ENABLE );		//AES时钟使能
    RCC_PERCLK_SetableEx( TRNGCLK, 		ENABLE );		//TRNG时钟使能
    RCC_PERCLK_SetableEx( CRCCLK, 		ENABLE );		//CRC时钟使能

    //PERCLKCON3 SETTING
    RCC_PERCLK_SetableEx( I2CCLK, 		ENABLE );		//I2C时钟使能
    RCC_PERCLK_SetableEx( U7816CLK1, 	ENABLE );		//78161时钟使能
    RCC_PERCLK_SetableEx( U7816CLK0, 	ENABLE );		//78160时钟使能
    RCC_PERCLK_SetableEx( UARTCOMCLK, 	ENABLE );		//UART0~5共享寄存器时钟使能
    RCC_PERCLK_SetableEx( UART5CLK, 	ENABLE );		//UART5时钟使能
    RCC_PERCLK_SetableEx( UART4CLK, 	ENABLE );		//UART4时钟使能
    RCC_PERCLK_SetableEx( UART3CLK, 	ENABLE );		//UART3时钟使能
    RCC_PERCLK_SetableEx( UART2CLK, 	ENABLE );		//UART2时钟使能
    RCC_PERCLK_SetableEx( UART1CLK, 	ENABLE );		//UART1时钟使能
    RCC_PERCLK_SetableEx( UART0CLK, 	ENABLE );		//UART0时钟使能
    RCC_PERCLK_SetableEx( HSPICLK, 		ENABLE );		//HSPI时钟使能
    RCC_PERCLK_SetableEx( SPI2CLK, 		ENABLE );		//SPI2时钟使能
    RCC_PERCLK_SetableEx( SPI1CLK, 		ENABLE );		//SPI1时钟使能

    //PERCLKCON4 SETTING
    RCC_PERCLK_SetableEx( ET4CLK, 		ENABLE );		//ET4时钟使能
    RCC_PERCLK_SetableEx( ET3CLK, 		ENABLE );		//ET3时钟使能
    RCC_PERCLK_SetableEx( ET2CLK, 		ENABLE );		//ET2时钟使能
    RCC_PERCLK_SetableEx( ET1CLK, 		ENABLE );		//ET1时钟使能
    RCC_PERCLK_SetableEx( BT2CLK, 		ENABLE );		//BT2时钟使能
    RCC_PERCLK_SetableEx( BT1CLK, 		ENABLE );		//BT1时钟使能
}

void Init_PLL(void)
{
    RCC_PLL_InitTypeDef PLL_InitStruct;

    PLL_InitStruct.PLLDB = 499;	//pll倍频数 = PLLDB + 1
    PLL_InitStruct.PLLINSEL = RCC_PLLCON_PLLINSEL_XTLF;	//PLL时钟源选择XTLF
    PLL_InitStruct.PLLOSEL = RCC_PLLCON_PLLOSEL_MUL1;	//默认选择1倍输出，当超出PLLDB的1023时，可使用2倍输出实现更高的倍频
    PLL_InitStruct.PLLEN = DISABLE;	//默认关闭PLL

    RCC_PLL_Init(&PLL_InitStruct);
    RCC_PLLCON_PLLEN_Setable(DISABLE);//关闭PLL
}

//系统时钟配置
//使用RCHF做主时钟,define_all.h 中SYSCLKdef宏控制系统时钟频率
void Init_SysClk(void)
{
    RCC_RCHF_InitTypeDef RCHF_InitStruct;
    RCC_SYSCLK_InitTypeDef SYSCLK_InitStruct;

    RCHF_InitStruct.FSEL = SYSCLKdef;//define_all.h 中SYSCLKdef宏控制系统时钟频率
    RCHF_InitStruct.RCHFEN = ENABLE;//打开RCHF

    RCC_RCHF_Init(&RCHF_InitStruct);

    SYSCLK_InitStruct.SYSCLKSEL = RCC_SYSCLKSEL_SYSCLKSEL_RCHF;	//选择RCHF做主时钟
    SYSCLK_InitStruct.AHBPRES = RCC_SYSCLKSEL_AHBPRES_DIV1;		//AHB不分频
    SYSCLK_InitStruct.APBPRES = RCC_SYSCLKSEL_APBPRES_DIV1;		//APB不分频
    SYSCLK_InitStruct.EXTICKSEL = RCC_SYSCLKSEL_EXTICKSEL_AHBCLK;	//EXTI,数字滤波时钟使用AHB时钟
    SYSCLK_InitStruct.SLP_ENEXTI = ENABLE;//休眠模式使能外部中断采样
    SYSCLK_InitStruct.LPM_RCLP_OFF = DISABLE;//休眠模式下开启RCLP

    RCC_SysClk_Init(&SYSCLK_InitStruct);
}

//Mode:0 仿真模式下运行看门狗，运行所有定时器
//Mode:1 仿真模式关闭看门狗，运行所有定时器
void SCU_Init(os_uint8_t Mode)
{
    if(Mode == 1)//debug
    {
        SCU_MCUDBGCR_DBG_WWDT_STOP_Setable(ENABLE);//仿真模式下关闭WWDT
        SCU_MCUDBGCR_DBG_IWDT_STOP_Setable(ENABLE);//仿真模式下关闭IWDT
    }
    else//release
    {
        SCU_MCUDBGCR_DBG_WWDT_STOP_Setable(DISABLE);//仿真模式下运行WWDT
        SCU_MCUDBGCR_DBG_IWDT_STOP_Setable(DISABLE);//仿真模式下运行IWDT
    }

    SCU_MCUDBGCR_DBG_ET4_STOP_Setable(DISABLE);//仿真模式下运行ET4
    SCU_MCUDBGCR_DBG_ET3_STOP_Setable(DISABLE);//仿真模式下运行ET3
    SCU_MCUDBGCR_DBG_ET2_STOP_Setable(DISABLE);//仿真模式下运行ET2
    SCU_MCUDBGCR_DBG_ET1_STOP_Setable(DISABLE);//仿真模式下运行ET1
    SCU_MCUDBGCR_DBG_BT2_STOP_Setable(DISABLE);//仿真模式下运行BT2
    SCU_MCUDBGCR_DBG_BT1_STOP_Setable(DISABLE);//仿真模式下运行BT1
}

void Init_SysClk_Gen( void )				//时钟选择相关
{

    /*系统时钟超过24M后需要打开wait1*/
    if( RCHFCLKCFG > 24 ) FLASH_FLSRDCON_WAIT_Set(FLASH_FLSRDCON_WAIT_1CYCLE);
    else FLASH_FLSRDCON_WAIT_Set(FLASH_FLSRDCON_WAIT_0CYCLE);

    /*PLL配置*/
    Init_PLL();	//默认关闭PLL

    /*系统时钟配置*/
    Init_SysClk();	//默认使用RCHF做主时钟

    /*外设时钟使能配置*/
    Init_RCC_PERIPH_clk();	//默认开启大部分外设时钟

    /*DMA访问RAM优先级配置*/
    RCC_MPRIL_MPRIL_Set(RCC_MPRIL_MPRIL_DMA);	//默认AHB Master优先级配置DMA优先


    /*下电复位配置*/
    //pdr和bor两个下电复位至少要打开一个
    //当电源电压低于下电复位电压时，芯片会被复位住
    //pdr电压档位不准但是功耗极低（几乎无法测量）
    //bor电压档位准确但是需要增加2uA功耗
    ANAC_PDRCON_PDREN_Setable(ENABLE);		//打开PDR
    ANAC_BORCON_OFF_BOR_Setable(DISABLE);	//打开PDR

    /*仿真控制寄存器配置*/
#ifdef __DEBUG
    SCU_Init(1);//仿真时运行定时器,关闭看门狗
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



