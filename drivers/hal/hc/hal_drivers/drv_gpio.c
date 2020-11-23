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
 * @brief       This file implements gpio driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_irq.h>
#include "drv_gpio.h"

#ifdef OS_USING_PIN

static struct pin_index pins[] = {
#if defined(GPIOPORTA)
    __INDEX_PIN(0, A, 0),    __INDEX_PIN(1, A, 1),    __INDEX_PIN(2, A, 2),    __INDEX_PIN(3, A, 3),
    __INDEX_PIN(4, A, 4),    __INDEX_PIN(5, A, 5),    __INDEX_PIN(6, A, 6),    __INDEX_PIN(7, A, 7),
    __INDEX_PIN(8, A, 8),    __INDEX_PIN(9, A, 9),    __INDEX_PIN(10, A, 10),  __INDEX_PIN(11, A, 11),
    __INDEX_PIN(12, A, 12),  __INDEX_PIN(13, A, 13),  __INDEX_PIN(14, A, 14),  __INDEX_PIN(15, A, 15),
#if defined(GPIOPORTB)
    __INDEX_PIN(16, B, 0),   __INDEX_PIN(17, B, 1),   __INDEX_PIN(18, B, 2),   __INDEX_PIN(19, B, 3),
    __INDEX_PIN(20, B, 4),   __INDEX_PIN(21, B, 5),   __INDEX_PIN(22, B, 6),   __INDEX_PIN(23, B, 7),
    __INDEX_PIN(24, B, 8),   __INDEX_PIN(25, B, 9),   __INDEX_PIN(26, B, 10),  __INDEX_PIN(27, B, 11),
    __INDEX_PIN(28, B, 12),  __INDEX_PIN(29, B, 13),  __INDEX_PIN(30, B, 14),  __INDEX_PIN(31, B, 15),
#if defined(GPIOPORTC)
    __INDEX_PIN(32, C, 0),   __INDEX_PIN(33, C, 1),   __INDEX_PIN(34, C, 2),   __INDEX_PIN(35, C, 3),
    __INDEX_PIN(36, C, 4),   __INDEX_PIN(37, C, 5),   __INDEX_PIN(38, C, 6),   __INDEX_PIN(39, C, 7),
    __INDEX_PIN(40, C, 8),   __INDEX_PIN(41, C, 9),   __INDEX_PIN(42, C, 10),  __INDEX_PIN(43, C, 11),
    __INDEX_PIN(44, C, 12),  __INDEX_PIN(45, C, 13),  __INDEX_PIN(46, C, 14),  __INDEX_PIN(47, C, 15),
#if defined(GPIOPORTD)
    __INDEX_PIN(48, D, 0),   __INDEX_PIN(49, D, 1),   __INDEX_PIN(50, D, 2),   __INDEX_PIN(51, D, 3),
    __INDEX_PIN(52, D, 4),   __INDEX_PIN(53, D, 5),   __INDEX_PIN(54, D, 6),   __INDEX_PIN(55, D, 7),
    __INDEX_PIN(56, D, 8),   __INDEX_PIN(57, D, 9),   __INDEX_PIN(58, D, 10),  __INDEX_PIN(59, D, 11),
    __INDEX_PIN(60, D, 12),  __INDEX_PIN(61, D, 13),  __INDEX_PIN(62, D, 14),  __INDEX_PIN(63, D, 15),
#if defined(GPIOPORTE)
    __INDEX_PIN(64, E, 0),   __INDEX_PIN(65, E, 1),   __INDEX_PIN(66, E, 2),   __INDEX_PIN(67, E, 3),
    __INDEX_PIN(68, E, 4),   __INDEX_PIN(69, E, 5),   __INDEX_PIN(70, E, 6),   __INDEX_PIN(71, E, 7),
    __INDEX_PIN(72, E, 8),   __INDEX_PIN(73, E, 9),   __INDEX_PIN(74, E, 10),  __INDEX_PIN(75, E, 11),
    __INDEX_PIN(76, E, 12),  __INDEX_PIN(77, E, 13),  __INDEX_PIN(78, E, 14),  __INDEX_PIN(79, E, 15),
#if defined(GPIOPORTF)
    __INDEX_PIN(80, F, 0),   __INDEX_PIN(81, F, 1),   __INDEX_PIN(82, F, 2),   __INDEX_PIN(83, F, 3),
    __INDEX_PIN(84, F, 4),   __INDEX_PIN(85, F, 5),   __INDEX_PIN(86, F, 6),   __INDEX_PIN(87, F, 7),
    __INDEX_PIN(88, F, 8),   __INDEX_PIN(89, F, 9),   __INDEX_PIN(90, F, 10),  __INDEX_PIN(91, F, 11),
    __INDEX_PIN(92, F, 12),  __INDEX_PIN(93, F, 13),  __INDEX_PIN(94, F, 14),  __INDEX_PIN(95, F, 15),
#if defined(GPIOPORTG)
    __INDEX_PIN(96, G, 0),   __INDEX_PIN(97, G, 1),   __INDEX_PIN(98, G, 2),   __INDEX_PIN(99, G, 3),
    __INDEX_PIN(100, G, 4),  __INDEX_PIN(101, G, 5),  __INDEX_PIN(102, G, 6),  __INDEX_PIN(103, G, 7),
    __INDEX_PIN(104, G, 8),  __INDEX_PIN(105, G, 9),  __INDEX_PIN(106, G, 10), __INDEX_PIN(107, G, 11),
    __INDEX_PIN(108, G, 12), __INDEX_PIN(109, G, 13), __INDEX_PIN(110, G, 14), __INDEX_PIN(111, G, 15),
#if defined(GPIOPORTH)
    __INDEX_PIN(112, H, 0),  __INDEX_PIN(113, H, 1),  __INDEX_PIN(114, H, 2),  __INDEX_PIN(115, H, 3),
    __INDEX_PIN(116, H, 4),  __INDEX_PIN(117, H, 5),  __INDEX_PIN(118, H, 6),  __INDEX_PIN(119, H, 7),
    __INDEX_PIN(120, H, 8),  __INDEX_PIN(121, H, 9),  __INDEX_PIN(122, H, 10), __INDEX_PIN(123, H, 11),
    __INDEX_PIN(124, H, 12), __INDEX_PIN(125, H, 13), __INDEX_PIN(126, H, 14), __INDEX_PIN(127, H, 15),
#if defined(GPIOPORTI)
    __INDEX_PIN(128, I, 0),  __INDEX_PIN(129, I, 1),  __INDEX_PIN(130, I, 2),  __INDEX_PIN(131, I, 3),
    __INDEX_PIN(132, I, 4),  __INDEX_PIN(133, I, 5),  __INDEX_PIN(134, I, 6),  __INDEX_PIN(135, I, 7),
    __INDEX_PIN(136, I, 8),  __INDEX_PIN(137, I, 9),  __INDEX_PIN(138, I, 10), __INDEX_PIN(139, I, 11),
    __INDEX_PIN(140, I, 12), __INDEX_PIN(141, I, 13), __INDEX_PIN(142, I, 14), __INDEX_PIN(143, I, 15),
#if defined(GPIOPORTJ)
    __INDEX_PIN(144, J, 0),  __INDEX_PIN(145, J, 1),  __INDEX_PIN(146, J, 2),  __INDEX_PIN(147, J, 3),
    __INDEX_PIN(148, J, 4),  __INDEX_PIN(149, J, 5),  __INDEX_PIN(150, J, 6),  __INDEX_PIN(151, J, 7),
    __INDEX_PIN(152, J, 8),  __INDEX_PIN(153, J, 9),  __INDEX_PIN(154, J, 10), __INDEX_PIN(155, J, 11),
    __INDEX_PIN(156, J, 12), __INDEX_PIN(157, J, 13), __INDEX_PIN(158, J, 14), __INDEX_PIN(159, J, 15),
#if defined(GPIOPORTK)
    __INDEX_PIN(160, K, 0),  __INDEX_PIN(161, K, 1),  __INDEX_PIN(162, K, 2),  __INDEX_PIN(163, K, 3),
    __INDEX_PIN(164, K, 4),  __INDEX_PIN(165, K, 5),  __INDEX_PIN(166, K, 6),  __INDEX_PIN(167, K, 7),
    __INDEX_PIN(168, K, 8),  __INDEX_PIN(169, K, 9),  __INDEX_PIN(170, K, 10), __INDEX_PIN(171, K, 11),
    __INDEX_PIN(172, K, 12), __INDEX_PIN(173, K, 13), __INDEX_PIN(174, K, 14), __INDEX_PIN(175, K, 15),
#endif /* defined(GPIOK) */
#endif /* defined(GPIOJ) */
#endif /* defined(GPIOI) */
#endif /* defined(GPIOH) */
#endif /* defined(GPIOG) */
#endif /* defined(GPIOF) */
#endif /* defined(GPIOE) */
#endif /* defined(GPIOD) */
#endif /* defined(GPIOC) */
#endif /* defined(GPIOB) */
#endif /* defined(GPIOA) */
};

