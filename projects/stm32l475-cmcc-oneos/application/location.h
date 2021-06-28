#include <string.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <os_task.h>    
#include <os_errno.h>  
#include <drv_gpio.h>  
  
#include <onepos_interface.h>  
   
#ifdef OS_USING_ST7789VW  
#include "cmcclogo.h"  
 #include "oneposimage.h"  
#include "st7789vw.h"  
  
#define CMCC_LCD_TIME_ROW_SP 65  
#define CMCC_LCD_TIME_ROW_H 50  
#define CMCC_LCD_TIME_LCOLUMN_SP 65  
#define CMCC_LCD_TIME_RCOLUMN_SP 110  
  
#define SHOW_COLOR_RED 0xFA27  
#define SHOW_COLOR_GREEN 0x1546  
#define SHOW_COLOR_YELLOW 0xFD20  
 
static void oneos_lcd_show_startup_page(void)  
{  
 /* show CMCC logo */  
lcd_show_image(20, 50, 200, 61, gImage_cmcc);  
lcd_show_image(20, 150, 200, 52, gImage_oneos);  
}  
  
static void oneos_lcd_show_position(void *arg)  
{  
	char    *str                 = OS_NULL;  
	char    time_get_now[9]      = {0,};  
	time_t  now                  = {0,};  
	os_int32_t     j             = 0;  
	static char    lat_show[20]  = {0,};  
	static char    lon_show[20]  = {0,};  
	static onepos_pos_t pos_data = {0,};  
	#ifdef OS_USING_ST7789VW  
	oneos_lcd_show_startup_page();  
	os_task_mdelay(500);  
	#endif 
	
	#ifdef PKG_USING_NTP  
	/* wait ntp sync */  
	extern time_t ntp_sync_to_rtc(const char *host_name);  
	ntp_sync_to_rtc(OS_NULL);  
	os_task_mdelay(500);  
	#endif 
	/* start onepos sever and wait once position */  	
	while(ONEPOS_CLOSING == onepos_get_server_sta()) 
	{  
		onepos_start_server();  
		os_task_mdelay(5000);  
	}		
	lcd_clear(BLACK);  
	lcd_show_image(0, 0, 240, 240, gImage_onepos); 
	
	while(1)  
	{  
		/* Get position */  
		onepos_get_latest_position(&pos_data);          
		/* Get time */  
		now = time(OS_NULL);  
		str = ctime(&now);  
		snprintf(lat_show, 24, "%f", pos_data.lat_coordinate);  
		snprintf(lon_show, 24, "%f", pos_data.lon_coordinate);
		for(j = 11; j <19; j++)  
		{  
			time_get_now[j-11] = str[j];  
		}  
		time_get_now[8] = '\0';  
		lcd_set_color(SHOW_COLOR_RED, BLACK);  
		lcd_show_string(CMCC_LCD_TIME_LCOLUMN_SP + 30,CMCC_LCD_TIME_ROW_SP+CMCC_LCD_TIME_ROW_H * 0 + 3, 24, time_get_now);  
		lcd_set_color(SHOW_COLOR_GREEN, BLACK);  
		lcd_show_string(CMCC_LCD_TIME_LCOLUMN_SP + 30,CMCC_LCD_TIME_ROW_SP+CMCC_LCD_TIME_ROW_H * 1 + 15, 24,lat_show);  
		lcd_set_color(SHOW_COLOR_YELLOW, BLACK);  
		lcd_show_string(CMCC_LCD_TIME_LCOLUMN_SP + 30,CMCC_LCD_TIME_ROW_SP+CMCC_LCD_TIME_ROW_H * 2 + 30, 24,lon_show);  
		os_task_mdelay(50);  
	}  
}  
void oneos_getposition(int lat[],int lon[])
{
	#ifdef PKG_USING_NTP  
	/* wait ntp sync */  
	extern time_t ntp_sync_to_rtc(const char *host_name);  
	ntp_sync_to_rtc(OS_NULL);  
	os_task_mdelay(500);  
	#endif
	/* start onepos sever and wait once position */ 
	while(ONEPOS_CLOSING==onepos_get_server_sta()) 
	{  
		onepos_start_server();  
		os_task_mdelay(5000);  
	}
	onepos_pos_t pos_data;
	onepos_get_latest_position(&pos_data);
	lat[0]=pos_data.lat_coordinate;
	float xiaoshu=pos_data.lat_coordinate-lat[0];
	xiaoshu=xiaoshu*1000000;
	lat[1]=xiaoshu;
	lon[0]=pos_data.lon_coordinate;
	xiaoshu=pos_data.lon_coordinate-lon[0];
	xiaoshu=xiaoshu*1000000;
	lon[1]=xiaoshu;
}
#endif  
