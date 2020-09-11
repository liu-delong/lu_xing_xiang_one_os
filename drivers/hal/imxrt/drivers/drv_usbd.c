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
 * @brief       This file implements usbd driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <usb/include/usb_device_config.h>
#include <usb/include/usb.h>
#include <os_task.h>
#include <usb/phy/usb_phy.h>
#include <usb/device/usb_device.h>
#include <usb/device/usb_device_dci.h>
#include <os_device.h>

/* USB PHY condfiguration */
#define BOARD_USB_PHY_D_CAL (0x0CU)
#define BOARD_USB_PHY_TXCAL45DP (0x06U)
#define BOARD_USB_PHY_TXCAL45DM (0x06U)

static usb_device_handle ehci0_handle;
static struct udcd _fsl_udc_0;

static usb_status_t usb_device_callback(usb_device_handle handle, uint32_t callbackEvent, void *eventParam);
static usb_status_t usb_device_endpoint_callback(usb_device_handle handle, usb_device_endpoint_callback_message_struct_t *message, void *callbackParam);

static void USB_DeviceIsrEnable(uint8_t controllerId)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber = usbDeviceEhciIrq[controllerId - kUSB_ControllerEhci0];
#endif
    /* Install isr, set priority, and enable IRQ. */
#if defined(__GIC_PRIO_BITS)
    GIC_SetPriority((IRQn_Type)irqNumber, 3);
#else
    NVIC_SetPriority((IRQn_Type)irqNumber, 3);
#endif
    EnableIRQ((IRQn_Type)irqNumber);
}

/*!
 * @brief Initializes USB specific setting that was not set by the Clocks tool.
 */
static void USB_DeviceClockInit(uint8_t controllerId)
{
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL, BOARD_USB_PHY_TXCAL45DP, BOARD_USB_PHY_TXCAL45DM,
    };
#endif
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    if (controllerId == kUSB_ControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, 480000000U);
    }
    USB_EhciPhyInit(controllerId, 0, &phyConfig);
#endif
}

static struct ep_id _ehci0_ep_pool[] =
{
    {0x0,  USB_EP_ATTR_CONTROL,     USB_DIR_INOUT,  64, ID_ASSIGNED  },
    {0x1,  USB_EP_ATTR_BULK,        USB_DIR_IN,     64, ID_UNASSIGNED},
    {0x1,  USB_EP_ATTR_BULK,        USB_DIR_OUT,    64, ID_UNASSIGNED},
    {0x2,  USB_EP_ATTR_INT,         USB_DIR_IN,     64, ID_UNASSIGNED},
    {0x2,  USB_EP_ATTR_INT,         USB_DIR_OUT,    64, ID_UNASSIGNED},
    {0x3,  USB_EP_ATTR_BULK,        USB_DIR_IN,     64, ID_UNASSIGNED},
    {0x3,  USB_EP_ATTR_BULK,        USB_DIR_OUT,    64, ID_UNASSIGNED},
    {0x4,  USB_EP_ATTR_INT,         USB_DIR_IN,     64, ID_UNASSIGNED},
    {0x4,  USB_EP_ATTR_INT,         USB_DIR_OUT,    64, ID_UNASSIGNED},
    {0x5,  USB_EP_ATTR_BULK,        USB_DIR_IN,     64, ID_UNASSIGNED},
    {0x5,  USB_EP_ATTR_BULK,        USB_DIR_OUT,    64, ID_UNASSIGNED},
    {0x6,  USB_EP_ATTR_INT,         USB_DIR_IN,     64, ID_UNASSIGNED},
    {0x6,  USB_EP_ATTR_INT,         USB_DIR_OUT,    64, ID_UNASSIGNED},
    {0x7,  USB_EP_ATTR_BULK,        USB_DIR_IN,     64, ID_UNASSIGNED},
    {0x7,  USB_EP_ATTR_BULK,        USB_DIR_OUT,    64, ID_UNASSIGNED},
    {0xFF, USB_EP_ATTR_TYPE_MASK,   USB_DIR_MASK,   0,  ID_ASSIGNED  },
};

/*!
 * @brief USB Interrupt service routine.
 *
 * This function serves as the USB interrupt service routine.
 *
 * @return None.
 */
