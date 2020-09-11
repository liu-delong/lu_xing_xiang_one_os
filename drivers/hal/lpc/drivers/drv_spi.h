/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
#ifndef __DRV_SPI_H__ 
#define __DRV_SPI_H__ 

#include <os_task.h> 
#include <os_device.h> 

int os_hw_spi_init(void);
os_err_t lpc_spi_bus_attach_device(const char *bus_name, const char *device_name, os_uint32_t pin); 

#endif
