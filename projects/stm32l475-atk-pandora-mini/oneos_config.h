#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BOARD_ATK_PANDORA
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M4

/* Kernel */

#define OS_NAME_MAX_15
#define OS_NAME_MAX 15
#define OS_ALIGN_SIZE 4
#define OS_TASK_PRIORITY_32
#define OS_TASK_PRIORITY_MAX 32
#define OS_TICK_PER_SECOND 100
#define OS_MAIN_TASK_STACK_SIZE 2048
#define OS_IDLE_TASK_STACK_SIZE 512

/* Task communication */

#define OS_USING_SEMAPHORE
#define OS_USING_MUTEX
#define OS_USING_EVENT
/* end of Task communication */

/* Memory management */

#define OS_USING_MEM_SMALL
#define OS_USING_HEAP
/* end of Memory management */

/* Kernel console */

/* end of Kernel console */

/* Enable assert */

/* end of Enable assert */
/* end of Kernel */

/* C standard library */

#define OS_USING_LIBC
/* end of C standard library */

/* Osal */

/* POSIX compatibility layer */

/* end of POSIX compatibility layer */

/* RT-Thread compatibility layer */

#define OS_USING_RTTHREAD_ADAPTER
/* end of RT-Thread compatibility layer */

/* CMSIS compatibility layer */

/* end of CMSIS compatibility layer */

/* FreeRTOS compatibility layer */

/* end of FreeRTOS compatibility layer */
/* end of Osal */

/* Drivers */

#define OS_USING_DEVICE

/* HAL */

#define SOC_FAMILY_STM32
#define SOC_SERIES_STM32L4
#define SOC_STM32L475VE

/* Configure base hal in STM32CubeMX */

/* end of HAL */

/* Audio */

/* end of Audio */

/* MISC */

/* end of MISC */

/* PIN */

/* end of PIN */

/* Serial */

#define OS_USING_SERIAL
#define OS_SERIAL_RX_BUFSZ 256
#define OS_SERIAL_TX_BUFSZ 64
/* end of Serial */

/* WDG */

/* end of WDG */

/* RTC */

/* end of RTC */

/* CAN */

/* end of CAN */

/* I2C */

/* end of I2C */

/* SPI */

/* end of SPI */

/* FAL */

/* end of FAL */

/* RTT */

/* end of RTT */

/* Timer */

#define OS_USING_TIMER_DRIVER
#define OS_USING_CLOCKSOURCE
#define OS_USING_CLOCKSOURCE_CORTEXM
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

/* end of Atest */

/* BLE */

/* end of BLE */

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

/* end of Dlog */

/* Network */

/* LwIP */

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

/* Websocket */

/* end of Websocket */

/* Httpclient */

/* end of Httpclient */
/* end of Protocols */

/* Socket */

/* end of Socket */

/* Tools */

/* end of Tools */
/* end of Network */

/* Security */

/* end of Security */

/* Shell */

/* end of Shell */

/* FileSystem */

/* end of FileSystem */

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

/* Amazon-iot */

/* end of Amazon-iot */
/* end of Thirdparty */

/* Boot Config */

/* end of Boot Config */

/* Debug */

#define LOG_BUFF_SIZE_256
#define OS_LOG_BUFF_SIZE 256
/* end of Debug */

#endif /* __ONEOS_CONFIG_H__ */

