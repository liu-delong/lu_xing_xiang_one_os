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

#include <os_task.h>
#ifdef BSP_USING_SPI

#include "drv_spi.h" 
#include "fsl_common.h" 
#include "fsl_iomuxc.h" 
#include "fsl_lpspi.h" 
#include "fsl_lpspi_edma.h"
#include "fsl_dmamux.h"

#define LOG_TAG             "drv.spi"
#include <drv_log.h>

#if defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL
    #error "Please don't define 'FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL'!"
#endif

enum
{
#ifdef BSP_USING_SPI1
    SPI1_INDEX,
#endif
#ifdef BSP_USING_SPI2
    SPI2_INDEX,
#endif
#ifdef BSP_USING_SPI3
    SPI3_INDEX,
#endif
#ifdef BSP_USING_SPI4
    SPI4_INDEX,
#endif
};

struct imxrt_sw_spi_cs
{
    os_uint32_t pin;
};

struct dma_config
{
    lpspi_master_edma_handle_t spi_edma;

    edma_handle_t rx_edma;
    dma_request_source_t rx_request;
    os_uint8_t rx_channel;

    edma_handle_t tx_edma;
    dma_request_source_t tx_request;
    os_uint8_t tx_channel;
};

struct imxrt_spi
{
    char *bus_name;
    LPSPI_Type *base;
    struct rt_spi_bus spi_bus;
    struct dma_config *dma;
    os_uint8_t dma_flag;
};

static struct imxrt_spi lpspis[] =
{
#ifdef BSP_USING_SPI1
    {
        .bus_name = "spi1",
        .base = LPSPI1,
        .dma = OS_NULL,
        .dma_flag = OS_FALSE,
    },
#endif
#ifdef BSP_USING_SPI2
    {
        .bus_name = "spi2",
        .base = LPSPI2,
        .dma = OS_NULL,
        .dma_flag = OS_FALSE,
    },
#endif
#ifdef BSP_USING_SPI3
    {
        .bus_name = "spi3",
        .base = LPSPI3,
        .dma = OS_NULL,
        .dma_flag = OS_FALSE,
    },
#endif
#ifdef BSP_USING_SPI4
    {
        .bus_name = "spi4",
        .base = LPSPI4,
        .dma = OS_NULL,
        .dma_flag = OS_FALSE,
    },
#endif
};

static void spi_get_dma_config(void)
{
#ifdef BSP_SPI1_USING_DMA
    static struct dma_config spi1_dma =
    {
    .rx_request = kDmaRequestMuxLPSPI1Rx,
    .rx_channel = BSP_SPI1_RX_DMA_CHANNEL,
    .tx_request = kDmaRequestMuxLPSPI1Tx,
    .tx_channel = BSP_SPI1_TX_DMA_CHANNEL,
    };

    lpspis[SPI1_INDEX].dma = &spi1_dma;
    lpspis[SPI1_INDEX].dma_flag = OS_TRUE;
#endif

#ifdef BSP_SPI2_USING_DMA
    static struct dma_config spi2_dma =
    {
    .rx_request = kDmaRequestMuxLPSPI2Rx,
    .rx_channel = BSP_SPI2_RX_DMA_CHANNEL,
    .tx_request = kDmaRequestMuxLPSPI2Tx,
    .tx_channel = BSP_SPI2_TX_DMA_CHANNEL,
    };

    lpspis[SPI2_INDEX].dma = &spi2_dma;
    lpspis[SPI2_INDEX].dma_flag = OS_TRUE;
#endif

#ifdef BSP_SPI3_USING_DMA
    static struct dma_config spi3_dma =
    {
    .rx_request = kDmaRequestMuxLPSPI3Rx,
    .rx_channel = BSP_SPI3_RX_DMA_CHANNEL,
    .tx_request = kDmaRequestMuxLPSPI3Tx,
    .tx_channel = BSP_SPI3_TX_DMA_CHANNEL,
    };

    lpspis[SPI3_INDEX].dma = &spi3_dma;
    lpspis[SPI3_INDEX].dma_flag = OS_TRUE;
#endif

#ifdef BSP_SPI4_USING_DMA
    static struct dma_config spi4_dma =
    {
    .rx_request = kDmaRequestMuxLPSPI4Rx,
    .rx_channel = BSP_SPI4_RX_DMA_CHANNEL,
    .tx_request = kDmaRequestMuxLPSPI4Tx,
    .tx_channel = BSP_SPI4_TX_DMA_CHANNEL,
    };

    lpspis[SPI4_INDEX].dma = &spi4_dma;
    lpspis[SPI4_INDEX].dma_flag = OS_TRUE;
#endif
}

void edma_xfer_callback(LPSPI_Type *base, lpspi_master_edma_handle_t *handle, status_t status, void *userData)
{
    /* xfer complete callback */
}

os_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, os_uint32_t pin)
{
    os_err_t ret = OS_EOK; 
    
    struct os_spi_device *spi_device = (struct os_spi_device *)os_malloc(sizeof(struct os_spi_device)); 
    OS_ASSERT(spi_device != OS_NULL); 
    
    struct imxrt_sw_spi_cs *cs_pin = (struct imxrt_sw_spi_cs *)os_malloc(sizeof(struct imxrt_sw_spi_cs)); 
    OS_ASSERT(cs_pin != OS_NULL);
    
    cs_pin->pin = pin;
    rt_pin_mode(pin, PIN_MODE_OUTPUT); 
    rt_pin_write(pin, PIN_HIGH); 
    
    ret = rt_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)cs_pin); 
    
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

