/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        drv_gpio.c
 *
 * @brief       This file implements gpio driver for MM32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_irq.h>
#include "drv_gpio.h"
#include <mm32_hal.h>


#ifdef OS_USING_PIN

#define MM32_PIN(index, rcc, gpio, gpio_index) { 0, RCC_##rcc##Periph_GPIO##gpio, GPIO##gpio, GPIO_Pin_##gpio_index, GPIO_PortSourceGPIO##gpio, GPIO_PinSource##gpio_index}
#define MM32_PIN_RESERVED {-1, 0, 0, 0, 0, 0}

static const struct pin_index pins[] = {
    MM32_PIN_RESERVED,
    MM32_PIN_RESERVED,
    MM32_PIN(2, APB2, C, 13),
    MM32_PIN(3, APB2, C, 14),
    MM32_PIN(4, APB2, C, 15),
    MM32_PIN_RESERVED,
    MM32_PIN_RESERVED,
    MM32_PIN_RESERVED,
    MM32_PIN_RESERVED,
    MM32_PIN_RESERVED,
    MM32_PIN(10, APB2, A, 0),
    MM32_PIN(11, APB2, A, 1),
    MM32_PIN(12, APB2, A, 2),
    MM32_PIN(13, APB2, A, 3),
    MM32_PIN(14, APB2, A, 4),
    MM32_PIN(15, APB2, A, 5),
    MM32_PIN(16, APB2, A, 6),
    MM32_PIN(17, APB2, A, 7),
    MM32_PIN(18, APB2, B, 0),
    MM32_PIN(19, APB2, B, 1),
    MM32_PIN(20, APB2, B, 2),
    MM32_PIN(21, APB2, B, 10),
    MM32_PIN(22, APB2, B, 11),
    MM32_PIN_RESERVED,
    MM32_PIN_RESERVED,
    MM32_PIN(25, APB2, B, 12),
    MM32_PIN(26, APB2, B, 13),
    MM32_PIN(27, APB2, B, 14),
    MM32_PIN(28, APB2, B, 15),
    MM32_PIN(29, APB2, A, 8),
    MM32_PIN(30, APB2, A, 9),
    MM32_PIN(31, APB2, A, 10),
    MM32_PIN(32, APB2, A, 11),
    MM32_PIN(33, APB2, A, 12),
    MM32_PIN(34, APB2, A, 13),
    MM32_PIN_RESERVED,
    MM32_PIN_RESERVED,
    MM32_PIN(37, APB2, A, 14),
    MM32_PIN(38, APB2, A, 15),
    MM32_PIN(39, APB2, B, 3),
    MM32_PIN(40, APB2, B, 4),
    MM32_PIN(41, APB2, B, 5),
    MM32_PIN(42, APB2, B, 6),
    MM32_PIN(43, APB2, B, 7),
    MM32_PIN_RESERVED,
    MM32_PIN(45, APB2, B, 8),
    MM32_PIN(46, APB2, B, 9),
    MM32_PIN_RESERVED,
    MM32_PIN_RESERVED,

};

static const struct pin_irq_map pin_irq_map[] = {
    {GPIO_Pin_0,  EXTI_Line0,  EXTI0_IRQn    },
    {GPIO_Pin_1,  EXTI_Line1,  EXTI1_IRQn    },
    {GPIO_Pin_2,  EXTI_Line2,  EXTI2_IRQn    },
    {GPIO_Pin_3,  EXTI_Line3,  EXTI3_IRQn    },
    {GPIO_Pin_4,  EXTI_Line4,  EXTI4_IRQn    },
    {GPIO_Pin_5,  EXTI_Line5,  EXTI9_5_IRQn  },
    {GPIO_Pin_6,  EXTI_Line6,  EXTI9_5_IRQn  },
    {GPIO_Pin_7,  EXTI_Line7,  EXTI9_5_IRQn  },
    {GPIO_Pin_8,  EXTI_Line8,  EXTI9_5_IRQn  },
    {GPIO_Pin_9,  EXTI_Line9,  EXTI9_5_IRQn  },
    {GPIO_Pin_10, EXTI_Line10, EXTI15_10_IRQn},
    {GPIO_Pin_11, EXTI_Line11, EXTI15_10_IRQn},
    {GPIO_Pin_12, EXTI_Line12, EXTI15_10_IRQn},
    {GPIO_Pin_13, EXTI_Line13, EXTI15_10_IRQn},
    {GPIO_Pin_14, EXTI_Line14, EXTI15_10_IRQn},
    {GPIO_Pin_15, EXTI_Line15, EXTI15_10_IRQn},

};

