/*******************************************************************************
* Copyright (C) 2018, Huada Semiconductor Co.,Ltd All rights reserved.
*
* This software is owned and published by:
* Huada Semiconductor Co.,Ltd ("HDSC").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with HDSC
* components. This software is licensed by HDSC to be adapted only
* for use in systems utilizing HDSC components. HDSC shall not be
* responsible for misuse or illegal use of this software for devices not
* supported herein. HDSC is providing this software "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the software.
*
* Disclaimer: 
* HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS),
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
* WARRANTY OF NONINFRINGEMENT.
* HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
* SAVINGS OR PROFITS,
* EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
* FROM, THE SOFTWARE.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Disclaimer and Copyright notice must be
* included with each copy of this software, whether used in part or whole,
* at all times.
*/
/******************************************************************************/
/** \file sysctrl.h
 **
 ** Headerfile for SYSCTRL functions
 ** @link SYSCTRL Group Some description @endlink 
 **
 ** History:
 **   - 2018-04-15   Lux     First Version
 **
 ******************************************************************************/

#ifndef __SYSCTRL_H__
#define __SYSCTRL_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc_ddl.h"
#include "interrupts_hc32l13x.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup SysCtrlGroup (SYSCTRL)
  **
 ******************************************************************************/
//@{

/**
 *******************************************************************************
 ** function prototypes.
 ******************************************************************************/

/******************************************************************************
 * Global type definitions
 ******************************************************************************/
#define SYSTEM_XTH            32*1000*1000u     //默认32MHz,具体值应根据实际系统修改
#define SYSTEM_XTL            32768u            //默认32768Hz,具体值应根据实际系统修改
 
/**
 *******************************************************************************
 ** \brief 系统时钟输入源类型定�?
 ** \note
 ******************************************************************************/
typedef enum en_sysctrl_clk_source
{
    SysctrlClkRCH  = 0u,               ///< 内部高速时�?
    SysctrlClkXTH  = 1u,               ///< 外部高速时�?
    SysctrlClkRCL  = 2u,               ///< 内部低速时�?
    SysctrlClkXTL  = 3u,               ///< 外部低速时�?
    SysctrlClkPLL  = 4u,               ///< PLL时钟
}en_sysctrl_clk_source_t;

/**
 *******************************************************************************
 ** \brief RCH频率值枚举类型定�?
 ******************************************************************************/
typedef enum en_sysctrl_rch_freq
{
    SysctrlRchFreq4MHz     = 0u,               ///< 4MHz
    SysctrlRchFreq8MHz     = 1u,               ///< 8MHz
    SysctrlRchFreq16MHz    = 2u,               ///< 16MHz
    SysctrlRchFreq22_12MHz = 3u,               ///< 22.12MHz
    SysctrlRchFreq24MHz    = 4u,               ///< 24MHz
}en_sysctrl_rch_freq_t;

/**
 *******************************************************************************
 ** \brief XTAL驱动能力类型定义
 ******************************************************************************/
typedef enum en_sysctrl_xtal_driver
{
    SysctrlXtalDriver0 = 0u,                ///< 最弱驱动能�?
    SysctrlXtalDriver1 = 1u,                ///< 弱驱动能�?
    SysctrlXtalDriver2 = 3u,                ///< 一般驱动能�?
    SysctrlXtalDriver3 = 3u,                ///< 最强驱动能�?
}en_sysctrl_xtal_driver_t;

/**
 *******************************************************************************
 ** \brief XTH频率值范围选择类型定义
 ******************************************************************************/
typedef enum en_sysctrl_xth_freq
{
    SysctrlXthFreq4_6MHz   = 0u,                ///< 4~6MHz
    SysctrlXthFreq6_12MHz  = 1u,                ///< 6~12MHz
    SysctrlXthFreq12_20MHz = 2u,                ///< 12~20MHz
    SysctrlXthFreq20_32MHz = 3u,                ///< 20~32MHz
}en_sysctrl_xth_freq_t;

/**
 *******************************************************************************
 ** \brief XTH时钟稳定周期数类型定�?
 ******************************************************************************/
typedef enum en_sysctrl_xth_cycle
{
    SysctrlXthStableCycle256   = 0u,              ///< 256 个周期数
    SysctrlXthStableCycle1024  = 1u,              ///< 1024 个周期数
    SysctrlXthStableCycle4096  = 2u,              ///< 4096 个周期数
    SysctrlXthStableCycle16384 = 3u,              ///< 16384 个周期数
}en_sysctrl_xth_cycle_t;

