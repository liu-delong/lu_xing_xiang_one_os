/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-13     Liuguang     the first version. 
 * 2018-03-19     Liuguang     add GPIO interrupt mode support.
 * 2019-07-15     Magicoe      The first version for LPC55S6x
 */
 
#ifndef __DRV_PIN_H__
#define __DRV_PIN_H__

#include "board.h"

#define GET_PIN(PORTx, PINx)      (32 * PORTx + PINx + 1)    /* PORTx:0,1, PINx:0,1...31 */

extern int os_hw_pin_init(void);

#endif /* __DRV_PIN_H__ */
