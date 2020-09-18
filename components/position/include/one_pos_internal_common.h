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
 * @file        one_pos_internal_common.h
 * 
 * @brief       The OneNet operating system locates the internal header file of the service
 * 
 * @details     Common functions and data structure definitions within the service component
 * 
 * @revision
 * Date         Author          Notes
 * 2020-06-24   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __ONE_POS_INTERNAL_COMMON_H__
#define __ONE_POS_INTERNAL_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "one_pos_types.h"
#include "one_pos_error.h"
#include "one_pos_memory_manager.h"

/* When debugging on PC, cross-platform adaptation is needed */
#if defined __OPS_WINDOWS__
#include <windows.h> 
#include <malloc.h>
#elif defined __OPS_ONE_OS__
#include <os_types.h>
#include <os_memory.h>
#else
#error Undefined platform
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Component debug */
#define OPS_RELATIVE_ERROR_MAX          0.000000000001   /* The maximum allowable relative error */
#define OPS_STATIC_MODE                 1                /* static test mode */
#define OPS_DEBUG_MODE                  1                /* debug mode */
#define OPS_RELEASE_MODE                0                /* release mode */
#define OPS_RUN_MODE                    OPS_DEBUG_MODE   /* run mode switch */

/* Fault-tolerant macro function definition */
/* When the assert_content result is false, the function in which this macro resides returns */
#define OPS_ASSERT(assert_content, ret) \
    if ((assert_content)) \
    { \
        if (OPS_RUN_MODE) \
        { \
            printf("one pos error: %s@%d:"#assert_content"\n", __func__, __LINE__); \
        } \
        return (ret); \
    }

/* The mathematics computes the macro function */
/* The square of the point spacing */
#define OPS_SQUARE_OF_POINT_SPACING(dot_one_x,\
                                    dot_one_y,\
                                    dot_tow_x,\
                                    dot_tow_y)\
    (ops_float_t)(((dot_one_x) - (dot_tow_x))\
          * ((dot_one_x) - (dot_tow_x))\
          + ((dot_one_y) - (dot_tow_y))\
          * ((dot_one_y) - (dot_tow_y)))

/* Absolute value of 32-bit integer data */
#define OPS_ABS(data) fabs((data))
#define OPS_COS(data) cos((data))

#define OPS_FLOAT_ZERO_THR              1e-15f                     /* Float data zero determines the threshold */

/* Determines whether ops_float_t data is equal */
#define OPS_CHECK_QRUAL_FLOAT(float_a, float_b)\
    (((ops_float_t)(float_a) - (ops_float_t)(float_b)) < OPS_FLOAT_ZERO_THR)

/* Determines whether ops_float_t data is less than or equal to zero */
#define OPS_CHECK_LESS_EQUAL_ZERO_FLOAT(float_data)\
    ((ops_float_t)(float_data) < OPS_FLOAT_ZERO_THR)

/* relative error */
#define OPS_RELATIVE_ERROR(data_a, data_b)\
    (fabs(((ops_float_t)(data_a) - (ops_float_t)(data_b)) / (ops_float_t)(data_b)))

/* natural constant */
#define OPS_EARTH_RADIUS                6378137                    /* earth radius */
#define OPS_EARTH_AXIS_LONG             6378137                    /* The earth is long axis */
#define OPS_EARTH_AXIS_SHORT            6356752.314                /* The earth is short axis */
#define OPS_PI                          3.141592653                /* pi */
#define OPS_C                           299792458                  /* velocity of light */

/* System configuration macro parameter definitions */
#define OPS_LINE_START_MAX              32                         /* the maxinum number of elements in line_start */
#define OPS_RSSI_TIME_MAX               32                         /* the maxinum number of elements in time */
#define OPS_SOURCE_MAC_MAX              6                          /* maximum number of MAC address elements */
#define OPS_PREV_NUM_MAX                OPS_BLOCK_NUM_MAX          /* The maximum value of the data before the 
                                                                    * current moment (this value is set to the maximum 
                                                                    * plus one, please pay attention when making 
                                                                    * fault-tolerant judgment) */

/* Receive source data macro parameter definitions */
#define OPS_SIGNLA_SOURCE_IN_GROUP_MAX  50                         /* Upper limit of the number of data sources detected
                                                                    * at a single time */
