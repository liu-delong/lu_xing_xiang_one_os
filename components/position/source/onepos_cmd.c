#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_kernel.h>
#include <shell.h>
#include "onepos_common.h"
struct onepos_cmd_des
{
    const os_int8_t *cmd;
    os_int32_t (*fun)(os_int32_t argc, os_int8_t *argv[]);
};

#if defined(ONEPOS_WIFI_POS) || defined(ONEPOS_CELL_POS)
#include "onepos_interface.h"

static os_int32_t onepos_status_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    os_int8_t sta_str[ONEPOS_MAX_STA][30] = {"closing", "runing", "sig_sev_runing", "will_close"};

    if (2 == argc)    //  read
    {
        os_kprintf("status : %s\n", sta_str[onepos_get_server_sta()]);
    }
    else
    {
        os_kprintf("this cmd do not support input param\n");
    }

    return 0;
}

static os_int32_t onepos_interval_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    if (2 == argc)    //  read
    {
        os_kprintf("interval : %d sec\n", onepos_get_pos_interval());
    }
    else if (3 == argc && _isdigit(argv[2]))
    {
        onepos_set_pos_interval(atoi((char*)(char*)argv[2]));
    }
    else
    {
        os_kprintf("this cmd do not support input format, shoud be (onepos interval <secodes>)\n");
    }

    return 0;
}

static os_int32_t onepos_start_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    if (ONEPOS_CLOSING == onepos_get_server_sta())
    {
        onepos_start_server();
    }
    else if (ONEPOS_WILL_CLOSE == onepos_get_server_sta())
    {
        os_kprintf("onepos is will close, pls wait ...\n");
    }
    else
    {
        os_kprintf("onepos is already runing!\n");
    }

    return 0;
}

static os_int32_t onepos_stop_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    if (ONEPOS_RUNING == onepos_get_server_sta())
    {
        onepos_stop_server();
    }
    else if (ONEPOS_SIG_RUNING == onepos_get_server_sta())
    {
        os_kprintf("onepos single server is runing, it will automatic stop!\n");
    }
    else if (ONEPOS_WILL_CLOSE == onepos_get_server_sta())
    {
        os_kprintf("onepos is will close, pls wait ...\n");
    }
    else
    {
        os_kprintf("onepos is already closed!\n");
    }

    return 0;
}

static os_int32_t onepos_get_latest_position_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    onepos_pos_t pos_info = {0};

    memset(&pos_info, 0, sizeof(onepos_pos_t));

    onepos_get_latest_position(&pos_info);
    onepos_info_print(&pos_info);

    return 0;
}

static os_int32_t onepos_server_type_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    os_int8_t server_type_str[ONEPOS_MAX_TYPE][15] = {"circulation", "single"};

    if (2 == argc)    //  read
    {
        os_kprintf("server_type : %s\n", server_type_str[onepos_get_server_type()]);
    }
    else if (3 == argc && IS_VAILD_SEV_TYPE(atoi((char*)argv[2])))    // set
    {
        onepos_set_server_type((onepos_serv_type_t)atoi((char*)argv[2]));
    }
    else
    {
        os_kprintf("this cmd do not support input format, shoud be (onepos sev_type <0/1>)\n");
    }

    return 0;
}
#endif

static os_int32_t onepos_test_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    printf("%s entry!\n", __func__);
    return 0;
}
#if defined(ONEPOS_GNSS_POS)
#include "nmea_0183.h"
static os_int32_t onepos_gnss_pos_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    nmea_t nmea_data;
    os_int8_t temp_str[256];
    memset(&nmea_data, 0, sizeof(nmea_t));
    memset(temp_str, 0, sizeof(temp_str));

    if (get_gnss_data(&nmea_data, GNSS_RMC_DATA_FLAG))
    {
        os_kprintf("one_position_test get data:\n");
        os_kprintf("\t date : 20%02d - %d - %d\n",
                   nmea_data.rmc_frame.date.year,
                   nmea_data.rmc_frame.date.month,
                   nmea_data.rmc_frame.date.day);
        os_kprintf("\t time : %d : %d : %d : %d\n",
                   nmea_data.rmc_frame.time.hours,
                   nmea_data.rmc_frame.time.minutes,
                   nmea_data.rmc_frame.time.seconds,
                   nmea_data.rmc_frame.time.microseconds);
        os_kprintf("\t latitude : %lld, %d\n",
                   nmea_data.rmc_frame.latitude.value,
                   nmea_data.rmc_frame.latitude.dec_len);
        os_kprintf("\t longitude : %lld, %d\n",
                   nmea_data.rmc_frame.longitude.value,
                   nmea_data.rmc_frame.longitude.dec_len);
        os_kprintf("\t speed : %lld, %d\n", nmea_data.rmc_frame.speed.value, nmea_data.rmc_frame.speed.dec_len);
    }
    else
    {
        os_kprintf("get_gnss_data Error\n");
    }
    return 0;
}
#endif

/* cmd table */
static const struct onepos_cmd_des onepos_cmd_tab[] = {
#if defined(ONEPOS_WIFI_POS) || defined(ONEPOS_CELL_POS)
                                                       {"status", onepos_status_cmd_func},
                                                       {"interval", onepos_interval_cmd_func},
                                                       {"sev_type", onepos_server_type_cmd_func},
                                                       {"start", onepos_start_cmd_func},
                                                       {"stop", onepos_stop_cmd_func},
                                                       {"pos", onepos_get_latest_position_cmd_func},
#endif                                                       
#if defined(ONEPOS_GNSS_POS)
                                                       {"gnss_pos", onepos_gnss_pos_cmd_func},
#endif
                                                       {"test", onepos_test_cmd_func}};

static os_int32_t onepos_help(os_int32_t argc, os_int8_t *argv[])
{
#if defined(ONEPOS_WIFI_POS) || defined(ONEPOS_CELL_POS)
    os_kprintf("onepos\n");
    os_kprintf("onepos help\n");
    os_kprintf("onepos status <0/1(0:closing;1:runing)>\n");
    os_kprintf("onepos interval <second(>=3)>\n");
    os_kprintf("onepos sev_type <0/1(0:circ;1:single)> \n");
    os_kprintf("onepos start\n");
    os_kprintf("onepos stop\n");
    os_kprintf("onepos pos\n");
#endif
#if defined(ONEPOS_GNSS_POS)
    os_kprintf("onepos gnss_pos\n");
#endif
    os_kprintf("onepos test\n");
    return 0;
}

static os_int32_t onepos_cmd(os_int32_t argc, os_int8_t *argv[])
{
    os_int32_t i;
    os_int32_t result = 0;

    const struct onepos_cmd_des *run_cmd = OS_NULL;

    if (argc == 1)
    {
        onepos_help(argc, argv);
        return 0;
    }

    /* find fun */
    for (i = 0; i < sizeof(onepos_cmd_tab) / sizeof(onepos_cmd_tab[0]); i++)
    {
        if (strcmp((const char*)onepos_cmd_tab[i].cmd, (const char*)argv[1]) == 0)
        {
            run_cmd = &onepos_cmd_tab[i];
            break;
        }
    }

    /* not find fun, print help */
    if (run_cmd == OS_NULL)
    {
        onepos_help(argc, argv);
        return 0;
    }

    /* run fun */
    if (run_cmd->fun != OS_NULL)
    {
        result = run_cmd->fun(argc, argv);
    }

    if (result)
    {
        onepos_help(argc, argv);
    }
    return 0;
}

SH_CMD_EXPORT(onepos, onepos_cmd, "onepos command.");
