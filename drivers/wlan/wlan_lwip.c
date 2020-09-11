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
 * @file        wlan_lwip.c
 *
 * @brief       wlan_lwip
 *
 * @details     wlan_lwip
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_hw.h>
#include <os_task.h>
#include <os_assert.h>
#include <os_memory.h>
#include <os_clock.h>
#include <os_errno.h>
#include <string.h>
#include <wlan/wlan_dev.h>
#include <wlan/wlan_prot.h>
#include <wlan/wlan_workqueue.h>

#if defined(OS_WLAN_PROT_ENABLE) && defined(OS_WLAN_PROT_LWIP_ENABLE)

#ifdef NET_USING_LWIP
#include <netif/ethernetif.h>
#include <lwip/netifapi.h>
#ifdef LWIP_USING_DHCPD
#include <dhcp_server.h>
#endif

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "wlan.lwip"
#include <drv_log.h>

#ifndef IPADDR_STRLEN_MAX
#define IPADDR_STRLEN_MAX (32)
#endif

#ifndef OS_WLAN_PROT_LWIP_NAME
#define OS_WLAN_PROT_LWIP_NAME ("lwip")
#endif

struct lwip_prot_des
{
    struct os_wlan_prot prot;
    struct eth_device   eth;
    os_int8_t           connected_flag;
    struct os_timer     timer;
    struct os_work      work;
};

static void netif_is_ready(struct os_work *work, void *data)
{
    ip_addr_t              ip_addr_zero = {0};
    struct os_wlan_device *wlan         = (struct os_wlan_device *)data;
    struct lwip_prot_des  *lwip_prot    = (struct lwip_prot_des *)wlan->prot;
    struct eth_device     *eth_dev;
    os_base_t              level;
    struct os_wlan_buff    buff;
    os_uint32_t            ip_addr[4];
    char                   str[IPADDR_STRLEN_MAX];

    if (lwip_prot == OS_NULL)
        return;

    eth_dev = &lwip_prot->eth;
    os_timer_stop(&lwip_prot->timer);
    if (ip_addr_cmp(&(eth_dev->netif->ip_addr), &ip_addr_zero) != 0)
    {
        os_timer_start(&lwip_prot->timer);
        goto exit;
    }
    memset(&ip_addr, 0, sizeof(ip_addr));
#if LWIP_IPV4 && LWIP_IPV6
    if (eth_dev->netif->ip_addr.type == IPADDR_TYPE_V4)
    {
        ip_addr[0] = ip4_addr_get_u32(&eth_dev->netif->ip_addr.u_addr.ip4);
        buff.data  = &ip_addr[0];
        buff.len   = sizeof(ip_addr[0]);
    }
    else if (eth_dev->netif->ip_addr.type == IPADDR_TYPE_V6)
    {
        *(ip6_addr_t *)(&ip_addr[0]) = eth_dev->netif->ip_addr.u_addr.ip6;
        buff.data                    = ip_addr;
        buff.len                     = sizeof(ip_addr);
    }
    else
    {
        LOG_EXT_W("F:%s L:%d ip addr type not support", __FUNCTION__, __LINE__);
    }
#else
#if LWIP_IPV4
    ip_addr[0] = ip4_addr_get_u32(&eth_dev->netif->ip_addr);
    buff.data  = &ip_addr[0];
    buff.len   = sizeof(ip_addr[0]);
#else
    *(ip_addr_t *)(&ip_addr[0]) = eth_dev->netif->ip_addr;
    buff.data                   = ip_addr;
    buff.len                    = sizeof(ip_addr);
#endif
#endif
    if (os_wlan_prot_ready(wlan, &buff) != 0)
    {
        os_timer_start(&lwip_prot->timer);
        goto exit;
    }
    memset(str, 0, IPADDR_STRLEN_MAX);
    os_enter_critical();
    memcpy(str, ipaddr_ntoa(&(eth_dev->netif->ip_addr)), IPADDR_STRLEN_MAX);
    os_exit_critical();
    LOG_EXT_I("Got IP address : %s", str);
exit:
    level = os_hw_interrupt_disable();
    os_hw_interrupt_enable(level);
}

static void timer_callback(void *parameter)
{
#ifdef OS_WLAN_WORK_TASK_ENABLE
    struct os_workqueue   *workqueue;
    struct os_wlan_device *wlan      = parameter;
    struct lwip_prot_des  *lwip_prot = (struct lwip_prot_des *)wlan->prot;
    struct os_work        *work;
    os_base_t              level;

    if (lwip_prot == OS_NULL)
        return;

    work      = &lwip_prot->work;
    workqueue = os_wlan_get_workqueue();
    if (workqueue != OS_NULL)
    {
        level = os_hw_interrupt_disable();
        os_work_init(work, netif_is_ready, parameter);
        os_hw_interrupt_enable(level);
        if (os_workqueue_submit_work(workqueue, work, 0) != OS_EOK)
        {
            level = os_hw_interrupt_disable();
            memset(work, 0, sizeof(struct os_work));
            os_hw_interrupt_enable(level);
        }
    }
#else
    netif_is_ready(OS_NULL, parameter);
#endif
}

