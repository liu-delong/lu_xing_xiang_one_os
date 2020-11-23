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

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif
#if ((defined(FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U)) && \
    ((defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
     (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))))
#include "usb_phy.h"
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
#include "fsl_power.h"
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if !((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "fsl_mrt.h"
#endif
#include "fsl_power.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)) && \
	!(defined(FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#define CHIRP_ISSUE_WORKAROUND_NEEDED 1
#else
#define CHIRP_ISSUE_WORKAROUND_NEEDED 0
#endif
 

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

static void USB_DeviceClockInit(void);

#if CHIRP_ISSUE_WORKAROUND_NEEDED
void USB_DeviceHsPhyChirpIssueWorkaround(void);
void USB_DeviceDisconnected(void);
#endif

#if defined (USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
static clock_usbfs_src_t USB0_GetClockSource(void);
#endif

static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
static usb_status_t USB_UpdateInterfaceSetting(uint8_t interface, uint8_t alternateSetting);

extern usb_status_t USB_DeviceInterface0HidMouseInit(usb_device_composite_struct_t *deviceComposite);
extern usb_status_t USB_DeviceInterface0HidMouseCallback(class_handle_t handle, uint32_t event, void *param);
extern usb_status_t USB_DeviceInterface0HidMouseSetConfiguration(class_handle_t handle, uint8_t configuration_index);
extern usb_status_t USB_DeviceInterface0HidMouseSetInterface(class_handle_t handle, uint8_t alternateSetting);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if CHIRP_ISSUE_WORKAROUND_NEEDED
volatile uint32_t hwTick;
uint32_t timerInterval;
uint32_t isConnectedToFsHost = 0U;
uint32_t isConnectedToHsHost = 0U;
#endif

#if (defined(USB_DEVICE_CHARGER_DETECT_ENABLE) && (USB_DEVICE_CHARGER_DETECT_ENABLE > 0U)) && \
    ((defined(FSL_FEATURE_SOC_USBDCD_COUNT) && (FSL_FEATURE_SOC_USBDCD_COUNT > 0U)) ||        \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)))
usb_device_dcd_charging_time_t g_UsbDeviceDcdTimingConfig;
#endif

usb_device_composite_struct_t g_UsbDeviceComposite;

extern usb_device_class_struct_t g_UsbDeviceInterface0HidMouseConfig;

/* Set class configurations. */
usb_device_class_config_struct_t g_CompositeClassConfig[USB_COMPOSITE_INTERFACE_COUNT] = {
    {
        USB_DeviceInterface0HidMouseCallback, (class_handle_t)NULL, &g_UsbDeviceInterface0HidMouseConfig,
    },
};

/* Set class configuration list. */
usb_device_class_config_list_struct_t g_UsbDeviceCompositeConfigList = {
    g_CompositeClassConfig, USB_DeviceCallback, USB_COMPOSITE_INTERFACE_COUNT,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

#if defined (USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
/*!
 * @brief Function to retrieve clock source for USB0.
 *
 * @return Used clock source
 */
static clock_usbfs_src_t USB0_GetClockSource()
{
    clock_usbfs_src_t src;
    switch (SYSCON->USB0CLKSEL)
    {
        case 0U:
            src = kCLOCK_UsbfsSrcMainClock;
            break;
        case 1U:
            src = kCLOCK_UsbfsSrcPll0;
            break;
        case 3U:
            src = kCLOCK_UsbfsSrcFro;
            break;
        case 5U:
            src = kCLOCK_UsbfsSrcPll1;
            break;
        default:
            src = kCLOCK_UsbfsSrcNone;
            break;
    }
    return src;
}
#endif
#if CHIRP_ISSUE_WORKAROUND_NEEDED
/*!
 * @brief Initialization on MRT timer
 *
 * @param uint8_t Instance of timer.
 * @interval uint32_t Interrupt interval.
 *
 */
void USB_TimerInit(uint8_t instance, uint32_t interval)
{
    MRT_Type *instanceList[] = MRT_BASE_PTRS;
    IRQn_Type instanceIrq[] = MRT_IRQS;
    /* Structure of initialize MRT */
    mrt_config_t mrtConfig;
    /* mrtConfig.enableMultiTask = false; */
    MRT_GetDefaultConfig(&mrtConfig);
    /* Init mrt module */
    MRT_Init(instanceList[instance], &mrtConfig);
    /* Setup Channel 0 to be repeated */
    MRT_SetupChannelMode(instanceList[instance], kMRT_Channel_0, kMRT_RepeatMode);
    /* Enable timer interrupts for channel 0 */
    MRT_EnableInterrupts(instanceList[instance], kMRT_Channel_0, kMRT_TimerInterruptEnable);
    timerInterval = interval;
    /* Enable at the NVIC */
    EnableIRQ(instanceIrq[instance]);
}

/*!
 * @brief Function used to start/stop the MRT timer
 *
 * @param uint8_t MRT timer instance.
 * @param uint8_t Set to 0 to disable the timer. Other values enable the timer.
 */
void USB_TimerInt(uint8_t instance, uint8_t enable)
{
    MRT_Type *instanceList[] = MRT_BASE_PTRS;
    uint32_t mrt_clock;
    mrt_clock = CLOCK_GetFreq(kCLOCK_BusClk);
    if (enable)
    {
        /* Start channel 0 */
        MRT_StartTimer(instanceList[instance], kMRT_Channel_0, USEC_TO_COUNT(timerInterval, mrt_clock));
    }
    else
    {
        /* Stop channel 0 */
        MRT_StopTimer(instanceList[instance], kMRT_Channel_0);
        /* Clear interrupt flag.*/
        MRT_ClearStatusFlags(instanceList[instance], kMRT_Channel_0, kMRT_TimerInterruptFlag);
    }
}

/*!
 * @brief MRT0 interrupt service routine
 */
void MRT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    MRT_ClearStatusFlags(MRT0, kMRT_Channel_0, kMRT_TimerInterruptFlag);
    if (hwTick)
    {
        hwTick--;
        if (!hwTick)
        {
            USB_TimerInt(0, 0);
        }
    }
    else
    {
        USB_TimerInt(0, 0);
    }
}

/*!
 * @brief Clears device connected flag for chirp issue
 */
void USB_DeviceDisconnected(void)
{
    isConnectedToFsHost = 0U;
}

/*!
 * @brief Chirp issue workaround
 *
 * This is a work-around to fix the HS device Chirping issue.
 * The device (IP3511HS controller) will sometimes not work when the cable
 * is attached first time after a Power-on Reset.
 */
void USB_DeviceHsPhyChirpIssueWorkaround(void)
{
    uint32_t startFrame = USBHSD->INFO & USBHSD_INFO_FRAME_NR_MASK;
    uint32_t currentFrame;
    uint32_t isConnectedToFsHostFlag = 0U;

    if ((!isConnectedToHsHost) && (!isConnectedToFsHost))
    {
        if (((USBHSD->DEVCMDSTAT & USBHSD_DEVCMDSTAT_Speed_MASK) >> USBHSD_DEVCMDSTAT_Speed_SHIFT) == 0x01U)
        {
            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U | USBHSD_DEVCMDSTAT_PHY_TEST_MODE_MASK))) |
                                 USBHSD_DEVCMDSTAT_PHY_TEST_MODE(0x05U);
            hwTick = 100;
            USB_TimerInt(0, 1);

            while (hwTick)
            {
            }

            currentFrame = USBHSD->INFO & USBHSD_INFO_FRAME_NR_MASK;

            if (currentFrame != startFrame)
            {
                isConnectedToHsHost = 1U;
            }
            else
            {
                hwTick = 1;
                USB_TimerInt(0, 1);

                while (hwTick)
                {
                }

                currentFrame = USBHSD->INFO & USBHSD_INFO_FRAME_NR_MASK;

                if (currentFrame != startFrame)
                {
                    isConnectedToHsHost = 1U;
                }
                else
                {
                    isConnectedToFsHostFlag = 1U;
                }
            }

            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U | USBHSD_DEVCMDSTAT_PHY_TEST_MODE_MASK)));
            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U | USBHSD_DEVCMDSTAT_DCON_MASK)));
            hwTick = 510;
            USB_TimerInt(0, 1);

            while (hwTick)
            {
            }

            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U))) | USB_DEVCMDSTAT_DCON_C_MASK;
            USBHSD->DEVCMDSTAT =
                (USBHSD->DEVCMDSTAT & (~(0x0F000000U))) | USBHSD_DEVCMDSTAT_DCON_MASK | USB_DEVCMDSTAT_DRES_C_MASK;

            if (isConnectedToFsHostFlag)
            {
                isConnectedToFsHost = 1U;
            }
        }
    }
}
#endif

