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
 * @file        drv_usbd.c
 *
 * @brief       This file implements USB driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_irq.h>
#include "board.h"
#include "drv_usbd.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_composite.h"
#include "usb_device_config.h"
#include "usb_device_descriptor.h"

#define DBG_SECTION_NAME "USBD"
#define DBG_LEVEL        DBG_LOG
#define DBG_COLOR
#include <drv_log.h>

typedef struct nxp_usb
{
    struct os_device parent;
    usb_device_handle deviceHandle;
    usb_device_composite_struct_t *s_UsbDeviceComposite;
}nxp_usb_t;
static nxp_usb_t nxp_usb;

#ifdef OS_USB_DEVICE_HID
#ifdef OS_USB_DEVICE_HID_MOUSE
usb_status_t USB_DeviceInterface0HidMouseInit(usb_device_composite_struct_t *deviceComposite)
{
    nxp_usb.s_UsbDeviceComposite = deviceComposite;
    return kStatus_USB_Success;
}

usb_status_t USB_DeviceHidMousecallback(void)
{
    if (nxp_usb.parent.cb_table[OS_DEVICE_CB_TYPE_RX].cb != OS_NULL)
    {
        os_interrupt_enter();
        nxp_usb.parent.cb_table[OS_DEVICE_CB_TYPE_RX].cb(&nxp_usb.parent, 0);
        os_interrupt_leave();
    }
    if (nxp_usb.parent.cb_table[OS_DEVICE_CB_TYPE_TX].cb != OS_NULL)
    {
        os_interrupt_enter();
        nxp_usb.parent.cb_table[OS_DEVICE_CB_TYPE_TX].cb(&nxp_usb.parent, 0);
        os_interrupt_leave();
    }
    return kStatus_USB_Success;
}

usb_status_t USB_DeviceInterface0HidMouseCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;

    switch (event)
    {
        case kUSB_DeviceHidEventSendResponse:
            if (nxp_usb.s_UsbDeviceComposite->attach)
            {
                return USB_DeviceHidMousecallback();
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

usb_status_t USB_DeviceInterface0HidMouseSetConfiguration(class_handle_t handle, uint8_t configuration_idx)
{
   return USB_DeviceHidMousecallback();
}

usb_status_t USB_DeviceInterface0HidMouseSetInterface(class_handle_t handle, uint8_t alternateSetting)
{
   return USB_DeviceHidMousecallback();
}

os_size_t nxp_usb_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    usb_status_t status;

    status = USB_DeviceHidRecv(nxp_usb.s_UsbDeviceComposite->interface0HidMouseHandle, USB_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN, (os_uint8_t *)buffer, USB_INTERFACE_0_HID_MOUSE_INPUT_REPORT_LENGTH);
    if (status != kStatus_USB_Success)
    {
        LOG_EXT_E("USB read failed!\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

os_size_t nxp_usb_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    usb_status_t status;
    
    status = USB_DeviceHidSend(nxp_usb.s_UsbDeviceComposite->interface0HidMouseHandle, USB_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN, (os_uint8_t *)buffer, USB_INTERFACE_0_HID_MOUSE_INPUT_REPORT_LENGTH);
    if (status != kStatus_USB_Success)
    {
        LOG_EXT_E("USB write failed!\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

usb_status_t nxp_usb_device_start()
{
    return USB_DeviceRun(nxp_usb.s_UsbDeviceComposite->deviceHandle);
}

usb_status_t nxp_usb_device_stop()
{
    return USB_DeviceStop(nxp_usb.s_UsbDeviceComposite->deviceHandle);
}
#endif
#endif

static os_err_t nxp_usb_init(os_device_t *device)
{
    nxp_usb_device_stop();
    
    return OS_EOK;
}

static os_err_t  nxp_usb_control(os_device_t *dev, os_int32_t cmd, void *args)
{
    usb_status_t status;
    
    switch (cmd)
    {
    case NXP_USB_START:
        status = nxp_usb_device_start();
        break;

    case NXP_USB_STOP:
        status = nxp_usb_device_stop();
        break;
    }
    
    return OS_EOK;
}

const static struct os_device_ops usbd_ops = {
    .init = nxp_usb_init,
    .read = nxp_usb_read,
    .write = nxp_usb_write,
    .control = nxp_usb_control,
};

static int nxp_usbd_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    nxp_usb.parent.type = OS_DEVICE_TYPE_USBDEVICE;
    nxp_usb.parent.ops = &usbd_ops;
    nxp_usb.parent.user_data = &nxp_usb,
    
    os_device_register((os_device_t *)&nxp_usb, "usbd", 0);
    return OS_EOK;
}

OS_DRIVER_INFO nxp_usbd_driver = {
    .name   = "USB_Type",
    .probe  = nxp_usbd_probe,
};

OS_DRIVER_DEFINE(nxp_usbd_driver, "3");

#ifdef OS_USB_DEVICE_HID_MOUSE
#include <shell.h>
#include <os_memory.h>

os_err_t usb_mouse_callback(os_device_t *dev, struct os_device_cb_info *info)
{
//    os_kprintf("usb run!\r\n");
    return OS_EOK;
}

static int USB_DeviceHidMouse_test(void)
{
    static int8_t x = 0U;
    static int8_t y = 0U;

    os_uint8_t *buffer = (os_uint8_t *)os_calloc(1, 8);

    os_device_t *device = os_device_find("usbd");
    
    OS_ASSERT(device != OS_NULL);
    os_device_open(device, OS_DEVICE_FLAG_WRONLY);

    struct os_device_cb_info *info = os_calloc(1, sizeof(struct os_device_cb_info));
    info->type = OS_DEVICE_CB_TYPE_TX;
    info->cb = usb_mouse_callback;
    os_device_control(device, IOC_SET_CB, info);
    
    os_device_control(device, NXP_USB_START, OS_NULL);
    enum
    {
        RIGHT,
        DOWN,
        LEFT,
        UP
    };
    static uint8_t dir = RIGHT;

    for (int i = 0; i < 800; i++)
    {
        switch (dir)
        {
            case RIGHT:
                /* Move right. Increase X value. */
                buffer[1] = 1U;
                buffer[2] = 0U;
                x++;
                if (x > 99U)
                {
                    dir++;
                }
                break;
            case DOWN:
                /* Move down. Increase Y value. */
                buffer[1] = 0U;
                buffer[2] = 1U;
                y++;
                if (y > 99U)
                {
                    dir++;
                }
                break;
            case LEFT:
                /* Move left. Decrease X value. */
                buffer[1] = (uint8_t)(0xFFU);
                buffer[2] = 0U;
                x--;
                if (x < 1U)
                {
                    dir++;
                }
                break;
            case UP:
                /* Move up. Decrease Y value. */
                buffer[1] = 0U;
                buffer[2] = (uint8_t)(0xFFU);
                y--;
                if (y < 1U)
                {
                    dir = RIGHT;
                }
                break;
            default:
                break;
        }
        os_task_msleep(10);
        os_device_write(device, 0, buffer, 4); 
    }
    os_device_control(device, NXP_USB_STOP, OS_NULL);
    
    return OS_EOK;
}

SH_CMD_EXPORT(usbd_test, USB_DeviceHidMouse_test, "usbd_hid_test");
#endif