static void lpspi_dma_config(struct imxrt_spi *spi)
{
    OS_ASSERT(spi != OS_NULL);

    DMAMUX_SetSource(DMAMUX, spi->dma->rx_channel, spi->dma->rx_request);
    DMAMUX_EnableChannel(DMAMUX, spi->dma->rx_channel);
    EDMA_CreateHandle(&spi->dma->rx_edma, DMA0, spi->dma->rx_channel);

    DMAMUX_SetSource(DMAMUX, spi->dma->tx_channel, spi->dma->tx_request);
    DMAMUX_EnableChannel(DMAMUX, spi->dma->tx_channel);
    EDMA_CreateHandle(&spi->dma->tx_edma, DMA0, spi->dma->tx_channel);
    
    LPSPI_MasterTransferCreateHandleEDMA(spi->base,
                                        &spi->dma->spi_edma,
                                        edma_xfer_callback,
                                        spi,
                                        &spi->dma->rx_edma,
                                        &spi->dma->tx_edma);

    LOG_D("%s dma config done\n", spi->bus_name);
}

static os_err_t spi_configure(struct os_spi_device *device, struct os_spi_configuration *cfg)
{
    lpspi_master_config_t masterConfig; 
    struct imxrt_spi *spi = OS_NULL; 

    OS_ASSERT(cfg != OS_NULL);
    OS_ASSERT(device != OS_NULL);

    spi = (struct imxrt_spi *)(device->bus->parent.user_data);
    OS_ASSERT(spi != OS_NULL);

    if(cfg->data_width != 8 && cfg->data_width != 16 && cfg->data_width != 32)
    {
        return OS_EINVAL; 
    }

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
    
    if(cfg->mode & RT_SPI_CPHA)
    {
        masterConfig.cpha = kLPSPI_ClockPhaseSecondEdge; 
    }
    else
    {
        masterConfig.cpha = kLPSPI_ClockPhaseFirstEdge; 
    }
    
    if(cfg->mode & RT_SPI_CPOL)
    {
        masterConfig.cpol = kLPSPI_ClockPolarityActiveLow; 
    }
    else
    {
        masterConfig.cpol = kLPSPI_ClockPolarityActiveHigh; 
    }

    masterConfig.pinCfg                        = kLPSPI_SdiInSdoOut; 
    masterConfig.dataOutConfig                 = kLpspiDataOutTristate;
    masterConfig.pcsToSckDelayInNanoSec        = 1000000000 / masterConfig.baudRate; 
    masterConfig.lastSckToPcsDelayInNanoSec    = 1000000000 / masterConfig.baudRate; 
    masterConfig.betweenTransferDelayInNanoSec = 1000000000 / masterConfig.baudRate; 

    LPSPI_MasterInit(spi->base, &masterConfig, imxrt_get_lpspi_freq()); 
    spi->base->CFGR1 |= LPSPI_CFGR1_PCSCFG_MASK; 

    return OS_EOK;
}

static os_uint32_t spixfer(struct os_spi_device *device, struct rt_spi_message *message)
{
    lpspi_transfer_t transfer; 
    status_t status;
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(device->bus->parent.user_data != OS_NULL);

    struct imxrt_spi *spi = (struct imxrt_spi *)(device->bus->parent.user_data); 
    struct imxrt_sw_spi_cs *cs = device->parent.user_data; 

    if(message->cs_take)
    {
        rt_pin_write(cs->pin, PIN_LOW);
    }

    transfer.dataSize = message->length; 
    transfer.rxData   = (uint8_t *)(message->recv_buf); 
    transfer.txData   = (uint8_t *)(message->send_buf); 

    if(OS_FALSE == spi->dma_flag)
    {
        status = LPSPI_MasterTransferBlocking(spi->base, &transfer);
    }
    else
    {
        status = LPSPI_MasterTransferEDMA(spi->base,&spi->dma->spi_edma,&transfer);
    }

    if(message->cs_release)
    {
        rt_pin_write(cs->pin, PIN_HIGH);
    }

    if (status != kStatus_Success)
    {
        LOG_E("%s transfer error : %d", spi->bus_name,status);
        message->length = 0;
    }

    return message->length; 
}

static struct rt_spi_ops imxrt_spi_ops = 
{
    .configure = spi_configure,
    .xfer      = spixfer
};

int rt_hw_spi_bus_init(void)
{
    int i;
    os_err_t ret = OS_EOK;

    spi_get_dma_config();

    for (i = 0; i < sizeof(lpspis) / sizeof(lpspis[0]); i++)
    {
        lpspis[i].spi_bus.parent.user_data = &lpspis[i];

        ret = rt_spi_bus_register(&lpspis[i].spi_bus, lpspis[i].bus_name, &imxrt_spi_ops);

        if(OS_TRUE == lpspis[i].dma_flag)
        {
            lpspi_dma_config(&lpspis[i]);
        }
    }

    return ret; 
}
INIT_BOARD_EXPORT(rt_hw_spi_bus_init); 

#endif /* BSP_USING_SPI */
