#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BOARD_ST_NUCLEO
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M7

/* Kernel */

#define OS_NAME_MAX_15
#define OS_NAME_MAX 15
#define OS_ALIGN_SIZE 4
#define OS_TASK_PRIORITY_32
#define OS_TASK_PRIORITY_MAX 32
#define OS_TICK_PER_SECOND 100
#define OS_USING_OVERFLOW_CHECK
#define OS_MAIN_TASK_STACK_SIZE 2048
#define OS_USING_HOOK
#define OS_USING_IDLE_HOOK
#define OS_IDLE_HOOK_LIST_SIZE 4
#define OS_IDLE_TASK_STACK_SIZE 512
#define OS_USING_TIMER_SOFT
#define OS_TIMER_TASK_PRIO 0
#define OS_TIMER_TASK_STACK_SIZE 512
#define OS_USING_WORKQUEUE
#define OS_USING_SYSTEM_WORKQUEUE
#define OS_SYSTEM_WORKQUEUE_STACK_SIZE 2048
#define OS_SYSTEM_WORKQUEUE_PRIORITY 8

/* Task communication */

#define OS_USING_SEMAPHORE
#define OS_USING_MUTEX
#define OS_USING_EVENT
#define OS_USING_MAILBOX
#define OS_USING_MESSAGEQUEUE
#define OS_USING_COMPLETION
#define OS_USING_DATAQUEUE
#define OS_USING_WAITQUEUE
/* end of Task communication */

/* Memory management */

#define OS_USING_MEM_POOL
#define OS_USING_MEM_SMALL
#define OS_USING_HEAP
#define OS_MEM_STATS
/* end of Memory management */

/* Kernel device object */

#define OS_USING_DEVICE
#define OS_USING_CONSOLE
#define OS_CONSOLE_DEVICE_NAME "uart3"
/* end of Kernel device object */

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

/* POSIX compatibility layer */

#define OS_USING_POSIX
/* end of POSIX compatibility layer */

/* RT-Thread compatibility layer */

/* end of RT-Thread compatibility layer */

/* CMSIS compatibility layer */

/* end of CMSIS compatibility layer */

/* FreeRTOS compatibility layer */

/* end of FreeRTOS compatibility layer */
/* end of Osal */

/* Drivers */

/* HAL */

#define SOC_FAMILY_STM32
#define SOC_SERIES_STM32F7
#define SOC_STM32F746xx
#define BSP_USING_ETH

/* config eth hw */

#define ETH_RESET_PIN 51
#define PHY_USING_LAN8720A

/* Configure base hal in STM32CubeMX */

/* end of HAL */

/* Audio */

/* end of Audio */

/* MISC */

#define OS_USING_PUSH_BUTTON
#define OS_USING_LED
#define OS_USING_ADC
#define OS_USING_DAC
/* end of MISC */

/* PIN */

#define OS_USING_PIN
#define OS_PIN_MAX_CHIP 1
/* end of PIN */

/* Serial */

#define OS_USING_SERIAL
#define OS_SERIAL_RX_BUFSZ 64
#define OS_SERIAL_TX_BUFSZ 64
/* end of Serial */

/* WDG */

#define OS_USING_WDG
/* end of WDG */

/* RTC */

#define OS_USING_RTC
/* end of RTC */

/* CAN */

#define OS_USING_CAN
/* end of CAN */

/* I2C */

#define OS_USING_I2C
/* end of I2C */

/* SPI */

/* end of SPI */

/* FAL */

#define OS_USING_FAL
/* end of FAL */

/* RTT */

/* end of RTT */

/* Timer */

#define OS_USING_TIMER_DRIVER
#define OS_USING_CLOCKSOURCE
#define OS_USING_CLOCKSOURCE_CORTEXM
#define OS_USING_CLOCKEVENT
#define OS_USING_HRTIMER
#define OS_USING_HRTIMER_FOR_SYSTICK
/* end of Timer */

/* HwCrypto */

/* end of HwCrypto */

/* SDIO */

/* end of SDIO */

/* WLAN */

/* end of WLAN */

/* Graphic */

/* end of Graphic */

/* Touch */

/* end of Touch */

/* Sensors */

/* end of Sensors */