static void netif_set_connected(void *parameter)
{
    struct os_wlan_device *wlan      = parameter;
    struct lwip_prot_des  *lwip_prot = wlan->prot;
    struct eth_device     *eth_dev;

    if (lwip_prot == OS_NULL)
        return;

    eth_dev = &lwip_prot->eth;

    if (lwip_prot->connected_flag)
    {
        if (wlan->mode == OS_WLAN_STATION)
        {
            LOG_EXT_D("F:%s L:%d dhcp start run", __FUNCTION__, __LINE__);
            netifapi_netif_common(eth_dev->netif, netif_set_link_up, NULL);
#ifdef LWIP_USING_DHCP
            dhcp_start(eth_dev->netif);
#endif
            os_timer_start(&lwip_prot->timer);
        }
        else if (wlan->mode == OS_WLAN_AP)
        {
            LOG_EXT_D("F:%s L:%d dhcpd start run", __FUNCTION__, __LINE__);

            netifapi_netif_common(eth_dev->netif, netif_set_link_up, NULL);
#ifdef LWIP_USING_DHCPD
            {
                char netif_name[OS_NAME_MAX];

                memset(netif_name, 0, sizeof(netif_name));
                os_memcpy(netif_name, eth_dev->netif->name, sizeof(eth_dev->netif->name));
                dhcpd_start(netif_name);
            }
#endif
        }
    }
    else
    {
        LOG_EXT_D("F:%s L:%d set linkdown", __FUNCTION__, __LINE__);
        netifapi_netif_common(eth_dev->netif, netif_set_link_down, NULL);
        os_timer_stop(&lwip_prot->timer);
#ifdef LWIP_USING_DHCP
        {
            ip_addr_t ip_addr = {0};
            dhcp_stop(eth_dev->netif);
            netif_set_addr(eth_dev->netif, &ip_addr, &ip_addr, &ip_addr);
        }
#endif
#ifdef LWIP_USING_DHCPD
        {
            char netif_name[OS_NAME_MAX];
            memset(netif_name, 0, sizeof(netif_name));
            os_memcpy(netif_name, lwip_prot->eth.netif->name, sizeof(lwip_prot->eth.netif->name));
            dhcpd_stop(netif_name);
        }
#endif
    }
}

static void os_wlan_lwip_event_handle(struct os_wlan_prot *port, struct os_wlan_device *wlan, int event)
{
    struct lwip_prot_des *lwip_prot = (struct lwip_prot_des *)wlan->prot;
    os_bool_t             flag_old;

    if (lwip_prot == OS_NULL)
        return;

    flag_old = lwip_prot->connected_flag;

    switch (event)
    {
    case OS_WLAN_PROT_EVT_CONNECT:
    {
        LOG_EXT_D("event: CONNECT");
        lwip_prot->connected_flag = OS_TRUE;
        break;
    }
    case OS_WLAN_PROT_EVT_DISCONNECT:
    {
        LOG_EXT_D("event: DISCONNECT");
        lwip_prot->connected_flag = OS_FALSE;
        break;
    }
    case OS_WLAN_PROT_EVT_AP_START:
    {
        LOG_EXT_D("event: AP_START");
        lwip_prot->connected_flag = OS_TRUE;
        break;
    }
    case OS_WLAN_PROT_EVT_AP_STOP:
    {
        LOG_EXT_D("event: AP_STOP");
        lwip_prot->connected_flag = OS_FALSE;
        break;
    }
    case OS_WLAN_PROT_EVT_AP_ASSOCIATED:
    {
        LOG_EXT_D("event: ASSOCIATED");
        break;
    }
    case OS_WLAN_PROT_EVT_AP_DISASSOCIATED:
    {
        LOG_EXT_D("event: DISASSOCIATED");
        break;
    }
    default:
    {
        LOG_EXT_D("event: UNKNOWN");
        break;
    }
    }
    if (flag_old != lwip_prot->connected_flag)
    {
#ifdef OS_WLAN_WORK_TASK_ENABLE
        os_wlan_workqueue_dowork(netif_set_connected, wlan);
#else
        netif_set_connected(wlan);
#endif
    }
}