/*!
 * @brief USB Interrupt service routine.
 *
 * This function serves as the USB interrupt service routine.
 *
 * @return None.
 */
void USB0_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_UsbDeviceComposite.deviceHandle);

#if (defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)) || (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
#endif
}

/*!
 * @brief Initializes USB specific setting that was not set by the Clocks tool.
 */
static void USB_DeviceClockInit(void)
{
#if defined (USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    /* Turn on USB Phy */
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY);

    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);

    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;

    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);

    if (USB0_GetClockSource() == kCLOCK_UsbfsSrcFro)
    {
        /* Turn ON FRO HF and let it adjust TRIM value based on USB SOF */
        ANACTRL->FRO192M_CTRL = (ANACTRL->FRO192M_CTRL & ~(ANACTRL_FRO192M_CTRL_USBCLKADJ_MASK)) | ANACTRL_FRO192M_CTRL_USBCLKADJ(1U);

    }

    CLOCK_EnableClock(kCLOCK_Usbd0);
    CLOCK_EnableClock(kCLOCK_UsbRam1);

#if defined(FSL_FEATURE_USB_USB_RAM) && (FSL_FEATURE_USB_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USB_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USB_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif
#endif

#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    /* enable usb1 host clock */
    CLOCK_EnableClock(kCLOCK_Usbh1);

    /* Put PHY powerdown under software control */
    *((uint32_t *)(USBHSH_BASE + 0x50)) = USBHSH_PORTMODE_SW_PDCOM_MASK;
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBHSH_BASE + 0x50)) |= USBHSH_PORTMODE_DEV_ENABLE_MASK;

    /* enable usb1 host clock */
    CLOCK_DisableClock(kCLOCK_Usbh1);

