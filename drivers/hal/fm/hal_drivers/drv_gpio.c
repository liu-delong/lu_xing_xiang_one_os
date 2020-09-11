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
 * \@file        drv_gpio.c
 *
 * \@brief       This file implements gpio driver for FM33A0xx.
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <os_hw.h>
#include <os_device.h>
#include <os_irq.h>
#include <os_memory.h>
#include "define_all.h"
#include "drv_gpio.h"
#include "drv_common.h"

#ifdef OS_USING_PIN

static const struct pin_index pins[] =
{
#if defined(GPIOA)
    __FM_PIN(0, A, 0),
    __FM_PIN(1, A, 1),
    __FM_PIN(2, A, 2),
    __FM_PIN(3, A, 3),
    __FM_PIN(4, A, 4),
    __FM_PIN(5, A, 5),
    __FM_PIN(6, A, 6),
    __FM_PIN(7, A, 7),
    __FM_PIN(8, A, 8),
    __FM_PIN(9, A, 9),
    __FM_PIN(10, A, 10),
    __FM_PIN(11, A, 11),
    __FM_PIN(12, A, 12),
    __FM_PIN(13, A, 13),
    __FM_PIN(14, A, 14),
    __FM_PIN(15, A, 15),
#if defined(GPIOB)
    __FM_PIN(16, B, 0),
    __FM_PIN(17, B, 1),
    __FM_PIN(18, B, 2),
    __FM_PIN(19, B, 3),
    __FM_PIN(20, B, 4),
    __FM_PIN(21, B, 5),
    __FM_PIN(22, B, 6),
    __FM_PIN(23, B, 7),
    __FM_PIN(24, B, 8),
    __FM_PIN(25, B, 9),
    __FM_PIN(26, B, 10),
    __FM_PIN(27, B, 11),
    __FM_PIN(28, B, 12),
    __FM_PIN(29, B, 13),
    __FM_PIN(30, B, 14),
    __FM_PIN(31, B, 15),
#if defined(GPIOC)
    __FM_PIN(32, C, 0),
    __FM_PIN(33, C, 1),
    __FM_PIN(34, C, 2),
    __FM_PIN(35, C, 3),
    __FM_PIN(36, C, 4),
    __FM_PIN(37, C, 5),
    __FM_PIN(38, C, 6),
    __FM_PIN(39, C, 7),
    __FM_PIN(40, C, 8),
    __FM_PIN(41, C, 9),
    __FM_PIN(42, C, 10),
    __FM_PIN(43, C, 11),
    __FM_PIN(44, C, 12),
    __FM_PIN(45, C, 13),
    __FM_PIN(46, C, 14),
    __FM_PIN(47, C, 15),
#if defined(GPIOD)
    __FM_PIN(48, D, 0),
    __FM_PIN(49, D, 1),
    __FM_PIN(50, D, 2),
    __FM_PIN(51, D, 3),
    __FM_PIN(52, D, 4),
    __FM_PIN(53, D, 5),
    __FM_PIN(54, D, 6),
    __FM_PIN(55, D, 7),
    __FM_PIN(56, D, 8),
    __FM_PIN(57, D, 9),
    __FM_PIN(58, D, 10),
    __FM_PIN(59, D, 11),
    __FM_PIN(60, D, 12),
    __FM_PIN(61, D, 13),
    __FM_PIN(62, D, 14),
    __FM_PIN(63, D, 15),
#if defined(GPIOE)
    __FM_PIN(64, E, 0),
    __FM_PIN(65, E, 1),
    __FM_PIN(66, E, 2),
    __FM_PIN(67, E, 3),
    __FM_PIN(68, E, 4),
    __FM_PIN(69, E, 5),
    __FM_PIN(70, E, 6),
    __FM_PIN(71, E, 7),
    __FM_PIN(72, E, 8),
    __FM_PIN(73, E, 9),
    __FM_PIN(74, E, 10),
    __FM_PIN(75, E, 11),
    __FM_PIN(76, E, 12),
    __FM_PIN(77, E, 13),
    __FM_PIN(78, E, 14),
    __FM_PIN(79, E, 15),
#if defined(GPIOF)
    __FM_PIN(80, F, 0),
    __FM_PIN(81, F, 1),
    __FM_PIN(82, F, 2),
    __FM_PIN(83, F, 3),
    __FM_PIN(84, F, 4),
    __FM_PIN(85, F, 5),
    __FM_PIN(86, F, 6),
    __FM_PIN(87, F, 7),
    __FM_PIN(88, F, 8),
    __FM_PIN(89, F, 9),
    __FM_PIN(90, F, 10),
    __FM_PIN(91, F, 11),
    __FM_PIN(92, F, 12),
    __FM_PIN(93, F, 13),
    __FM_PIN(94, F, 14),
    __FM_PIN(95, F, 15),
#endif /* defined(GPIOF) */
#endif /* defined(GPIOE) */
#endif /* defined(GPIOD) */
#endif /* defined(GPIOC) */
#endif /* defined(GPIOB) */
#endif /* defined(GPIOA) */
};

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

