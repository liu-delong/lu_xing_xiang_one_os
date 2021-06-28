#ifndef __ONEPOS_WIFI_LOCA_H__
#define __ONEPOS_WIFI_LOCA_H__

extern os_err_t wifi_pos_pub_msg(cJSON* json_src);
extern void onepos_wifi_location_deinit(void);
extern os_err_t onepos_wifi_location_init(void);
extern os_err_t init_onepos_wifi_device(void);
extern os_bool_t onepos_get_wifi_sta(void);

#endif /* __ONEPOS_WIFI_LOCA_H__ */
