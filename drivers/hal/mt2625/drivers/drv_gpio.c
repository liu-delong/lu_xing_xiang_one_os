/*
 * Copyright (c) 2012-2020, CMCC IOT
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */


#include "drv_gpio.h"
#include "hal_gpio.h"
#include "hal_eint.h"
#include "os_kernel.h"
#include "drv_cfg.h"

#ifdef OS_USING_PIN

typedef struct __tag_MT2625_EINT_S
{
    hal_gpio_pin_t gpio_pin;
    uint32_t function_index;
    hal_eint_number_t eint_number;
}MT2625_EINT_S;

MT2625_EINT_S mt2625_eint[] = 
{
    {HAL_GPIO_1, HAL_GPIO_1_EINT1, HAL_EINT_NUMBER_1},
    {HAL_GPIO_2, HAL_GPIO_2_EINT2, HAL_EINT_NUMBER_2},
    {HAL_GPIO_3, HAL_GPIO_3_EINT3, HAL_EINT_NUMBER_3},
    {HAL_GPIO_4, HAL_GPIO_4_EINT4, HAL_EINT_NUMBER_4},
    {HAL_GPIO_5, HAL_GPIO_5_EINT5, HAL_EINT_NUMBER_5},
    {HAL_GPIO_6, HAL_GPIO_6_EINT6, HAL_EINT_NUMBER_6},
    {HAL_GPIO_7, HAL_GPIO_7_EINT7, HAL_EINT_NUMBER_7},
    {HAL_GPIO_8, HAL_GPIO_8_EINT8, HAL_EINT_NUMBER_8},
    {HAL_GPIO_9, HAL_GPIO_9_EINT9, HAL_EINT_NUMBER_9},
    {HAL_GPIO_10, HAL_GPIO_10_EINT10, HAL_EINT_NUMBER_10},
    {HAL_GPIO_11, HAL_GPIO_11_EINT11, HAL_EINT_NUMBER_11},
    {HAL_GPIO_12, HAL_GPIO_12_EINT12, HAL_EINT_NUMBER_12},
    {HAL_GPIO_13, HAL_GPIO_13_EINT13, HAL_EINT_USB},
    {HAL_GPIO_14, HAL_GPIO_14_EINT14, HAL_EINT_NUMBER_14},
    {HAL_GPIO_15, HAL_GPIO_15_EINT15, HAL_EINT_NUMBER_15},
    {HAL_GPIO_16, HAL_GPIO_16_EINT16, HAL_EINT_NUMBER_16},
    {HAL_GPIO_17, HAL_GPIO_17_EINT17, HAL_EINT_NUMBER_17},
    {HAL_GPIO_18, HAL_GPIO_18_EINT18, HAL_EINT_NUMBER_18},
    {HAL_GPIO_19, HAL_GPIO_19_EINT19, HAL_EINT_NUMBER_19},
    {HAL_GPIO_20, HAL_GPIO_20_EINT20, HAL_EINT_NUMBER_20},
    {HAL_GPIO_21, HAL_GPIO_21_EINT21, HAL_EINT_NUMBER_21},
    {HAL_GPIO_22, HAL_GPIO_22_EINT22, HAL_EINT_NUMBER_22},
    {HAL_GPIO_23, HAL_GPIO_23_EINT0, HAL_EINT_NUMBER_0},
    {HAL_GPIO_24, HAL_GPIO_24_EINT1, HAL_EINT_NUMBER_1},
    {HAL_GPIO_25, HAL_GPIO_25_EINT2, HAL_EINT_NUMBER_2},
    {HAL_GPIO_26, HAL_GPIO_26_EINT3, HAL_EINT_NUMBER_3},
    {HAL_GPIO_27, HAL_GPIO_27_EINT4, HAL_EINT_NUMBER_4},
    {HAL_GPIO_28, HAL_GPIO_28_EINT5, HAL_EINT_NUMBER_5},
    {HAL_GPIO_29, HAL_GPIO_29_EINT6, HAL_EINT_NUMBER_6},
    {HAL_GPIO_30, HAL_GPIO_30_EINT7, HAL_EINT_NUMBER_7},
    {HAL_GPIO_31, HAL_GPIO_31_EINT8  , HAL_EINT_NUMBER_8},
    {HAL_GPIO_32, HAL_GPIO_32_EINT9  , HAL_EINT_NUMBER_9},
    {HAL_GPIO_33, HAL_GPIO_33_EINT10, HAL_EINT_NUMBER_10},
    {HAL_GPIO_34, HAL_GPIO_34_EINT11, HAL_EINT_NUMBER_11},
    {HAL_GPIO_35, HAL_GPIO_35_EINT12, HAL_EINT_NUMBER_12},
    {HAL_GPIO_36, HAL_GPIO_36_EINT13, HAL_EINT_USB},
};