#if CHIRP_ISSUE_WORKAROUND_NEEDED
    USB_TimerInit(0, 1000U);
#endif

    POWER_DisablePD(kPDRUNCFG_PD_LDOUSBHS); /*!< Ensure xtal32k is on  */

    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY); /* Turn on power for USB PHY */
    uint32_t delay = 100000;
    while (delay --)
    {
        __asm("nop");
    }

    CLOCK_EnableClock(kCLOCK_Usbd1);
    CLOCK_EnableClock(kCLOCK_UsbRam1);
    CLOCK_EnableClock(kCLOCK_Usb1Clk);

	uint32_t pllDivSelValue = 0;
	uint32_t inputFrequency = CLOCK_GetUsb1ClkFreq();
    switch(CLOCK_GetUsb1ClkFreq()) {
    case 32000000:
    	pllDivSelValue = 0;
    	break;
    case 30000000:
    	pllDivSelValue = 1;
    	break;
    case 24000000:
    	pllDivSelValue = 2;
    	break;
    case 20000000:
    	pllDivSelValue = 4;
    	break;
    case 19200000:
    	pllDivSelValue = 5;
    	break;
    case 16000000:
    	pllDivSelValue = 6;
    	break;
    case 12000000:
    	pllDivSelValue = 7;
    	break;
    default:
    	usb_echo("Unsupported input frequency (%d) for USB PHY.\r\n", inputFrequency);
    }

    USBPHY->CTRL_CLR    = USBPHY_CTRL_SFTRST_MASK;
    USBPHY->PLL_SIC     = (USBPHY->PLL_SIC & ~USBPHY_PLL_SIC_PLL_DIV_SEL(0x7)) | USBPHY_PLL_SIC_PLL_DIV_SEL(pllDivSelValue);
    USBPHY->PLL_SIC_SET = USBPHY_PLL_SIC_SET_PLL_REG_ENABLE_MASK;
    USBPHY->PLL_SIC_CLR = (1 << 16);
    USBPHY->PLL_SIC_SET = USBPHY_PLL_SIC_SET_PLL_POWER_MASK;
    USBPHY->PLL_SIC_SET = USBPHY_PLL_SIC_SET_PLL_EN_USB_CLKS_MASK;

    USBPHY->CTRL_CLR = USBPHY_CTRL_CLR_CLKGATE_MASK;
    USBPHY->PWD_SET  = 0x0;

    uint32_t not_used = 0;
    USB_EhciPhyInit(USB_DEVICE_CONTROLLER_ID, not_used, NULL);

