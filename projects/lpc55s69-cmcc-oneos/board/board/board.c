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
 * @file        board.c
 *
 * @brief       Initializes the CPU, System clocks, and Peripheral device
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include "board.h"
#include <drv_gpio.h>
#include <os_clock.h>

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
 * @file        board.c
 *
 * @brief       Initializes the CPU, System clocks, and Peripheral device
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_gpio.h>
#include <os_stddef.h>

#define PWR_4G_PIN      GET_PIN(1, 27)
#define PWR_WIFI_PIN    GET_PIN(1, 18)
#define PWR_SENSOR_PIN  GET_PIN(1, 19)
#define PWR_AUDIO_PIN   GET_PIN(1, 14)

#define AUDIO_DATA_DIR_PIN   GET_PIN(0, 17)

const led_t led_table[] = 
{
    {GET_PIN(0, 27), PIN_LOW},
    {GET_PIN(0, 28), PIN_LOW},
    {GET_PIN(0, 29), PIN_LOW},
};

const int led_table_size = ARRAY_SIZE(led_table);

const struct push_button key_table[] = 
{
    {GET_PIN(1, 0),    PIN_MODE_INPUT_PULLUP,      PIN_IRQ_MODE_FALLING},
    {GET_PIN(1, 1),    PIN_MODE_INPUT_PULLUP,      PIN_IRQ_MODE_FALLING},
    {GET_PIN(1, 7),     PIN_MODE_INPUT_PULLUP,      PIN_IRQ_MODE_FALLING},
    {GET_PIN(1, 9),     PIN_MODE_INPUT_PULLUP,      PIN_IRQ_MODE_FALLING},
};

const int key_table_size = ARRAY_SIZE(key_table);

const buzzer_t buzzer_table[] =
{
    {GET_PIN(0, 23), PIN_HIGH},
};

const int buzzer_table_size = ARRAY_SIZE(buzzer_table);

os_err_t board_power_on(void)
{
    os_pin_mode(PWR_4G_PIN, PIN_MODE_OUTPUT);
    os_pin_mode(PWR_WIFI_PIN, PIN_MODE_OUTPUT);
    os_pin_mode(PWR_SENSOR_PIN, PIN_MODE_OUTPUT);
    os_pin_mode(PWR_AUDIO_PIN, PIN_MODE_OUTPUT);
    os_pin_mode(AUDIO_DATA_DIR_PIN, PIN_MODE_OUTPUT);
    
    os_pin_write(PWR_4G_PIN, PIN_HIGH);
    os_pin_write(PWR_WIFI_PIN, PIN_HIGH);
    os_pin_write(PWR_SENSOR_PIN, PIN_HIGH);
    os_pin_write(PWR_AUDIO_PIN, PIN_HIGH);
    
#if defined(I2S7_TX_DMA_CHANNEL)
    os_pin_write(AUDIO_DATA_DIR_PIN, PIN_LOW);
#else
    os_pin_write(AUDIO_DATA_DIR_PIN, PIN_HIGH);
#endif
    return 0;
}

OS_INIT_EXPORT(board_power_on,"0.end.2");
