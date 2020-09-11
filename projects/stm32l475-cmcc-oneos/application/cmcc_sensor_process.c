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
 * @file        cmcc_sensor_process.c
 *
 * @brief       The sensors read temperature, humidity and so on.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_device.h>
#include <os_hw.h>
#include <board.h>
#include <stdint.h>

#include <sensors/sensor.h>
#include "cmcc_sensor_process.h"
#include "st7789vw.h"
#include "cmcc_lcd_process.h"

#define DBG_ENABLE
#define DBG_SECTION_NAME               "CMCC_SENSOR"
#ifdef  CMCC_SENSOR_DEBUG
#define DBG_LEVEL                      DBG_LOG
#else
#define DBG_LEVEL                      DBG_INFO /* DBG_ERROR */
#endif
#define DBG_COLOR
#include <os_dbg_ext.h>

static cmcc_sensor_data_t cmcc_sensor_data_result =
{
    .aht10_data_temp = 0.0f,
    .aht10_data_humi = 0.0f,
    .ap3216_data_als = 0.0f,
    .ap3216_data_ps  = 0,
};

void cmcc_sensor_data_upload(cmcc_sensor_data_t *in_data, cmcc_sensor_data_t *out_data)
{
    os_base_t level = os_hw_interrupt_disable();

    out_data->aht10_data_temp = in_data->aht10_data_temp;
    out_data->aht10_data_humi = in_data->aht10_data_humi;

    out_data->ap3216_data_als = in_data->ap3216_data_als;
    out_data->ap3216_data_ps = in_data->ap3216_data_ps;

    os_hw_interrupt_enable(level);
}

void cmcc_sensor_data_read(void *arg)
{
    struct os_sensor_data sensor_data;
    cmcc_sensor_data_t sensor_update_data;
    
    
    cmcc_sensor_data_upload(cmcc_sensor_data_result_get(), &sensor_update_data);

#ifdef OS_USING_AHT10

    os_device_t *sensor_temp = os_device_find("temp_aht10");
    OS_ASSERT(sensor_temp != NULL);
    os_device_open(sensor_temp, OS_DEVICE_FLAG_RDWR);

    os_device_read(sensor_temp, 0, &sensor_data, sizeof(struct os_sensor_data));
    sensor_update_data.aht10_data_temp = sensor_data.data.temp;

    os_device_t *sensor_humi = os_device_find("humi_aht10");
    OS_ASSERT(sensor_humi != NULL);
    os_device_open(sensor_humi, OS_DEVICE_FLAG_RDWR);

    os_device_read(sensor_humi, 0, &sensor_data, sizeof(struct os_sensor_data));
    sensor_update_data.aht10_data_humi = sensor_data.data.humi;
    
 #endif
 
 #ifdef OS_USING_AP3216C

    os_device_t *sensor_light = os_device_find("li_ap3216c");
    OS_ASSERT(sensor_light != NULL);
    os_device_open(sensor_light, OS_DEVICE_FLAG_RDWR);

    os_device_read(sensor_light, 0, &sensor_data, sizeof(struct os_sensor_data));
    sensor_update_data.ap3216_data_als = sensor_data.data.light;

    os_device_t *sensor_ps = os_device_find("pr_ap3216c");
    OS_ASSERT(sensor_ps != NULL);
    os_device_open(sensor_ps, OS_DEVICE_FLAG_RDWR);

    os_device_read(sensor_ps, 0, &sensor_data, sizeof(struct os_sensor_data));
    sensor_update_data.ap3216_data_ps = sensor_data.data.proximity;
    
 #endif

    cmcc_sensor_data_upload(&sensor_update_data, cmcc_sensor_data_result_get());
}

cmcc_sensor_data_t *cmcc_sensor_data_result_get(void)
{
    return &cmcc_sensor_data_result;
}


static void _cmcc_sensor_read_task(void *arg)
{
    do
    {
        cmcc_sensor_data_read(OS_NULL);
        os_task_mdelay(500);
    } while(1);
}

void cmcc_sensor_read_start(void)
{
    os_task_t *sensor_read_task = OS_NULL;
    sensor_read_task = os_task_create("sensor",
                                    _cmcc_sensor_read_task,
                                    OS_NULL,
                                    2048, 12, 10);
    if (sensor_read_task != OS_NULL)
    {
        os_task_startup(sensor_read_task);
    }
}


void cmcc_sensor_init(void)
{
    /* Sensor data can be read after one second of power-on */
    os_task_mdelay(1000);

    cmcc_sensor_read_start();

}
