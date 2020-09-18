#include <os_kernel.h>
#include <shell.h>
#include "onepos_interface.h"

#define _isdigit(argv)          ((((*argv) - '0') < 10u) && (((*argv) - '0') >= 0u))

struct onepos_cmd_des
{
    const char *cmd;
    int (*fun)(int argc, char *argv[]);
};

static int onepos_status_cmd_func(int argc, char *argv[])
{
    char sta_str[ONEPOS_MAX_STA][30] = {
                                            "closing",
                                            "runing",
                                            "sig_sev_runing",
                                            "will_close"};

    if(2 == argc)   //  read
    {
        os_kprintf("status : %s\n", sta_str[onepos_get_server_sta()]);
    }
    else
    {
        os_kprintf("this cmd do not support input param\n");
    }

    return 0;
}

static int onepos_interval_cmd_func(int argc, char *argv[])
{
    if(2 == argc)   //  read
    {
        os_kprintf("interval : %d sec\n", onepos_get_pos_interval());
    }
    else if(3 == argc && _isdigit(argv[2]))
    {
        onepos_set_pos_interval(atoi(argv[2]));
    }
    else
    {
        os_kprintf("this cmd do not support input format, shoud be (onepos interval <secodes>)\n");
    }

    return 0;
}

static int onepos_mode_cmd_func(int argc, char *argv[])
{
    char mode_str[ONEPOS_MAX_POS_MODE][15] = {
                                            "invaild",
                                            "single_wifi",
                                            "mul_wifis",
                                            "mul_cells"};

    if(2 == argc)   //  read
    {
        os_kprintf("mode : %s\n", mode_str[onepos_get_pos_mode()]);
    }
    else if(3 == argc && IS_VALID_POS_MODE(atoi(argv[2])))  //set
    {
        onepos_set_pos_mode((onepos_pos_mode_t)atoi(argv[2]));
    }
    else
    {
        os_kprintf("this cmd do not support input format, shoud be (onepos mode <1~3>)\n");
    }

    return 0;
}

static int onepos_sev_pro_cmd_func(int argc, char *argv[])
{
    char pro_str[ONEPOS_MAX_SEV_PRO][15] = {
                                            "own",
                                            "onenet" };

    if(2 == argc)   //  read
    {
        os_kprintf("sev_pro : %s\n", pro_str[onepos_get_sev_pro()]);
    }
    else if(3 == argc && IS_VAILD_SEV_PRO(atoi(argv[2])))  // set
    {
        onepos_set_sev_pro((onepos_sev_pro_t)atoi(argv[2]));
    }
    else
    {
        os_kprintf("this cmd do not support input format, shoud be (onepos sev_pro <0/1>)\n");
    }

    return 0;
}


static int onepos_start_cmd_func(int argc, char *argv[])
{
    if(ONEPOS_CLOSING == onepos_get_server_sta())
    {
        onepos_start_server();
    }
    else if(ONEPOS_WILL_CLOSE == onepos_get_server_sta())
    {
        os_kprintf("onepos is will close, pls wait ...\n");
    }
    else
    {
        os_kprintf("onepos is already runing!\n");
    }

    return 0;
}

static int onepos_stop_cmd_func(int argc, char *argv[])
{
    if(ONEPOS_RUNING == onepos_get_server_sta())
    {
        onepos_stop_server();
    }
    else if(ONEPOS_SIG_RUNING == onepos_get_server_sta())
    {
        os_kprintf("onepos single server is runing, it will automatic stop!\n");
    }
    else if(ONEPOS_WILL_CLOSE == onepos_get_server_sta())
    {
        os_kprintf("onepos is will close, pls wait ...\n");
    }
    else
    {
        os_kprintf("onepos is already closed!\n");
    }

    return 0;
}

static int onepos_get_latest_position_cmd_func(int argc, char* argv[])
{
    ops_sigle_wifi_info_t    wifi_info;
    ops_platform_wifi_info_t wifis_info;
    ops_platform_lbs_info_t  lbs_info;
    onepos_pos_mode_t        mode = onepos_get_pos_mode();
    ops_src_info_t           onepos_src_info = {
                                                &wifi_info,
                                                &wifis_info,
                                                &lbs_info};
                                                
    memset(&wifi_info, 0, sizeof(ops_sigle_wifi_info_t));
    memset(&wifis_info, 0, sizeof(ops_platform_wifi_info_t));
    memset(&lbs_info, 0, sizeof(ops_platform_lbs_info_t));
    
    onepos_get_latest_position(&onepos_src_info);
    onepos_info_print(&onepos_src_info, mode);

    return 0;
}

static int onepos_server_type_cmd_func(int argc, char *argv[])
{
    char server_type_str[ONEPOS_MAX_TYPE][15] = {
                                            "circulation",
                                            "single"
                                            };

    if(2 == argc)   //  read
    {
        os_kprintf("server_type : %s\n", server_type_str[onepos_get_server_type()]);
    }
    else if(3 == argc && IS_VAILD_SEV_TYPE(atoi(argv[2])))  // set
    {
        onepos_set_server_type((onepos_serv_type)atoi(argv[2]));
    }
    else
    {
        os_kprintf("this cmd do not support input format, shoud be (onepos server_type <1/2>)\n");
    }

    return 0;
}

static int onepos_test_cmd_func(int argc, char *argv[])
{
    printf("%s entry!\n", __func__);
    return 0;
}

#if defined(OS_USING_GNSS_POS)
static int onepos_gnss_pos_cmd_func(int argc, char *argv[])
{
    nmea_t nmea_data;
    char   temp_str[256];
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
static const struct onepos_cmd_des onepos_cmd_tab[] =
{
    {"status",   onepos_status_cmd_func},
    {"interval", onepos_interval_cmd_func},
    {"mode", onepos_mode_cmd_func},
    {"sev_pro", onepos_sev_pro_cmd_func},
    {"sev_type", onepos_server_type_cmd_func},
    {"start", onepos_start_cmd_func},
    {"stop", onepos_stop_cmd_func},
    {"pos", onepos_get_latest_position_cmd_func},    
    #if defined(OS_USING_GNSS_POS)
    {"gnss_pos", onepos_gnss_pos_cmd_func},
    #endif
    {"test", onepos_test_cmd_func}
};

static int onepos_help(int argc, char *argv[])
{
    os_kprintf("onepos\n");
    os_kprintf("onepos help\n");
    os_kprintf("onepos status <0/1(0:closing;1:runing)>\n");
    os_kprintf("onepos interval <second(>=3)>\n");
    os_kprintf("onepos mode <1~3(1:single wifi;2:multiple wifis;3:multiple cells)>\n");
    os_kprintf("onepos sev_pro <0/1(0:own;1:onenet)> \n");
    os_kprintf("onepos sev_type <0/1(0:circ;1:single)> \n");
    os_kprintf("onepos start\n");
    os_kprintf("onepos stop\n");
    os_kprintf("onepos pos\n");    
    #if defined(OS_USING_GNSS_POS)
    os_kprintf("onepos gnss_pos\n");
    #endif
    os_kprintf("onepos test\n");
    return 0;
}

static int onepos_cmd(int argc, char *argv[])
{
    int i;
    int result = 0;
    
    const struct onepos_cmd_des *run_cmd = OS_NULL;

    if (argc == 1)
    {
        onepos_help(argc, argv);
        return 0;
    }

    /* find fun */
    for (i = 0; i < sizeof(onepos_cmd_tab) / sizeof(onepos_cmd_tab[0]); i++)
    {
        if (strcmp(onepos_cmd_tab[i].cmd, argv[1]) == 0)
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
