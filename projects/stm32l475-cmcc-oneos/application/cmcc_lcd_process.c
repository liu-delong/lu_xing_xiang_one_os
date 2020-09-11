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
 * @file        cmcc_lcd_process.c
 *
 * @brief       LCD dispalys the board data 
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_hw.h>
#include <os_device.h>
#include <os_event.h>
#include <stdint.h>
#include <stdio.h>
#include <board.h>
#include <rtc/rtc.h>
#include <st7789vw.h>
#include <cmcclogo.h>

#include "cmcc_lcd_img.h"
#include "cmcc_sensor_process.h"
#include "cmcc_version.h"
#include "cmcc_lcd_process.h"

#undef DBG_SECTION_NAME
#undef DBG_LEVEL
#undef DBG_COLOR
#undef DBG_ENABLE

#define DBG_ENABLE
#define DBG_SECTION_NAME               "CMCC_LCD"
#ifdef  CMCC_LCD_DEBUG
#define DBG_LEVEL                      DBG_LOG
#else
#define DBG_LEVEL                      DBG_INFO /* DBG_ERROR */
#endif
#define DBG_COLOR
#include <os_dbg_ext.h>

#define CMCC_LCD_TASK_STACK_SIZE    2048


static struct os_event lcd_event;
static os_mutex_t *lcd_mutex = OS_NULL;

static void cmcc_lcd_show_index_page(cmcc_lcd_content_type_t display_type);    /* menu1  */
static void cmcc_lcd_show_sensor(cmcc_lcd_content_type_t display_type);        /* menu2  */
static void cmcc_lcd_show_time(cmcc_lcd_content_type_t display_type);          /* menu3  */
static void cmcc_lcd_show_info(cmcc_lcd_content_type_t display_type);          /* menu4  */
static void cmcc_lcd_show(void *arg);


void cmcc_lcd_event_put(cmcc_lcd_event_t event)
{
    os_event_send(&lcd_event, (os_uint32_t)event);
}


static void cmcc_lcd_show(void *arg) /* dynamic task */
{
    os_err_t rst = OS_EOK;
    os_uint32_t event = 0;
    uint16_t cnt = 0;
    cmcc_lcd_content_type_t display_type;
    static os_uint8_t menu_index;
    
    while (1)
    {
        display_type = CMCC_LCD_STATIC_CONTENT;
        cmcc_lcd_show_index_page(display_type);
        
        while(1)   
        {            
            rst = os_event_recv(&lcd_event,
                                LCD_EVENT_SENSOR|LCD_EVENT_INDEX|LCD_EVENT_TIME|LCD_EVENT_INFO,
                                OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                                0, (os_uint32_t *)&event);
            if (rst == OS_EOK)
            {
                if (event & LCD_EVENT_INDEX)
                {
                    menu_index = 1;
                    cnt = 0;
                    display_type = CMCC_LCD_STATIC_CONTENT;
                    cmcc_lcd_show_index_page(display_type);                    
                    display_type = CMCC_LCD_DYNAMIC_CONTENT;
                }
                
                if (event & LCD_EVENT_SENSOR)
                {
                    menu_index = 2;
                    cnt = 0;
                    display_type = CMCC_LCD_STATIC_CONTENT;
                    cmcc_lcd_show_sensor(display_type);
                    display_type = CMCC_LCD_DYNAMIC_CONTENT;
                }
                
                if (event & LCD_EVENT_TIME)
                {
                    menu_index = 3;
                    cnt = 0;
                    display_type = CMCC_LCD_STATIC_CONTENT;
                    cmcc_lcd_show_time(display_type);
                    display_type = CMCC_LCD_DYNAMIC_CONTENT;
                }
                
                if (event & LCD_EVENT_INFO)
                {
                    menu_index = 4;
                    cnt = 0;
                    display_type = CMCC_LCD_STATIC_CONTENT;
                    cmcc_lcd_show_info(display_type);
                    display_type = CMCC_LCD_DYNAMIC_CONTENT;
                }

            }
        
            if(display_type == CMCC_LCD_DYNAMIC_CONTENT)
            {
                
                if(cnt >= 10)
                {
                    if(menu_index == 2)                    
                        cmcc_lcd_show_sensor(display_type);
                    
                    if(menu_index == 3)                    
                        cmcc_lcd_show_time(display_type);
                    
                    cnt = 0;
                }
                cnt++;
            }
            
            os_task_mdelay(50);  
        }
               
    }
}



