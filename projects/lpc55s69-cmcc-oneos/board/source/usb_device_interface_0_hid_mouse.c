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

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_hid.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "usb_device_composite.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _usb_device_hid_mouse_struct
{
    uint8_t *buffer;
    uint8_t idleRate;
} usb_device_hid_mouse_struct_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static usb_status_t USB_DeviceHidMouseAction(void);

usb_status_t USB_DeviceInterface0HidMouseCallback(class_handle_t handle, uint32_t event, void *param);

usb_status_t USB_DeviceInterface0HidMouseSetConfiguration(class_handle_t handle, uint8_t configuration_idx);

usb_status_t USB_DeviceInterface0HidMouseSetInterface(class_handle_t handle, uint8_t alternateSetting);

usb_status_t USB_DeviceInterface0HidMouseInit(usb_device_composite_struct_t *deviceComposite);


/*******************************************************************************
 * Variables
 ******************************************************************************/

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_MouseBuffer[USB_INTERFACE_0_HID_MOUSE_INPUT_REPORT_LENGTH];
static usb_device_composite_struct_t *s_UsbDeviceComposite;
static usb_device_hid_mouse_struct_t s_UsbDeviceHidMouse;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
* @brief Function to prepare buffer for next IN transaction.
*
* @return usb_status_t Status of USB transaction.
*/
 static usb_status_t USB_DeviceHidMouseAction(void)
{
    static int8_t x = 0U;
    static int8_t y = 0U;
    enum
    {
        RIGHT,
        DOWN,
        LEFT,
        UP
    };
    static uint8_t dir = RIGHT;

    switch (dir)
    {
        case RIGHT:
            /* Move right. Increase X value. */
            s_UsbDeviceHidMouse.buffer[1] = 1U;
            s_UsbDeviceHidMouse.buffer[2] = 0U;
            x++;
            if (x > 99U)
            {
                dir++;
            }
            break;
        case DOWN:
            /* Move down. Increase Y value. */
            s_UsbDeviceHidMouse.buffer[1] = 0U;
            s_UsbDeviceHidMouse.buffer[2] = 1U;
            y++;
            if (y > 99U)
            {
                dir++;
            }
            break;
        case LEFT:
            /* Move left. Decrease X value. */
            s_UsbDeviceHidMouse.buffer[1] = (uint8_t)(0xFFU);
            s_UsbDeviceHidMouse.buffer[2] = 0U;
            x--;
            if (x < 1U)
            {
                dir++;
            }
            break;
        case UP:
            /* Move up. Decrease Y value. */
            s_UsbDeviceHidMouse.buffer[1] = 0U;
            s_UsbDeviceHidMouse.buffer[2] = (uint8_t)(0xFFU);
            y--;
            if (y < 1U)
            {
                dir = RIGHT;
            }
            break;
        default:
            break;
    }

    return USB_DeviceHidSend(s_UsbDeviceComposite->interface0HidMouseHandle, USB_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN,
                             s_UsbDeviceHidMouse.buffer, USB_INTERFACE_0_HID_MOUSE_INPUT_REPORT_LENGTH);
}

/*!
* @brief Function that processes class specific events.
*
* @param handle Handle to USB device class.
* @param event Class event code.
* @param param    The parameter of the class specific event.
* @return usb_status_t Status of USB transaction.
*/
usb_status_t USB_DeviceInterface0HidMouseCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;

    switch (event)
    {
        case kUSB_DeviceHidEventSendResponse:
            if (s_UsbDeviceComposite->attach)
            {
                return USB_DeviceHidMouseAction();
            }
            break;
        case kUSB_DeviceHidEventGetReport:
        case kUSB_DeviceHidEventSetReport:
        case kUSB_DeviceHidEventRequestReportBuffer:
            error = kStatus_USB_InvalidRequest;
            break;
        case kUSB_DeviceHidEventGetIdle:
        case kUSB_DeviceHidEventGetProtocol:
        case kUSB_DeviceHidEventSetIdle:
        case kUSB_DeviceHidEventSetProtocol:
            break;
        default:
            break;
    }

    return error;
}

/*!
* @brief Notifies application layer about set configuration event.
*
* @param handle Handle to USB device class.
* @param configuration_idx Id of device configuration.
* @return usb_status_t Status of USB transaction.
*/
usb_status_t USB_DeviceInterface0HidMouseSetConfiguration(class_handle_t handle, uint8_t configuration_idx)
{
   return USB_DeviceHidMouseAction();
}

/*!
* @brief Notifies application layer about set configuration event.
*
* @param class_handle_t Handle to USB device class.
* @param alternateSetting Id of device alternative setting.
* @return usb_status_t Status of USB transaction.
*/
usb_status_t USB_DeviceInterface0HidMouseSetInterface(class_handle_t handle, uint8_t alternateSetting)
{
   return USB_DeviceHidMouseAction();
}

/*!
* @brief Initializes device structure and buffer pointers.
*
* @param *device Pointer to structure to initialize to.
* @return usb_status_t Always return kStatus_USB_Success value.
*/
usb_status_t USB_DeviceInterface0HidMouseInit(usb_device_composite_struct_t *deviceComposite)
{
    s_UsbDeviceComposite = deviceComposite;
    s_UsbDeviceHidMouse.buffer = s_MouseBuffer;
    return kStatus_USB_Success;
}
