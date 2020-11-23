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
#include "driver/gpio.h"

#define ESP32_PIN_NUM_MAX 8

struct esp32_pin {
    os_uint8_t    map[ESP32_PIN_NUM_MAX];
    gpio_config_t config[ESP32_PIN_NUM_MAX];
};

/* Pin map must be defined */
static struct esp32_pin g_esp32_pin = {
    {4, 5, 14, 15, 16, 17, 18, 19},
};

static void esp32_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    struct esp32_pin *p_pin    = (struct esp32_pin *)(dev->user_data);

    if(pin < ESP32_PIN_NUM_MAX)
        return;

    gpio_set_level((gpio_num_t)(p_pin->map[pin]), value);
}

static int esp32_pin_read(struct os_device *dev, os_base_t pin)
{
    struct esp32_pin *p_pin    = (struct esp32_pin *)(dev->user_data);

    if(pin < ESP32_PIN_NUM_MAX)
        return 0;

    return gpio_get_level((gpio_num_t)(p_pin->map[pin]));
}

static void esp32_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    struct esp32_pin *p_pin = (struct esp32_pin *)(dev->user_data);

    if(pin < ESP32_PIN_NUM_MAX)
        return;

    if (mode == PIN_MODE_OUTPUT)
    {
        p_pin->config[pin].mode = GPIO_MODE_OUTPUT;
        p_pin->config[pin].pull_down_en = 0;
        p_pin->config[pin].pull_up_en = 0;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        p_pin->config[pin].mode = GPIO_MODE_INPUT;
        p_pin->config[pin].pull_down_en = 0;
        p_pin->config[pin].pull_up_en = 0;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        p_pin->config[pin].mode = GPIO_MODE_INPUT;
        p_pin->config[pin].pull_down_en = 0;
        p_pin->config[pin].pull_up_en = 1;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        p_pin->config[pin].mode = GPIO_MODE_INPUT;
        p_pin->config[pin].pull_down_en = 1;
        p_pin->config[pin].pull_up_en = 0;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        p_pin->config[pin].mode = GPIO_MODE_OUTPUT_OD;
        p_pin->config[pin].pull_down_en = 0;
        p_pin->config[pin].pull_up_en = 0;
    }

    gpio_config(&(p_pin->config[pin]));

}

static os_err_t
esp32_pin_attach_irq(struct os_device *dev, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    os_base_t level;
    struct esp32_pin *p_pin    = (struct esp32_pin *)(dev->user_data);

    if(pin < ESP32_PIN_NUM_MAX)
        return OS_ERROR;

    switch (mode)
    {
    case PIN_IRQ_MODE_RISING:
        p_pin->config[pin].intr_type = GPIO_PIN_INTR_POSEDGE;
        break;
    case PIN_IRQ_MODE_FALLING:
        p_pin->config[pin].intr_type = GPIO_PIN_INTR_NEGEDGE;
        break;
    case PIN_IRQ_MODE_RISING_FALLING:
        p_pin->config[pin].intr_type = GPIO_PIN_INTR_ANYEDGE;
        break;
    case PIN_IRQ_MODE_HIGH_LEVEL:
        p_pin->config[pin].intr_type = GPIO_PIN_INTR_HILEVEL;
        break;
    case PIN_IRQ_MODE_LOW_LEVEL:
        p_pin->config[pin].intr_type = GPIO_PIN_INTR_LOLEVEL;
        break;
    default:
        return OS_ERROR;
    }

    level = os_hw_interrupt_disable();

    gpio_config(&(p_pin->config[pin]));
    gpio_isr_handler_add((gpio_num_t)(p_pin->map[pin]), hdr, args);

    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t esp32_pin_dettach_irq(struct os_device *dev, os_int32_t pin)
{
    os_base_t level;
    struct esp32_pin *p_pin    = (struct esp32_pin *)(dev->user_data);
    gpio_num_t        gpio_num = (gpio_num_t)(p_pin->map[pin]);

    if(pin < ESP32_PIN_NUM_MAX)
        return OS_ERROR;

    level = os_hw_interrupt_disable();
    
    gpio_isr_handler_remove(gpio_num);

    os_hw_interrupt_enable(level);

    return OS_EOK;
}

static os_err_t esp32_pin_irq_enable(struct os_device *dev, os_base_t pin, os_uint32_t enabled)
{
    os_base_t level;
    struct esp32_pin *p_pin    = (struct esp32_pin *)(dev->user_data);

    if(pin < ESP32_PIN_NUM_MAX)
        return OS_ERROR;

    if (enabled == PIN_IRQ_ENABLE)
    {
        level = os_hw_interrupt_disable();
        gpio_intr_enable((gpio_num_t)(p_pin->map[pin]));
        os_hw_interrupt_enable(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        level = os_hw_interrupt_disable();
        gpio_intr_disable((gpio_num_t)(p_pin->map[pin]));
        os_hw_interrupt_enable(level);
    }
    else
    {
        return OS_ENOSYS;
    }

    return OS_EOK;
}

const static struct os_pin_ops esp32_pin_ops = {
    esp32_pin_mode,
    esp32_pin_write,
    esp32_pin_read,
    esp32_pin_attach_irq,
    esp32_pin_dettach_irq,
    esp32_pin_irq_enable,
};

int os_hw_pin_init(void)
{
    os_uint8_t i;

    for(i=0; i<ESP32_PIN_NUM_MAX; i++)
    {
        g_esp32_pin.config[i].pin_bit_mask = 1ULL << g_esp32_pin.map[i];
        g_esp32_pin.config[i].mode = GPIO_MODE_DISABLE,
        g_esp32_pin.config[i].pull_up_en = true,
        g_esp32_pin.config[i].pull_down_en = false,
        g_esp32_pin.config[i].intr_type = GPIO_INTR_DISABLE,
        gpio_config(&g_esp32_pin.config[i]);
    }

    gpio_install_isr_service(0);

    return os_device_pin_register(0, &esp32_pin_ops, &g_esp32_pin);
}

#endif /* OS_USING_PIN */
