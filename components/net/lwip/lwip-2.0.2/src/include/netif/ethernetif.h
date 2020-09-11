#ifndef __NETIF_ETHERNETIF_H__
#define __NETIF_ETHERNETIF_H__

#include "lwip/netif.h"
#include <os_task.h>
#include <os_device.h>
#include <os_sem.h>

#define NIOCTL_GADDR		0x01
#ifndef RT_LWIP_ETH_MTU
#define ETHERNET_MTU		1500
#else
#define ETHERNET_MTU		RT_LWIP_ETH_MTU
#endif

/* eth flag with auto_linkup or phy_linkup */
#define ETHIF_LINK_AUTOUP	0x0000
#define ETHIF_LINK_PHYUP	0x0100

struct eth_device
{
    /* inherit from os_device */
    struct os_device parent;

    /* network interface for lwip */
    struct netif *netif;
    struct os_semaphore tx_ack;

    os_uint16_t flags;
    os_uint8_t  link_changed;
    os_uint8_t  link_status;

    /* eth device interface */
    struct pbuf* (*eth_rx)(os_device_t* dev);
    os_err_t (*eth_tx)(os_device_t* dev, struct pbuf* p);
};

#ifdef __cplusplus
extern "C" {
#endif

    os_err_t eth_device_ready(struct eth_device* dev);
    os_err_t eth_device_init(struct eth_device * dev, const char *name);
    os_err_t eth_device_init_with_flag(struct eth_device *dev, const char *name, os_uint16_t flag);
    os_err_t eth_device_linkchange(struct eth_device* dev, os_bool_t up);
    void eth_device_deinit(struct eth_device *dev);

    int eth_system_device_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __NETIF_ETHERNETIF_H__ */