/**
 *******************************************************************************
 ** \brief RCL频率值枚举类型定�?
 ******************************************************************************/
typedef enum en_sysctrl_rcl_freq
{
    SysctrlRclFreq32768    = 0u,               ///< 32.768KHz
    SysctrlRclFreq38400    = 1u,               ///< 38.4KHz
}en_sysctrl_rcl_freq_t;

/**
 *******************************************************************************
 ** \brief RCL时钟稳定周期数类型定�?
 ******************************************************************************/
typedef enum en_sysctrl_rcl_cycle
{
    SysctrlRclStableCycle4   = 0u,                  ///< 4 个周期数
    SysctrlRclStableCycle16  = 1u,                  ///< 16 个周期数
    SysctrlRclStableCycle64  = 2u,                  ///< 64 个周期数
    SysctrlRclStableCycle256 = 3u,                  ///< 256 个周期数
}en_sysctrl_rcl_cycle_t;

/**
 *******************************************************************************
 ** \brief XTL时钟稳定周期数类型定�?
 ******************************************************************************/
typedef enum en_sysctrl_xtl_cycle
{
    SysctrlXtlStableCycle256   = 0u,                  ///< 256 个周期数
    SysctrlXtlStableCycle1024  = 1u,                  ///< 1024 个周期数
    SysctrlXtlStableCycle4096  = 2u,                  ///< 4096 个周期数
    SysctrlXtlStableCycle16384 = 3u,                  ///< 16384 个周期数
}en_sysctrl_xtl_cycle_t;

/**
 *******************************************************************************
 ** \brief XTL晶体振幅枚举类型定义
 ******************************************************************************/
typedef enum en_sysctrl_xtl_amp
{
    SysctrlXtlAmp0 = 0u,                ///< 最小振�?
    SysctrlXtlAmp1 = 1u,                ///< 小振�?
    SysctrlXtlAmp2 = 2u,                ///< 一般振�?
    SysctrlXtlAmp3 = 3u,                ///< 最大振�?
}en_sysctrl_xtl_amp_t;

/**
 *******************************************************************************
 ** \brief PLL时钟稳定周期数类型定�?
 ******************************************************************************/
typedef enum en_sysctrl_pll_cycle
{
    SysctrlPllStableCycle128   = 0u,                  ///< 128个周期数
    SysctrlPllStableCycle256   = 1u,                  ///< 256个周期数
    SysctrlPllStableCycle512   = 2u,                  ///< 512个周期数
    SysctrlPllStableCycle1024  = 3u,                  ///< 1024个周期数
    SysctrlPllStableCycle2048  = 4u,                  ///< 2048个周期数
    SysctrlPllStableCycle4096  = 5u,                  ///< 4096个周期数
    SysctrlPllStableCycle8192  = 6u,                  ///< 8192个周期数
    SysctrlPllStableCycle16384 = 7u,                  ///< 16384个周期数    
}en_sysctrl_pll_cycle_t;

/**
 *******************************************************************************
 ** \brief PLL输入频率范围类型定义
 ******************************************************************************/
typedef enum en_sysctrl_pll_infreq
{
    SysctrlPllInFreq4_6MHz   = 0u,                  ///< 4~16MHz
    SysctrlPllInFreq6_12MHz  = 1u,                  ///< 6~12MHz
    SysctrlPllInFreq12_20MHz = 2u,                  ///< 12~20MHz
    SysctrlPllInFreq20_24MHz = 3u,                  ///< 20~24MHz  
}en_sysctrl_pll_infreq_t;

/**
 *******************************************************************************
 ** \brief PLL输出频率范围类型定义
 ******************************************************************************/
typedef enum en_sysctrl_pll_outfreq
{
    SysctrlPllOutFreq8_12MHz  = 0u,                 ///< 8~12MHz
    SysctrlPllOutFreq12_18MHz = 1u,                 ///< 12~18MHz
    SysctrlPllOutFreq18_24MHz = 2u,                 ///< 18~24MHz
    SysctrlPllOutFreq24_36MHz = 3u,                 ///< 24~36MHz
    SysctrlPllOutFreq36_48MHz = 4u,                 ///< 36~48MHz
}en_sysctrl_pll_outfreq_t;

/**
 *******************************************************************************
 ** \brief PLL输入时钟源类型定�?
 ******************************************************************************/