/* Invalid longitude and latitude flag */
#define PLATFORM_WIFI_SRC_EMPTY         0xffff                     /* Mark longitude and latitude invalid */

/**
 ***********************************************************************************************************************
 * @enum        ops_use_prev_res_t
 * 
 * @brief       A marker that USES the results of the previous result for calibration, there are two states:
 *                  - true
 *                  - false
 ***********************************************************************************************************************
 */
typedef enum
{
    Ops_Use_Prev_Select_Res_False = 0,
    Ops_Use_Prev_Select_Res_True
}ops_use_prev_res_t;

/**
 ***********************************************************************************************************************
 * @enum        ops_dist_calc_type_t
 * 
 * @brief       Calculation method of distance from detection point to AP hot spot, there are four states:
 *                  - Logarithm      Logarithmic model
 *                  - Difference     Difference model
 *                  - Radar          RADAR model
 *                  - Mk             MK model
 ***********************************************************************************************************************
 */
typedef enum
{
    Ops_Logarithm = 0,
    Ops_Difference,
    Ops_Radar,
    Ops_Mk
}ops_dist_calc_type_t;

/**
 ***********************************************************************************************************************
 * @enum        ops_pos_calc_type_t
 * 
 * @brief       Coordinate calculation mode, there are six states:
 *                  - Centroid           Centroiding algorithm
 *                  - Weighted_Centroid  Weighted centroid algorithm
 *                  - Ls                 Least square method
 *                  - Newton_Gauss_LS    Gauss-newton iterative least square method
 *                  - TLS                Total least square method
 *                  - Hyperbola          Hyperbola location algorithm
 ***********************************************************************************************************************
 */
typedef enum
{
    Ops_Centroid = 0,
    Ops_Weighted_Centroid,
    Ops_Ls,
    Ops_Newton_Gauss_LS,
    Ops_Tls,
    Ops_Hyperbola
}ops_pos_calc_type_t;

/**
 ***********************************************************************************************************************
 * @enum        ops_filter_proc_type_t
 * 
 * @brief       The processing method of coordinate result filtering, there are one states:
 *                  - Ops_Alpha_Beta     Alpha-beta filtering method
 ***********************************************************************************************************************
 */
typedef enum
{
    Ops_Alpha_Beta_Filter = 0,
    Ops_Scope_Filter,
    Ops_Jump_Point_Filter,
    Ops_Smoothing_Filter
}ops_filter_proc_type_t;

/**
 ***********************************************************************************************************************
 * @enum        ops_alpha_beta_calc_state_t
 * 
 * @brief       Alpha-beta filter initialization status, there are two states:
 *                  - Ops_Init_Distance  Alpha-beta filter distance are initialized
 *                  - Ops_Init_Velocity  Alpha-beta filter velocity are initialized
 *                  - Ops_Filter         Initialization is complete and alpha-beta filtering is performed
 ***********************************************************************************************************************
 */
typedef enum
{
    Ops_Init_Distance = 0,
    Ops_Init_Velocity,
    Ops_Filter
}ops_alpha_beta_calc_state_t;

/**
 ***********************************************************************************************************************
 * @enum        ops_centroid_calibration_flag_t
 * 
 * @brief       Used to define whether or not to perform centroid calibration, there are two states:
 *                  - true
 *                  - false
 ***********************************************************************************************************************
 */
typedef enum
{
    Ops_Centroid_Calibration_False = 0,
    Ops_Centroid_Calibration_True
}ops_centroid_calibration_flag_t;

/**
 ***********************************************************************************************************************
 * @enum        ops_location_service_type_t
 * 
 * @brief       Location method, there are two states:
 *                  - Platform_Lbs
 *                  - Platform_Wifi
 *                  - Ops_Wifi
 ***********************************************************************************************************************
 */
typedef enum
{
    Ops_Platform_Lbs = 0,
    Ops_Platform_Wifi,
    Ops_Wifi
}ops_location_service_type_t;

/**
 ***********************************************************************************************************************
 * @enum        ops_jump_point_historic_t
 * 
 * @brief       Location method, there are two states:
 *                  - Ops_Jump_Point_Historic_Invalid      After the hopping point filtering, 
 *                                                         the historical data is invalid
 *                  - Ops_Jump_Point_Historic_Valid        After the hopping point filtering, 
 *                                                         the historical data is valid
 *                  - Ops_Wifi
 ***********************************************************************************************************************
 */