#if defined(FSL_FEATURE_USBHSD_USB_RAM) && (FSL_FEATURE_USBHSD_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USBHSD_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif
#endif
}

/*!
 * @brief Enables interrupt service routines for device.
 */
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    uint8_t usbDeviceKhciIrq[] = USB_IRQS;
    irqNumber = usbDeviceKhciIrq[USB_DEVICE_CONTROLLER_ID - kUSB_ControllerKhci0];
#endif
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber = usbDeviceEhciIrq[USB_DEVICE_CONTROLLER_ID - kUSB_ControllerEhci0];
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
    irqNumber = usbDeviceIP3511Irq[USB_DEVICE_CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
    irqNumber = usbDeviceIP3511Irq[USB_DEVICE_CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0];
#endif
/* Install isr, set priority, and enable IRQ. */
#if defined(__GIC_PRIO_BITS)
    GIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
#else
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
#endif
    EnableIRQ((IRQn_Type)irqNumber);
}

#if USB_DEVICE_CONFIG_USE_TASK
/*!
 * @brief USB device task. When USB device does not use interrupt service routines, this function should be called periodically.
 *
 * @param *deviceHandle Pointer to device handle.
 */
void USB_DeviceTaskFn(void *deviceHandle)
{
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
        USB_DeviceKhciTaskFunction(deviceHandle);
#endif
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
        USB_DeviceEhciTaskFunction(deviceHandle);
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
}
#endif

/*!
 * @brief Standard device callback function.
 *
 * This function handle the USB standard event. For more information, please refer to usb spec chapter 9.
 * @param handle          The USB device handle.
 * @param event           The USB device event type.
 * @param *param          The parameter of the device specific request.
 * @return usb_status_t Returns status of operation.
 */
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;
    uint16_t *temp16 = (uint16_t *)param;
    uint8_t *temp8 = (uint8_t *)param;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
            /* USB bus reset signal detected */
            g_UsbDeviceComposite.attach = 0U;
            error = kStatus_USB_Success;
#if CHIRP_ISSUE_WORKAROUND_NEEDED
            /* The work-around is used to fix the HS device Chirping issue.
             * Please refer to the implementation for the detail information.
             */
            USB_DeviceHsPhyChirpIssueWorkaround();
#endif
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(USB_DEVICE_CONTROLLER_ID, &g_UsbDeviceComposite.speed))
            {
                USB_DeviceSetSpeed(handle, g_UsbDeviceComposite.speed);
            }
#endif
            break;
#if (defined(USB_DEVICE_CONFIG_DETACH_ENABLE) && (USB_DEVICE_CONFIG_DETACH_ENABLE > 0U))
        case kUSB_DeviceEventAttach:
#if (defined(USB_DEVICE_CHARGER_DETECT_ENABLE) && (USB_DEVICE_CHARGER_DETECT_ENABLE > 0U)) && \
    ((defined(FSL_FEATURE_SOC_USBDCD_COUNT) && (FSL_FEATURE_SOC_USBDCD_COUNT > 0U)) ||        \
     (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)))
            g_UsbDeviceComposite.vReginInterruptDetected = 1;
            g_UsbDeviceComposite.vbusValid = 1;
#else
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
#else
            USB_DeviceRun(g_UsbDeviceComposite.deviceHandle);
#endif
#endif
            break;
        case kUSB_DeviceEventDetach:
#if (defined(USB_DEVICE_CHARGER_DETECT_ENABLE) && (USB_DEVICE_CHARGER_DETECT_ENABLE > 0U)) && \
    ((defined(FSL_FEATURE_SOC_USBDCD_COUNT) && (FSL_FEATURE_SOC_USBDCD_COUNT > 0U)) ||        \
     (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)))
            g_UsbDeviceComposite.vReginInterruptDetected = 1;
            g_UsbDeviceComposite.vbusValid = 0;
            g_UsbDeviceComposite.attach = 0;
#else
            g_UsbDeviceComposite.attach = 0;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U))
        USB_DeviceStop(g_UsbDeviceComposite.deviceHandle);
#else
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            USB_DeviceDisconnected();
#endif
#endif
#endif
            break;