MT2625_EINT_S *mt2625_eint_get(hal_gpio_pin_t gpio_pin)
{
    uint8_t i = 0;
    uint8_t eint_num = sizeof(mt2625_eint)/sizeof(mt2625_eint[0]);
    MT2625_EINT_S *mt_eint = mt2625_eint;

    for (i = 0; i < eint_num; i++, mt_eint++)
    {
        if (mt_eint->gpio_pin == gpio_pin)
        {
            return mt_eint;
        }
    }

    return NULL;
}

static void _drv_pin_write(struct os_device * dev, os_base_t pin, os_base_t value)
{
    hal_gpio_pin_t gpio_pin;
    hal_gpio_direction_t gpio_direction;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return;
    }
    gpio_pin = (hal_gpio_pin_t)pin;

    hal_gpio_get_direction(gpio_pin, &gpio_direction);
    if (gpio_direction) /* output */
    {
        hal_gpio_set_output(gpio_pin, (hal_gpio_data_t)value);
    }
}

static int _drv_pin_read(struct os_device * dev, os_base_t pin)
{
    int value = PIN_LOW;
    hal_gpio_pin_t gpio_pin;
    hal_gpio_direction_t gpio_direction;
    hal_gpio_data_t gpio_data;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return value;
    }
    gpio_pin = (hal_gpio_pin_t)pin;

    hal_gpio_get_direction(gpio_pin, &gpio_direction);
    if (gpio_direction) /* output */
    {
        hal_gpio_get_output(gpio_pin, &gpio_data);
    }
    else /* input */
    {
        hal_gpio_get_input(gpio_pin, &gpio_data);
    }

    value = ((gpio_data == HAL_GPIO_DATA_LOW) ? PIN_LOW : PIN_HIGH);

    return value;
}

static void _drv_pin_mode(struct os_device * dev, os_base_t pin, os_base_t mode)
{
    hal_gpio_pin_t gpio_pin;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return;
    }

    gpio_pin = (hal_gpio_pin_t)pin;
    hal_gpio_init(gpio_pin);
    hal_pinmux_set_function(gpio_pin, 0); // Set the pin to operate in GPIO mode.

    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_OUTPUT);
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_INPUT);
        hal_gpio_disable_pull(gpio_pin);
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_INPUT);
        hal_gpio_pull_up(gpio_pin);
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */
        hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_INPUT);
        hal_gpio_pull_down(gpio_pin);
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
        hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_OUTPUT);
        hal_gpio_disable_pull(gpio_pin);
    }
}