typedef enum
{
    Ops_Jump_Point_Historic_Invalid = 0,
    Ops_Jump_Point_Historic_Valid
}ops_jump_point_historic_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_src_select_param_t
 *      
 * @brief       Signal source select paramters
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_float_t src_sel_dist_max;  /* Input source filter maximum distance threshold */
}ops_src_select_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_moving_average_param_t
 *      
 * @brief       Moving average paramters
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_float_t mov_avg_coef;   /* The coefficient of the sliding average */
}ops_moving_average_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_calc_dist_param_t
 *      
 * @brief       Distance calculation parameters
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_float_t rssi_propagation_coef;            /* Rssi Propagation coefficient */
    ops_float_t rssi_std_const;                   /* Rssi Standard point constant */
    ops_int_t   signal_transmission_param;        /* Signal propagation environment parameters */
    ops_dist_calc_type_t calc_type;               /* Calculation method of distance */
}ops_calc_dist_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_calc_coordinate_param_t
 *      
 * @brief       Coordinate calculation parameters
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_pos_calc_type_t calc_type;  /* Coordinate calculation mode */
}ops_calc_coordinate_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_alpha_beta_param_t
 *      
 * @brief       Alpha-beta filtering parameter
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_float_t alpha_beta_a;                     /* Coefficient calculation constant A of Alpha-Beta filtering */
    ops_float_t alpha_beta_b;                     /* Coefficient calculation constant B of Alpha-Beta filtering */
    ops_float_t alpha_beta_c;                     /* Coefficient calculation constant C of Alpha-Beta filtering */
    ops_float_t position_time_interval;           /* The time interval between two positions  */
}ops_alpha_beta_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_scope_param_t
 *      
 * @brief       Scope filtering parameter
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_float_t lat_coordinate_max;               /* Maximum latitude */
    ops_float_t lat_coordinate_min;               /* Minimum latitude */
    ops_float_t lon_coordinate_max;               /* Maximum longitude */
    ops_float_t lon_coordinate_min;               /* Minimum longitude */
}ops_scope_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_jump_point_param_t
 *      
 * @brief       Jump point filtering parameter
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_ushort_t jump_point_num;                       /* The number of consecutive hops */
    ops_ushort_t jump_point_num_max;                   /* The maximum number of consecutive hops */
    ops_ushort_t jump_dist_threshold;                  /* The threshold value of the distance of the jump point */
    ops_float_t  position_time_interval;               /* The time interval between two positions */
    ops_jump_point_historic_t historic_validity;       /* Validity of historical data */
}ops_jump_point_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_smoothing_param_t
 *      
 * @brief       Smoothing filtering parameter
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_ushort_t smoothing_len;                   /* Smooth data length */
}ops_smoothing_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_coordinate_filter_param_t
 *      
 * @brief       Coordinate results filter parameters
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_filter_proc_type_t calc_type;             /* The calculate method of coordinate result filtering */
    ops_alpha_beta_param_t alpha_beta_param;      /* Alpha-beta filtering parameter */
    ops_scope_param_t      scope_param;           /* The parameters of the scope filter */
    ops_jump_point_param_t jump_point_param;      /* The parameters of the jump point filter */
    ops_smoothing_param_t  smooth_param;          /* The parameters of the smoothing filter */
}ops_coordinate_filter_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_platform_lbs_settings_t
 *      
 * @brief       Platform lbs mode system configuration parameters
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_mem_manager_t *mem_manager;               /* Memory manager */
    ops_uint_t         block_num;                 /* Maximum number of memory blocks */
}ops_platform_lbs_settings_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_platform_wifi_settings_t
 *      
 * @brief       Platform wifi mode system configuration parameters
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_mem_manager_t *mem_manager;               /* Memory manager */
    ops_uint_t         block_num;                 /* Maximum number of memory blocks */
}ops_platform_wifi_settings_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_wifi_settings_t
 *      
 * @brief       Wifi mode system configuration parameters
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_mem_manager_t            *src_mem_manager;               /* The input buffer */
    ops_mem_manager_t            *detection_status_manager;      /* Locate the results of the cache */
    ops_mem_manager_t            *detection_status_tmp_manager;  /* Locate the intermediate results of the cache */
    ops_uint_t                    block_num;                     /* Maximum number of memory blocks */
    ops_src_select_param_t        src_sel_param;                 /* Signal source select paramters */
    ops_moving_average_param_t    moving_average_param;          /* Moving average paramters */
    ops_calc_dist_param_t         calc_dist_param;               /* Distance calculation parameters */
    ops_calc_coordinate_param_t   calc_coordinate_param;         /* Coordinate calculation parameters */
    ops_coordinate_filter_param_t filter_param;                  /* Coordinate results filter parameters */
    ops_ushort_t                  jump_num;                      /* The number of frames to start the hops filter */
}ops_wifi_settings_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_settings_t
 *      
 * @brief       System configuration parameters
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_platform_lbs_settings_t  platform_lbs_settings;   /* Platform wifi mode system configuration parameters */
    ops_platform_wifi_settings_t platform_wifi_settings;  /* Platform lbs mode system configuration parameters */
    ops_wifi_settings_t          wifi_settings;           /* Wifi mode system configuration parameters */
}ops_settings_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_wifi_src_info_t
 *      
 * @brief       Data from a single signal source
 ***********************************************************************************************************************
 */