static const struct pin_index *get_pin(os_uint8_t pin)
{
    const struct pin_index *index;

    if (pin < ITEM_NUM(pins))
    {
        index = &pins[pin];
        if (index->index == -1)
            index = OS_NULL;
    }
    else
    {
        index = OS_NULL;
    }

    return index;
};

typedef enum
{
    FM_IQR_NO0 = 0,
    FM_IQR_NO1,
    FM_IQR_NO2,
    FM_IQR_NULL,
} FM_IQR_NO_E;

#define FM33A0XX_EXTI_NUM   3
#define FM33A0XX_PEREXTI_NUM   8

#define FM33A0XX_IRQ_NUMBERS  (FM33A0XX_EXTI_NUM * FM33A0XX_PEREXTI_NUM)
struct os_pin_irq_hdr fm_pin_irq_hdr_tab[FM33A0XX_IRQ_NUMBERS];

void fm_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    const struct pin_index *index;
    GPIO_InitTypeDef  GPIO_InitStructure;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }

    GPIO_InitStructure.Pin = index->pin;

    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        GPIO_InitStructure.PxINEN = GPIO_IN_Dis;
        GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        GPIO_InitStructure.PxFCR = GPIO_FCR_OUT;
        GPIO_InitStructure.PxODEN = GPIO_OD_Dis;
    }
    else  if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting */
        GPIO_InitStructure.PxINEN = GPIO_IN_Dis;
        GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        GPIO_InitStructure.PxFCR = GPIO_FCR_OUT;
        GPIO_InitStructure.PxODEN = GPIO_OD_En;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        GPIO_InitStructure.PxFCR = GPIO_FCR_IN;
        GPIO_InitStructure.PxPUEN = GPIO_PU_Dis;
        GPIO_InitStructure.PxINEN = GPIO_IN_En;
        GPIO_InitStructure.PxODEN = GPIO_OD_Dis;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        GPIO_InitStructure.PxFCR = GPIO_FCR_IN;
        GPIO_InitStructure.PxPUEN = GPIO_PU_En;
        GPIO_InitStructure.PxINEN = GPIO_IN_En;
        GPIO_InitStructure.PxODEN = GPIO_OD_Dis;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting:default. */
    }

    GPIO_Init(index->gpio, &GPIO_InitStructure);

}

void fm_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }

    if (value == PIN_LOW)
    {
        GPIO_SetBits(index->gpio, index->pin);
    }
    else if (value == PIN_HIGH)
    {
        GPIO_ResetBits(index->gpio, index->pin);
    }
}

os_int32_t fm_pin_read(struct os_device *dev, os_base_t pin)
{
    os_int32_t value = PIN_LOW;
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return value;
    }

    value = GPIO_ReadInputDataBit(index->gpio, index->pin);

    return value;
}

