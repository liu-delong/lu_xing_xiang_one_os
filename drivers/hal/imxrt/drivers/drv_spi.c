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
 * @file        drv_spi.c
 *
 * @brief       This file implements spi driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <drv_log.h>

#include <board.h>
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_memory.h>
#include <drv_gpio.h>
#include <drv_common.h>
#include <drv_spi.h>
#include <string.h>

#if defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL
    #error "Please don't define 'FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL'!"
#endif

os_err_t os_hw_spi_device_attach(const char *bus_name, const char *device_name, os_base_t pin)
{
    os_err_t ret = OS_EOK; 
    
    struct os_spi_device *spi_device = (struct os_spi_device *)os_malloc(sizeof(struct os_spi_device)); 
    OS_ASSERT(spi_device != OS_NULL); 
    
    struct imxrt_sw_spi_cs *cs_pin = (struct imxrt_sw_spi_cs *)os_malloc(sizeof(struct imxrt_sw_spi_cs)); 
    OS_ASSERT(cs_pin != OS_NULL);
    
    cs_pin->pin = pin;
    os_pin_mode(pin, PIN_MODE_OUTPUT); 
    os_pin_write(pin, PIN_HIGH); 
    
    ret = os_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)cs_pin); 
    
    return ret; 
}

static uint32_t imxrt_get_lpspi_freq(void)
{
    uint32_t freq = 0;

    /* CLOCK_GetMux(kCLOCK_LpspiMux):
       00b: derive clock from PLL3 PFD1 720M 
       01b: derive clock from PLL3 PFD0 720M 
       10b: derive clock from PLL2      528M 
       11b: derive clock from PLL2 PFD2 396M 
    */
    switch(CLOCK_GetMux(kCLOCK_LpspiMux))
    {
    case 0:
        freq = CLOCK_GetFreq(kCLOCK_Usb1PllPfd1Clk); 
        break; 
        
    case 1:
        freq = CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk); 
        break; 
    
    case 2:
        freq = CLOCK_GetFreq(kCLOCK_SysPllClk); 
        break; 
    
    case 3:
        freq = CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk); 
        break; 
    }
    
    freq /= (CLOCK_GetDiv(kCLOCK_LpspiDiv) + 1U); 

    return freq;
}

static os_err_t imxrt_spi_configure(struct os_spi_device *device, struct os_spi_configuration *cfg)
{
    lpspi_master_config_t masterConfig; 
    imxrt_spi_t *spi = OS_NULL; 

    OS_ASSERT(cfg != OS_NULL);
    OS_ASSERT(device != OS_NULL);

    spi = (imxrt_spi_t *)(device->bus->parent.user_data);
    OS_ASSERT(spi != OS_NULL);

    if(cfg->data_width != 8 && cfg->data_width != 16 && cfg->data_width != 32)
    {
        os_kprintf("invalid data width:%d\n", cfg->data_width);
        return OS_EINVAL; 
    }

    os_kprintf("SPI CFG:0x%x 0x%x 0x%x\n", cfg->mode, cfg->data_width, cfg->max_hz);
    
    LPSPI_MasterGetDefaultConfig(&masterConfig); 
    
    if(cfg->max_hz > 40*1000*1000)
    {
        cfg->max_hz = 40*1000*1000;
    }
    masterConfig.baudRate     = cfg->max_hz; 
    masterConfig.bitsPerFrame = cfg->data_width; 
    
    if(cfg->mode & OS_SPI_MSB)
    {
        masterConfig.direction = kLPSPI_MsbFirst; 
    }
    else
    {
        masterConfig.direction = kLPSPI_LsbFirst; 
    }
    
    if(cfg->mode & OS_SPI_CPHA)
    {
        masterConfig.cpha = kLPSPI_ClockPhaseSecondEdge; 
    }
    else
    {
        masterConfig.cpha = kLPSPI_ClockPhaseFirstEdge; 
    }
    
    if(cfg->mode & OS_SPI_CPOL)
    {
        masterConfig.cpol = kLPSPI_ClockPolarityActiveLow; 
    }
    else
    {
        masterConfig.cpol = kLPSPI_ClockPolarityActiveHigh; 
    }

#if 0
    if(cfg->mode & OS_SPI_CS_HIGH)
    {
        masterConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveHigh;
    }
    else
    {
        masterConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;
    }
#endif
    masterConfig.pinCfg                        = kLPSPI_SdiInSdoOut; 
    masterConfig.dataOutConfig                 = kLpspiDataOutTristate;
    masterConfig.pcsToSckDelayInNanoSec        = 1000000000 / masterConfig.baudRate; 
    masterConfig.lastSckToPcsDelayInNanoSec    = 1000000000 / masterConfig.baudRate; 
    masterConfig.betweenTransferDelayInNanoSec = 1000000000 / masterConfig.baudRate; 

    LPSPI_MasterInit(spi->base, &masterConfig, imxrt_get_lpspi_freq()); 
    spi->base->CFGR1 |= LPSPI_CFGR1_PCSCFG_MASK; 

    return OS_EOK;
}

static os_uint32_t imxrt_spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    lpspi_transfer_t transfer; 
    status_t status;
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(device->bus->parent.user_data != OS_NULL);

    imxrt_spi_t *spi = (imxrt_spi_t *)(device->bus->parent.user_data); 
    struct imxrt_sw_spi_cs *cs = device->parent.user_data; 

    if(message->cs_take)
    {
        os_pin_write(cs->pin, PIN_LOW);
    }

    transfer.dataSize = message->length; 
    transfer.rxData   = (uint8_t *)(message->recv_buf); 
    transfer.txData   = (uint8_t *)(message->send_buf); 

    status = LPSPI_MasterTransferBlocking(spi->base, &transfer);

    if(message->cs_release)
    {
        os_pin_write(cs->pin, PIN_HIGH);
    }

    if (status != kStatus_Success)
    {
        message->length = 0;
    }

    return message->length; 
}

static const struct os_spi_ops imxrt_spi_ops = {
    .configure = imxrt_spi_configure,
    .xfer      = imxrt_spixfer
};

static int imxrt_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    ret  = OS_ERROR;
    imxrt_spi_t *imxrtSpi = NULL;
    struct nxp_lpspi_info *lpspiInfo = (struct nxp_lpspi_info *)dev->info;

    imxrtSpi = (imxrt_spi_t *)os_calloc(1, sizeof(imxrt_spi_t));
    imxrtSpi->base = lpspiInfo->spi_base;
    imxrtSpi->spi_bus.parent.user_data = imxrtSpi;

    ret = os_spi_bus_register(&imxrtSpi->spi_bus, dev->name, &imxrt_spi_ops);
    OS_ASSERT(ret == OS_EOK);

    return ret;
}

OS_DRIVER_INFO imxrt_spi_driver = {
    .name   = "LPSPI_Type",
    .probe  = imxrt_spi_probe,
};

OS_DRIVER_DEFINE(imxrt_spi_driver, "1");