typedef struct
{
    char  mac[OPS_SOURCE_MAC_MAX]; /* Mac address */
    ops_short_t  rssi;             /* Received signal strength indcator */
//    ops_ushort_t rad_preci;        /* Radius of precision */
//    ops_float_t  dist_ap;          /* Detect the distance from the location to this AP hot spot */
    ops_float_t  lat_coordinate;   /* Latitude coordinate */
    ops_float_t  lon_coordinate;   /* Longitude coordinate */
//    ops_float_t  rssi_move_avg;    /* Rssi after smooth filtering */
}ops_wifi_src_info_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_wifi_src_grp_t
 *      
 * @brief       All source data detected at a single moment
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_uint_t   time;             /* Information acquisition time */
    ops_ushort_t src_num;          /* Number of sources in group */
    ops_wifi_src_info_t *sig_src;  /* Each source in the group */
}ops_wifi_src_grp_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_wifi_src_sel_t
 *      
 * @brief       Source selection results
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_ushort_t  sel_num;         /* Number of sources selected */
    ops_ushort_t *sel_idx;         /* The serial number of the selected source */
}ops_wifi_src_sel_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_platform_wifi_info_t
 *      
 * @brief       Data from a single signal source
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_uint_t  time;             /* Information acquisition time */
    ops_float_t lat_coordinate;   /* Latitude coordinate */
    ops_float_t lon_coordinate;   /* Longitude coordinate */
}ops_platform_wifi_info_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_platform_lbs_info_t
 *      
 * @brief       Data from a single signal source
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_uint_t  time;             /* Information acquisition time */
    ops_float_t lat_coordinate;   /* Latitude coordinate */
    ops_float_t lon_coordinate;   /* Longitude coordinate */
}ops_platform_lbs_info_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_sigle_wifi_info_t
 *      
 * @brief       Data from a sigle wifi and algorithm calculate
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_uint_t  time;             /* Information acquisition time */
    ops_float_t lat_coordinate;   /* Latitude coordinate */
    ops_float_t lon_coordinate;   /* Longitude coordinate */
}ops_sigle_wifi_info_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_platform_lbs_info_t
 *      
 * @brief       Locate the source data for the service
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_sigle_wifi_info_t    *sigle_wifi_src_info;    /* The input source of single wifi and algorithm calculate */
    ops_platform_wifi_info_t *platform_wifi_src_info; /* The input source of wifi mode under the platform */
    ops_platform_lbs_info_t  *platform_lbs_src_info;  /* The input source of lbs mode under the platform */
}ops_src_info_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_detection_point_status_t
 *      
 * @brief       Monitoring point attribute
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_float_t lat_coordinate; /* Longitude coordinate */
    ops_float_t lon_coordinate; /* Latitude coordinate */
