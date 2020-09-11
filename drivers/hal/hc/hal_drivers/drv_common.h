/*
 * Copyright (c) 2012-2020, CMCC IOT
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __DRV_COMMON_H__
#define __DRV_COMMON_H__

#include <os_hw.h>
#include <os_device.h>

#ifdef __cplusplus
extern "C" {
#endif

void _Error_Handler(char *s, int num);

#ifndef Error_Handler
#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#endif

#define DMA_NOT_AVAILABLE ((DMA_INSTANCE_TYPE *)0xFFFFFFFFU)

#ifdef __cplusplus
}
#endif

#endif
