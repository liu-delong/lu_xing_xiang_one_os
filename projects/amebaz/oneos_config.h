#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BOARD_RLT8710B

/* Kernel */

#define OS_NAME_MAX_15
#define OS_NAME_MAX 15
#define OS_ALIGN_SIZE 4
#define OS_TASK_PRIORITY_32
#define OS_TASK_PRIORITY_MAX 32
#define OS_TICK_PER_SECOND 100
#define OS_USING_OVERFLOW_CHECK
#define OS_USING_HOOK
#define OS_USING_IDLE_HOOK
#define OS_IDEL_HOOK_LIST_SIZE 4
#define OS_IDLE_TASK_STACK_SIZE 2048
#define OS_USING_TIMER_SOFT
#define OS_TIMER_TASK_PRIO 0
#define OS_TIMER_TASK_STACK_SIZE 512

/* Inter-Task communication */

#define OS_USING_SEMAPHORE
#define OS_USING_MUTEX
#define OS_USING_EVENT
#define OS_USING_MAILBOX
#define OS_USING_MESSAGEQUEUE
#define OS_USING_WORKQUEUE
#define OS_USING_SYSTEM_WORKQUEUE
#define OS_SYSTEM_WORKQUEUE_STACK_SIZE 2048
#define OS_SYSTEM_WORKQUEUE_PRIORITY 8
#define OS_USING_COMPLETION
#define OS_USING_DATAQUEUE
#define OS_USING_WAITQUEUE
/* end of Inter-Task communication */

/* Memory Management */

#define OS_USING_MEM_POOL
#define OS_USING_MEM_SMALL
#define OS_USING_HEAP
#define OS_MEM_STATS
/* end of Memory Management */

/* Kernel Device Object */

#define OS_USING_DEVICE
#define OS_USING_CONSOLE
#define OS_CONSOLE_DEVICE_NAME "loguart"
/* end of Kernel Device Object */

/* Enable assert */

#define OS_USING_ASSERT
/* end of Enable assert */

/* Kernel debug */

#define OS_USING_KERNEL_DEBUG
#define KLOG_GLOBAL_LEVEL_WARNING
#define KLOG_GLOBAL_LEVEL 1
#define KLOG_USING_COLOR
/* end of Kernel debug */
/* end of Kernel */

/* C standard library */

#define OS_USING_LIBC
/* end of C standard library */

/* Osal */

/* POSIX layer */

/* end of POSIX layer */
/* end of Osal */

/* Drivers */

/* HAL */

#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART1
#define BSP_USING_ON_CHIP_FLASH
#define PKG_USING_FAL
#define TIM_NUM 8
/* end of HAL */

/* AUDIO */

/* end of AUDIO */

/* MISC */

#define OS_USING_PIN
#define OS_USING_PUSH_BUTTON
/* end of MISC */

/* Serial */

#define OS_USING_SERIAL
/* end of Serial */

/* WDT */
#define OS_USING_WDT
#define BSP_USING_WDT
/* end of WDT */

/* RTC */
#define OS_USING_RTC
#define BSP_USING_ONCHIP_RTC
/* end of RTC */

/* CAN */

/* end of CAN */

/* I2C */

/* end of I2C */

/* SPI */

/* end of SPI */

/* MTD */

/* end of MTD */

/* RTT */

/* end of RTT */

/* CPUTIME */

/* end of CPUTIME */

/* HWTIMER */

/* end of HWTIMER */

/* HWCRYPTO */

/* end of HWCRYPTO */

/* SDIO */

/* end of SDIO */

/* WLAN */

#define OS_USING_WIFI
#define OS_WLAN_DEVICE_STA_NAME "wlan0"
#define OS_WLAN_DEVICE_AP_NAME "wlan1"
#define OS_WLAN_SSID_MAX_LENGTH 32
#define OS_WLAN_PASSWORD_MAX_LENGTH 32
#define OS_WLAN_DEV_EVENT_NUM 2
#define OS_WLAN_MANAGE_ENABLE
#define OS_WLAN_SCAN_WAIT_MS 10000
#define OS_WLAN_CONNECT_WAIT_MS 10000
#define OS_WLAN_SCAN_SORT
#define OS_WLAN_MSH_CMD_ENABLE
#define OS_WLAN_AUTO_CONNECT_ENABLE
#define AUTO_CONNECTION_PERIOD_MS 2000
#define OS_WLAN_CFG_ENABLE
#define OS_WLAN_CFG_INFO_MAX 3
#define OS_WLAN_PROT_ENABLE
#define OS_WLAN_PROT_NAME_LEN 8
#define OS_WLAN_PROT_MAX 2
#define OS_WLAN_DEFAULT_PROT "lwip"
#define OS_WLAN_PROT_LWIP_ENABLE
#define OS_WLAN_PROT_LWIP_NAME "lwip"
#define OS_WLAN_WORK_TASK_ENABLE
#define OS_WLAN_WORKQUEUE_TASK_NAME "wlan"
#define OS_WLAN_WORKQUEUE_TASK_SIZE 2048
#define OS_WLAN_WORKQUEUE_TASK_PRIO 15
#define OS_WLAN_CMD_DEBUG
/* end of WLAN */