void cmcc_lcd_show_startup_page(void)
{
    /* show CMCC logo */
    lcd_show_image(20, 50, 200, 61, gImage_cmcc);
    
    lcd_show_image(20, 150, 200, 52, gImage_oneos);
}

static void cmcc_lcd_show_index_page(cmcc_lcd_content_type_t display_type)
{
    LOG_EXT_D("show [index page]");
    if (display_type == CMCC_LCD_STATIC_CONTENT)
    {
        /* clear lcd */
        lcd_clear(WHITE);

        /* set the background color and foreground color */
        lcd_set_color(WHITE, BLACK);
        lcd_show_string(60, 15, 32, "Welcome!");

        lcd_show_string(CMCC_LCD_HOME_LCOLUMN_SP, CMCC_LCD_HOME_ROW_SP - CMCC_LCD_HOME_ROW_H, 24, "Key function:");

        lcd_set_color(WHITE, BLUE);
        lcd_show_string(CMCC_LCD_HOME_LCOLUMN_SP, CMCC_LCD_HOME_ROW_SP + CMCC_LCD_HOME_ROW_H * 0, 24, "S2");
        lcd_show_string(CMCC_LCD_HOME_LCOLUMN_SP, CMCC_LCD_HOME_ROW_SP + CMCC_LCD_HOME_ROW_H * 1, 24, "S3");
        lcd_show_string(CMCC_LCD_HOME_LCOLUMN_SP, CMCC_LCD_HOME_ROW_SP + CMCC_LCD_HOME_ROW_H * 2, 24, "S4");
        lcd_show_string(CMCC_LCD_HOME_LCOLUMN_SP, CMCC_LCD_HOME_ROW_SP + CMCC_LCD_HOME_ROW_H * 3, 24, "S5");

        lcd_set_color(WHITE, BLACK);
        lcd_show_string(CMCC_LCD_HOME_RCOLUMN_SP, CMCC_LCD_HOME_ROW_SP + CMCC_LCD_HOME_ROW_H * 0, 24, "Home");
        lcd_show_string(CMCC_LCD_HOME_RCOLUMN_SP, CMCC_LCD_HOME_ROW_SP + CMCC_LCD_HOME_ROW_H * 1, 24, "Sensor");
        lcd_show_string(CMCC_LCD_HOME_RCOLUMN_SP, CMCC_LCD_HOME_ROW_SP + CMCC_LCD_HOME_ROW_H * 2, 24, "About");
        lcd_show_string(CMCC_LCD_HOME_RCOLUMN_SP, CMCC_LCD_HOME_ROW_SP + CMCC_LCD_HOME_ROW_H * 3, 24, "Calendar");
    }
}

