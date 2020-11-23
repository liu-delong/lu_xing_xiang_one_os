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
 * @file        fal_cfg.h
 *
 * @brief       Flash abstract layer partition definition
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <board.h>

#define OS_ONCHIP_FLASH_NAME "onchip_flash"

#define OS_APP_PART_NAME  "app"
#define OS_USER_PART_NAME "user"

#define OS_APP_PART_ADDR     ((uint32_t)0x0800B000)    /* 0x08020000 */
#define OS_APP_PART_SIZE     (468 * 1024)              /* unit: bytes, total 468Kbytes */
#define OS_APP_PART_END_ADDR (OS_APP_PART_ADDR + OS_APP_PART_SIZE)

#define OS_USER_PART_ADDR     ((uint32_t)0x080F5000)    /* 0x08020000 */
#define OS_USER_PART_SIZE     (4 * 1024)                /* unit: bytes, total 4Kbytes */
#define OS_USER_PART_END_ADDR (OS_USER_PART_ADDR + OS_USER_PART_END_ADDR)

#define FAL_PART_TABLE                                                                                                         \
{                                                                                                                              \
    {FAL_PART_MAGIC_WROD, OS_APP_PART_NAME, OS_ONCHIP_FLASH_NAME, (OS_APP_PART_ADDR - AMEBA_FLASH_START_ADRESS), OS_APP_PART_SIZE, 0}, \
    {FAL_PART_MAGIC_WROD, OS_USER_PART_NAME, OS_ONCHIP_FLASH_NAME, (OS_USER_PART_ADDR - AMEBA_FLASH_START_ADRESS), OS_USER_PART_SIZE, 0}, \
}
#endif /* _FAL_CFG_H_ */