static struct os_pin_irq_hdr pin_irq_hdr_tab[] = {
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
};
//static os_uint32_t pin_irq_enable_mask = 0;

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

//static struct pin_pull_state pin_pulls[ITEM_NUM(pins)] = {0};

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

static void MM32_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }

    GPIO_WriteBit(index->gpio, index->pin, (BitAction)value);
}

static int MM32_pin_read(struct os_device *dev, os_base_t pin)
{
    int                     value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return value;
    }

    if (GPIO_ReadInputDataBit(index->gpio, index->pin) == Bit_RESET)
    {
        value = PIN_LOW;
    }
    else
    {
        value = PIN_HIGH;
    }  

    return value;
}

static void MM32_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    const struct pin_index *index;
    GPIO_InitTypeDef        GPIO_InitStruct;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }
    /* GPIO Periph clock enable */
    RCC_APB2PeriphClockCmd(index->rcc, ENABLE);
    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.GPIO_Pin   = index->pin;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;   
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;


    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; 
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
    }
    else
    {
        /* input setting:default. */
        GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IPD;
    }
   
    GPIO_Init(index->gpio, &GPIO_InitStruct);
}

OS_INLINE os_int32_t bit2bitno(os_uint32_t bit)
{
    int i;
    for (i = 0; i < 32; i++)
    {
        if ((0x01 << i) == bit)
        {
            return i;
        }
    }
    return -1;
}

OS_INLINE const struct pin_irq_map *get_pin_irq_map(os_uint32_t pinbit)
{
    os_int32_t mapindex = bit2bitno(pinbit);
    if (mapindex < 0 || mapindex >= ITEM_NUM(pin_irq_map))
    {
        return OS_NULL;
    }
    return &pin_irq_map[mapindex];
};