os_int32_t fm_irqno_get(GPIOx_Type* GPIOx, os_uint32_t GPIO_Pin)
{
    os_uint32_t pin_num = 0,i;
    os_int32_t Result = -1;

    for(i= 0; i<16; i++)
    {
        if(GPIO_Pin&(1<<i))
        {
            pin_num = i;
            break;
        }
    }

    switch((os_uint32_t)GPIOx)
    {
    case (os_uint32_t)GPIOA:
    case (os_uint32_t)GPIOB:
        Result = FM_IQR_NO0 * 8 + (pin_num&0x07);
        break;

    case (os_uint32_t)GPIOC:
        Result = FM_IQR_NO1 * 8 + (pin_num&0x07);
        break;

    case (os_uint32_t)GPIOD:/* 0~10 */
        if(pin_num <= 10)
        {
            Result = FM_IQR_NO1 * 8 + (pin_num&0x07);
        }
        break;

    case (os_uint32_t)GPIOE:/* 2~9(-2)*/
        if((pin_num >= 2)&&(pin_num <= 9))
        {
            Result = FM_IQR_NO2 * 8 + (pin_num - 2);
        }
        break;

    case (os_uint32_t)GPIOF:/* 1~7   8~11(-4) */
        if((pin_num >= 1)&&(pin_num <= 7))
        {
            Result = FM_IQR_NO2 * 8 + (pin_num&0x07);
        }
        else if((pin_num >= 8)&&(pin_num <= 11))
        {
            Result = FM_IQR_NO2 * 8 + (pin_num - 4);
        }
        break;

    case (os_uint32_t)GPIOG:/* 2(+1) 5~8(-1) */
        if(pin_num == 2)
        {
            Result = FM_IQR_NO1 * 8 + (pin_num + 1);
        }
        else if((pin_num >= 5)&&(pin_num <= 8))
        {
            Result = FM_IQR_NO1 * 8 + (pin_num - 1);
        }
        break;

    default:
        break;
    }

    return Result;
}

os_err_t fm_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    os_base_t  level;
    os_int32_t irqindex = -1;
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_EEMPTY;
    }

    irqindex = fm_irqno_get(index->gpio, index->pin);

    level = os_hw_interrupt_disable();
    if(fm_pin_irq_hdr_tab[irqindex].pin == pin   &&
            fm_pin_irq_hdr_tab[irqindex].hdr == hdr   &&
            fm_pin_irq_hdr_tab[irqindex].mode == mode &&
            fm_pin_irq_hdr_tab[irqindex].args == args)
    {
        os_hw_interrupt_enable(level);
        return OS_EOK;
    }
    if (fm_pin_irq_hdr_tab[irqindex].pin != -1)
    {
        os_hw_interrupt_enable(level);
        return OS_EBUSY;
    }
    fm_pin_irq_hdr_tab[irqindex].pin  = pin;
    fm_pin_irq_hdr_tab[irqindex].hdr  = hdr;
    fm_pin_irq_hdr_tab[irqindex].mode = mode;
    fm_pin_irq_hdr_tab[irqindex].args = args;

    os_hw_interrupt_enable(level);

    return OS_EOK;
}

os_err_t fm_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    os_base_t  level;
    os_int32_t irqindex = -1;
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_EEMPTY;
    }

    irqindex = fm_irqno_get(index->gpio, index->pin);

    level = os_hw_interrupt_disable();
    if (fm_pin_irq_hdr_tab[irqindex].pin == -1)
    {
        os_hw_interrupt_enable(level);
        return OS_EOK;
    }
    fm_pin_irq_hdr_tab[irqindex].pin  = -1;
    fm_pin_irq_hdr_tab[irqindex].hdr  = OS_NULL;
    fm_pin_irq_hdr_tab[irqindex].mode = 0;
    fm_pin_irq_hdr_tab[irqindex].args = OS_NULL;
    os_hw_interrupt_enable(level);

    return OS_EOK;
}


