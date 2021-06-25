#include <serial.h>
#include "esp8266.h"

#include <stdlib.h>
#include <string.h>
#include <os_dbg_ext.h>
#include <mo_api.h>
static mo_object_t *module=OS_NULL;
void connet_wifi(void* para)
{
		if(!module) module =mo_get_by_name(ESP8266_NAME);
		while(esp8266_wifi_get_stat(module)!=MO_WIFI_STAT_CONNECTED)
		{
				mo_wifi_connect_ap(module, ESP8266_CONNECT_SSID, ESP8266_CONNECT_PASSWORD);
				os_task_msleep(2000);
		}
}
int wifi_is_connet(void)
{
		if(!module) module =mo_get_by_name(ESP8266_NAME);
		if(esp8266_wifi_get_stat(module)==MO_WIFI_STAT_CONNECTED) return 1;
		else return 0;
}

