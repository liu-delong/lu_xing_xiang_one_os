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
 * @file        power_control.c
 *
 * @brief       power control
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "drv_gpio.h"
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
/* sensor power:
 * 1. AXIS SENSOR
 * 2. TEMP&HUMI SENSOR
 * 3. ALS&PS SENSOR
 * 4. BEEP
 * 5. INFRARED EMISSION/RECEPTION
 */
#define SENSOR_POWER_PIN     GET_PIN(D, 0)
#define AUDIO_POWER_PIN      GET_PIN(D, 1)
#define POWER_4G_PIN         GET_PIN(D, 2)
#define WIFI_POWER_PIN       GET_PIN(D, 3)


int power_control(void)
{
    os_pin_mode(SENSOR_POWER_PIN, PIN_MODE_OUTPUT);
    os_pin_mode(AUDIO_POWER_PIN,  PIN_MODE_OUTPUT);
    os_pin_mode(WIFI_POWER_PIN,   PIN_MODE_OUTPUT);
    os_pin_mode(POWER_4G_PIN,     PIN_MODE_OUTPUT);


    os_pin_write(SENSOR_POWER_PIN, PIN_HIGH);
    os_pin_write(AUDIO_POWER_PIN,  PIN_HIGH);
    os_pin_write(WIFI_POWER_PIN,   PIN_HIGH);
    os_pin_write(POWER_4G_PIN,     PIN_HIGH);


    return 0;
}
OS_BOARD_INIT(power_control);