typedef enum en_sysctrl_pll_clksource
{
    SysctrlPllXthXtal   = 0u,                  ///< XTH晶振输入的时�?
    SysctrlPllXthPd00In = 2u,                  ///< XTH从端口PD00输入的时�?
    SysctrlPllRch       = 3u,                  ///< RCH时钟
}en_sysctrl_pll_clksource_t;

/**
 *******************************************************************************
 ** \brief PLL输入时钟源类型定�?
 ******************************************************************************/
typedef enum en_sysctrl_pll_mul
{
    SysctrlPllMul1  = 1u,                   ///< 1倍频
    SysctrlPllMul2  = 2u,                   ///< 2倍频
    SysctrlPllMul3  = 3u,                   ///< 3倍频
    SysctrlPllMul4  = 4u,                   ///< 4倍频
    SysctrlPllMul5  = 5u,                   ///< 5倍频
    SysctrlPllMul6  = 6u,                   ///< 6倍频
    SysctrlPllMul7  = 7u,                   ///< 7倍频
    SysctrlPllMul8  = 8u,                   ///< 8倍频
    SysctrlPllMul9  = 9u,                   ///< 9倍频
    SysctrlPllMul10 = 10u,                  ///< 10倍频
    SysctrlPllMul11 = 11u,                  ///< 11倍频
    SysctrlPllMul12 = 12u,                  ///< 12倍频
}en_sysctrl_pll_mul_t;

/**
 *******************************************************************************
 ** \brief HCLK时钟分频系数类型定义
 ******************************************************************************/
typedef enum en_sysctrl_hclk_div
{
    SysctrlHclkDiv1   = 0u,              ///< SystemClk
    SysctrlHclkDiv2   = 1u,              ///< SystemClk/2
    SysctrlHclkDiv4   = 2u,              ///< SystemClk/4
    SysctrlHclkDiv8   = 3u,              ///< SystemClk/8
    SysctrlHclkDiv16  = 4u,              ///< SystemClk/16
    SysctrlHclkDiv32  = 5u,              ///< SystemClk/32
    SysctrlHclkDiv64  = 6u,              ///< SystemClk/64
    SysctrlHclkDiv128 = 7u,              ///< SystemClk/128
}en_sysctrl_hclk_div_t;

/**
 *******************************************************************************
 ** \brief PCLK分频系数
 ******************************************************************************/
typedef enum en_sysctrl_pclk_div
{
    SysctrlPclkDiv1 = 0u,                ///< HCLK
    SysctrlPclkDiv2 = 1u,                ///< HCLK/2
    SysctrlPclkDiv4 = 2u,                ///< HCLK/4
    SysctrlPclkDiv8 = 3u,                ///< HCLK/8
}en_sysctrl_pclk_div_t;

/**
 *******************************************************************************
 ** \brief RTC高速时钟补偿时钟频率数据类型定�?
 ******************************************************************************/
typedef enum en_sysctrl_rtc_adjust
{
    SysctrlRTC4MHz  = 0u,                ///< 4MHz
    SysctrlRTC6MHz  = 1u,                ///< 6MHz
    SysctrlRTC8MHz  = 2u,                ///< 8MHz
    SysctrlRTC12MHz = 3u,                ///< 12MHz
    SysctrlRTC16MHz = 4u,                ///< 16MHz
    SysctrlRTC20MHz = 5u,                ///< 20MHz
    SysctrlRTC24MHz = 6u,                ///< 24MHz
    SysctrlRTC32MHz = 7u,                ///< 32MHz
}en_sysctrl_rtc_adjust_t;

/**
 *******************************************************************************
 ** \brief 系统控制模块其他功能数据类型定义
 ******************************************************************************/
typedef enum en_sysctrl_func
{
    SysctrlWkupByRCHEn      =0u,                    ///< 唤醒时使用RCH时钟
    SysctrlEXTHEn           =1u,                    ///< 使能外部高速时钟从输入引脚PD00输入
    SysctrlEXTLEn           =2u,                    ///< 使能外部低速速时钟从输入引脚PC14输入
    SysctrlXTLAlwaysOnEn    =3u,                    ///< 使能后XTL_EN只可置位
    SysctrlClkFuncRTCLpmEn  =4u,                    ///< 使能RTC低功耗模�?
    SysctrlCMLockUpEn       =5u,                    ///< 使能后CPU执行无效指令会复位MCU
    SysctrlSWDUseIOEn       =6u,                    ///< SWD端口设为IO功能
}en_sysctrl_func_t;