/* Graphic */

/* end of Graphic */

/* TOUCH */

/* end of TOUCH */

/* SENSORS */

/* end of SENSORS */

/* USB */

/* end of USB */
/* end of Drivers */

/* Components */

/* Atest */

#define OS_USING_ATEST
#define ATEST_TASK_STACK_SIZE 4096
#define ATEST_TASK_PRIORITY 20
/* end of Atest */

/* Dlog */

#define OS_USING_DLOG
#define DLOG_OUTPUT_LVL_W
#define DLOG_GLOBAL_LEVEL 4
#define DLOG_BACKEND_USING_CONSOLE
#define DLOG_USING_FILTER

/* Log format */

#define DLOG_USING_COLOR
#define DLOG_OUTPUT_TIME_INFO
#define DLOG_OUTPUT_LEVEL_INFO
#define DLOG_OUTPUT_TAG_INFO
/* end of Log format */
/* end of Dlog */

/* FAL */

/* end of FAL */

/* Network */

/* Socket abstraction layer */

#define NET_USING_SAL

/* protocol stack implement */

#define SAL_USING_LWIP
/* end of protocol stack implement */
#define SAL_SOCKETS_NUM 16
/* end of Socket abstraction layer */

/* Network Application Protocols */

/* CoAP Protocol */

/* end of CoAP Protocol */

/* Paho MQTT */

/* end of Paho MQTT */
/* end of Network Application Protocols */

/* AT Components */

/* end of AT Components */

/* AT Components Legacy */

/* end of AT Components Legacy */

/* Network interface device */

#define OS_USING_NETDEV
#define NETDEV_USING_IFCONFIG
#define NETDEV_USING_PING
#define NETDEV_USING_NETSTAT
#define NETDEV_USING_AUTO_DEFAULT
#define NETDEV_IPV4 1
#define NETDEV_IPV6 0
/* end of Network interface device */

/* light weight TCP/IP stack */

#define NET_USING_LWIP
#define LWIP_USING_IGMP
#define LWIP_USING_ICMP
#define LWIP_USING_DNS
#define LWIP_USING_DHCP
#define IP_SOF_BROADCAST 1
#define IP_SOF_BROADCAST_RECV 1
//#define LWIP_USING_DHCPD
#define DHCPD_SERVER_IP "192.168.169.1"
#define DHCPD_USING_ROUTER
#define LWIP_USING_PING
/* Static IPv4 Address */

#define LWIP_STATIC_IPADDR "192.168.1.30"
#define LWIP_STATIC_GWADDR "192.168.1.1"
#define LWIP_STATIC_MSKADDR "255.255.255.0"
/* end of Static IPv4 Address */
#define LWIP_USING_RAW
#define LWIP_USING_UDP
#define LWIP_USING_TCP
#define LWIP_MEMP_NUM_NETCONN 8
#define LWIP_PBUF_NUM 16
#define LWIP_RAW_PCB_NUM 4
#define LWIP_UDP_PCB_NUM 4
#define LWIP_TCP_PCB_NUM 4
#define LWIP_TCP_SEG_NUM 40
#define LWIP_TCP_SND_BUF 8196
#define LWIP_TCP_WND_SIZE 8196
#define LWIP_TCP_TASK_PRIORITY 10
#define LWIP_TCP_TASK_MBOX_SIZE 8
#define LWIP_TCP_TASK_STACKSIZE 1024
#define LWIP_ETH_TASK_PRIORITY 12
#define LWIP_ETH_TASK_STACKSIZE 1024
#define LWIP_ETH_TASK_MBOX_SIZE 8
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK 1
#define SO_REUSE 1
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_SNDTIMEO 1
#define LWIP_SO_RCVBUF 1
#define LWIP_NETIF_LOOPBACK 0
/* end of light weight TCP/IP stack */
/* end of Network */

/* security */

/* end of security */

/* Shell */

#define OS_USING_SHELL
#define SHELL_TASK_NAME "tshell"
#define SHELL_TASK_PRIORITY 20
#define SHELL_TASK_STACK_SIZE 2048
#define SHELL_USING_HISTORY
#define SHELL_HISTORY_LINES 5
#define SHELL_USING_SYMTAB
#define SHELL_USING_DESCRIPTION
#define SHELL_CMD_SIZE 80
#define SHELL_ARG_MAX 10
/* end of Shell */

/* Virtual file system */

/* end of Virtual file system */

/* GUI */

/* end of GUI */
/* end of Components */

/* Thirdparty */

/* cJSON */

/* end of cJSON */

/* Mpu6xxx */

/* end of Mpu6xxx */

/* Easyflash */

/* end of Easyflash */

/* NTP */

/* end of NTP */

/* WebClient */

/* end of WebClient */
/* end of Thirdparty */

/* Debug */

#define OS_DEBUG
#define OS_DEBUG_LOG_WITH_FUNC_LINE
#define LOG_BUFF_SIZE_256
#define OS_LOG_BUFF_SIZE 256
/* end of Debug */

#endif /* __ONEOS_CONFIG_H__ */

