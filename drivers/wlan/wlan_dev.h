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
 * @file        wlan_dev.h
 *
 * @brief       wlan_dev
 *
 * @details     wlan_dev
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __WLAN_DEVICE_H__
#define __WLAN_DEVICE_H__
#include <os_device.h>
#include <os_mutex.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    OS_WLAN_NONE,
    OS_WLAN_STATION,
    OS_WLAN_AP,
    OS_WLAN_MODE_MAX
} os_wlan_mode_t;

typedef enum
{
    OS_WLAN_CMD_MODE = 0x10,
    OS_WLAN_CMD_SCAN,              /* trigger scanning (list cells) */
    OS_WLAN_CMD_JOIN,
    OS_WLAN_CMD_SOFTAP,            /* start soft-AP */
    OS_WLAN_CMD_DISCONNECT,
    OS_WLAN_CMD_AP_STOP,           /* stop soft-AP */
    OS_WLAN_CMD_AP_DEAUTH,
    OS_WLAN_CMD_SCAN_STOP,
    OS_WLAN_CMD_GET_RSSI,          /* get sensitivity (dBm) */
    OS_WLAN_CMD_SET_POWERSAVE,
    OS_WLAN_CMD_GET_POWERSAVE,
    OS_WLAN_CMD_CFG_PROMISC,       /* start/stop minitor */
    OS_WLAN_CMD_CFG_FILTER,        /* start/stop frame filter */
    OS_WLAN_CMD_CFG_MGNT_FILTER,   /* start/stop management frame filter */
    OS_WLAN_CMD_SET_CHANNEL,
    OS_WLAN_CMD_GET_CHANNEL,
    OS_WLAN_CMD_SET_COUNTRY,
    OS_WLAN_CMD_GET_COUNTRY,
    OS_WLAN_CMD_SET_MAC,
    OS_WLAN_CMD_GET_MAC
} os_wlan_cmd_t;

typedef enum
{
    OS_WLAN_DEV_EVT_INIT_DONE = 0,
    OS_WLAN_DEV_EVT_CONNECT,
    OS_WLAN_DEV_EVT_CONNECT_FAIL,
    OS_WLAN_DEV_EVT_DISCONNECT,
    OS_WLAN_DEV_EVT_AP_START,
    OS_WLAN_DEV_EVT_AP_STOP,
    OS_WLAN_DEV_EVT_AP_ASSOCIATED,
    OS_WLAN_DEV_EVT_AP_DISASSOCIATED,
    OS_WLAN_DEV_EVT_AP_ASSOCIATE_FAILED,
    OS_WLAN_DEV_EVT_SCAN_REPORT,
    OS_WLAN_DEV_EVT_SCAN_DONE,
    OS_WLAN_DEV_EVT_MAX,
} os_wlan_dev_event_t;

#define SHARED_ENABLED 0x00008000
#define WPA_SECURITY   0x00200000
#define WPA2_SECURITY  0x00400000
#define WPS_ENABLED    0x10000000
#define WEP_ENABLED    0x0001
#define TKIP_ENABLED   0x0002
#define AES_ENABLED    0x0004
#define WSEC_SWFLAG    0x0008

#define OS_WLAN_FLAG_STA_ONLY (0x1 << 0)
#define OS_WLAN_FLAG_AP_ONLY  (0x1 << 1)

#ifndef OS_WLAN_SSID_MAX_LENGTH
#define OS_WLAN_SSID_MAX_LENGTH (32) /* SSID MAX LEN */
#endif

#ifndef OS_WLAN_BSSID_MAX_LENGTH
#define OS_WLAN_BSSID_MAX_LENGTH (6) /* BSSID MAX LEN (default is 6) */
#endif

#ifndef OS_WLAN_PASSWORD_MAX_LENGTH
#define OS_WLAN_PASSWORD_MAX_LENGTH (32) /* PASSWORD MAX LEN*/
#endif

#ifndef OS_WLAN_DEV_EVENT_NUM
#define OS_WLAN_DEV_EVENT_NUM (2) /* EVENT GROUP MAX NUM */
#endif

/**
 * Enumeration of Wi-Fi security modes
 */
