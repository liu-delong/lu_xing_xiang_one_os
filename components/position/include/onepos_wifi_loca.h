#ifndef __ONEPOS_WIFI_LOCA_H__
#define __ONEPOS_WIFI_LOCA_H__

extern void onepos_wifi_location_deinit(void);
extern os_err_t onepos_wifi_location_init(void);
extern char *mul_wifis_pos_pub_msg(onepos_sev_pro_t sev_pro);
extern char *single_wifi_query_pos_pub_msg(onepos_sev_pro_t sev_pro);
extern os_err_t onepos_parse_sig_wifi_pos(ops_sigle_wifi_info_t *sig_wifi_info, os_uint16_t src_num, cJSON *data_item);
extern os_err_t onepos_parse_mul_wifi_pos(ops_platform_wifi_info_t *mul_wifi_info, cJSON *data_item);
extern os_err_t init_onepos_wifi_device(void);
extern os_bool_t onepos_get_wifi_sta(void);
extern void clean_ap_pos_info(ops_wifi_src_grp_t* src);

#endif  /* __ONEPOS_WIFI_LOCA_H__ */