static os_err_t
MM32_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    const struct pin_index *index;
    os_base_t               level;
    os_int32_t              irqindex = -1;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }
    irqindex = bit2bitno(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
    {
        return OS_ENOSYS;
    }

    level = os_hw_interrupt_disable();
    if (pin_irq_hdr_tab[irqindex].pin == pin       &&
            pin_irq_hdr_tab[irqindex].hdr == hdr   &&
            pin_irq_hdr_tab[irqindex].mode == mode &&
            pin_irq_hdr_tab[irqindex].args == args)
    {
        os_hw_interrupt_enable(level);
        return OS_EOK;
    }
    if (pin_irq_hdr_tab[irqindex].pin != -1)
    {
        os_hw_interrupt_enable(level);
        return OS_EBUSY;
    }
    pin_irq_hdr_tab[irqindex].pin  = pin;
    pin_irq_hdr_tab[irqindex].hdr  = hdr;
    pin_irq_hdr_tab[irqindex].mode = mode;
    pin_irq_hdr_tab[irqindex].args = args;
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t MM32_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    const struct pin_index *index;
    os_base_t               level;
    os_int32_t              irqindex = -1;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }
    irqindex = bit2bitno(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
    {
        return OS_ENOSYS;
    }

    level = os_hw_interrupt_disable();
    if (pin_irq_hdr_tab[irqindex].pin == -1)
    {
        os_hw_interrupt_enable(level);
        return OS_EOK;
    }
    pin_irq_hdr_tab[irqindex].pin  = -1;
    pin_irq_hdr_tab[irqindex].hdr  = OS_NULL;
    pin_irq_hdr_tab[irqindex].mode = 0;
    pin_irq_hdr_tab[irqindex].args = OS_NULL;
    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t MM32_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    const struct pin_index   *index;
    const struct pin_irq_map *irqmap;
    os_base_t                 level;
    os_int32_t                irqindex = -1;

    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {
        irqindex = bit2bitno(index->pin);
        if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
        {
            return OS_ENOSYS;
        }

        level = os_hw_interrupt_disable();

        if (pin_irq_hdr_tab[irqindex].pin == -1)
        {
            os_hw_interrupt_enable(level);
            return OS_ENOSYS;
        }

        irqmap = &pin_irq_map[irqindex];

        /* GPIO Periph clock enable */
        RCC_APB2PeriphClockCmd(index->rcc, ENABLE);

        /* Configure GPIO_InitStructure */
        GPIO_InitStructure.GPIO_Pin     = index->pin;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(index->gpio, &GPIO_InitStructure);

        NVIC_InitStructure.NVIC_IRQChannel = irqmap->irqno;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        GPIO_EXTILineConfig(index->port_source, index->pin_source);
        EXTI_InitStructure.EXTI_Line = irqmap->irqbit;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

        switch (pin_irq_hdr_tab[irqindex].mode)
        {
        case PIN_IRQ_MODE_RISING:
           	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
            break;
        case PIN_IRQ_MODE_FALLING:
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
            break;
        case PIN_IRQ_MODE_RISING_FALLING:
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
            break;
        case PIN_IRQ_MODE_HIGH_LEVEL:
        case PIN_IRQ_MODE_LOW_LEVEL:
        default:
            os_hw_interrupt_enable(level);
            return OS_ERROR;
        }

        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        os_hw_interrupt_enable(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        irqmap = get_pin_irq_map(index->pin);
        if (irqmap == OS_NULL)
        {
            return OS_ENOSYS;
        }

        EXTI_InitStructure.EXTI_Line = irqmap->irqbit;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;
        EXTI_Init(&EXTI_InitStructure);
    }
    else
    {
        return OS_ENOSYS;
    }

    return OS_EOK;
}
const static struct os_pin_ops _MM32_pin_ops = {
    MM32_pin_mode,
    MM32_pin_write,
    MM32_pin_read,
    MM32_pin_attach_irq,
    MM32_pin_dettach_irq,
    MM32_pin_irq_enable,
};

OS_INLINE void pin_irq_hdr(int irqno)
{
    EXTI_ClearITPendingBit(pin_irq_map[irqno].irqbit);

    if (pin_irq_hdr_tab[irqno].hdr)
    {
        pin_irq_hdr_tab[irqno].hdr(pin_irq_hdr_tab[irqno].args);
    }
}


void EXTI0_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();	
    pin_irq_hdr(0);
    /* leave interrupt */
    os_interrupt_enter();
}
void EXTI1_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();
    pin_irq_hdr(1);
    /* leave interrupt */
    os_interrupt_enter();
}
void EXTI2_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();
    pin_irq_hdr(2);
    /* leave interrupt */
    os_interrupt_enter();
}
void EXTI3_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();
    pin_irq_hdr(3);
    /* leave interrupt */
    os_interrupt_enter();
}
void EXTI4_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();
    pin_irq_hdr(4);
    /* leave interrupt */
    os_interrupt_enter();
}
void EXTI9_5_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        pin_irq_hdr(5);
    }
    if (EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        pin_irq_hdr(6);
    }
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        pin_irq_hdr(7);
    }
    if (EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        pin_irq_hdr(8);
    }
    if (EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        pin_irq_hdr(9);
    }
    /* leave interrupt */
    os_interrupt_enter();
}
void EXTI15_10_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
    {
        pin_irq_hdr(10);
    }
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
        pin_irq_hdr(11);
    }
    if (EXTI_GetITStatus(EXTI_Line12) != RESET)
    {
        pin_irq_hdr(12);
    }
    if (EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
        pin_irq_hdr(13);
    }
    if (EXTI_GetITStatus(EXTI_Line14) != RESET)
    {
        pin_irq_hdr(14);
    }
    if (EXTI_GetITStatus(EXTI_Line15) != RESET)
    {
        pin_irq_hdr(15);
    }
    /* leave interrupt */
    os_interrupt_enter();
}


/**
 ***********************************************************************************************************************
 * @brief           os_hw_pin_init:enable gpio clk,register pin device.
 *
 * @param[in]       none
 *
 * @return          Return init result.
 * @retval          OS_EOK       init success.
 * @retval          Others       init failed.
 ***********************************************************************************************************************
 */
int os_hw_pin_init(void)
{
    /*enable clock*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE); 

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    /*disable jtag only swd mode can be used*/
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);		

    return os_device_pin_register(0, &_MM32_pin_ops, OS_NULL);
}

#endif /* OS_USING_PIN */