struct HC32_IRQ_STAT hc32_irq_stat[] = {

	{PORTA_IRQn,0},
	{PORTB_IRQn,0},
	{PORTC_E_IRQn,0},
	{PORTD_F_IRQn,0},
	
};

struct HC32_IRQ_STAT *pState[] = {

#if defined(GPIOPORTA)
	&hc32_irq_stat[0],
#if defined(GPIOPORTB)
	&hc32_irq_stat[1],
#if defined(GPIOPORTC)
	&hc32_irq_stat[2],
#if defined(GPIOPORTD)
	&hc32_irq_stat[3],
#if defined(GPIOPORTE)
	&hc32_irq_stat[2],
#if defined(GPIOPORTF)
	&hc32_irq_stat[3],
#endif	
#endif	
#endif	
#endif	
#endif
#endif

};

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

//static struct pin_pull_state pin_pulls[ITEM_NUM(pins)] = {0};

static struct pin_index *get_pin(os_uint8_t pin)
{
    struct pin_index *index;

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

en_gpio_port_t PIN_BASE(os_base_t pin)
{
	struct pin_index *index;

	index = get_pin(pin);

	OS_ASSERT(index != OS_NULL);

	return index->port;
}

en_gpio_pin_t PIN_OFFSET(os_base_t pin)
{
	struct pin_index *index;

	index = get_pin(pin);

	OS_ASSERT(index != OS_NULL);

	return index->pin;
}


static void hc32_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }

    Gpio_WriteOutputIO(index->port, (en_gpio_pin_t)index->pin, (boolean_t)value);
}