/**
 *******************************************************************************
 ** \brief 外设时钟门控开关类型枚�?
 ******************************************************************************/
typedef enum en_sysctrl_peripheral_gate
{
    SysctrlPeripheralUart0      = 0u,       ///< 串口0
    SysctrlPeripheralUart1      = 1u,       ///< 串口1
    SysctrlPeripheralLpUart0    = 2u,       ///< 低功耗串�?
    SysctrlPeripheralLpUart1    = 3u,       ///< 低功耗串�?
    SysctrlPeripheralI2c0       = 4u,       ///< I2C0
    SysctrlPeripheralI2c1       = 5u,       ///< I2C1
    SysctrlPeripheralSpi0       = 6u,       ///< SPI0
    SysctrlPeripheralSpi1       = 7u,       ///< SPI1
    SysctrlPeripheralBTim       = 8u,       ///< 基础定时�?
    SysctrlPeripheralLpTim      = 9u,       ///< 低功耗定时器
    SysctrlPeripheralAdvTim     = 10u,      ///< 高级定时�?
    SysctrlPeripheralTim3       = 11u,      ///< 定时�?
    SysctrlPeripheralOpa        = 13u,      ///< OPA
    SysctrlPeripheralPca        = 14u,      ///< 可编程计数阵�?
    SysctrlPeripheralWdt        = 15u,      ///< 看门�?
    SysctrlPeripheralAdcBgr     = 16u,      ///< ADC&BGR
    SysctrlPeripheralVcLvd      = 17u,      ///< 电压比较和低电压检�?
    SysctrlPeripheralRng        = 18u,      ///< RNG
    SysctrlPeripheralPcnt       = 19u,      ///< PCNT
    SysctrlPeripheralRtc        = 20u,      ///< RTC
    SysctrlPeripheralTrim       = 21u,      ///< 时钟校准
    SysctrlPeripheralLcd        = 22u,      ///< LCD
    SysctrlPeripheralTick       = 24u,      ///< 系统定时�?
    SysctrlPeripheralSwd        = 25u,      ///< SWD
    SysctrlPeripheralCrc        = 26u,      ///< CRC
    SysctrlPeripheralAes        = 27u,      ///< AES
    SysctrlPeripheralGpio       = 28u,      ///< GPIO
    SysctrlPeripheralDma        = 29u,      ///< DMA
    SysctrlPeripheralDiv        = 30u,      ///< 除法�?
    SysctrlPeripheralFlash      = 31u,      ///< Flash
}en_sysctrl_peripheral_gate_t;

/**
 *******************************************************************************
 ** \brief 时钟初始化配置结构体定义
 ******************************************************************************/
typedef struct
{
    en_sysctrl_clk_source_t  enClkSrc;       ///< 时钟源选择
    en_sysctrl_hclk_div_t    enHClkDiv;      ///< HCLK分频系数
    en_sysctrl_pclk_div_t    enPClkDiv;      ///< PCLK分频系数
}stc_sysctrl_clk_config_t;

/**
 *******************************************************************************
 ** \brief 时钟初始化配置结构体定义
 ******************************************************************************/
typedef struct
{
    en_sysctrl_pll_infreq_t    enInFreq;        ///< PLL输入时钟频率范围选择
    en_sysctrl_pll_outfreq_t   enOutFreq;       ///< PLL输出时钟频率范围选择
    en_sysctrl_pll_clksource_t enPllClkSrc;     ///< PLL输入时钟源选择
    en_sysctrl_pll_mul_t       enPllMul;        ///< PLL倍频系数选择
}stc_sysctrl_pll_config_t;

/******************************************************************************
 * Global variable declarations ('extern', definition in C source)
 ******************************************************************************/