/* USB */

/* end of USB */

/* Infrared */

/* end of Infrared */

/* Low power manager */

/* end of Low power manager */

/* NAND */

/* end of NAND */
/* end of Drivers */

/* Components */

/* Atest */

#define OS_USING_ATEST
#define ATEST_TASK_STACK_SIZE 4096
#define ATEST_TASK_PRIORITY 20
/* end of Atest */

/* Cloud */

/* OneNET */

/* MQTT kit */

/* end of MQTT kit */

/* NB-IoT kit */

/* end of NB-IoT kit */

/* EDP */

/* end of EDP */
/* end of OneNET */
/* end of Cloud */

/* Dlog */

#define OS_USING_DLOG
#define DLOG_OUTPUT_LVL_D
#define DLOG_GLOBAL_LEVEL 7
#define DLOG_USING_ISR_LOG

/* Dlog backend option */

#define DLOG_BACKEND_USING_CONSOLE
/* end of Dlog backend option */
#define DLOG_USING_FILTER

/* Log format */

#define DLOG_USING_COLOR
#define DLOG_OUTPUT_TIME_INFO
#define DLOG_OUTPUT_LEVEL_INFO
#define DLOG_OUTPUT_TAG_INFO
/* end of Log format */
/* end of Dlog */

/* Network */

/* LwIP */

#define NET_USING_LWIP
#define NET_USING_LWIP212
#define LWIP_USING_IGMP
#define LWIP_USING_ICMP
#define LWIP_USING_DNS
#define LWIP_USING_DHCP
#define IP_SOF_BROADCAST 1
#define IP_SOF_BROADCAST_RECV 1

/* Static IPv4 Address */

#define LWIP_STATIC_IPADDR "192.168.1.30"
#define LWIP_STATIC_GWADDR "192.168.1.1"
#define LWIP_STATIC_MSKADDR "255.255.255.0"
/* end of Static IPv4 Address */
#define LWIP_USING_UDP
#define LWIP_USING_TCP
#define LWIP_USING_RAW
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
#define LWIP_USING_PING
/* end of LwIP */

/* Molink */

/* end of Molink */

/* OTA */

/* Fota by CMIOT */

/* end of Fota by CMIOT */
/* end of OTA */

/* Protocols */

/* CoAP */

/* end of CoAP */

/* MQTT */

/* end of MQTT */
/* end of Protocols */

/* Socket */

/* end of Socket */

/* Tools */

/* end of Tools */
/* end of Network */

/* Security */

/* end of Security */

/* Shell */

#define OS_USING_SHELL
#define SHELL_TASK_NAME "tshell"
#define SHELL_TASK_PRIORITY 20
#define SHELL_TASK_STACK_SIZE 2048
#define SHELL_USING_HISTORY
#define SHELL_HISTORY_LINES 5
#define SHELL_USING_DESCRIPTION
#define SHELL_CMD_SIZE 80
#define SHELL_ARG_MAX 10
/* end of Shell */

/* Virtual file system */

#define OS_USING_VFS
#define VFS_USING_WORKDIR
#define VFS_FILESYSTEMS_MAX 2
#define VFS_FILESYSTEM_TYPES_MAX 2
#define VFS_FD_MAX 16
#define OS_USING_VFS_DEVFS
/* end of Virtual file system */

/* GUI */

/* end of GUI */

/* OnePos */

/* end of OnePos */

/* Ramdisk */

/* end of Ramdisk */

/* Diagnose */

/* end of Diagnose */
/* end of Components */

/* Thirdparty */

/* cJSON */

/* end of cJSON */

/* Easyflash */

/* end of Easyflash */

/* NTP */

/* end of NTP */

/* WebClient */

/* end of WebClient */

/* Ali-iotkit */

/* end of Ali-iotkit */

/* MicroPython */

/* end of MicroPython */
/* end of Thirdparty */

/* Boot Config */

/* end of Boot Config */

/* Debug */

#define OS_DEBUG
#define OS_DEBUG_LOG_WITH_FUNC_LINE
#define LOG_BUFF_SIZE_256
#define OS_LOG_BUFF_SIZE 256
/* end of Debug */

#endif /* __ONEOS_CONFIG_H__ */