#endif
        case kUSB_DeviceEventSetConfiguration:
            if (0 == temp8)
            {
                g_UsbDeviceComposite.attach = 0U;
                g_UsbDeviceComposite.currentConfiguration = 0U;
                /* The device is detached - the zero configuration is set. */
                USB_DeviceInterface0HidMouseSetConfiguration(g_UsbDeviceComposite.interface0HidMouseHandle, *temp8);
            }
            else if (USB_COMPOSITE_CONFIGURATION_INDEX == (*temp8))
            {
                /* Set device configuration request */
                g_UsbDeviceComposite.attach = 1U;
                g_UsbDeviceComposite.currentConfiguration = *temp8;
                USB_DeviceInterface0HidMouseSetConfiguration(g_UsbDeviceComposite.interface0HidMouseHandle, *temp8);
                error = kStatus_USB_Success;
            }
            else
            {
                error = kStatus_USB_InvalidRequest;
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (g_UsbDeviceComposite.attach)
            {
                /* Set device interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);

                if (interface < USB_COMPOSITE_INTERFACE_COUNT)
                {
                    error = USB_UpdateInterfaceSetting(interface, alternateSetting);
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                /* Get current configuration request */
                *temp8 = g_UsbDeviceComposite.currentConfiguration;
                error = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                /* Get current alternate setting of the interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_COMPOSITE_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_UsbDeviceComposite.currentInterfaceAlternateSetting[interface];
                    error = kStatus_USB_Success;
                }
                else
                {
                    error = kStatus_USB_InvalidRequest;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                /* Get device descriptor request */
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                /* Get device configuration descriptor request */
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                /* Get device string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
#if (USB_DEVICE_CONFIG_HID > 0U)
        case kUSB_DeviceEventGetHidDescriptor:
            if (param)
            {
                /* Get hid descriptor request */
                error = USB_DeviceGetHidDescriptor(handle, (usb_device_get_hid_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetHidReportDescriptor:
            if (param)
            {
                /* Get hid report descriptor request */
                error =
                    USB_DeviceGetHidReportDescriptor(handle, (usb_device_get_hid_report_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetHidPhysicalDescriptor:
            if (param)
            {
                /* Get hid physical descriptor request */
                error = USB_DeviceGetHidPhysicalDescriptor(handle,
                                                           (usb_device_get_hid_physical_descriptor_struct_t *)param);
            }
            break;
#endif /* USB_DEVICE_CONFIG_HID */
#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
        case kUSB_DeviceEventGetDeviceQualifierDescriptor:
            if (param)
            {
                /* Get Qualifier descriptor request */
                error = USB_DeviceGetDeviceQualifierDescriptor(
                    handle, (usb_device_get_device_qualifier_descriptor_struct_t *)param);
            }
            break;
#endif
#if (defined(USB_DEVICE_CHARGER_DETECT_ENABLE) && (USB_DEVICE_CHARGER_DETECT_ENABLE > 0U)) && \
    ((defined(FSL_FEATURE_SOC_USBDCD_COUNT) && (FSL_FEATURE_SOC_USBDCD_COUNT > 0U)) ||        \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)))
        case kUSB_DeviceEventDcdTimeOut:
            if (g_UsbDeviceComposite.dcdDevStatus == kUSB_DeviceDCDDevStatusVBUSDetect)
            {
                g_UsbDeviceComposite.dcdDevStatus = kUSB_DeviceDCDDevStatusTimeOut;
            }
            break;
        case kUSB_DeviceEventDcdUnknownType:
            if (g_UsbDeviceComposite.dcdDevStatus == kUSB_DeviceDCDDevStatusVBUSDetect)
            {
                g_UsbDeviceComposite.dcdDevStatus = kUSB_DeviceDCDDevStatusUnknownType;
            }
            break;
        case kUSB_DeviceEventSDPDetected:
            if (g_UsbDeviceComposite.dcdDevStatus == kUSB_DeviceDCDDevStatusVBUSDetect)
            {
                g_UsbDeviceComposite.dcdPortType = kUSB_DeviceDCDPortTypeSDP;
                g_UsbDeviceComposite.dcdDevStatus = kUSB_DeviceDCDDevStatusDetectFinish;
            }
            break;
        case kUSB_DeviceEventChargingPortDetected:
            if (g_UsbDeviceComposite.dcdDevStatus == kUSB_DeviceDCDDevStatusVBUSDetect)
            {
                g_UsbDeviceComposite.dcdDevStatus = kUSB_DeviceDCDDevStatusChargingPortDetect;
            }
            break;
        case kUSB_DeviceEventChargingHostDetected:
            if (g_UsbDeviceComposite.dcdDevStatus == kUSB_DeviceDCDDevStatusVBUSDetect)
            {
                g_UsbDeviceComposite.dcdDevStatus = kUSB_DeviceDCDDevStatusDetectFinish;
                g_UsbDeviceComposite.dcdPortType = kUSB_DeviceDCDPortTypeCDP;
            }
            break;
        case kUSB_DeviceEventDedicatedChargerDetected:
            if (g_UsbDeviceComposite.dcdDevStatus == kUSB_DeviceDCDDevStatusVBUSDetect)
            {
                g_UsbDeviceComposite.dcdDevStatus = kUSB_DeviceDCDDevStatusDetectFinish;
                g_UsbDeviceComposite.dcdPortType = kUSB_DeviceDCDPortTypeDCP;
            }
            break;
#endif       
        default:
            break;
    }
    return error;
}

/*!
 * @brief Select interface and call setInterface callback.
 *
 * @param interface          Number of interface to be set to.
 * @param alternateSetting    Alternate setting to be used.
 * @return usb_status_t Returns status of operation.
 */
usb_status_t USB_UpdateInterfaceSetting(uint8_t interface, uint8_t alternateSetting)
{
    usb_status_t ret = kStatus_USB_Error;

    /* select appropriate interface to be updated*/
    switch (interface)
    {

    case USB_INTERFACE_0_HID_MOUSE_INDEX:
        ret = USB_DeviceInterface0HidMouseSetInterface(g_UsbDeviceComposite.interface0HidMouseHandle, alternateSetting);
        break;
    }

    if (ret == kStatus_USB_Success)
    {
        //interface setting was set
        g_UsbDeviceComposite.currentInterfaceAlternateSetting[interface] = alternateSetting;
    }

    return ret;
}

/*!
 * @brief Completely initializes USB device.
 *
 * This function calls other USB device functions and directly initializes following: USB specific clocks, g_UsbDeviceComposite values, USB stack, class drivers and device isr.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceApplicationInit(void)
{
	usb_status_t status;

    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    g_UsbDeviceComposite.speed = USB_SPEED_FULL;
    g_UsbDeviceComposite.attach = 0U;
    g_UsbDeviceComposite.interface0HidMouseHandle = (class_handle_t)NULL;
    g_UsbDeviceComposite.deviceHandle = NULL;

#if (defined(USB_DEVICE_CHARGER_DETECT_ENABLE) && (USB_DEVICE_CHARGER_DETECT_ENABLE > 0U)) && \
    ((defined(FSL_FEATURE_SOC_USBDCD_COUNT) && (FSL_FEATURE_SOC_USBDCD_COUNT > 0U)) ||        \
     (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)))
    g_UsbDeviceComposite.dcdDevStatus = kUSB_DeviceDCDDevStatusDetached;

    g_UsbDeviceDcdTimingConfig.dcdSeqInitTime = USB_DEVICE_DCD_SEQ_INIT_TIME;
    g_UsbDeviceDcdTimingConfig.dcdDbncTime = USB_DEVICE_DCD_DBNC_MSEC;
    g_UsbDeviceDcdTimingConfig.dcdDpSrcOnTime = USB_DEVICE_DCD_VDPSRC_ON_MSEC;
    g_UsbDeviceDcdTimingConfig.dcdTimeWaitAfterPrD = USB_DEVICE_DCD_TIME_WAIT_AFTER_PRI_DETECTION;
    g_UsbDeviceDcdTimingConfig.dcdTimeDMSrcOn = USB_DEVICE_DCD_TIME_DM_SRC_ON;
#endif

    /* Initialize the usb stack and class drivers. */
    status = USB_DeviceClassInit(USB_DEVICE_CONTROLLER_ID, &g_UsbDeviceCompositeConfigList, &g_UsbDeviceComposite.deviceHandle);

    if (kStatus_USB_Success != status)
    {
        return status;
    }
    
    /* Get the class handle. */
    g_UsbDeviceComposite.interface0HidMouseHandle = g_UsbDeviceCompositeConfigList.config[0].classHandle;
    USB_DeviceInterface0HidMouseInit(&g_UsbDeviceComposite);

    USB_DeviceIsrEnable();

#if (defined(USB_DEVICE_CHARGER_DETECT_ENABLE) && (USB_DEVICE_CHARGER_DETECT_ENABLE > 0U)) && \
    ((defined(FSL_FEATURE_SOC_USBDCD_COUNT) && (FSL_FEATURE_SOC_USBDCD_COUNT > 0U)) ||        \
     (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)))
#else    
    USB_DeviceRun(g_UsbDeviceComposite.deviceHandle);
#endif
    return status;
}

/*!
 * @brief USB device charger task function.
 *
 * This function handles the USB device charger events.
 * @param usbDeviceCompositeDcd Device charger configuration.
 * @return None.
 */
#if (defined(USB_DEVICE_CHARGER_DETECT_ENABLE) && (USB_DEVICE_CHARGER_DETECT_ENABLE > 0U)) && \
    ((defined(FSL_FEATURE_SOC_USBDCD_COUNT) && (FSL_FEATURE_SOC_USBDCD_COUNT > 0U)) ||        \
     (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)))