//    ops_float_t lat_velocity;   /* Longitude velocity */
//    ops_float_t lon_velocity;   /* Latitude velocity */
}ops_detection_point_status_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_prev_point_status_t
 *      
 * @brief       Attributes of the previous prev_num's checkpoint points
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_ushort_t prev_num;                                             /* The number of the previous points */
    ops_detection_point_status_t *prev_point_status[OPS_PREV_NUM_MAX]; /* Attributes of the previous points */
}ops_prev_point_status_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_prev_src_sel_t
 *      
 * @brief       Attributes of the previous prev_num's source data
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_ushort_t prev_num;                                             /* The number of the previous points */
    ops_wifi_src_grp_t prev_src_grp[OPS_PREV_NUM_MAX];                /* Historical input source */
}ops_prev_src_grps_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_run_status
 *      
 * @brief       Location service health status
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_ushort_t cache_data_num;            /* Current number of cache space data */
    ops_uint_t   valid_data_num;            /* The number of valid data from service startup to the present */
    ops_uint_t   data_gather_num;           /* The number of times the data was collected */
    ops_uint_t   cur_invalid_data_num;      /* The number of invalid data currently appearing in a row */
    ops_ushort_t cur_invalid_data_num_max;  /* The maximum number of invalid data allowed to occur in a row
                                             * (When the maximum value is exceeded, 
                                             * the historical data is meaningless) */
    ops_uint_t historical_invalid_data_num; /* The number of invalid data occurrences since service startup to date */
    ops_uint_t wrong_num;                   /* Number of function run errors */
}ops_run_status;

/* For testing purposes only */
typedef struct
{
    ops_uint_t num;
    ops_wifi_src_grp_t src[400];
}ops_wifi_src_create;

/* For testing purposes only */
typedef struct
{
    ops_uint_t num;
    ops_detection_point_status_t dest[400];
}ops_wifi_dest;

/* For testing purposes only */
typedef struct
{
    ops_uint_t src_num;
    ops_uint_t dest_num;
    ops_wifi_src_create src;
    ops_wifi_dest dest;
}ops_service_test;

ops_err_t one_pos_signal_source_select(ops_wifi_src_grp_t     *sig_src,
                                       ops_wifi_src_sel_t     *sel_res,
                                       ops_wifi_src_grp_t     *prev_sig_src,
                                       ops_wifi_src_sel_t     *prev_sel_res,
                                       ops_src_select_param_t *param,
                                       ops_use_prev_res_t      use_prev_res_flag);

ops_err_t one_pos_moving_average(ops_wifi_src_grp_t         *sig_src,
                                 ops_wifi_src_sel_t         *sel_res,
                                 ops_wifi_src_grp_t         *prev_sig_src,
                                 ops_wifi_src_sel_t         *prev_sel_res,
                                 ops_moving_average_param_t *param,
                                 ops_use_prev_res_t          use_prev_res_flag);

ops_err_t one_pos_calculate_distance(ops_wifi_src_grp_t    *sig_src,
                                     ops_wifi_src_sel_t    *sel_res,
                                     ops_calc_dist_param_t *param);

ops_err_t one_pos_calculate_coordinate(ops_wifi_src_grp_t           *sig_src,
                                       ops_wifi_src_sel_t           *sel_res,
                                       ops_detection_point_status_t *pos_res,
                                       ops_calc_coordinate_param_t  *param);

ops_err_t one_pos_coordinate_filter(ops_wifi_src_grp_t            *cur_sig_src,
                                    ops_wifi_src_sel_t            *cur_sel_res,
                                    ops_detection_point_status_t  *cur_status,
                                    ops_prev_point_status_t       *prev_n_status,
                                    ops_coordinate_filter_param_t *param);

ops_float_t one_pos_calculate_utm_distance_aquared(ops_float_t point_one_lat,
                                                   ops_float_t point_one_lon,
                                                   ops_float_t point_two_lat,
                                                   ops_float_t point_tow_lon);

ops_err_t onepos_arithmetic_init(ops_wifi_settings_t *sys_settings,
                                 ops_run_status      *run_status);

ops_err_t onepos_arithmetic_run(ops_run_status      *run_status,
                                ops_wifi_src_grp_t  *src_grp,
                                ops_wifi_settings_t *sys_setting,
                                ops_float_t         *lat_coordinate,
                                ops_float_t         *lon_coordinate);

ops_void_t onepos_arthmetic_exit(ops_wifi_settings_t *sys_settings);

#ifdef __cplusplus
}
#endif

#endif /* __ONE_POS_INTERNAL_COMMON_H__ */
