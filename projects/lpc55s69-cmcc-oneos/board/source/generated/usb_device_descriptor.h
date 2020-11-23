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

#ifndef __USB_DEVICE_DESCRIPTOR_H__
#define __USB_DEVICE_DESCRIPTOR_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define USB_DEVICE_SPECIFIC_BCD_VERSION (0x0200U)
#define USB_DEVICE_DEMO_BCD_VERSION (0x0101U)

#define USB_DEVICE_CLASS (0x00U)
#define USB_DEVICE_SUBCLASS (0x00U)
#define USB_DEVICE_PROTOCOL (0x00U)
/*! @brief Maximum current USB device can draw from bus in milliamperes.*/
#define USB_DEVICE_MAX_POWER (0x64U)	

#define USB_DEVICE_CONFIGURATION_COUNT (1U)
#define USB_DEVICE_STRING_COUNT (3U)
#define USB_DEVICE_LANGUAGE_COUNT (1U)

#define USB_COMPOSITE_CONFIGURATION_INDEX (1U)
/* Length of Interface Association Descriptor in bytes */
#define USB_IAD_DESC_SIZE (8U)



#define USB_ALTERNATE_SETTING_0 (0U)

#define USB_INTERFACE_0_HID_MOUSE_CLASS (0x03U)
#define USB_INTERFACE_0_HID_MOUSE_SUBCLASS (0x01U)
#define USB_INTERFACE_0_HID_MOUSE_PROTOCOL (0x02U)
#define USB_INTERFACE_0_HID_MOUSE_INTERFACE_COUNT (1U)
#define USB_INTERFACE_0_HID_MOUSE_INDEX (0U)
#define USB_INTERFACE_0_HID_MOUSE_INPUT_REPORT_LENGTH (4U)
#define USB_INTERFACE_0_HID_MOUSE_OUTPUT_REPORT_LENGTH (0U)
#define USB_INTERFACE_0_HID_MOUSE_FEATURE_REPORT_LENGTH (0U)
#define USB_INTERFACE_0_HID_MOUSE_SETTING_0_DEFAULT_ENDPOINT_COUNT (1U)
#define USB_INTERFACE_0_HID_MOUSE_SETTING_0_DEFAULT_INDEX USB_ALTERNATE_SETTING_0
#define USB_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN (1U)
#define USB_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN_DIRECTION USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_IN
#define FS_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN_PACKET_SIZE (8U)
#define HS_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN_PACKET_SIZE (8U)
#define FS_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN_INTERVAL (4U)
#define HS_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN_INTERVAL (6U)

#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL (sizeof(g_UsbDeviceConfigurationDescriptor))

#define USB_DESCRIPTOR_LENGTH_INTERFACE_0_HID_MOUSE_REPORT (sizeof(g_UsbDeviceInterface0HidMouseReportDescriptor))
#define USB_DESCRIPTOR_LENGTH_HID (9U)

#define USB_DESCRIPTOR_LENGTH_STRING0 (sizeof(g_UsbDeviceString0))
#define USB_DESCRIPTOR_LENGTH_STRING1 (sizeof(g_UsbDeviceString1))
#define USB_DESCRIPTOR_LENGTH_STRING2 (sizeof(g_UsbDeviceString2))

#define USB_COMPOSITE_INTERFACE_COUNT (USB_INTERFACE_0_HID_MOUSE_INTERFACE_COUNT)

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
* @brief Configure the device according to the USB speed.
*
* Due to the difference of HS and FS descriptors, the device descriptors and configurations need to be updated to match
* the current speed.
* As the default, the device descriptors and configurations are configured by using FS parameters for both EHCI and
* KHCI.
* When the EHCI is enabled, the application needs to call this fucntion to update device by using current speed.
* The updated information includes endpoint max packet size, endpoint interval, etc.
*
* @param usb_device_handle Handle to USB device.
* @param uint8_t Value contains information on speed in coded form:
* @verbatim
*    0 - full speed
*    1 - low speed
*    2 - high speed
* @endverbatim
* @retval kStatus_USB_Success Function always returns kStatus_USB_Success value.
*/
usb_status_t USB_DeviceSetSpeed(usb_device_handle handle, uint8_t speed);

#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
/*!
* @brief Get device qualifier descriptor request.
*
* @param usb_device_handle Handle to USB device.
* @param usb_device_get_device_qualifier_descriptor_struct_t USB device qualifier descriptor request structure that will be set by this function.
* @retval kStatus_USB_Success Function always returns kStatus_USB_Success value.
*/
usb_status_t USB_DeviceGetDeviceQualifierDescriptor(
    usb_device_handle handle, usb_device_get_device_qualifier_descriptor_struct_t *deviceQualifierDescriptor);
#endif

/*!
* @brief Gets device descriptor request.
*
* @param usb_device_handle Handle to USB device.
* @param usb_device_get_configuration_descriptor_struct_t USB device descriptor request structure.
* @retval kStatus_USB_Success Function always returns kStatus_USB_Success value.
*/
usb_status_t USB_DeviceGetDeviceDescriptor(usb_device_handle handle,
                                           usb_device_get_device_descriptor_struct_t *deviceDescriptor);

/*!
* @brief Get device configuration descriptor request .
*
* @param usb_device_handle Handle to USB device.
* @param usb_device_get_configuration_descriptor_struct_t USB device configuration descriptor request structure that will be set by this function.
* @retval kStatus_USB_Success Function always returns kStatus_USB_Success value.
*/
usb_status_t USB_DeviceGetConfigurationDescriptor(
    usb_device_handle handle, usb_device_get_configuration_descriptor_struct_t *configurationDescriptor);

/*!
* @brief Get device string descriptor request.
*
* @param usb_device_handle Handle to USB device.
* @param usb_device_get_string_descriptor_struct_t USB device string descriptor request structure that will be set by this function.
* @retval kStatus_USB_Success Function always returns kStatus_USB_Success value.
*/
usb_status_t USB_DeviceGetStringDescriptor(usb_device_handle handle,
                                           usb_device_get_string_descriptor_struct_t *stringDescriptor);


/*!
* @brief Get device HID descriptor request.
*
* @param usb_device_handle Handle to USB device.
* @param usb_device_get_hid_descriptor_struct_t USB device HID descriptor request structure that will be set by this function.
* @retval kStatus_USB_Success Function always returns kStatus_USB_Success value.
*/
usb_status_t USB_DeviceGetHidDescriptor(usb_device_handle handle,
                                        usb_device_get_hid_descriptor_struct_t *hidDescriptor);

/*!
* @brief Get device HID report descriptor request.
*
* @param usb_device_handle Handle to USB device.
* @param usb_device_get_hid_report_descriptor_struct_t USB device HID report descriptor request structure that will be set by this function.
* @retval kStatus_USB_Success Function always returns kStatus_USB_Success value.
*/
usb_status_t USB_DeviceGetHidReportDescriptor(usb_device_handle handle,
                                              usb_device_get_hid_report_descriptor_struct_t *hidReportDescriptor);

/*!
* @brief Get device HID report descriptor request.
*
* @param usb_device_handle Handle to USB device.
* @param usb_device_get_hid_physical_descriptor_struct_t USB device physical descriptor request structure that will be set by this function.
* @retval kStatus_USB_Success Function always returns kStatus_USB_Success value.
*/
usb_status_t USB_DeviceGetHidPhysicalDescriptor(usb_device_handle handle,
                                                usb_device_get_hid_physical_descriptor_struct_t *hidPhysicalDescriptor);

#endif /* __USB_DEVICE_DESCRIPTOR_H__ */