static os_err_t os_wlan_lwip_protocol_control(os_device_t *device, int cmd, void *args)
{
    struct eth_device     *eth_dev = (struct eth_device *)device;
    struct os_wlan_device *wlan;
    os_err_t               err = OS_EOK;

    OS_ASSERT(eth_dev != OS_NULL);

    LOG_EXT_D("F:%s L:%d device:0x%08x user_data:0x%08x", __FUNCTION__, __LINE__, eth_dev, eth_dev->parent.user_data);

    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get MAC address */
        wlan = eth_dev->parent.user_data;
        err  = os_device_control((os_device_t *)wlan, OS_WLAN_CMD_GET_MAC, args);
        break;
    default:
        break;
    }
    return err;
}

static os_err_t os_wlan_lwip_protocol_recv(struct os_wlan_device *wlan, void *buff, int len)
{
    struct pbuf *p     = OS_NULL;
    
    struct eth_device *eth_dev = &((struct lwip_prot_des *)wlan->prot)->eth;    

    LOG_EXT_D("F:%s L:%d run", __FUNCTION__, __LINE__);

    if (eth_dev == OS_NULL)
    {
        return OS_ERROR;
    }
#ifdef OS_WLAN_PROT_LWIP_PBUF_FORCE
    {
        p = buff;
        if ((eth_dev->netif->input(p, eth_dev->netif)) != ERR_OK)
        {
            return OS_ERROR;
        }
        return OS_EOK;
    }
#else
    {
        int count = 0;

        while (p == OS_NULL)
        {
            p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
            if (p != OS_NULL)
                break;

            p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
            if (p != OS_NULL)
                break;

            LOG_EXT_D("F:%s L:%d wait for pbuf_alloc!", __FUNCTION__, __LINE__);
            os_task_delay(1);
            count++;

            // wait for 10ms or give up!!
            if (count >= 10)
            {
                LOG_EXT_W("F:%s L:%d pbuf allocate fail!!!", __FUNCTION__, __LINE__);
                return OS_ENOMEM;
            }
        }
        /*copy data dat -> pbuf*/
        pbuf_take(p, buff, len);
        if ((eth_dev->netif->input(p, eth_dev->netif)) != ERR_OK)
        {
            LOG_EXT_D("F:%s L:%d IP input error", __FUNCTION__, __LINE__);
            pbuf_free(p);
            p = OS_NULL;
        }
        LOG_EXT_D("F:%s L:%d netif iput success! len:%d", __FUNCTION__, __LINE__, len);
        return OS_EOK;
    }
#endif
}

static os_err_t os_wlan_lwip_protocol_send(os_device_t *device, struct pbuf *p)
{
    struct os_wlan_device *wlan = ((struct eth_device *)device)->parent.user_data;

    LOG_EXT_D("F:%s L:%d run", __FUNCTION__, __LINE__);

    if (wlan == OS_NULL)
    {
        return OS_EOK;
    }

#ifdef OS_WLAN_PROT_LWIP_PBUF_FORCE
    {
        os_wlan_prot_transfer_dev(wlan, p, p->tot_len);
        return OS_EOK;
    }
#else
    {
        os_uint8_t *frame;

        /* sending data directly */
        if (p->len == p->tot_len)
        {
            frame = (os_uint8_t *)p->payload;
            os_wlan_prot_transfer_dev(wlan, frame, p->tot_len);
            LOG_EXT_D("F:%s L:%d run len:%d", __FUNCTION__, __LINE__, p->tot_len);
            return OS_EOK;
        }
        frame = os_malloc(p->tot_len);
        if (frame == OS_NULL)
        {
            LOG_EXT_E("F:%s L:%d malloc out_buf fail\n", __FUNCTION__, __LINE__);
            return OS_ENOMEM;
        }
        /*copy pbuf -> data dat*/
        pbuf_copy_partial(p, frame, p->tot_len, 0);
        /* send data */
        os_wlan_prot_transfer_dev(wlan, frame, p->tot_len);
        LOG_EXT_D("F:%s L:%d run len:%d", __FUNCTION__, __LINE__, p->tot_len);
        os_free(frame);
        return OS_EOK;
    }
#endif
}

#ifdef OS_USING_DEVICE_OPS
const static struct os_device_ops wlan_lwip_ops =
{
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    os_wlan_lwip_protocol_control
};
#endif

static struct os_wlan_prot *os_wlan_lwip_protocol_register(struct os_wlan_prot *prot, struct os_wlan_device *wlan)
{
    struct eth_device *   eth = OS_NULL;
    os_uint8_t            id  = 0;
    char                  eth_name[4], timer_name[16];
    os_device_t *         device = OS_NULL;
    struct lwip_prot_des *lwip_prot;

    if (wlan == OS_NULL || prot == OS_NULL)
        return OS_NULL;

    LOG_EXT_D("F:%s L:%d is run wlan:0x%08x", __FUNCTION__, __LINE__, wlan);

