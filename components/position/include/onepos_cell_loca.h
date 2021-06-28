#ifndef __ONEPOS_CELL_LOCA_H__
#define __ONEPOS_CELL_LOCA_H__

#include <mo_api.h>

/**
 ***********************************************************************************************************************
 * @enum        onepos_cell_net_type_t
 *
 * @brief       onepos Supported Cell Network Type
 ***********************************************************************************************************************
 */
typedef enum
{
    ONEPOS_CELL_TYPE_GSM = 1,
    ONEPOS_CELL_TYPE_CDMA,
    ONEPOS_CELL_TYPE_WCDMA,
    ONEPOS_CELL_TYPE_TD_CDMA,
    ONEPOS_CELL_TYPE_LTE,
    /* Add others cell network type */
} onepos_cell_net_type_t;

extern os_err_t  cell_pos_pub_msg(cJSON* json_src);
extern os_err_t  init_onepos_cell_device(void);
extern os_bool_t onepos_get_cell_sta(void);
extern void      onepos_cell_info_clean(onepos_cell_info_t *onepos_cell_info);

#endif /* __ONEPOS_CELL_LOCA_H__ */