typedef enum
{
    SECURITY_OPEN           = 0,                              /* Open security                           */
    SECURITY_WEP_PSK        = WEP_ENABLED,                    /* WEP Security with open authentication   */
    SECURITY_WEP_SHARED     = (WEP_ENABLED | SHARED_ENABLED), /* WEP Security with shared authentication */
    SECURITY_WPA_TKIP_PSK   = (WPA_SECURITY | TKIP_ENABLED),  /* WPA Security with TKIP                  */
    SECURITY_WPA_AES_PSK    = (WPA_SECURITY | AES_ENABLED),   /* WPA Security with AES                   */
    SECURITY_WPA2_AES_PSK   = (WPA2_SECURITY | AES_ENABLED),  /* WPA2 Security with AES                  */
    SECURITY_WPA2_TKIP_PSK  = (WPA2_SECURITY | TKIP_ENABLED), /* WPA2 Security with TKIP                 */
    SECURITY_WPA2_MIXED_PSK = (WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED), /* WPA2 Security with AES & TKIP */
    SECURITY_WPS_OPEN       = WPS_ENABLED,                 /* WPS with open security                  */
    SECURITY_WPS_SECURE     = (WPS_ENABLED | AES_ENABLED), /* WPS with AES security                   */
    SECURITY_UNKNOWN        = -1,                          /* May be returned by scan function if security is unknown.
                                                               Do not pass this to the join function! */
} os_wlan_security_t;

typedef enum
{
    OS_802_11_BAND_5GHZ    = 0,          /* Denotes 5GHz radio band   */
    OS_802_11_BAND_2_4GHZ  = 1,          /* Denotes 2.4GHz radio band */
    OS_802_11_BAND_UNKNOWN = 0x7fffffff, /* unknown */
} os_802_11_band_t;

