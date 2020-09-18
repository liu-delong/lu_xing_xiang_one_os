#ifndef __ONEPOS_CELL_LOCA_H__
#define __ONEPOS_CELL_LOCA_H__

#include <mo_api.h>

extern char *mul_cells_pos_pub_msg(onepos_sev_pro_t sev_pro);
extern os_err_t onepos_parse_mul_cell_pos(ops_platform_lbs_info_t *mul_cell_info, cJSON *data_item);
extern os_err_t init_onepos_cell_device(void);
extern os_bool_t onepos_get_cell_sta(void);
extern void onepos_cell_info_clean(onepos_cell_info_t *onepos_cell_info);

#endif  /* __ONEPOS_CELL_LOCA_H__ */