static int hc32_pin_read(struct os_device *dev, os_base_t pin)
{
    int                     value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return value;
    }

	value = Gpio_GetInputIO(index->port, index->pin);

    return value;
}

static void hc32_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    const struct pin_index *index;
	stc_gpio_cfg_t GPIO_Cfg;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }

    /* Configure GPIO_InitStructure */
	GPIO_Cfg.enCtrlMode = GpioAHB;
	GPIO_Cfg.enDrv = GpioDrvH;

	switch(mode){
		case PIN_MODE_OUTPUT:
			/* output setting */
			GPIO_Cfg.enDir = GpioDirOut;
			GPIO_Cfg.enPd = GpioPdDisable;
			GPIO_Cfg.enPu = GpioPuDisable;
			break;
		case PIN_MODE_INPUT:
			/* input setting: not pull. */
			GPIO_Cfg.enDir = GpioDirIn;
			GPIO_Cfg.enPd = GpioPdDisable;
			GPIO_Cfg.enPu = GpioPuDisable;
			break;
		
		case PIN_MODE_INPUT_PULLUP:
			/* input setting: pull up. */	
			GPIO_Cfg.enDir = GpioDirIn;
			GPIO_Cfg.enPd = GpioPdDisable;
			GPIO_Cfg.enPu = GpioPuEnable;
				
			break;
		case PIN_MODE_INPUT_PULLDOWN:
			
			/* input setting: pull down. */ 
			GPIO_Cfg.enDir = GpioDirIn;
			GPIO_Cfg.enPd = GpioPdEnable;
			GPIO_Cfg.enPu = GpioPuDisable;	  
			break;
		case PIN_MODE_OUTPUT_OD:
			/* output setting: od. */
			GPIO_Cfg.enDir = GpioDirOut;
			GPIO_Cfg.enPd = GpioPdDisable;
			GPIO_Cfg.enPu = GpioPuDisable;
			GPIO_Cfg.enOD = GpioOdEnable;
				
			break;

		default:
			
			break;
	}
