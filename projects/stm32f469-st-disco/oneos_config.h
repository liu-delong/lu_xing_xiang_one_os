#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BOARD_F469_DISCO
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M4

/* Kernel */

#define OS_NAME_MAX_15
#define OS_NAME_MAX 15
#define OS_ALIGN_SIZE 4
#define OS_TASK_PRIORITY_32
#define OS_TASK_PRIORITY_MAX 32
#define OS_TICK_PER_SECOND 1000
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

/* Kernel console */

#define OS_USING_CONSOLE
#define OS_CONSOLE_DEVICE_NAME "uart3"
/* end of Kernel console */

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

#define OS_USING_DEVICE

/* HAL */

#define SOC_FAMILY_STM32
#define SOC_SERIES_STM32F4
#define SOC_STM32F469xx

/* Configure base hal in STM32CubeMX */

/* end of HAL */

/* Audio */

#define OS_USING_AUDIO
#define OS_AUDIO_DATD_CONFIG
#define OS_AUDIO_REPLAY_MP_BLOCK_SIZE 2048
#define OS_AUDIO_REPLAY_MP_BLOCK_COUNT 0x3
#define OS_AUDIO_RECORD_PIPE_SIZE 2048
#define OS_USING_SAI
#define BSP_SAI_BLOCK "sai_BlockA1"
#define BSP_USING_CS43L22
#define BSP_USING_CS43L22_CONFIG
#define BSP_CS43L22_I2C_BUS "soft_i2c2"
#define BSP_CS43L22_I2C_ADDR 0x4a
#define BSP_USING_CS43L22_DATA
#define BSP_CS43L22_DATA_BUS "sai_BlockA1"
#define BSP_CS43L22_RST_PIN 66
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
#define OS_USING_I2C_BITOPS
#define SOFT_I2C_BUS_DELAY_US 10
#define BSP_USING_SOFT_I2C1
#define BSP_SOFT_I2C1_SCL_PIN 24
#define BSP_SOFT_I2C1_SDA_PIN 25
#define BSP_USING_SOFT_I2C2
#define BSP_SOFT_I2C2_SCL_PIN 116
#define BSP_SOFT_I2C2_SDA_PIN 117
/* end of I2C */

/* SPI */

#define OS_USING_SPI
#define OS_USING_QSPI
#define OS_USING_SFUD
#define OS_SFUD_USING_SFDP
#define OS_SFUD_USING_FLASH_INFO_TABLE
#define OS_SFUD_USING_QSPI
#define OS_QSPI_FLASH_BUS_NAME "qspi"
#define OS_EXTERN_FLASH_PORT_CFG
#define OS_EXTERN_FLASH_DEV_NAME "N25Q128"
#define OS_EXTERN_FLASH_BUS_NAME "sfud_bus"
#define OS_EXTERN_FLASH_NAME "nor_flash"
#define OS_EXTERN_FLASH_SIZE 16777216
#define OS_EXTERN_FLASH_BLOCK_SIZE 4096
#define OS_EXTERN_FLASH_PAGE_SIZE 4096
#define BSP_USING_W25QXX
/* end of SPI */

/* FAL */

#define OS_USING_FAL
#define OS_FAL_RAM
#define OS_FAL_RAM_SIZE 0x80000
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

#define OS_USING_SDIO
#define OS_SDIO_STACK_SIZE 512
#define OS_SDIO_TASK_PRIORITY 15
#define OS_MMCSD_STACK_SIZE 1024
#define OS_MMCSD_TASK_PREORITY 22
#define OS_MMCSD_MAX_PARTITION 16
/* end of SDIO */

/* WLAN */

/* end of WLAN */

/* Graphic */

#define OS_USING_GRAPHIC
#define OS_GRAPHIC_WIDTH 480
#define OS_GRAPHIC_HEIGHT 800
#define OS_USING_DSI
#define OS_USING_DSI_RST 119
#define BSP_USING_OTM8009A
#define BSP_OTM8009A_DSI_CHANNEL 0
/* end of Graphic */

/* Touch */

#define OS_USING_TOUCH
#define OS_USING_FT6X06
#define OS_FT6X060_I2C_BUS_NAME "soft_i2c1"
#define OS_FT6X06_I2C_ADDR 0x2a
/* end of Touch */

/* Sensors */

/* end of Sensors */

/* USB */

#define OS_USING_USB_DEVICE
#define OS_USBD_TASK_STACK_SZ 4096
#define USB_VENDOR_ID 0x0FFE
#define USB_PRODUCT_ID 0x0001
#define OS_USB_DEVICE_COMPOSITE
#define OS_USB_DEVICE_NONE
#define OS_USB_DEVICE_HID
#define OS_USB_DEVICE_HID_MOUSE
/* end of USB */

/* Infrared */

/* end of Infrared */

/* Low power manager */

/* end of Low power manager */

/* NAND */

/* end of NAND */
#define OS_USING_DRIVERS_SAMPLES
#define OS_FAL_TEST_DEMO
#define OS_GUI_DEMO
/* end of Drivers */

/* Components */

/* Atest */

#define OS_USING_ATEST
#define ATEST_TASK_STACK_SIZE 4096
#define ATEST_TASK_PRIORITY 20
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

#define OS_USING_DLOG
#define DLOG_OUTPUT_LVL_D
#define DLOG_GLOBAL_LEVEL 7

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

/* FileSystem */

#define OS_USING_VFS
#define VFS_USING_WORKDIR
#define VFS_FILESYSTEMS_MAX 2
#define VFS_FILESYSTEM_TYPES_MAX 2
#define VFS_FD_MAX 16
#define OS_USING_VFS_FATFS

/* Elm-ChaN's FatFs, generic FAT filesystem module */

#define OS_VFS_FAT_CODE_PAGE 437
#define OS_VFS_FAT_USE_LFN_3
#define OS_VFS_FAT_USE_LFN 3
#define OS_VFS_FAT_MAX_LFN 255
#define OS_VFS_FAT_DRIVES 2
#define OS_VFS_FAT_MAX_SECTOR_SIZE 1024
#define OS_VFS_FAT_REENTRANT
/* end of Elm-ChaN's FatFs, generic FAT filesystem module */
#define OS_USING_VFS_DEVFS
/* end of FileSystem */

/* GUI */

#define OS_USING_GUI
#define OS_GUI_DISP_DEV_NAME "lcd"
#define OS_GUI_INPUT_DEV_NAME "touch"
#define OS_USING_GUI_LVGL
#define OS_USING_GUI_LVGL_CONSOLE
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

#define OS_DEBUG
#define OS_DEBUG_LOG_WITH_FUNC_LINE
#define LOG_BUFF_SIZE_256
#define OS_LOG_BUFF_SIZE 256
/* end of Debug */

#endif /* __ONEOS_CONFIG_H__ */