typedef enum
{
    OS_COUNTRY_AFGHANISTAN,
    OS_COUNTRY_ALBANIA,
    OS_COUNTRY_ALGERIA,
    OS_COUNTRY_AMERICAN_SAMOA,
    OS_COUNTRY_ANGOLA,
    OS_COUNTRY_ANGUILLA,
    OS_COUNTRY_ANTIGUA_AND_BARBUDA,
    OS_COUNTRY_ARGENTINA,
    OS_COUNTRY_ARMENIA,
    OS_COUNTRY_ARUBA,
    OS_COUNTRY_AUSTRALIA,
    OS_COUNTRY_AUSTRIA,
    OS_COUNTRY_AZERBAIJAN,
    OS_COUNTRY_BAHAMAS,
    OS_COUNTRY_BAHRAIN,
    OS_COUNTRY_BAKER_ISLAND,
    OS_COUNTRY_BANGLADESH,
    OS_COUNTRY_BARBADOS,
    OS_COUNTRY_BELARUS,
    OS_COUNTRY_BELGIUM,
    OS_COUNTRY_BELIZE,
    OS_COUNTRY_BENIN,
    OS_COUNTRY_BERMUDA,
    OS_COUNTRY_BHUTAN,
    OS_COUNTRY_BOLIVIA,
    OS_COUNTRY_BOSNIA_AND_HERZEGOVINA,
    OS_COUNTRY_BOTSWANA,
    OS_COUNTRY_BRAZIL,
    OS_COUNTRY_BRITISH_INDIAN_OCEAN_TERRITORY,
    OS_COUNTRY_BRUNEI_DARUSSALAM,
    OS_COUNTRY_BULGARIA,
    OS_COUNTRY_BURKINA_FASO,
    OS_COUNTRY_BURUNDI,
    OS_COUNTRY_CAMBODIA,
    OS_COUNTRY_CAMEROON,
    OS_COUNTRY_CANADA,
    OS_COUNTRY_CAPE_VERDE,
    OS_COUNTRY_CAYMAN_ISLANDS,
    OS_COUNTRY_CENTRAL_AFRICAN_REPUBLIC,
    OS_COUNTRY_CHAD,
    OS_COUNTRY_CHILE,
    OS_COUNTRY_CHINA,
    OS_COUNTRY_CHRISTMAS_ISLAND,
    OS_COUNTRY_COLOMBIA,
    OS_COUNTRY_COMOROS,
    OS_COUNTRY_CONGO,
    OS_COUNTRY_CONGO_THE_DEMOCRATIC_REPUBLIC_OF_THE,
    OS_COUNTRY_COSTA_RICA,
    OS_COUNTRY_COTE_DIVOIRE,
    OS_COUNTRY_CROATIA,
    OS_COUNTRY_CUBA,
    OS_COUNTRY_CYPRUS,
    OS_COUNTRY_CZECH_REPUBLIC,
    OS_COUNTRY_DENMARK,
    OS_COUNTRY_DJIBOUTI,
    OS_COUNTRY_DOMINICA,
    OS_COUNTRY_DOMINICAN_REPUBLIC,
    OS_COUNTRY_DOWN_UNDER,
    OS_COUNTRY_ECUADOR,
    OS_COUNTRY_EGYPT,
    OS_COUNTRY_EL_SALVADOR,
    OS_COUNTRY_EQUATORIAL_GUINEA,
    OS_COUNTRY_ERITREA,
    OS_COUNTRY_ESTONIA,
    OS_COUNTRY_ETHIOPIA,
    OS_COUNTRY_FALKLAND_ISLANDS_MALVINAS,
    OS_COUNTRY_FAROE_ISLANDS,
    OS_COUNTRY_FIJI,
    OS_COUNTRY_FINLAND,
    OS_COUNTRY_FRANCE,
    OS_COUNTRY_FRENCH_GUINA,
    OS_COUNTRY_FRENCH_POLYNESIA,
    OS_COUNTRY_FRENCH_SOUTHERN_TERRITORIES,
    OS_COUNTRY_GABON,
    OS_COUNTRY_GAMBIA,
    OS_COUNTRY_GEORGIA,
    OS_COUNTRY_GERMANY,
    OS_COUNTRY_GHANA,
    OS_COUNTRY_GIBRALTAR,
    OS_COUNTRY_GREECE,
    OS_COUNTRY_GRENADA,
    OS_COUNTRY_GUADELOUPE,
    OS_COUNTRY_GUAM,
    OS_COUNTRY_GUATEMALA,
    OS_COUNTRY_GUERNSEY,
    OS_COUNTRY_GUINEA,
    OS_COUNTRY_GUINEA_BISSAU,
    OS_COUNTRY_GUYANA,
    OS_COUNTRY_HAITI,
    OS_COUNTRY_HOLY_SEE_VATICAN_CITY_STATE,
    OS_COUNTRY_HONDURAS,
    OS_COUNTRY_HONG_KONG,
    OS_COUNTRY_HUNGARY,
    OS_COUNTRY_ICELAND,
    OS_COUNTRY_INDIA,
    OS_COUNTRY_INDONESIA,
    OS_COUNTRY_IRAN_ISLAMIC_REPUBLIC_OF,
    OS_COUNTRY_IRAQ,
    OS_COUNTRY_IRELAND,
    OS_COUNTRY_ISRAEL,
    OS_COUNTRY_ITALY,
    OS_COUNTRY_JAMAICA,
    OS_COUNTRY_JAPAN,
    OS_COUNTRY_JERSEY,
    OS_COUNTRY_JORDAN,
    OS_COUNTRY_KAZAKHSTAN,
    OS_COUNTRY_KENYA,
    OS_COUNTRY_KIRIBATI,
    OS_COUNTRY_KOREA_REPUBLIC_OF,
    OS_COUNTRY_KOSOVO,
    OS_COUNTRY_KUWAIT,
    OS_COUNTRY_KYRGYZSTAN,
    OS_COUNTRY_LAO_PEOPLES_DEMOCRATIC_REPUBIC,
    OS_COUNTRY_LATVIA,
    OS_COUNTRY_LEBANON,
    OS_COUNTRY_LESOTHO,
    OS_COUNTRY_LIBERIA,
    OS_COUNTRY_LIBYAN_ARAB_JAMAHIRIYA,
    OS_COUNTRY_LIECHTENSTEIN,
    OS_COUNTRY_LITHUANIA,
    OS_COUNTRY_LUXEMBOURG,
    OS_COUNTRY_MACAO,
    OS_COUNTRY_MACEDONIA_FORMER_YUGOSLAV_REPUBLIC_OF,
    OS_COUNTRY_MADAGASCAR,
    OS_COUNTRY_MALAWI,
    OS_COUNTRY_MALAYSIA,
    OS_COUNTRY_MALDIVES,
    OS_COUNTRY_MALI,
    OS_COUNTRY_MALTA,
    OS_COUNTRY_MAN_ISLE_OF,
    OS_COUNTRY_MARTINIQUE,
    OS_COUNTRY_MAURITANIA,
    OS_COUNTRY_MAURITIUS,
    OS_COUNTRY_MAYOTTE,
    OS_COUNTRY_MEXICO,
    OS_COUNTRY_MICRONESIA_FEDERATED_STATES_OF,
    OS_COUNTRY_MOLDOVA_REPUBLIC_OF,
    OS_COUNTRY_MONACO,
    OS_COUNTRY_MONGOLIA,
    OS_COUNTRY_MONTENEGRO,
    OS_COUNTRY_MONTSERRAT,
    OS_COUNTRY_MOROCCO,
    OS_COUNTRY_MOZAMBIQUE,
    OS_COUNTRY_MYANMAR,
    OS_COUNTRY_NAMIBIA,
    OS_COUNTRY_NAURU,
    OS_COUNTRY_NEPAL,
    OS_COUNTRY_NETHERLANDS,
    OS_COUNTRY_NETHERLANDS_ANTILLES,
    OS_COUNTRY_NEW_CALEDONIA,
    OS_COUNTRY_NEW_ZEALAND,
    OS_COUNTRY_NICARAGUA,
    OS_COUNTRY_NIGER,
    OS_COUNTRY_NIGERIA,
    OS_COUNTRY_NORFOLK_ISLAND,
    OS_COUNTRY_NORTHERN_MARIANA_ISLANDS,
    OS_COUNTRY_NORWAY,
    OS_COUNTRY_OMAN,
    OS_COUNTRY_PAKISTAN,
    OS_COUNTRY_PALAU,
    OS_COUNTRY_PANAMA,
    OS_COUNTRY_PAPUA_NEW_GUINEA,
    OS_COUNTRY_PARAGUAY,
    OS_COUNTRY_PERU,
    OS_COUNTRY_PHILIPPINES,
    OS_COUNTRY_POLAND,
    OS_COUNTRY_PORTUGAL,
    OS_COUNTRY_PUETO_RICO,
    OS_COUNTRY_QATAR,
    OS_COUNTRY_REUNION,
    OS_COUNTRY_ROMANIA,
    OS_COUNTRY_RUSSIAN_FEDERATION,
    OS_COUNTRY_RWANDA,
    OS_COUNTRY_SAINT_KITTS_AND_NEVIS,
    OS_COUNTRY_SAINT_LUCIA,
    OS_COUNTRY_SAINT_PIERRE_AND_MIQUELON,
    OS_COUNTRY_SAINT_VINCENT_AND_THE_GRENADINES,
    OS_COUNTRY_SAMOA,
    OS_COUNTRY_SANIT_MARTIN_SINT_MARTEEN,
    OS_COUNTRY_SAO_TOME_AND_PRINCIPE,
    OS_COUNTRY_SAUDI_ARABIA,
    OS_COUNTRY_SENEGAL,
    OS_COUNTRY_SERBIA,
    OS_COUNTRY_SEYCHELLES,
    OS_COUNTRY_SIERRA_LEONE,
    OS_COUNTRY_SINGAPORE,
    OS_COUNTRY_SLOVAKIA,
    OS_COUNTRY_SLOVENIA,
    OS_COUNTRY_SOLOMON_ISLANDS,
    OS_COUNTRY_SOMALIA,
    OS_COUNTRY_SOUTH_AFRICA,
    OS_COUNTRY_SPAIN,
    OS_COUNTRY_SRI_LANKA,
    OS_COUNTRY_SURINAME,
    OS_COUNTRY_SWAZILAND,
    OS_COUNTRY_SWEDEN,
    OS_COUNTRY_SWITZERLAND,
    OS_COUNTRY_SYRIAN_ARAB_REPUBLIC,
    OS_COUNTRY_TAIWAN_PROVINCE_OF_CHINA,
    OS_COUNTRY_TAJIKISTAN,
    OS_COUNTRY_TANZANIA_UNITED_REPUBLIC_OF,
    OS_COUNTRY_THAILAND,
    OS_COUNTRY_TOGO,
    OS_COUNTRY_TONGA,
    OS_COUNTRY_TRINIDAD_AND_TOBAGO,
    OS_COUNTRY_TUNISIA,
    OS_COUNTRY_TURKEY,
    OS_COUNTRY_TURKMENISTAN,
    OS_COUNTRY_TURKS_AND_CAICOS_ISLANDS,
    OS_COUNTRY_TUVALU,
    OS_COUNTRY_UGANDA,
    OS_COUNTRY_UKRAINE,
    OS_COUNTRY_UNITED_ARAB_EMIRATES,
    OS_COUNTRY_UNITED_KINGDOM,
    OS_COUNTRY_UNITED_STATES,
    OS_COUNTRY_UNITED_STATES_REV4,
    OS_COUNTRY_UNITED_STATES_NO_DFS,
    OS_COUNTRY_UNITED_STATES_MINOR_OUTLYING_ISLANDS,
    OS_COUNTRY_URUGUAY,
    OS_COUNTRY_UZBEKISTAN,
    OS_COUNTRY_VANUATU,
    OS_COUNTRY_VENEZUELA,
    OS_COUNTRY_VIET_NAM,
    OS_COUNTRY_VIRGIN_ISLANDS_BRITISH,
    OS_COUNTRY_VIRGIN_ISLANDS_US,
    OS_COUNTRY_WALLIS_AND_FUTUNA,
    OS_COUNTRY_WEST_BANK,
    OS_COUNTRY_WESTERN_SAHARA,
    OS_COUNTRY_WORLD_WIDE_XX,
    OS_COUNTRY_YEMEN,
    OS_COUNTRY_ZAMBIA,
    OS_COUNTRY_ZIMBABWE,
    OS_COUNTRY_UNKNOWN
} os_country_code_t;