#if 0
    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
		GPIO_Cfg.enDir = GpioDirOut;
		GPIO_Cfg.enPd = GpioPdDisable;
		GPIO_Cfg.enPu = GpioPuDisable;
		
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
		GPIO_Cfg.enDir = GpioDirIn;
		GPIO_Cfg.enPd = GpioPdDisable;
		GPIO_Cfg.enPu = GpioPuDisable;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */	
		GPIO_Cfg.enDir = GpioDirIn;
		GPIO_Cfg.enPd = GpioPdDisable;
		GPIO_Cfg.enPu = GpioPuEnable;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */	
		GPIO_Cfg.enDir = GpioDirIn;
		GPIO_Cfg.enPd = GpioPdEnable;
		GPIO_Cfg.enPu = GpioPuDisable;    
	}
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
		GPIO_Cfg.enDir = GpioDirOut;
		GPIO_Cfg.enPd = GpioPdDisable;
		GPIO_Cfg.enPu = GpioPuDisable;
		GPIO_Cfg.enOD = GpioOdEnable;
    }
#endif
	/* remeber the pull state. */
    //pin_pulls[index->index].pd = GPIO_Cfg.enPd;
	//pin_pulls[index->index].pu = GPIO_Cfg.enPu;
	
	Gpio_Init(index->port, index->pin, &GPIO_Cfg);
}


static os_err_t
hc32_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    struct pin_index *index;
    os_base_t               level;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    level = os_hw_interrupt_disable();

	if( index->hdr == hdr   &&
		index->mode == mode &&
		index->args == args){
		
		os_hw_interrupt_enable(level);
		return OS_EOK;
	}

	if(index->args != OS_NULL){
		
		os_hw_interrupt_enable(level);
		return OS_EBUSY;
	}

    index->hdr  = hdr;
    index->mode = mode;
    index->args = args;

    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t hc32_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    struct pin_index *index;
    os_base_t               level;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    level = os_hw_interrupt_disable();
    if (index->hdr == OS_NULL)
    {
        os_hw_interrupt_enable(level);
        return OS_EOK;
    }

    index->hdr  = OS_NULL;
    index->mode = 0;
    index->args = OS_NULL;
	
    os_hw_interrupt_enable(level);
	
	return OS_EOK;
}