void USB_OTG1_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();

    USB_DeviceEhciIsrFunction(ehci0_handle);
    /* leave interrupt */
    os_interrupt_leave();
}

static os_err_t _ehci0_ep_set_stall(os_uint8_t address)
{
    USB_DeviceStallEndpoint(ehci0_handle, address);
    return OS_EOK;
}

static os_err_t _ehci0_ep_clear_stall(os_uint8_t address)
{
    USB_DeviceUnstallEndpoint(ehci0_handle, address);
    return OS_EOK;
}

static os_err_t _ehci0_set_address(os_uint8_t address)
{
    USB_DeviceSetStatus(ehci0_handle, kUSB_DeviceStatusAddress, &address);
    return OS_EOK;
}

static os_err_t _ehci0_set_config(os_uint8_t address)
{
    return OS_EOK;
}

static os_err_t _ehci0_ep_enable(uep_t ep)
{
    usb_device_endpoint_init_struct_t ep_init;
    usb_device_endpoint_callback_struct_t ep_callback;
    os_uint32_t param = ep->ep_desc->bEndpointAddress;
    OS_ASSERT(ep != OS_NULL);
    OS_ASSERT(ep->ep_desc != OS_NULL);
    ep_init.maxPacketSize = ep->ep_desc->wMaxPacketSize;
    ep_init.endpointAddress = ep->ep_desc->bEndpointAddress;
    ep_init.transferType = ep->ep_desc->bmAttributes;
    ep_init.zlt = 0;
    ep_callback.callbackFn = usb_device_endpoint_callback;
    ep_callback.callbackParam = (void *)param;
    ep_callback.isBusy = 0;
    USB_DeviceInitEndpoint(ehci0_handle, &ep_init, &ep_callback);
    return OS_EOK;
}
static os_err_t _ehci0_ep_disable(uep_t ep)
{
    OS_ASSERT(ep != OS_NULL);
    OS_ASSERT(ep->ep_desc != OS_NULL);
    USB_DeviceDeinitEndpoint(ehci0_handle, ep->ep_desc->bEndpointAddress);
    return OS_EOK;
}

static os_size_t _ehci0_ep_read(os_uint8_t address, void *buffer)
{
    os_size_t size = 0;

    OS_ASSERT(buffer != OS_NULL);

    return size;
}

static os_size_t _ehci0_ep_read_prepare(os_uint8_t address, void *buffer, os_size_t size)
{
    USB_DeviceRecvRequest(ehci0_handle, address, buffer, size);
    return size;
}

static os_size_t _ehci0_ep_write(os_uint8_t address, void *buffer, os_size_t size)
{
    USB_DeviceSendRequest(ehci0_handle, address, buffer, size);
    return size;
}

static os_err_t _ehci0_ep0_send_status(void)
{
    _ehci0_ep_write(0x00, NULL, 0);
    return OS_EOK;
}

static os_err_t _ehci0_suspend(void)
{
    return OS_EOK;
}

static os_err_t _ehci0_wakeup(void)
{
    return OS_EOK;
}

const static struct udcd_ops _ehci0_udc_ops =
{
    _ehci0_set_address,
    _ehci0_set_config,
    _ehci0_ep_set_stall,
    _ehci0_ep_clear_stall,
    _ehci0_ep_enable,
    _ehci0_ep_disable,
    _ehci0_ep_read_prepare,
    _ehci0_ep_read,
    _ehci0_ep_write,
    _ehci0_ep0_send_status,
    _ehci0_suspend,
    _ehci0_wakeup,
};

static os_err_t drv_ehci0_usbd_init(os_device_t device)
{
    usb_status_t result;
    USB_DeviceClockInit(kUSB_ControllerEhci0);

    result = USB_DeviceInit(kUSB_ControllerEhci0, usb_device_callback, &ehci0_handle);
    OS_ASSERT(ehci0_handle);
    if(result == kStatus_USB_Success)
    {
        USB_DeviceIsrEnable(kUSB_ControllerEhci0);
        USB_DeviceRun(ehci0_handle);
    }
    else
    {
        os_kprintf("USB_DeviceInit ehci0 error\r\n");
        return OS_ERROR;
    }
    return OS_EOK;
}

