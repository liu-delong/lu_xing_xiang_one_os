/**

*******************************************************************************
****************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        drv_common.c
 *
 * @brief       This file provides systick time init/IRQ and board init
functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version

*******************************************************************************
****************************************
 */

#include "board.h"
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_irq.h>
#include <os_clock.h>
#include <os_hw.h>
#include <os_memory.h>
#include "board.h"

#include "drv_gpio.h"
#include "drv_uart.h"
#include "drv_common.h"


void Init_SysTick(void)
{
    SysTick_Config(0x1000000UL);
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_ENABLE_Msk;
}


void TicksDelay(os_uint32_t ClkNum)
{
    os_uint32_t last = SysTick->VAL;

    if(clkmode != 4)
    {
        ClkNum = ClkNum*clkmode;
    }
    else
    {
        ClkNum = ClkNum*clkmode*30/32;
    }

    if(ClkNum>0xF00000)
    {
        ClkNum = 0xF00000;
    }
    while(((last - SysTick->VAL)&0xFFFFFFUL ) < ClkNum);
}
typedef int (*ConditionHook)(void);


void TicksDelayMs(os_uint32_t ms, ConditionHook Hook)
{
    os_uint32_t ClkNum;

    ClkNum = (__SYSTEM_CLOCK/1000) ;
    for(; ms>0; ms--)
    {
        if(Hook!=NULL)
        {
            if(Hook()) return ;
        }
        TicksDelay(ClkNum);
    }
}

void TicksDelayUs(os_uint32_t us)
{
    os_uint32_t ClkNum;

    if(us>100000)
    {
        us = 100000;
    }
    ClkNum = us*(__SYSTEM_CLOCK/1000000) ;
    TicksDelay(ClkNum);
}