static os_err_t hc32_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    struct pin_index *index;
    os_base_t level;
	os_uint32_t port_base = pin/16;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {

        level = os_hw_interrupt_disable();

		if(index->irq_ref > 0){
			os_hw_interrupt_enable(level);
			return OS_EOK;
		}

        switch (index->mode)
        {
        case PIN_IRQ_MODE_RISING:
			Gpio_EnableIrq(index->port,index->pin, GpioIrqRising);
            break;
        case PIN_IRQ_MODE_FALLING:
            Gpio_EnableIrq(index->port,index->pin, GpioIrqFalling);
            break;
        case PIN_IRQ_MODE_HIGH_LEVEL:			
			Gpio_EnableIrq(index->port,index->pin, GpioIrqHigh);
			break;
        case PIN_IRQ_MODE_LOW_LEVEL:			
			Gpio_EnableIrq(index->port,index->pin, GpioIrqLow);
			break;
		case PIN_IRQ_MODE_RISING_FALLING:
		default:
            os_hw_interrupt_enable(level);
            return OS_ENOSYS;
        }

		index->irq_ref++;

		if(pState[port_base]->ref == 0)
			EnableNvic(pState[port_base]->irq, IrqLevel3, TRUE);

		pState[port_base]->ref++;
		
        os_hw_interrupt_enable(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        level = os_hw_interrupt_disable();

		if(index->irq_ref ==0){
			os_hw_interrupt_enable(level);
			return OS_EOK;
		}

        switch (index->mode)
        {
        case PIN_IRQ_MODE_RISING:
			Gpio_DisableIrq(index->port,index->pin, GpioIrqRising);
            break;
        case PIN_IRQ_MODE_FALLING:
            Gpio_DisableIrq(index->port,index->pin, GpioIrqFalling);
            break;
        case PIN_IRQ_MODE_HIGH_LEVEL:			
			Gpio_DisableIrq(index->port,index->pin, GpioIrqHigh);
			break;
        case PIN_IRQ_MODE_LOW_LEVEL:			
			Gpio_DisableIrq(index->port,index->pin, GpioIrqLow);
			break;
		case PIN_IRQ_MODE_RISING_FALLING:
		default:
            os_hw_interrupt_enable(level);
            return OS_ENOSYS;
        }

		index->irq_ref--;

		pState[port_base]->ref--;
		if(pState[port_base]->ref == 0)
			EnableNvic(pState[port_base]->irq, IrqLevel3, FALSE);

        os_hw_interrupt_enable(level);
    }
    else
    {
        return OS_ENOSYS;
    }
    return OS_EOK;
}
const static struct os_pin_ops _hc32_pin_ops = {
    hc32_pin_mode,
    hc32_pin_write,
    hc32_pin_read,
    hc32_pin_attach_irq,
    hc32_pin_dettach_irq,
    hc32_pin_irq_enable,
};

OS_INLINE void pin_irq_hdr(os_uint32_t gpio_port_index,en_gpio_port_t gpio_port_base)
{
	
    const struct pin_index *index;
	en_gpio_pin_t pin;
	
	for(pin=GpioPin0;pin<=GpioPin15;pin++){
		
		index = get_pin(pin + gpio_port_index * 16);

		if (index == OS_NULL)
			continue;

		if(TRUE == Gpio_GetIrqStatus(gpio_port_base, pin))
		{			 

			index->hdr(index->args);			
			Gpio_ClearIrq(gpio_port_base, pin);    
		}
			
	}
}


#if defined(GPIOPORTA)
void PortA_IRQHandler(void)
{

    os_interrupt_enter();
	pin_irq_hdr(GPIOPORTA,GpioPortA);
    os_interrupt_leave();
}
#if defined(GPIOPORTB)
void PortB_IRQHandler(void)
{
	os_interrupt_enter();
	pin_irq_hdr(GPIOPORTB,GpioPortB);
	os_interrupt_leave();
}

#if defined(GPIOPORTC)
void PortC_IRQHandler(void)
{
	os_interrupt_enter();
	pin_irq_hdr(GPIOPORTC,GpioPortC);
	os_interrupt_leave();
}
#if defined(GPIOPORTD)
void PortD_IRQHandler(void)
{
	os_interrupt_enter();
	pin_irq_hdr(GPIOPORTD,GpioPortD);
	os_interrupt_leave();

}
#if defined(GPIOPORTE)
void PortE_IRQHandler(void)
{
	os_interrupt_enter();
	pin_irq_hdr(GPIOPORTE,GpioPortE);
	os_interrupt_leave();

}
#if defined(GPIOPORTF)
void PortF_IRQHandler(void)
{
	os_interrupt_enter();
	pin_irq_hdr(GPIOPORTF,GpioPortF);
	os_interrupt_leave();

}
#endif
#endif
#endif
#endif
#endif

#endif


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

	Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE); 

    return os_device_pin_register(0, &_hc32_pin_ops, OS_NULL);
}

#endif /* OS_USING_PIN */