static int rt_usbd_init(void)
{
    os_memset((void *)&_fsl_udc_0, 0, sizeof(struct udcd));
    _fsl_udc_0.parent.type = OS_DEVICE_TYPE_USBDEVICE;
    _fsl_udc_0.parent.init = drv_ehci0_usbd_init;
    _fsl_udc_0.ops = &_ehci0_udc_ops;
    /* Register endpoint infomation */
    _fsl_udc_0.ep_pool = _ehci0_ep_pool;
    _fsl_udc_0.ep0.id = &_ehci0_ep_pool[0];

    _fsl_udc_0.device_is_hs = OS_FALSE;
    os_device_register((os_device_t)&_fsl_udc_0, "usbd", 0);
    rt_usb_device_init();

    return 0;
}
INIT_DEVICE_EXPORT(rt_usbd_init);

static usb_status_t usb_device_endpoint_callback(usb_device_handle handle, usb_device_endpoint_callback_message_struct_t *message, void *callbackParam)
{
    os_uint32_t ep_addr = (os_uint32_t)callbackParam;
    usb_device_struct_t *deviceHandle = (usb_device_struct_t *)handle;
    udcd_t udcd = OS_NULL;
    uint8_t state;
    if(deviceHandle->controllerId == kUSB_ControllerEhci0)
        udcd = &_fsl_udc_0;

    if(message->isSetup)
    {
        rt_usbd_ep0_setup_handler(udcd, (struct urequest*)message->buffer);
    }
    else if(ep_addr == 0x00)
    {
        USB_DeviceGetStatus(handle, kUSB_DeviceStatusDeviceState, &state);
        if(state == kUSB_DeviceStateAddressing)
        {
            if (kStatus_USB_Success == USB_DeviceSetStatus(handle, kUSB_DeviceStatusAddress, NULL))
            {
                state = kUSB_DeviceStateAddress;
                USB_DeviceSetStatus(handle, kUSB_DeviceStatusDeviceState, &state);
            }
        }
        rt_usbd_ep0_out_handler(udcd, message->length);
    }
    else if(ep_addr == 0x80)
    {
        USB_DeviceGetStatus(handle, kUSB_DeviceStatusDeviceState, &state);
        if(state == kUSB_DeviceStateAddressing)
        {
            if (kStatus_USB_Success == USB_DeviceSetStatus(handle, kUSB_DeviceStatusAddress, NULL))
            {
                state = kUSB_DeviceStateAddress;
                USB_DeviceSetStatus(handle, kUSB_DeviceStatusDeviceState, &state);
            }
        }
        rt_usbd_ep0_in_handler(udcd);
    }
    else if(ep_addr & 0x80)
    {
        rt_usbd_ep_in_handler(udcd, ep_addr, message->length);
    }
    else
    {
        rt_usbd_ep_out_handler(udcd, ep_addr, message->length);
    }
    return kStatus_USB_Success;
}

static usb_status_t usb_device_callback(usb_device_handle handle, uint32_t callbackEvent, void *eventParam)
{
    usb_status_t error = kStatus_USB_Error;
    usb_device_struct_t *deviceHandle = (usb_device_struct_t *)handle;
    usb_device_endpoint_init_struct_t ep0_init =
    {
        0x40,
        0x00,
        USB_EP_ATTR_CONTROL,
        0
    };
    usb_device_endpoint_callback_struct_t ep0_callback =
    {
        usb_device_endpoint_callback,
        0,
        0
    };
    udcd_t udcd = OS_NULL;
    if(deviceHandle->controllerId == kUSB_ControllerEhci0)
        udcd = &_fsl_udc_0;

    switch (callbackEvent)
    {
    case kUSB_DeviceEventBusReset:
        ep0_init.endpointAddress = 0x00;
        ep0_callback.callbackParam = (void *)0x00;
        USB_DeviceInitEndpoint(deviceHandle, &ep0_init, &ep0_callback);
        ep0_init.endpointAddress = 0x80;
        ep0_callback.callbackParam = (void *)0x80;
        USB_DeviceInitEndpoint(deviceHandle, &ep0_init, &ep0_callback);
        rt_usbd_reset_handler(udcd);
        break;
    case kUSB_DeviceEventAttach:
        rt_usbd_connect_handler(udcd);
        break;
    case kUSB_DeviceEventDetach:
        rt_usbd_disconnect_handler(udcd);
        break;
    }
    return error;
}

/********************* end of file ************************/