/******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/
///< 系统时钟初始化API:用于上电后，系统工作之前对主频及外设时钟进行初始化； 
///< 注意1：使用该初始化函数前需要根据系统，必须优先设置目标内部时钟源的TRIM值或外部时钟源的频率范围�?
///< 注意2：XTH、XTL的频率范围设定，需要根据外部晶振决定，
///< 注意3：本驱动默认宏定义：SYSTEM_XTH=32MHz,SYSTEM_XTL=32768Hz,如使用其它外部晶振，必须修改这两个宏定义的值�?
en_result_t Sysctrl_ClkInit(stc_sysctrl_clk_config_t *pstcCfg);

///< 系统时钟去初始化API:恢复为上电默认状�?>PCLK=HCLK=SystemClk=RCH4MHz
en_result_t Sysctrl_ClkDeInit(void);

///< 系统时钟模块的基本功能设�?
///< 注意：使能需要使用的时钟源之前，必须优先设置目标内部时钟源的TRIM值或外部时钟源的频率范围
en_result_t Sysctrl_ClkSourceEnable(en_sysctrl_clk_source_t enSource, boolean_t bFlag);

///<外部晶振驱动配置：系统初始化Sysctrl_ClkInit()之后，可根据需要配置外部晶振的驱动能力，时钟初始化Sysctrl_ClkInit()默认为最大�?
en_result_t Sysctrl_XTHDriverConfig(en_sysctrl_xtal_driver_t enDriver);
en_result_t Sysctrl_XTLDriverConfig(en_sysctrl_xtl_amp_t enAmp, en_sysctrl_xtal_driver_t enDriver);

///<时钟稳定周期设置:系统初始化Sysctrl_ClkInit()之后，可根据需要配置时钟开启后的稳定之间，默认为最大�?
en_result_t Sysctrl_SetXTHStableTime(en_sysctrl_xth_cycle_t enCycle);
en_result_t Sysctrl_SetRCLStableTime(en_sysctrl_rcl_cycle_t enCycle);
en_result_t Sysctrl_SetXTLStableTime(en_sysctrl_xtl_cycle_t enCycle);
en_result_t Sysctrl_SetPLLStableTime(en_sysctrl_pll_cycle_t enCycle);

///<系统时钟源切换并更新系统时钟：如果需要在系统时钟初始化Sysctrl_ClkInit()之后切换主频时钟源，则使用该函数�?
///< 时钟切换前后，必须根据目标频率值设置Flash读等待周期，可配置插入周期为0�?�?�?
///< 注意!!!：当HCLK大于24MHz时，FLASH等待周期插入必须至少�?,否则程序运行可能产生未知错误
en_result_t Sysctrl_SysClkSwitch(en_sysctrl_clk_source_t enSource);

///< 时钟源频率设定：根据系统情况，单独设置不同时钟源的频率�?
///< 时钟频率设置前，必须根据目标频率值设置Flash读等待周期，可配置插入周期为0�?�?�?
///< 其中XTL的时钟由外部晶振决定，无需设置�?
en_result_t Sysctrl_SetRCHTrim(en_sysctrl_rch_freq_t enRCHFreq);
en_result_t Sysctrl_SetRCLTrim(en_sysctrl_rcl_freq_t enRCLFreq);
en_result_t Sysctrl_SetXTHFreq(en_sysctrl_xth_freq_t enXTHFreq);
en_result_t Sysctrl_SetPLLFreq(stc_sysctrl_pll_config_t *pstcPLLCfg);

///< 时钟分频设置:根据系统情况，单独设置HCLK、PCLK的分配�?
en_result_t Sysctrl_SetHCLKDiv(en_sysctrl_hclk_div_t enHCLKDiv);
en_result_t Sysctrl_SetPCLKDiv(en_sysctrl_pclk_div_t enPCLKDiv);

///< 时钟频率获取：根据系统需要，获取当前HCLK及PCLK的频率�?
uint32_t Sysctrl_GetHClkFreq(void);
uint32_t Sysctrl_GetPClkFreq(void);

///< 外设门控开�?状态获取：用于控制外设模块的使能，使用该模块的功能之前，必须使能该模块的门控时钟；
en_result_t Sysctrl_SetPeripheralGate(en_sysctrl_peripheral_gate_t enPeripheral, boolean_t bFlag);
boolean_t   Sysctrl_GetPeripheralGate(en_sysctrl_peripheral_gate_t enPeripheral);

///< 系统功能配置：用于设置其他系统相关特殊功能；
en_result_t Sysctrl_SetFunc(en_sysctrl_func_t enFunc, boolean_t bFlag);

///< RTC高速时钟补�?用于设置RTC高速时钟下的频率补�?
en_result_t Sysctrl_SetRTCAdjustClkFreq(en_sysctrl_rtc_adjust_t enRtcAdj);

//@} // Sysctrl Group

#ifdef __cplusplus
#endif

#endif /* __SYSCTRL_H__ */
/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