struct os_wlan_device;
struct os_wlan_buff;

typedef void (*os_wlan_dev_event_handler)(struct os_wlan_device *device,
                                          os_wlan_dev_event_t    event,
                                          struct os_wlan_buff   *buff,
                                          void                  *parameter);

typedef void (*os_wlan_pormisc_callback_t)(struct os_wlan_device *device, void *data, int len);

typedef void (*os_wlan_mgnt_filter_callback_t)(struct os_wlan_device *device, void *data, int len);

struct os_wlan_ssid
{
    os_uint8_t len;
    os_uint8_t val[OS_WLAN_SSID_MAX_LENGTH + 1];
};
typedef struct os_wlan_ssid os_wlan_ssid_t;

struct os_wlan_key
{
    os_uint8_t len;
    os_uint8_t val[OS_WLAN_PASSWORD_MAX_LENGTH + 1];
};
typedef struct os_wlan_key os_wlan_key_t;

#define INVALID_INFO(_info)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        memset((_info), 0, sizeof(struct os_wlan_info));                                                               \
        (_info)->band     = OS_802_11_BAND_UNKNOWN;                                                                    \
        (_info)->security = SECURITY_UNKNOWN;                                                                          \
        (_info)->channel  = -1;                                                                                        \
    } while (0)

#define SSID_SET(_info, _ssid)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        strncpy((char *)(_info)->ssid.val, (_ssid), OS_WLAN_SSID_MAX_LENGTH);                                          \
        (_info)->ssid.len = strlen((char *)(_info)->ssid.val);                                                         \
    } while (0)

