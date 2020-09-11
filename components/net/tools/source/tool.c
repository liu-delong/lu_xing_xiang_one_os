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
 * @file        tool.c
 *
 * @brief       The AT module network debug functions implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <os_kernel.h>
#include <tool.h>

#ifdef OS_USING_SHELL
#include <shell.h>

#define DBG_EXT_TAG "tool"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef NET_USING_MOLINK
#include <mo_api.h>


#if defined(MODULE_USING_IFCONFIG) && defined(MOLINK_USING_NETSERV_OPS) && defined(MOLINK_USING_GENERAL_OPS)
static void intf_status_parser(os_int8_t val, char status[])
{
    switch(val)
    {
    case MO_NET_DETACH:
        strcpy(status, "Module detach from PS");
        break;

    case MO_NET_ATTACH:
        strcpy(status, "Module attach to PS");
        break;

    case MO_NET_EPS_REG_FAIL:
        strcpy(status, "Module reg EPS fail");
        break;

    case MO_NET_EPS_REG_OK:
        strcpy(status, "Module reg EPS OK");
        break;

    case MO_NET_DEACTIVATED:
        strcpy(status, "Module deactivate PDP context");
        break;
    
    case MO_NET_ACTIVATED:
        strcpy(status, "Module activate PDP context");
        break;

    case MO_NET_NETWORK_REG_OK:
        strcpy(status, "Module network reg success");
        break;

    case MO_NET_NETWORK_REG_FAIL:
        strcpy(status, "Module network reg fail");
        break;

    default:
        os_kprintf("AT intf status parser fail: status type[0x%02x] error", val);
        strcpy(status, "Unknown status");
        break;
    }

    return;
}