os_err_t fm_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    os_base_t                 level;
    os_int32_t irqindex;
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_EEMPTY;
    }

    irqindex = fm_irqno_get(index->gpio, index->pin);

    if (enabled == PIN_IRQ_ENABLE)
    {
        level = os_hw_interrupt_disable();

        /* Configure the GPIO/button interrupt polarity */
        if (fm_pin_irq_hdr_tab[irqindex].mode == PIN_IRQ_MODE_RISING)
        {
            GPIO_EXTI_Init(index->gpio, index->pin, EXTI_RISING);
        }
        else if (fm_pin_irq_hdr_tab[irqindex].mode == PIN_IRQ_MODE_FALLING)
        {
            GPIO_EXTI_Init(index->gpio, index->pin, EXTI_FALLING);
        }
        else if (fm_pin_irq_hdr_tab[irqindex].mode == PIN_IRQ_MODE_RISING_FALLING)
        {
            GPIO_EXTI_Init(index->gpio, index->pin, EXTI_BOTH);
        }

        NVIC_DisableIRQ(GPIO_IRQn);
        NVIC_SetPriority(GPIO_IRQn, FM_IRQ_PRI_GPIO);
        NVIC_EnableIRQ(GPIO_IRQn);

        os_hw_interrupt_enable(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        /* Disable the GPIO/button interrupt */
        GPIO_EXTI_Init(index->gpio, index->pin, EXTI_DISABLE);
    }
    else
    {
        return OS_ENOSYS;
    }

    return OS_EOK;
}


OS_INLINE void fm_pin_irq_hdr(os_int32_t irqno)
{
    if (fm_pin_irq_hdr_tab[irqno].hdr)
    {
        fm_pin_irq_hdr_tab[irqno].hdr(fm_pin_irq_hdr_tab[irqno].args);
    }
}

void HAL_GPIO_EXTI_Callback(os_uint16_t pin)
{
    os_int32_t irqindex;
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }

    irqindex = fm_irqno_get(index->gpio, index->pin);
    if (irqindex != -1)
    {
        fm_pin_irq_hdr(irqindex);
    }
}


void HAL_GPIO_EXTI_IRQHandler(os_uint16_t Pin)
{
    const struct pin_index *index;

    index = get_pin(Pin);
    if (index == OS_NULL)
    {
        return;
    }

    /* EXTI line interrupt detected */
    if(SET == GPIO_EXTI_EXTIxIF_ChkEx(index->gpio, index->pin))
    {
        GPIO_EXTI_EXTIxIF_ClrEx(index->gpio, index->pin);
        HAL_GPIO_EXTI_Callback(Pin);
    }
}

void GPIO_IRQHandler(void)
{
    os_int32_t i;

    os_interrupt_enter();
    for (i = 0; i < FM33A0XX_IRQ_NUMBERS; i++)
    {
        if (fm_pin_irq_hdr_tab[i].pin != -1)
        {
            HAL_GPIO_EXTI_IRQHandler(fm_pin_irq_hdr_tab[i].pin);
        }
    }

    os_interrupt_leave();

}

const static struct os_pin_ops am_pin_ops =
{
    fm_pin_mode,
    fm_pin_write,
    fm_pin_read,
    fm_pin_attach_irq,
    fm_pin_dettach_irq,
    fm_pin_irq_enable,
};




os_int32_t os_hw_pin_init(void)
{
    os_int32_t i = 0;

    for (i = 0; i < FM33A0XX_IRQ_NUMBERS; i++)
        fm_pin_irq_hdr_tab[i].pin = -1;

    RCC_PERCLK_SetableEx(PDCCLK, ENABLE);

    os_device_pin_register(0, &am_pin_ops, OS_NULL);

    os_kprintf("pin_init!\n");

    return 0;
}

/* INIT_BOARD_EXPORT(os_hw_pin_init); */
#endif