void USB_DeviceChargerTask(usb_device_composite_struct_t *usbDeviceCompositeDcd)
{
    if (usbDeviceCompositeDcd->vReginInterruptDetected)
    {
        usbDeviceCompositeDcd->vReginInterruptDetected = 0;
        if (usbDeviceCompositeDcd->vbusValid)
        {
            USB_DeviceDcdInitModule(usbDeviceCompositeDcd->deviceHandle, &g_UsbDeviceDcdTimingConfig);
            usbDeviceCompositeDcd->dcdDevStatus = kUSB_DeviceDCDDevStatusVBUSDetect;
        }
        else
        {
            USB_DeviceDcdDeinitModule(usbDeviceCompositeDcd->deviceHandle);
            USB_DeviceStop(usbDeviceCompositeDcd->deviceHandle);
            usbDeviceCompositeDcd->dcdPortType = kUSB_DeviceDCDPortTypeNoPort;
            usbDeviceCompositeDcd->dcdDevStatus = kUSB_DeviceDCDDevStatusDetached;
        }
    }

    if (usbDeviceCompositeDcd->dcdDevStatus == kUSB_DeviceDCDDevStatusChargingPortDetect) /* This is only for BC1.1 */
    {
        USB_DeviceRun(usbDeviceCompositeDcd->deviceHandle);
    }
    if (usbDeviceCompositeDcd->dcdDevStatus == kUSB_DeviceDCDDevStatusTimeOut)
    {
        usb_echo("Timeout error.\r\n");
        usbDeviceCompositeDcd->dcdDevStatus = kUSB_DeviceDCDDevStatusComplete;
    }
    if (usbDeviceCompositeDcd->dcdDevStatus == kUSB_DeviceDCDDevStatusUnknownType)
    {
        usb_echo("Unknown port type.\r\n");
        usbDeviceCompositeDcd->dcdDevStatus = kUSB_DeviceDCDDevStatusComplete;
    }
    if (usbDeviceCompositeDcd->dcdDevStatus == kUSB_DeviceDCDDevStatusDetectFinish)
    {
        if (usbDeviceCompositeDcd->dcdPortType == kUSB_DeviceDCDPortTypeSDP)
        {
            USB_DeviceRun(usbDeviceCompositeDcd->deviceHandle); /* If the facility attached is SDP, start enumeration */
        }
        else if (usbDeviceCompositeDcd->dcdPortType == kUSB_DeviceDCDPortTypeCDP)
        {
            USB_DeviceRun(usbDeviceCompositeDcd->deviceHandle); /* If the facility attached is CDP, start enumeration */
        }
        
        usbDeviceCompositeDcd->dcdDevStatus = kUSB_DeviceDCDDevStatusComplete;
    }
}
#endif

/*!
 * @brief USB device tasks function.
 *
 * This function runs the tasks for USB device.
 *
 * @return None.
 */
void USB_DeviceTasks(void)
{
#if USB_DEVICE_CONFIG_USE_TASK
    USB_DeviceTaskFn(g_UsbDeviceComposite.deviceHandle);
#endif
#if (defined(USB_DEVICE_CHARGER_DETECT_ENABLE) && (USB_DEVICE_CHARGER_DETECT_ENABLE > 0U)) && \
    ((defined(FSL_FEATURE_SOC_USBDCD_COUNT) && (FSL_FEATURE_SOC_USBDCD_COUNT > 0U)) ||        \
     (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)))
    USB_DeviceChargerTask(&g_UsbDeviceComposite);
#endif
}