    do
    {
        /* find ETH device name */
        eth_name[0] = 'w';
        eth_name[1] = '0' + id++;
        eth_name[2] = '\0';
        device      = os_device_find(eth_name);
    } while (device);

    if (id > 9)
    {
        LOG_EXT_E("F:%s L:%d not find Empty name", __FUNCTION__, __LINE__, eth_name);
        return OS_NULL;
    }

    if (os_device_open((os_device_t *)wlan, OS_DEVICE_OFLAG_RDWR) != OS_EOK)
    {
        LOG_EXT_E("F:%s L:%d open wlan failed", __FUNCTION__, __LINE__);
        return OS_NULL;
    }

    lwip_prot = os_malloc(sizeof(struct lwip_prot_des));
    if (lwip_prot == OS_NULL)
    {
        LOG_EXT_E("F:%s L:%d malloc mem failed", __FUNCTION__, __LINE__);
        os_device_close((os_device_t *)wlan);
        return OS_NULL;
    }
    memset(lwip_prot, 0, sizeof(struct lwip_prot_des));

    eth = &lwip_prot->eth;

#ifdef OS_USING_DEVICE_OPS
    eth->parent.ops = &wlan_lwip_ops;
#else
    eth->parent.init    = OS_NULL;
    eth->parent.open    = OS_NULL;
    eth->parent.close   = OS_NULL;
    eth->parent.read    = OS_NULL;
    eth->parent.write   = OS_NULL;
    eth->parent.control = os_wlan_lwip_protocol_control;
#endif

    eth->parent.user_data = wlan;
    eth->eth_rx           = OS_NULL;
    eth->eth_tx           = os_wlan_lwip_protocol_send;

    /* register ETH device */
    if (eth_device_init(eth, eth_name) != OS_EOK)
    {
        LOG_EXT_E("eth device init failed");
        os_device_close((os_device_t *)wlan);
        os_free(lwip_prot);
        return OS_NULL;
    }
    memcpy(&lwip_prot->prot, prot, sizeof(struct os_wlan_prot));
    os_kprintf(timer_name, "timer_%s", eth_name);
    os_timer_init(&lwip_prot->timer,
                  timer_name,
                  timer_callback,
                  wlan,
                  os_tick_from_ms(1000),
                  OS_TIMER_FLAG_SOFT_TIMER | OS_TIMER_FLAG_ONE_SHOT);
    netif_set_up(eth->netif);
    LOG_EXT_I("eth device init ok name:%s", eth_name);

    return &lwip_prot->prot;
}

static void os_wlan_lwip_protocol_unregister(struct os_wlan_prot *prot, struct os_wlan_device *wlan)
{
    struct lwip_prot_des *lwip_prot = (struct lwip_prot_des *)prot;

    LOG_EXT_D("F:%s L:%d is run wlan:0x%08x", __FUNCTION__, __LINE__, wlan);
#if !defined(NET_USING_LWIP141)
    wlan->prot = OS_NULL;
    if (lwip_prot == OS_NULL)
    {
        return;
    }

#ifdef LWIP_USING_DHCPD
    {
        char netif_name[OS_NAME_MAX];
        memset(netif_name, 0, sizeof(netif_name));
        os_memcpy(netif_name, lwip_prot->eth.netif->name, sizeof(lwip_prot->eth.netif->name));
        dhcpd_stop(netif_name);
    }
#endif
    eth_device_deinit(&lwip_prot->eth);
    os_device_close((os_device_t *)wlan);
    os_timer_deinit(&lwip_prot->timer);

    os_free(lwip_prot);
#endif
}

static struct os_wlan_prot_ops ops =
{
    os_wlan_lwip_protocol_recv,
    os_wlan_lwip_protocol_register,
    os_wlan_lwip_protocol_unregister
};

int os_wlan_lwip_init(void)
{
    static struct os_wlan_prot prot;
    os_wlan_prot_event_t       event;

    memset(&prot, 0, sizeof(prot));
    strncpy(&prot.name[0], OS_WLAN_PROT_LWIP_NAME, OS_WLAN_PROT_NAME_LEN);
    prot.ops = &ops;

    if (os_wlan_prot_regisetr(&prot) != OS_EOK)
    {
        LOG_EXT_E("F:%s L:%d protocol regisetr failed", __FUNCTION__, __LINE__);
        return -1;
    }

    for (event = OS_WLAN_PROT_EVT_INIT_DONE; event < OS_WLAN_PROT_EVT_MAX; event++)
    {
        os_wlan_prot_event_register(&prot, event, os_wlan_lwip_event_handle);
    }

    return 0;
}
OS_PREV_INIT(os_wlan_lwip_init);

#endif
#endif