static os_err_t _drv_pin_attach_irq(struct os_device *device, os_int32_t pin,
                                     os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    hal_eint_status_t status;
    hal_eint_config_t eint_config;
    MT2625_EINT_S *mt_eint;

    hal_gpio_pin_t gpio_pin;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return OS_ERROR;
    }

    gpio_pin = (hal_gpio_pin_t)pin;
    
    mt_eint = mt2625_eint_get(gpio_pin);
    if (mt_eint == NULL)
    {
        return OS_EINVAL;
    }

    hal_gpio_init(mt_eint->gpio_pin);
    /* 7: eint func */
    if (hal_pinmux_set_function(mt_eint->gpio_pin, mt_eint->function_index) != HAL_PINMUX_STATUS_OK)
    {
        return OS_ERROR;
    }

    switch (mode)
    {
    case PIN_IRQ_MODE_RISING:
        eint_config.trigger_mode = HAL_EINT_EDGE_RISING;
        break;
    case PIN_IRQ_MODE_FALLING:
        eint_config.trigger_mode = HAL_EINT_EDGE_FALLING;
        break;
    case PIN_IRQ_MODE_RISING_FALLING:
        eint_config.trigger_mode = HAL_EINT_EDGE_FALLING_AND_RISING;
        break;
    case PIN_IRQ_MODE_HIGH_LEVEL:
        eint_config.trigger_mode = HAL_EINT_LEVEL_HIGH;
        break;
    case PIN_IRQ_MODE_LOW_LEVEL:
        eint_config.trigger_mode = HAL_EINT_LEVEL_LOW;
        break;
    default:
        return OS_ERROR;
    }

    eint_config.debounce_time = 10;

    /* disable eint, then enable it in _drv_pin_irq_enable function */
    if (hal_eint_mask(mt_eint->eint_number) != HAL_EINT_STATUS_OK)
    {
        return OS_ERROR;
    }

    if (HAL_EINT_STATUS_OK != hal_eint_init(mt_eint->eint_number, &eint_config))
    {
        return OS_ERROR;
    }

    status = hal_eint_register_callback(mt_eint->eint_number, hdr, args);
    
    return (status == HAL_EINT_STATUS_OK) ? OS_EOK : (OS_ERROR);
}

static os_err_t _drv_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    hal_eint_status_t status;
    hal_gpio_pin_t gpio_pin;
    MT2625_EINT_S *mt_eint;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return OS_ERROR;
    }

    gpio_pin = (hal_gpio_pin_t)pin;
    mt_eint = mt2625_eint_get(gpio_pin);
    if (mt_eint == NULL)
    {
        return OS_EINVAL;
    }

    /* disable eint interrupt */
    status = hal_eint_mask(mt_eint->eint_number);
    if (status != HAL_EINT_STATUS_OK)
    {
        return OS_ERROR;
    }

    /* 0: gpio func */
    if (hal_pinmux_set_function(mt_eint->gpio_pin, 0) != HAL_PINMUX_STATUS_OK)
    {
        return OS_ERROR;
    }

    /* deinit eint */
    status = hal_eint_deinit(mt_eint->eint_number);
    if (status != HAL_EINT_STATUS_OK)
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t _drv_pin_irq_enable(struct os_device *device, os_base_t pin,
                                     os_uint32_t enabled)
{
    hal_eint_status_t status;
    MT2625_EINT_S *mt_eint;
    hal_gpio_pin_t gpio_pin;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return OS_ERROR;
    }

    gpio_pin = (hal_gpio_pin_t)pin;
    mt_eint = mt2625_eint_get(gpio_pin);
    if (mt_eint == NULL)
    {
        return OS_EINVAL;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {
        status = hal_eint_unmask(mt_eint->eint_number);
    }
    else
    {
        status = hal_eint_mask(mt_eint->eint_number);
    }

    return (status == HAL_EINT_STATUS_OK) ? OS_EOK : (OS_ERROR);
}

const static struct os_pin_ops _drv_pin_ops =
{
    _drv_pin_mode,
    _drv_pin_write,
    _drv_pin_read,
    _drv_pin_attach_irq,
    _drv_pin_dettach_irq,
    _drv_pin_irq_enable,
};

int cm_hw_pin_init(void)
{
    /* TODO for init gpio clock */

    /* register 'pin' device */
    return os_device_pin_register(0, &_drv_pin_ops, OS_NULL);
}

#endif /* OS_USING_PIN */