static void cmcc_lcd_show_sensor(cmcc_lcd_content_type_t display_type)
{
    char buf[48];
    cmcc_sensor_data_t sensor_data;

    cmcc_sensor_data_upload(cmcc_sensor_data_result_get(), &sensor_data);

    if (display_type == CMCC_LCD_STATIC_CONTENT)
    {
        LOG_EXT_D("show [sensor page]");

        lcd_clear(WHITE);
        lcd_set_color(WHITE, BLACK);
        lcd_show_string(72, 15, 32, "sensor");

        lcd_show_image(CMCC_LCD_SENSOR_LCOLUMN_SP, CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 0, 24, 24, gImage_tem24x24);
        lcd_show_image(CMCC_LCD_SENSOR_LCOLUMN_SP, CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 1, 24, 24, gImage_humi24x24);
        lcd_show_image(CMCC_LCD_SENSOR_LCOLUMN_SP, CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 2, 24, 24, gImage_sun24x24);
        lcd_show_image(CMCC_LCD_SENSOR_LCOLUMN_SP, CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 3, 24, 24, gImage_dis24x24);
        
        lcd_show_string(CMCC_LCD_SENSOR_LCOLUMN_SP + 26,CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 0, 24, "temp");
        lcd_show_string(CMCC_LCD_SENSOR_LCOLUMN_SP + 26,CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 1, 24, "humi");
        lcd_show_string(CMCC_LCD_SENSOR_LCOLUMN_SP + 26,CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 2, 24, "light");
        lcd_show_string(CMCC_LCD_SENSOR_LCOLUMN_SP + 26,CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 3, 24, "ps");  
        
        lcd_show_string(CMCC_LCD_SENSOR_RCOLUMN_SP + 12,CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 0, 24, "C");
        lcd_show_string(CMCC_LCD_SENSOR_RCOLUMN_SP + 12,CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 1, 24, "%");
        lcd_show_string(CMCC_LCD_SENSOR_RCOLUMN_SP,CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 2, 24, "lux");
        lcd_show_string(CMCC_LCD_SENSOR_RCOLUMN_SP + 6,CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 3, 24, "ps");  
    }

    lcd_set_color(WHITE, BLACK);


    memset(buf, 0x0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%3.1f",
             (sensor_data.aht10_data_temp/1000));
    LOG_EXT_D(buf);
    lcd_show_string(CMCC_LCD_SENSOR_RCOLUMN_SP - 50, CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 0, 24, buf);


    memset(buf, 0x0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%4.1f",
             (sensor_data.aht10_data_humi)/1000);
    LOG_EXT_D(buf);
    lcd_show_string(CMCC_LCD_SENSOR_RCOLUMN_SP - 50, CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 1, 24, buf);


    memset(buf, 0x0, sizeof(buf));
    os_snprintf(buf, sizeof(buf), "%4d",
                (uint16_t)(sensor_data.ap3216_data_als)/1000);
    lcd_show_string(CMCC_LCD_SENSOR_RCOLUMN_SP - 62, CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 2, 24, buf);


    memset(buf, 0x0, sizeof(buf));
    os_snprintf(buf, sizeof(buf), "%4d",
                sensor_data.ap3216_data_ps);
    lcd_show_string(CMCC_LCD_SENSOR_RCOLUMN_SP - 62, CMCC_LCD_SENSOR_ROW_SP + CMCC_LCD_SENSOR_ROW_H * 3, 24, buf);
    
}

