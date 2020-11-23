/*
 * Copyright 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * 3. Neither the name of copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __USB_DEVICE_COMPOSITE_H__
#define __USB_DEVICE_COMPOSITE_H__

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#if (defined(USB_DEVICE_CHARGER_DETECT_ENABLE) && (USB_DEVICE_CHARGER_DETECT_ENABLE > 0U))
#include "usb_device_hid.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

 /*! @brief USB controller ID */
#define USB_DEVICE_CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
/*! @brief USB interrupt priority ID */
#define USB_DEVICE_INTERRUPT_PRIORITY (3U)

/*!
 * @brief Initialize USB module hardware and software.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceApplicationInit(void);

typedef enum _usb_power_status
{
    kStatus_Idle = 0U,
    kStatus_StartSuspend,
    kStatus_Suspending,
    kStatus_Suspended,
    kStatus_StartResume,
    kStatus_Resuming,
    kStatus_Resumed,
} usb_power_status_t;

/*!
 * @brief Structure containing device handle, hadle of interfaces and information on curren configuration, alternate setting, speed and attachment status.
 *
 */
typedef struct _usb_device_composite_struct
{
    usb_device_handle deviceHandle;
    class_handle_t interface0HidMouseHandle;
    uint8_t currentConfiguration; /*Current configuration number*/
    uint8_t currentInterfaceAlternateSetting[USB_COMPOSITE_INTERFACE_COUNT]; /*alternate setting number*/
    uint8_t speed;    /*USB speed code, one of the following: USB_SPEED_FULL(0x00U),USB_SPEED_LOW(0x01U),USB_SPEED_HIGH(0x02U)*/
    volatile uint8_t attach;    /*status of device attachment*/
#if (defined(USB_DEVICE_CHARGER_DETECT_ENABLE) && (USB_DEVICE_CHARGER_DETECT_ENABLE > 0U))
    volatile uint8_t vReginInterruptDetected;
    volatile uint8_t vbusValid;
    volatile usb_device_dcd_port_type_t dcdPortType;
    volatile usb_device_dcd_dev_status_t dcdDevStatus;
#endif
    volatile uint64_t hwTick;
    uint64_t startTick;
    volatile uint8_t remoteWakeup;
    volatile uint8_t selfWakeup;
    volatile uint8_t isResume;
    volatile usb_power_status_t suspend;
} usb_device_composite_struct_t;

/*!
 * @brief Check power status of device
 */
usb_power_status_t getPowerStatus(void);

/*!
 * @brief USB device tasks function.
 */
void USB_DeviceTasks(void);

#endif /* __USB_DEVICE_COMPOSITE_H__ */