unsigned char CheckSysReg( __IO os_uint32_t *RegAddr, os_uint32_t Value )
{
    if( *RegAddr != Value )
    {
        *RegAddr = Value;
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char CheckNvicIrqEn( IRQn_Type IRQn )
{
    if( 0 == ( NVIC->ISER[0U] & ((os_uint32_t)(1UL << (((os_uint32_t)(os_int32_t)IRQn) & 0x1FUL)))) )
        return 0;
    else
        return 1;
}

void AnalogIO( GPIOx_Type* GPIOx, os_uint32_t PinNum )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructureRun;

    GPIO_Get_InitPara(GPIOx, PinNum, &GPIO_InitStructureRun);

    if( (GPIO_InitStructureRun.Pin		!= PinNum) ||
            (GPIO_InitStructureRun.PxINEN	!= GPIO_IN_Dis) ||
            (GPIO_InitStructureRun.PxODEN	!= GPIO_OD_En) ||
            (GPIO_InitStructureRun.PxPUEN	!= GPIO_PU_Dis) ||
            (GPIO_InitStructureRun.PxFCR	!= GPIO_FCR_ANA) )
    {
        GPIO_InitStructure.Pin = PinNum;
        GPIO_InitStructure.PxINEN = GPIO_IN_Dis;
        GPIO_InitStructure.PxODEN = GPIO_OD_En;
        GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        GPIO_InitStructure.PxFCR = GPIO_FCR_ANA;

        GPIO_Init(GPIOx, &GPIO_InitStructure);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           IO INPUT config
 *
 * @details         config IO INPUT to normal or pullup
 *
 * @attention       Attention description(Optional).
 *
 * @param[in]       GPIOx       GPIO num
 * @param[in]       PinNum      GPIO Pin num
 * @param[in]       Type        0-IN_NORMAL, 1-IN_PULLUP
 *
 * @return          void
 ***********************************************************************************************************************
 */
void InputtIO( GPIOx_Type* GPIOx, os_uint32_t PinNum, os_uint8_t Type )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructureRun;

    GPIO_Get_InitPara(GPIOx, PinNum, &GPIO_InitStructureRun);

    if( (GPIO_InitStructureRun.Pin		!= PinNum) ||
            (GPIO_InitStructureRun.PxINEN	!= GPIO_IN_En) ||
            (GPIO_InitStructureRun.PxODEN	!= GPIO_OD_En) ||
            ((Type == IN_NORMAL)&&(GPIO_InitStructureRun.PxPUEN != GPIO_PU_Dis)) ||
            ((Type == IN_PULLUP)&&(GPIO_InitStructureRun.PxPUEN != GPIO_PU_En)) ||
            (GPIO_InitStructureRun.PxFCR	!= GPIO_FCR_IN) )
    {
        GPIO_InitStructure.Pin = PinNum;
        GPIO_InitStructure.PxINEN = GPIO_IN_En;
        GPIO_InitStructure.PxODEN = GPIO_OD_En;
        if(Type == IN_NORMAL)		GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        else						GPIO_InitStructure.PxPUEN = GPIO_PU_En;
        GPIO_InitStructure.PxFCR = GPIO_FCR_IN;

        GPIO_Init(GPIOx, &GPIO_InitStructure);
    }
}



/**
 ***********************************************************************************************************************
 * @brief           IO OUTPUT config
 *
 * @details         config IO OUTPUT to normal or OD
 *
 * @attention       Attention description(Optional).
 *
 * @param[in]       GPIOx       GPIO num
 * @param[in]       PinNum      GPIO Pin num
 * @param[in]       Type        0-OUT_PUSHPULL, 1-OUT_OPENDRAIN
 *
 * @return          void
 ***********************************************************************************************************************
 */
void OutputIO( GPIOx_Type* GPIOx, os_uint32_t PinNum, os_uint8_t Type )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructureRun;

    GPIO_Get_InitPara(GPIOx, PinNum, &GPIO_InitStructureRun);

    if( (GPIO_InitStructureRun.Pin		!= PinNum) ||
            (GPIO_InitStructureRun.PxINEN	!= GPIO_IN_Dis) ||
            ((Type == OUT_PUSHPULL)&&(GPIO_InitStructureRun.PxODEN	!= GPIO_OD_Dis)) ||
            ((Type == OUT_OPENDRAIN)&&(GPIO_InitStructureRun.PxODEN	!= GPIO_OD_En)) ||
            (GPIO_InitStructureRun.PxPUEN	!= GPIO_PU_Dis) ||
            (GPIO_InitStructureRun.PxFCR	!= GPIO_FCR_OUT) )
    {
        GPIO_InitStructure.Pin = PinNum;
        GPIO_InitStructure.PxINEN = GPIO_IN_Dis;
        if(Type == OUT_PUSHPULL)	GPIO_InitStructure.PxODEN = GPIO_OD_Dis;
        else						GPIO_InitStructure.PxODEN = GPIO_OD_En;
        GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        GPIO_InitStructure.PxFCR = GPIO_FCR_OUT;

        GPIO_Init(GPIOx, &GPIO_InitStructure);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           IO digital special function port
 *
 * @details         
 *
 * @attention       Attention description(Optional).
 *
 * @param[in]       GPIOx       GPIO num
 * @param[in]       PinNum      GPIO Pin num
 * @param[in]       Type        0-ALTFUN_NORMAL, 1-ALTFUN_OPENDRAIN, 2-ALTFUN_PULLUP, 3-ALTFUN_OPENDRAIN_PULLUP
 *
 * @return          void
 ***********************************************************************************************************************
 */
void AltFunIO( GPIOx_Type* GPIOx, os_uint32_t PinNum, os_uint8_t Type  )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructureRun;

    GPIO_Get_InitPara(GPIOx, PinNum, &GPIO_InitStructureRun);

    if( (GPIO_InitStructureRun.Pin		!= PinNum) ||
            (GPIO_InitStructureRun.PxINEN	!= GPIO_IN_Dis) ||
            (((Type & 0x01) == 0)&&(GPIO_InitStructureRun.PxODEN	!= GPIO_OD_Dis)) ||
            (((Type & 0x01) != 0)&&(GPIO_InitStructureRun.PxODEN	!= GPIO_OD_En)) ||
            (((Type & 0x02) == 0)&&(GPIO_InitStructureRun.PxPUEN	!= GPIO_PU_Dis)) ||
            (((Type & 0x02) != 0)&&(GPIO_InitStructureRun.PxPUEN	!= GPIO_PU_En)) ||
            (GPIO_InitStructureRun.PxFCR	!= GPIO_FCR_DIG) )
    {
        GPIO_InitStructure.Pin = PinNum;
        GPIO_InitStructure.PxINEN = GPIO_IN_Dis;
        if( (Type & 0x01) == 0 )	GPIO_InitStructure.PxODEN = GPIO_OD_Dis;
        else						GPIO_InitStructure.PxODEN = GPIO_OD_En;
        if( (Type & 0x02) == 0 )	GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        else						GPIO_InitStructure.PxPUEN = GPIO_PU_En;
        GPIO_InitStructure.PxFCR = GPIO_FCR_DIG;

        GPIO_Init(GPIOx, &GPIO_InitStructure);
    }
}

void CloseeIO( GPIOx_Type* GPIOx, os_uint32_t PinNum )
{
    GPIO_InitTypeDef  GPIO_InitStructureRun;

    GPIO_Get_InitPara(GPIOx, PinNum, &GPIO_InitStructureRun);

    if((GPIO_InitStructureRun.PxFCR	!= GPIO_FCR_OUT))
    {
        GPIO_SetBits(GPIOx, PinNum);
        OutputIO( GPIOx, PinNum, OUT_OPENDRAIN );
    }
    else
    {
        OutputIO( GPIOx, PinNum, OUT_OPENDRAIN );
        GPIO_SetBits(GPIOx, PinNum);
    }
}

void Init_Pad_Io(void)
{
    GPIOx_DO_Write(GPIOA, 0x0000);
    GPIOx_DO_Write(GPIOB, 0x0000);
    GPIOx_DO_Write(GPIOC, 0x0000);
    GPIOx_DO_Write(GPIOD, 0x0000);
    GPIOx_DO_Write(GPIOE, 0x0000);
    GPIOx_DO_Write(GPIOF, 0x0000);
    GPIOx_DO_Write(GPIOG, 0x0000);
}

void Close_None_GPIO_80pin(void)
{
    /* Turn off the unusable IO of the 80-pin chip */
    CloseeIO( GPIOC, GPIO_Pin_0 );
    CloseeIO( GPIOC, GPIO_Pin_1 );

    CloseeIO( GPIOD, GPIO_Pin_8);
    CloseeIO( GPIOD, GPIO_Pin_9 );
    CloseeIO( GPIOD, GPIO_Pin_10 );
    CloseeIO( GPIOD, GPIO_Pin_11 );
    CloseeIO( GPIOD, GPIO_Pin_12 );
    CloseeIO( GPIOD, GPIO_Pin_13 );
    CloseeIO( GPIOD, GPIO_Pin_14 );
    CloseeIO( GPIOD, GPIO_Pin_15 );

    CloseeIO( GPIOE, GPIO_Pin_0 );
    CloseeIO( GPIOE, GPIO_Pin_1 );
    CloseeIO( GPIOE, GPIO_Pin_5 );
    CloseeIO( GPIOE, GPIO_Pin_6 );
    CloseeIO( GPIOE, GPIO_Pin_7 );
    CloseeIO( GPIOE, GPIO_Pin_8 );
    CloseeIO( GPIOE, GPIO_Pin_9 );
    CloseeIO( GPIOE, GPIO_Pin_10 );
    CloseeIO( GPIOE, GPIO_Pin_11 );
    CloseeIO( GPIOE, GPIO_Pin_12 );
    CloseeIO( GPIOE, GPIO_Pin_13 );
    CloseeIO( GPIOE, GPIO_Pin_14 );
    CloseeIO( GPIOE, GPIO_Pin_15 );

    CloseeIO( GPIOF, GPIO_Pin_0 );
    CloseeIO( GPIOF, GPIO_Pin_1 );
    CloseeIO( GPIOF, GPIO_Pin_2 );
    CloseeIO( GPIOF, GPIO_Pin_7 );
    CloseeIO( GPIOF, GPIO_Pin_8 );
    CloseeIO( GPIOF, GPIO_Pin_9 );
    CloseeIO( GPIOF, GPIO_Pin_10 );

    CloseeIO( GPIOG, GPIO_Pin_0 );
    CloseeIO( GPIOG, GPIO_Pin_1 );
    CloseeIO( GPIOG, GPIO_Pin_4 );
    CloseeIO( GPIOG, GPIO_Pin_5 );
    CloseeIO( GPIOG, GPIO_Pin_10 );
    CloseeIO( GPIOG, GPIO_Pin_11 );
    CloseeIO( GPIOG, GPIO_Pin_12 );
    CloseeIO( GPIOG, GPIO_Pin_13 );
    CloseeIO( GPIOG, GPIO_Pin_14 );
    CloseeIO( GPIOG, GPIO_Pin_15 );
}

void Close_AllIOEXTI(void)
{
    CheckSysReg( (__IO os_uint32_t *)(&GPIO->EXTI0_SEL)	, 0xFFFF0000 );
    CheckSysReg( (__IO os_uint32_t *)(&GPIO->EXTI1_SEL)	, 0xFFFF0000 );
    CheckSysReg( (__IO os_uint32_t *)(&GPIO->EXTI2_SEL)	, 0xFFFF0000 );
}

void IO_AnalogFunSet(void)
{
    /* PE4 Analog function selection */
    GPIO_ANASEL_PE4ANS_Set(GPIO_ANASEL_PE4ANS_ACMP2_INP1);

    /* PE3 Analog function selection */
    GPIO_ANASEL_PE3ANS_Set(GPIO_ANASEL_PE3ANS_ACMP2_INN1);

    /* PC15 Analog function selection */
    GPIO_ANASEL_PC15ANS_Set(GPIO_ANASEL_PC15ANS_ACMP2_INP0);

    /* PC14 Analog function selection */
    GPIO_ANASEL_PC14ANS_Set(GPIO_ANASEL_PC14ANS_ACMP1_INN0);

    /* PC13 Analog function selection */
    GPIO_ANASEL_PC13ANS_Set(GPIO_ANASEL_PC13ANS_ADC_IN2);

    /* PC12 Analog function selection */
    GPIO_ANASEL_PC12ANS_Set(GPIO_ANASEL_PC12ANS_ADC_IN1);
}

void IO_DFFunSet(void)
{
    GPIO_IODF_SetableEx(IODF_PF3	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PF2	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PF1	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PE9	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PE8	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PE7	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PE6	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PE2	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PD3	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PD2	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PD1	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PD0	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PC15	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PC14	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PC13	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PC12	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PB7	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PB6	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PB5	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PB4	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PA11	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PA10	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PA9	, DISABLE);
    GPIO_IODF_SetableEx(IODF_PA8	, DISABLE);
}

void IO_WKENFunSet(void)
{
    GPIO_PINWKEN_SetableEx(PINWKEN_PD6	, DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PE9	, DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PE2	, DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PA13	, DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PG7	, DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PC13	, DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PB0	, DISABLE);
    GPIO_PINWKEN_SetableEx(PINWKEN_PF5	, DISABLE);
}

void Close_AllIO_GPIO_80pin( void )
{
    /* Disable all IO interrupt functions */
    Close_AllIOEXTI();

    /* Strong drive off */
    GPIO_HDSEL_PE2HDEN_Setable(DISABLE);
    GPIO_HDSEL_PG6HDEN_Setable(DISABLE);

    /* FOUT output signal selection */
    GPIO_FOUTSEL_FOUTSEL_Set(GPIO_FOUTSEL_FOUTSEL_LSCLK);

    /* Analog LCD, AD function selection */
    IO_AnalogFunSet();

    /* Partial IO input digital filter function */
    IO_DFFunSet();

    /* Disable NWKUP */
    IO_WKENFunSet();

    CloseeIO( GPIOA, GPIO_Pin_0 );	
    CloseeIO( GPIOA, GPIO_Pin_1 );	
    CloseeIO( GPIOA, GPIO_Pin_2 );	
    CloseeIO( GPIOA, GPIO_Pin_3 );	
    CloseeIO( GPIOA, GPIO_Pin_4 );	
    CloseeIO( GPIOA, GPIO_Pin_5 );	
    CloseeIO( GPIOA, GPIO_Pin_6 );	
    CloseeIO( GPIOA, GPIO_Pin_7 );	
    CloseeIO( GPIOA, GPIO_Pin_8 );	
    CloseeIO( GPIOA, GPIO_Pin_9 );	
    CloseeIO( GPIOA, GPIO_Pin_10 );
    CloseeIO( GPIOA, GPIO_Pin_11 );
    CloseeIO( GPIOA, GPIO_Pin_12 );
    CloseeIO( GPIOA, GPIO_Pin_13 );
    CloseeIO( GPIOA, GPIO_Pin_14 );
    CloseeIO( GPIOA, GPIO_Pin_15 );

    CloseeIO( GPIOB, GPIO_Pin_0 );
    CloseeIO( GPIOB, GPIO_Pin_1 );
    CloseeIO( GPIOB, GPIO_Pin_2 );
    CloseeIO( GPIOB, GPIO_Pin_3 );
    CloseeIO( GPIOB, GPIO_Pin_4 );
    CloseeIO( GPIOB, GPIO_Pin_5 );
    CloseeIO( GPIOB, GPIO_Pin_6 );
    CloseeIO( GPIOB, GPIO_Pin_7 );
    CloseeIO( GPIOB, GPIO_Pin_8 );
    CloseeIO( GPIOB, GPIO_Pin_9 );
    CloseeIO( GPIOB, GPIO_Pin_10 );
    CloseeIO( GPIOB, GPIO_Pin_11 );
    CloseeIO( GPIOB, GPIO_Pin_12 );
    CloseeIO( GPIOB, GPIO_Pin_13 );
    CloseeIO( GPIOB, GPIO_Pin_14 );
    CloseeIO( GPIOB, GPIO_Pin_15 );

    CloseeIO( GPIOC, GPIO_Pin_2 );
    CloseeIO( GPIOC, GPIO_Pin_3 );
    CloseeIO( GPIOC, GPIO_Pin_4 );
    CloseeIO( GPIOC, GPIO_Pin_5 );
    CloseeIO( GPIOC, GPIO_Pin_6 );
    CloseeIO( GPIOC, GPIO_Pin_7 );
    CloseeIO( GPIOC, GPIO_Pin_8 );
    CloseeIO( GPIOC, GPIO_Pin_9 );
    CloseeIO( GPIOC, GPIO_Pin_10 );
    CloseeIO( GPIOC, GPIO_Pin_11 );
    CloseeIO( GPIOC, GPIO_Pin_12 );
    CloseeIO( GPIOC, GPIO_Pin_13 );
    CloseeIO( GPIOC, GPIO_Pin_14 );
    CloseeIO( GPIOC, GPIO_Pin_15 );

    CloseeIO( GPIOD, GPIO_Pin_0 );
    CloseeIO( GPIOD, GPIO_Pin_1 );
    CloseeIO( GPIOD, GPIO_Pin_2 );
    CloseeIO( GPIOD, GPIO_Pin_3 );
    CloseeIO( GPIOD, GPIO_Pin_4 );
    CloseeIO( GPIOD, GPIO_Pin_5 );
    CloseeIO( GPIOD, GPIO_Pin_6 );
    CloseeIO( GPIOD, GPIO_Pin_7 );

    CloseeIO( GPIOE, GPIO_Pin_2 );
    CloseeIO( GPIOE, GPIO_Pin_3 );
    CloseeIO( GPIOE, GPIO_Pin_4 );

    CloseeIO( GPIOF, GPIO_Pin_3 );
    CloseeIO( GPIOF, GPIO_Pin_4 );
    CloseeIO( GPIOF, GPIO_Pin_5 );
    CloseeIO( GPIOF, GPIO_Pin_6 );
    CloseeIO( GPIOF, GPIO_Pin_11 );
    CloseeIO( GPIOF, GPIO_Pin_12 );
    CloseeIO( GPIOF, GPIO_Pin_13 );
    CloseeIO( GPIOF, GPIO_Pin_14 );
    CloseeIO( GPIOF, GPIO_Pin_15 );

    CloseeIO( GPIOG, GPIO_Pin_2 );
    CloseeIO( GPIOG, GPIO_Pin_3 );
    CloseeIO( GPIOG, GPIO_Pin_6 );
    CloseeIO( GPIOG, GPIO_Pin_7 );
    /* Note that PG8 and 9 of SWD interface will not be emulated if the program changes their configuration */
    AltFunIO( GPIOG, GPIO_Pin_8, ALTFUN_NORMAL );
    AltFunIO( GPIOG, GPIO_Pin_9, ALTFUN_NORMAL );
}



void Init_IO(void)
{
    LED0_OFF;

    OutputIO( LED0_GPIO, LED0_PIN, 0 );	
}

void LED0_Flash(os_uint8_t Times)
{
    os_uint8_t i;

    for( i=0; i<Times; i++ )
    {
        LED0_ON;
        TicksDelayMs( 100, NULL );
        LED0_OFF;
        TicksDelayMs( 100, NULL );
    }
}

void Init_System(void)
{
    /*»ù´¡ÏµÍ³ÅäÖÃ*/
    __disable_irq();

    Init_SysTick();
    /* The software is delayed. Do not switch the clock to non-RCHF8M immediately after the system is powered on, and do not go to sleep immediately. Otherwise, the program may not be 
downloaded. */
    TicksDelayMs( 10, NULL );

    Init_SysClk_Gen();      /* System clock configuration */
    /* Load RCHF oscillator calibration value (automatically load 8M calibration value after chip reset) */
    RCC_Init_RCHF_Trim(clkmode);

    /* Peripheral initialization configuration */
    Init_Pad_Io();				/* IO port output register initial state configuration */
    Close_None_GPIO_80pin();
    Close_AllIO_GPIO_80pin();

    TicksDelayMs( 100, NULL );

}

/**

*******************************************************************************
****************************************
 * @brief           os_hw_systick_init:SysTick configuration.
 *
 * @param[in]       none
 *
 * @return          none

*******************************************************************************
****************************************
 */
void os_hw_systick_init(void)
{
    SysTick_Config(__SYSTEM_CLOCK / OS_TICK_PER_SECOND);

    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    NVIC_SetPriority(SysTick_IRQn, FM_IRQ_PRI_SYSTICK);
}


/**

*******************************************************************************
****************************************
 * @brief           SysTick_Handler:handler SysTick timer interrupt service
routine.
 *
 * @param[in]       none
 *
 * @return          none

*******************************************************************************
****************************************
 */
void SysTick_Handler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    //HAL_IncTick();
    os_tick_increase();

    /* leave interrupt */
    os_interrupt_leave();
}


void os_hw_board_init()
{
    /* System clock initialization */
    Init_System();

    os_hw_systick_init();

    /* Pin driver initialization is open by default */
#ifdef OS_USING_PIN
    os_hw_pin_init();
#endif

    /* Heap initialization */
#if defined(OS_USING_HEAP)
    os_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif

    /* USART driver initialization is open by default */
#ifdef OS_USING_SERIAL
    os_hw_usart_init();
#endif

    /* Set the shell console output device */
#ifdef OS_USING_CONSOLE
    os_console_set_device(OS_CONSOLE_DEVICE_NAME);
#endif

    os_board_auto_init();
}

