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
 * @file        main.c
 *
 * @brief       User application entry
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include "os_clock.h"
#include "lpmgr_dev.h"

#include "lpmgr_lower.h"
#include "th_i2c.h"
int nb_temp_hi_read(float* temperature, float* humidity)
{
    etError error; 				// error code

    error = NO_ERROR;
    error = SHT3X_GetTempAndHumiClkStretch(temperature, humidity, REPEATAB_HIGH, 50);
    if(error != NO_ERROR) 		// do error handling here
    {
        os_kprintf("error:GetTempAndHumi1:%d\n", error);
        return -1;
    }
    else
    {
        return 0;
    }
}

void task_run(void)
{
    float temperature = 0;
    float humidity = 0;
    int temp;
    int hum;
    
    /* 读取温湿度数据 */
    nb_temp_hi_read(&temperature, &humidity);
    temp = temperature * 100;
    hum = humidity * 100;

    os_kprintf("[%s]-[%d], temperature[%2d.%-2d], humidity[%2d.%-2d], cur_tick[%d]\r\n", 
        __FILE__, __LINE__, temp/100, temp%100, hum/100, hum%100, os_tick_get());

}

extern void lpmgr_low_set(void);

static void send_task(void *parameter)
{
    LPMGR_TIME_S *time = OS_NULL;

    lpmgr_hard_init();
    lpmgr_dev_init();
    
    os_lpmgr_request(3);
    os_kprintf("[%s]-[%d], reques sleep mode, tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());
    
    while (1)
    {
        os_lpmgr_request(0);
        os_kprintf("[%s]-[%d], exit sleep mode, tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());

        time = lpmgr_time_get();
        os_task_msleep(time->run_time * 1000);

        task_run();

        os_kprintf("[%s]-[%d], enter sleep mode, tick[%d], run_time[%d], sleep_time[%d]\r\n\n\n", 
            __FILE__, __LINE__, os_tick_get(), time->run_time, time->sleep_time);

        os_lpmgr_release(0);
        os_task_msleep(time->sleep_time * 1000);
    }
}

int main(void)
{
    os_task_t *task;

    task = os_task_create("send_task", send_task, OS_NULL, 1024, 3, 5);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}
