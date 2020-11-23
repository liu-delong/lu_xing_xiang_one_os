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
 * @file        drv_spi.h
 *
 * @brief       This file implements SPI driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_SPI_H_
#define __DRV_SPI_H_

#include <board.h>
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_memory.h>
#include <drv_gpio.h>
#include <drv_common.h>
#include <string.h>
#include <drv_log.h>
#include "hc_spi.h"
#include "hc_reset.h"


#define SPI_USING_RX_DMA_FLAG (1 << 0)
#define SPI_USING_TX_DMA_FLAG (1 << 1)

struct hc32_spi_device
{
	os_uint32_t pin;
	char *bus_name;
	char *device_name;
};

struct hc32_hw_spi_cs
{
	en_gpio_port_t GPIOx;
	en_gpio_pin_t  GPIO_Pin;
	os_uint8_t cs_pin_ref;
};

struct hc32_spi_config
{
	//SPI_TypeDef *Instance;
	char        *bus_name;
};

struct hc32_spi
{
	struct os_spi_bus spi_bus;   
	M0P_SPI_TypeDef *spi_base;
	en_reset_peripheral0_t reset;
	en_sysctrl_peripheral_gate_t peripheral;
	en_gpio_port_t port_cs;
	en_gpio_pin_t pin_cs;
	en_gpio_port_t port_sck;
	en_gpio_pin_t pin_sck;
	en_gpio_port_t port_miso;
	en_gpio_pin_t pin_miso;
	en_gpio_port_t port_mosi;
	en_gpio_pin_t pin_mosi;
	en_gpio_af_t gpio_af;
	struct hc32_spi_config     *config;
	struct os_spi_configuration *cfg;
	os_uint8_t     spi_dma_flag;
	const char *name;
};


os_err_t os_hw_spi_device_attach(const char *bus_name, const char *device_name, os_base_t cs_pin);

#endif /*__DRV_SPI_H_ */