static void cmcc_lcd_show_time(cmcc_lcd_content_type_t display_type)
{
    time_t  now;
    char    *str;
    char    date[11];
    char    time_get_now[9];
    int     i,j;
    char    month_test[3];
    char    date_show[20];
    os_uint8_t month_show = 0;
    const char month[][12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    if (display_type == CMCC_LCD_STATIC_CONTENT)
    {
        /* clear lcd */
        lcd_clear(WHITE);

        /* set the background color and foreground color */
        lcd_set_color(WHITE, BLACK);

        lcd_show_string(65, 15, 32, "Calendar");
    }
    
   lcd_set_color(WHITE, BLACK);
   /* Get time */   
    now = time(OS_NULL);    
    str = ctime(&now);

    month_test[0] = str[4];
    month_test[1] = str[5];
    month_test[2] = str[6];

    for(i=0;i<12;i++)
    {
        if (strcmp(month_test,month[i]) == 0)
        {
            month_show = i + 1;
            break;
        }
    }

    date[0] = 'W';
    date[1] = 'e';
    date[2] = 'e';
    date[3] = 'k';
    date[4] = ':';
	date[5] = ' ';
    date[6] = str[0];
    date[7] = str[1];
    date[8] = str[2];
    date[9] = '\0';
    
    date_show[0] = str[20];
    date_show[1] = str[21];
    date_show[2] = str[22];
    date_show[3] = str[23];
    date_show[4] = '.';
    os_snprintf(&date_show[5], 3, "%02d", month_show);
    date_show[7] = '.';
    if (str[8] == ' ')
    {
        date_show[8] = '0';
    }
    else
    {
        date_show[8] = str[8];
    }
    
    date_show[9] = str[9];
    date_show[10] = '\0';
    
    for(j=11;j<19;j++)
    {
        time_get_now[j-11] = str[j];
    }
    time_get_now[8] = '\0';

    lcd_show_string(CMCC_LCD_TIME_LCOLUMN_SP + 12, CMCC_LCD_TIME_ROW_SP + CMCC_LCD_TIME_ROW_H * 0, 24,  time_get_now);
    
    lcd_show_string(CMCC_LCD_TIME_LCOLUMN_SP, CMCC_LCD_TIME_ROW_SP + CMCC_LCD_TIME_ROW_H * 1, 24, date_show);

    lcd_show_string(CMCC_LCD_TIME_LCOLUMN_SP + 6, CMCC_LCD_TIME_ROW_SP + CMCC_LCD_TIME_ROW_H * 2, 24, date);
}

static void cmcc_lcd_show_info(cmcc_lcd_content_type_t display_type)
{
    if (display_type == CMCC_LCD_STATIC_CONTENT)
    {
        char buf[21];
        
        /* clear lcd */
        lcd_clear(WHITE);

        /* set the background color and foreground color */
        lcd_set_color(WHITE, BLACK);

        lcd_show_string(80, 15, 32, "OneOS");

        memset(buf, 0x0, sizeof(buf));
        os_snprintf(buf, sizeof(buf), "OneOS:   %d.%d.%d", OS_VERSION, OS_SUBVERSION, OS_REVISION);
        lcd_show_string(CMCC_LCD_INFO_LCOLUMN_SP, CMCC_LCD_INFO_ROW_SP + CMCC_LCD_INFO_ROW_H * 0, 24, buf);

        memset(buf, 0x0, sizeof(buf));
        os_snprintf(buf, sizeof(buf), "Hardware:%d.%d.%d", (uint8_t)(HARDWARE_VERSION >> 8 & 0x0F), (uint8_t)(HARDWARE_VERSION >> 4 & 0x0F), (uint8_t)(HARDWARE_VERSION & 0x0F));
        lcd_show_string(CMCC_LCD_INFO_LCOLUMN_SP, CMCC_LCD_INFO_ROW_SP + CMCC_LCD_INFO_ROW_H * 1, 24,  buf); 

        lcd_show_string(CMCC_LCD_INFO_LCOLUMN_SP, CMCC_LCD_INFO_ROW_SP + CMCC_LCD_INFO_ROW_H * 2, 24, "Web site:");

        lcd_show_string(CMCC_LCD_INFO_LCOLUMN_SP, CMCC_LCD_INFO_ROW_SP + CMCC_LCD_INFO_ROW_H * 3, 24, "os.iot.10086.cn");
    }
    
}

void cmcc_lcd_start(void)
{
    os_task_t *cmcc_lcd_tid;

    lcd_mutex = os_mutex_create("lcd_mutex", OS_IPC_FLAG_FIFO,OS_FALSE);
    OS_ASSERT(lcd_mutex != OS_NULL);

    os_event_init(&lcd_event, "lcd_event", OS_IPC_FLAG_FIFO);

    /* create lcd show task 'cmcc_lcd_show'*/
    cmcc_lcd_tid = os_task_create("lcd_task",
                                    cmcc_lcd_show,
                                    OS_NULL,
                                    CMCC_LCD_TASK_STACK_SIZE, OS_TASK_PRIORITY_MAX / 2 - 4, 50);

    if (cmcc_lcd_tid != OS_NULL)
    {
        os_task_startup(cmcc_lcd_tid);
    }
}