struct os_wlan_info
{
    /* security type */
    os_wlan_security_t security;
    /* 2.4G/5G */
    os_802_11_band_t band;
    /* maximal data rate */
    os_uint32_t datarate;
    /* radio channel */
    os_int16_t channel;
    /* signal strength */
    os_int16_t rssi;
    /* ssid */
    os_wlan_ssid_t ssid;
    /* hwaddr */
    os_uint8_t bssid[OS_WLAN_BSSID_MAX_LENGTH];
    os_uint8_t hidden;
};

struct os_wlan_buff
{
    void      *data;
    os_int32_t len;
};

struct os_filter_pattern
{
    os_uint16_t offset;    /* Offset in bytes to start filtering (referenced to the start of the ethernet packet) */
    os_uint16_t mask_size; /* Size of the mask in bytes */
    os_uint8_t *mask; /* Pattern mask bytes to be ANDed with the pattern eg. "\xff00" (must be in network byte order) */
    os_uint8_t *pattern; /* Pattern bytes used to filter eg. "\x0800"  (must be in network byte order) */
};

typedef enum
{
    OS_POSITIVE_MATCHING = 0, /* Receive the data matching with this pattern and discard the other data  */
    OS_NEGATIVE_MATCHING = 1  /* Discard the data matching with this pattern and receive the other data */
} os_filter_rule_t;