void at_module_show_info(void)
{
    mo_object_t  *defmo_obj  = OS_NULL;
    radio_info_t  radio_info = {0};
    os_uint16_t   i          = 0;
    os_uint8_t    stat_val   = 0;
    os_uint8_t    rssi       = 0;
    os_uint8_t    ber        = 0;

    char imei[16]            = {0};
    char ipaddr[16]          = {0}; 
    char status[32]          = {0};

    /* Step 1: get atintf obj and ops interface */
    defmo_obj = mo_get_default();

    /* Step 2: get IMEI number */
    if (mo_get_imei(defmo_obj, imei, sizeof(imei)) != OS_EOK)
    {
        LOG_EXT_D("AT tool: get IMEI number fail\n");
    }
    
    /* Step 3: get network signal quality */
    if (mo_get_csq(defmo_obj, &rssi, &ber) != OS_EOK)
    {
        LOG_EXT_D("AT tool: get network signal quality fail\n");
    }

    /* Step 4: get radio information */
    if (mo_get_radio(defmo_obj, &radio_info) != OS_EOK)
    {
        LOG_EXT_D("AT tool: get radio information fail\n");
    }
    
    /* Step 5: get atintf network status */
    if (mo_get_netstat(defmo_obj, &stat_val) != OS_EOK)
    {
        LOG_EXT_D("AT tool: get netstat fail\n");
    }
    /* Always parser the status, even if it fails */
    intf_status_parser(stat_val, status);
    
    /* Step 6: get atintf IP address */
    if (mo_get_ipaddr(defmo_obj, ipaddr) != OS_EOK)
    {
        LOG_EXT_D("AT tool: get ipaddress fail\n");
    }

    /* TODO: get atintf other info and show to user if necessary */

    os_kprintf("\nLIST AT MODULE INFORMATIONS\n");
    for (i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\n");
    os_kprintf("Network Interface Controller: %s\n", defmo_obj->name);
    os_kprintf("IMEI   Number  : %s\n", imei);
    os_kprintf("Signal Quality : rssi(%d), ber(%d)\n", rssi, ber);
    os_kprintf("Network Status : %s\n", status);
    os_kprintf("Intf IP Address: %s\n", strlen(ipaddr) ? ipaddr : "0.0.0.0");
    os_kprintf("Radio  Info    : cell ID(%s), ecl(%d), snr(%d), rsrq(%d)\n",
               radio_info.cell_id,
               radio_info.ecl,
               radio_info.snr,
               radio_info.rsrq);

    for (i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\n");

    return;       
}

int at_module_cmd_ifconfig(int argc, char **argv)
{
    if (argc != 1)
    {
        os_kprintf("Input errror, please input: ifconfig\n");
    }
    else
    {
        at_module_show_info();
    }

    return 0;
}
SH_CMD_EXPORT(mo_ifconfig, at_module_cmd_ifconfig, "List the information of AT module");
#endif /* defined(MODULE_USING_IFCONFIG) && defined(MOLINK_USING_NETSERV_OPS) && defined(MOLINK_USING_GENERAL_OPS) */


#if defined(MODULE_USING_PING) && defined(MOLINK_USING_NETSERV_OPS)
#define AT_PING_DATA_SIZE  (64)
#define AT_PING_TIMES      (4)                       
#define AT_PING_RECV_TIMEO (50 * OS_TICK_PER_SECOND) /* ping recv timeout - in milliseconds */
#define AT_PING_DELAY      (1 * OS_TICK_PER_SECOND)  /* ping delay time interval - in milliseconds */

void at_module_ping(char *target_name, os_uint16_t times, os_uint16_t size)
{
    struct ping_resp resp;

    mo_object_t *defmo_obj = OS_NULL;
    os_tick_t   start_tick = 0;
    os_tick_t   delay_tick = 0;
    os_err_t    ret        = OS_EOK;
    os_uint16_t index      = 0;
    os_uint16_t maxtime    = 0;
    os_uint16_t mintime    = 0xFFFF;
    os_uint16_t recv_cnt   = 0;
    os_bool_t   ping_flag  = OS_FALSE;
	char ip[16] = {0};

    if (size == 0)
    {
        size = AT_PING_DATA_SIZE;
    }
    
    if (times == 0)
    {
        times = AT_PING_TIMES;
    }

    defmo_obj = mo_get_default();
    if (defmo_obj == OS_NULL)
    {
        os_kprintf("Ping: get defmo obj failed, module is not create.\n");
        return;
    }

    /* Step 1: get at intf address, if it failed to get address that means module is not ready for ping */
    if (mo_get_ipaddr(defmo_obj, ip) != OS_EOK)
    {
        os_kprintf("Ping: get at intf address failed, module is not ready for ping\n");
        return;
    }

    /* Step 2: call the ping API of the module */
    for (index = 0; index < times; index++)
    {
        memset(&resp, 0x00, sizeof(struct ping_resp));
        start_tick = os_tick_get();
        ret        = mo_ping(defmo_obj, target_name, size, AT_PING_RECV_TIMEO, &resp);

        /* Parse ping ret and show to user */
        if (ret == OS_ETIMEOUT)
        {
            os_kprintf("Ping: from %s icmp_seq=%d timeout\n",
                       (ip_addr_isany(&(resp.ip_addr))) ? target_name : inet_ntoa(resp.ip_addr),
                       index);
        }
        else if (ret == OS_EOK)
        {
            if (resp.ttl != 0)
            {
                os_kprintf("%d bytes from %s icmp_seq=%d ttl=%d time=%d ms\n",
                           resp.data_len,
                           (ip_addr_isany(&(resp.ip_addr))) ? target_name : inet_ntoa(resp.ip_addr),
                           index,
                           resp.ttl,
                           resp.time);
            }
            else
            {
                os_kprintf("%d bytes from %s icmp_seq=%d time=%d ms\n",
                           resp.data_len,
                           (ip_addr_isany(&(resp.ip_addr))) ? target_name : inet_ntoa(resp.ip_addr),
                           index,
                           resp.time);
            }

            recv_cnt++;
            if ((resp.time > 0) && (resp.time < mintime))
            {
                mintime = resp.time;
            }

            if ((resp.time > 0) && (resp.time > maxtime))
            {
                maxtime = resp.time;
            }
            
            if (ping_flag == OS_FALSE)
            {
                ping_flag = OS_TRUE;
            }
        }
        else
        {
            os_kprintf("Ping: %s %s failed\n",
                       (ip_addr_isany(&(resp.ip_addr))) ? "host" : "address",
                       (ip_addr_isany(&(resp.ip_addr))) ? target_name : inet_ntoa(resp.ip_addr));
        }
        
        /* If the response time is more than AT_PING_DELAY, no nead to delay */
        delay_tick = ((os_tick_get() - start_tick) > AT_PING_DELAY) || (index == times) ? 0 : AT_PING_DELAY;
        os_task_delay(delay_tick);
    }

    os_kprintf("\nPing statistics for %s :\n",
              (ip_addr_isany(&(resp.ip_addr))) ? target_name : inet_ntoa(resp.ip_addr));
    if (ping_flag == OS_TRUE)
    {
        os_kprintf("Packets: Sent = %d, Received = %d, Lost = %d, Mintime = %d ms, Maxtime = %d ms\n", 
               times, 
               recv_cnt, 
               (times - recv_cnt),
               mintime,
               maxtime);
    }
    else
    {
        os_kprintf("Packets: Sent = %d, Received = %d, Lost = %d\n", times, recv_cnt, (times - recv_cnt));
    }
    
    return;
}

/*Note that:  uint32: 0~4294967295 */
static os_uint32_t strnum_to_uint(char *strnum)
{
    os_uint32_t  ret  = 0;
    char        *ptr = OS_NULL;
	
    ptr = strnum;
	while(((*ptr) >= '0') && ((*ptr) <= '9'))
	{
		ret *=10;
		ret += *ptr - '0';
		ptr++;
	}

	return ret;
}

static int at_module_cmd_ping(int argc, char **argv)
{
    os_uint16_t times = 0;
    os_uint16_t size  = 0;
    os_err_t    ret   = OS_EOK;

    switch (argc)
    {
    case 1:
    {
        /* Input: ping */
        os_kprintf("Please input: ping <host address> <times[1-1000]> <pkg_size[64-1500]>\n");
        ret = OS_ERROR;
        break;
    }

    case 2:
    {
        /* Input: ping <host address> */
        at_module_ping(argv[1], 0, 0);
        ret = OS_EOK;
        break;
    }

    case 3:
    {
        /* Input: ping <host address> <times> */
        times = strnum_to_uint(argv[2]);
        if ((times <= 0) || (times > 1000))
        {
            os_kprintf("Input error: ping times is out of range [1-1000], times = %d\n", times);
            ret = OS_ERROR;
            break;
        }

        at_module_ping(argv[1], times, 0);
        ret = OS_EOK;
        break;
    }

    case 4:
    {
        /* Input: ping <host address> <times> <pkg_siz> */
        times = strnum_to_uint(argv[2]);
        if ((times <= 0) || (times > 1000))
        {
            os_kprintf("Input error: ping times is out of range [1-1000], times = %d\n", times);
            ret = OS_ERROR;
            break;
        }
        
        size = strnum_to_uint(argv[3]);

        at_module_ping(argv[1], times, size);
        ret = OS_EOK;
        break;
    }

    default:
        os_kprintf("Input error, please input: ping <host address> <times[1-1000]> <pkg_size>\n");
        ret = OS_ERROR;
        break;
    }

    return ret;
}
SH_CMD_EXPORT(mo_ping, at_module_cmd_ping, "Ping AT network host: ping <host address> <times[1-1000]> <pkg_size>");
#endif /* defined(MODULE_USING_PING) && defined(MOLINK_USING_NETSERV_OPS) */


#if defined(MODULE_USING_SOCKETSTAT) && defined(MOLINK_USING_NETCONN_OPS)
#if 0
static void socket_status_convert_to_char_info(os_uint8_t stat, char status[])
{
    switch(stat)
    {
    case NETCONN_STAT_NULL:
        strcpy(status, "Not create");
        break;

    case NETCONN_STAT_INIT:
        strcpy(status, "Not connect");
        break;

    case NETCONN_STAT_CONNECT:
        strcpy(status, "Connect OK");
        break;

    case NETCONN_STAT_CLOSE:
        strcpy(status, "Closed");
        break;

    default:
        os_kprintf("Convert fail: socket status[0x%02x] unknown", stat);
        strcpy(status, "Unknown");
        break;
    }

    return;
}

static void socket_type_convert_to_char_info(os_uint8_t type, char type_info[])
{
    switch(type)
    {
    case NETCONN_TYPE_TCP:
        strcpy(type_info, "TCP");
        break;

    case NETCONN_TYPE_UDP:
        strcpy(type_info, "UDP");
        break;

    default:
        os_kprintf("Convert fail: socket type[0x%02x] error", type);
        strcpy(type_info, "Unknown");
        break;
    }

    return;
}
#endif

void at_module_show_socket_stat(void)
{   
    mo_object_t *defmo_obj       = OS_NULL;
    os_uint8_t   i               = 0;
    //char         socket_type[8]  = {0};
    //char         socket_stat[32] = {0};
    char         ipaddr[32]      = {0};

    /* Get at intf obj, if it failed to get address that means module sockets are not ready */
    defmo_obj = mo_get_default();
    if (defmo_obj == OS_NULL)
    {
        os_kprintf("Show socket status: get defmo obj failed, module is not create.\n");
        return;
    }

    /* Get at intf address, if it failed to get address that means module is not ready for ping */
    if (mo_get_ipaddr(defmo_obj, ipaddr) != OS_EOK)
    {
        os_kprintf("Show socket status:: get at intf address failed, module is not ready to provide socket service\n");
        return;
    }

    /* Show socket information */
    os_kprintf("\nThe max socket connections supported by module %s is xxx\n", defmo_obj->name);
    os_kprintf("\nThe connected socket status information list\n");
    for (i = 0; i < 50; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\n");

    os_kprintf("%-8s", "socket");
    os_kprintf("%-6s", "type");
    os_kprintf("%-18s", "ip address");
    os_kprintf("%-8s", "port");
    os_kprintf("%-16s\n", "status");

#if 0
    for (i = 0; i < MAX_SOCKETS_NUM; i++)
    {
        if (mo_netconn[i] != OS_NULL)
        {
            socket_type_convert_to_char_info(mo_netconn[i].type, socket_type);
            socket_status_convert_to_char_info(mo_netconn[i].stat, socket_stat);

            os_kprintf("%-8d", mo_netconn[i].connect_id);
            os_kprintf("%-6s", socket_type);
            os_kprintf("%-18s", net_ip4addr_ntoa(mo_netconn[i].ipaddr));
            os_kprintf("%-8d", mo_netconn[i].port);
            os_kprintf("%-16s\n", socket_stat);
        }  
    }
#endif

    for (i = 0; i < 50; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\n");

    return;
}

static int at_module_cmd_socket_stat(int argc, char **argv)
{
    if (argc != 1)
    {
        os_kprintf("Input errror, please input: socket_stat\n");
    }
    else
    {
        at_module_show_socket_stat();
    }

    return 0;
}
SH_CMD_EXPORT(mo_socketstat, at_module_cmd_socket_stat, "List the informations of at module sockets status");
#endif /* defined(MODULE_USING_SOCKETSTAT) && defined(MOLINK_USING_NETCONN_OPS) */

#endif /* NET_USING_MOLINK */
#endif /* OS_USING_SHELL */