struct os_wlan_filter
{
    struct os_filter_pattern patt;
    os_filter_rule_t         rule;
    os_uint8_t               enable;
};

struct os_wlan_dev_event_desc
{
    os_wlan_dev_event_handler handler;
    void                     *parameter;
};

struct os_wlan_device
{
    struct os_device               device;
    os_wlan_mode_t                 mode;
    struct os_mutex                lock;
    struct os_wlan_dev_event_desc  handler_table[OS_WLAN_DEV_EVT_MAX][OS_WLAN_DEV_EVENT_NUM];
    os_wlan_pormisc_callback_t     pormisc_callback;
    os_wlan_mgnt_filter_callback_t mgnt_filter_callback;
    const struct os_wlan_dev_ops  *ops;
    os_uint32_t                    flags;
    void                          *prot;
    void                          *user_data;
};

struct os_sta_info
{
    os_wlan_ssid_t     ssid;
    os_wlan_key_t      key;
    os_uint8_t         bssid[6];
    os_uint16_t        channel;
    os_wlan_security_t security;
};

struct os_ap_info
{
    os_wlan_ssid_t     ssid;
    os_wlan_key_t      key;
    os_bool_t          hidden;
    os_uint16_t        channel;
    os_wlan_security_t security;
};

struct os_scan_info
{
    os_wlan_ssid_t ssid;
    os_uint8_t     bssid[6];
    os_int16_t     channel_min;
    os_int16_t     channel_max;
    os_bool_t      passive;
};

struct os_wlan_dev_ops
{
    os_err_t (*wlan_init)(struct os_wlan_device *wlan);
    os_err_t (*wlan_mode)(struct os_wlan_device *wlan, os_wlan_mode_t mode);
    os_err_t (*wlan_scan)(struct os_wlan_device *wlan, struct os_scan_info *scan_info);
    os_err_t (*wlan_join)(struct os_wlan_device *wlan, struct os_sta_info *sta_info);
    os_err_t (*wlan_softap)(struct os_wlan_device *wlan, struct os_ap_info *ap_info);
    os_err_t (*wlan_disconnect)(struct os_wlan_device *wlan);
    os_err_t (*wlan_ap_stop)(struct os_wlan_device *wlan);
    os_err_t (*wlan_ap_deauth)(struct os_wlan_device *wlan, os_uint8_t mac[]);
    os_err_t (*wlan_scan_stop)(struct os_wlan_device *wlan);
    int (*wlan_get_rssi)(struct os_wlan_device *wlan);
    os_err_t (*wlan_set_powersave)(struct os_wlan_device *wlan, int level);
    int (*wlan_get_powersave)(struct os_wlan_device *wlan);
    os_err_t (*wlan_cfg_promisc)(struct os_wlan_device *wlan, os_bool_t start);
    os_err_t (*wlan_cfg_filter)(struct os_wlan_device *wlan, struct os_wlan_filter *filter);
    os_err_t (*wlan_cfg_mgnt_filter)(struct os_wlan_device *wlan, os_bool_t start);
    os_err_t (*wlan_set_channel)(struct os_wlan_device *wlan, int channel);
    int (*wlan_get_channel)(struct os_wlan_device *wlan);
    os_err_t (*wlan_set_country)(struct os_wlan_device *wlan, os_country_code_t country_code);
    os_country_code_t (*wlan_get_country)(struct os_wlan_device *wlan);
    os_err_t (*wlan_set_mac)(struct os_wlan_device *wlan, os_uint8_t mac[]);
    os_err_t (*wlan_get_mac)(struct os_wlan_device *wlan, os_uint8_t mac[]);
    int (*wlan_recv)(struct os_wlan_device *wlan, void *buff, int len);
    int (*wlan_send)(struct os_wlan_device *wlan, void *buff, int len);
    int (*wlan_send_raw_frame)(struct os_wlan_device *wlan, void *buff, int len);
};

/* wlan device init */
os_err_t os_wlan_dev_init(struct os_wlan_device *device, os_wlan_mode_t mode);

/***************************************wlan device station interface begin*******************************************/
os_err_t os_wlan_dev_connect(struct os_wlan_device *device, struct os_wlan_info *info, const char *password, int password_len);
os_err_t os_wlan_dev_disconnect(struct os_wlan_device *device);
int      os_wlan_dev_get_rssi(struct os_wlan_device *device);
/****************************************wlan device station interface end********************************************/

/******************************************wlan device ap interface begin*********************************************/
os_err_t os_wlan_dev_ap_start(struct os_wlan_device *device, struct os_wlan_info *info, const char *password, int password_len);
os_err_t os_wlan_dev_ap_stop(struct os_wlan_device *device);
os_err_t os_wlan_dev_ap_deauth(struct os_wlan_device *device, os_uint8_t mac[6]);
/*******************************************wlan device ap interface end**********************************************/

/*****************************************wlan device scan interface begin********************************************/
os_err_t os_wlan_dev_scan(struct os_wlan_device *device, struct os_wlan_info *info);
os_err_t os_wlan_dev_scan_stop(struct os_wlan_device *device);
/******************************************wlan device scan interface end*********************************************/

/*****************************************wlan device mac interface begin*********************************************/
os_err_t os_wlan_dev_get_mac(struct os_wlan_device *device, os_uint8_t mac[6]);
os_err_t os_wlan_dev_set_mac(struct os_wlan_device *device, os_uint8_t mac[6]);
/******************************************wlan device mac interface end**********************************************/

/**************************************wlan device powersave interface begin******************************************/
os_err_t os_wlan_dev_set_powersave(struct os_wlan_device *device, int level);
int      os_wlan_dev_get_powersave(struct os_wlan_device *device);
/***************************************wlan device powersave interface end*******************************************/

/****************************************wlan device event interface begin********************************************/
os_err_t os_wlan_dev_register_event_handler(struct os_wlan_device    *device,
                                            os_wlan_dev_event_t       event,
                                            os_wlan_dev_event_handler handler,
                                            void *                    parameter);
os_err_t os_wlan_dev_unregister_event_handler(struct os_wlan_device *   device,
                                              os_wlan_dev_event_t       event,
                                              os_wlan_dev_event_handler handler);
void     os_wlan_dev_indicate_event_handle(struct os_wlan_device *device,
                                           os_wlan_dev_event_t    event,
                                           struct os_wlan_buff   *buff);
/*****************************************wlan device event interface end*********************************************/

/***************************************wlan device promisc interface begin*******************************************/
os_err_t os_wlan_dev_enter_promisc(struct os_wlan_device *device);
os_err_t os_wlan_dev_exit_promisc(struct os_wlan_device *device);
os_err_t os_wlan_dev_set_promisc_callback(struct os_wlan_device *device, os_wlan_pormisc_callback_t callback);
void     os_wlan_dev_promisc_handler(struct os_wlan_device *device, void *data, int len);
/****************************************wlan device promisc interface end********************************************/

/* wlan device filter interface */
os_err_t os_wlan_dev_cfg_filter(struct os_wlan_device *device, struct os_wlan_filter *filter);

/***************************************wlan device channel interface begin*******************************************/
os_err_t os_wlan_dev_set_channel(struct os_wlan_device *device, int channel);
int      os_wlan_dev_get_channel(struct os_wlan_device *device);
/****************************************wlan device channel interface end********************************************/

/***************************************wlan device country interface begin*******************************************/
os_err_t          os_wlan_dev_set_country(struct os_wlan_device *device, os_country_code_t country_code);
os_country_code_t os_wlan_dev_get_country(struct os_wlan_device *device);
/****************************************wlan device country interface end********************************************/

/* wlan device datat transfer interface */
os_err_t os_wlan_dev_report_data(struct os_wlan_device *device, void *buff, int len);

/* wlan device register interface */
os_err_t os_wlan_dev_register(struct os_wlan_device        *wlan,
                              const char                   *name,
                              const struct os_wlan_dev_ops *ops,
                              os_uint32_t                   flag,
                              void                         *user_data);

#ifdef __cplusplus
}
#endif

#endif
