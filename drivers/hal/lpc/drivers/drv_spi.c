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
 * @brief       This file implements spi driver for nxp.
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
#include <string.h>
#include <drv_spi.h>

struct nxp_spi
{
    struct os_spi_bus spi;
    struct nxp_spi_info *spi_info;
    spi_transfer_t transXfer;
    os_uint32_t clk_src;

    IRQn_Type irqn;
    spi_master_handle_t *spi_handle;
    spi_dma_handle_t *spi_DmaHandle;

    os_list_node_t list;
};

static os_list_node_t nxp_spi_list = OS_LIST_INIT(nxp_spi_list);

void nxp_spi_irq_callback(struct nxp_spi *nxp_spi)
{
    if (SPI_GetStatusFlags(nxp_spi->spi_info->spi_base) & kSPI_RxNotEmptyFlag)
    {
        if (nxp_spi->spi.parent.cb_table[OS_DEVICE_CB_TYPE_RX].cb != OS_NULL)
        {
            os_interrupt_enter();
            nxp_spi->spi.parent.cb_table[OS_DEVICE_CB_TYPE_RX].cb(&nxp_spi->spi.parent, 0);
            os_interrupt_leave();
        }
    }
}

SPI_IRQHandler_DEFINE(0);
SPI_IRQHandler_DEFINE(1);
SPI_IRQHandler_DEFINE(2);
SPI_IRQHandler_DEFINE(3);
SPI_IRQHandler_DEFINE(4);
SPI_IRQHandler_DEFINE(5);
SPI_IRQHandler_DEFINE(6);
SPI_IRQHandler_DEFINE(7);
SPI_IRQHandler_DEFINE(8);

void nxp_spi_dma_callback(SPI_Type *base, spi_dma_handle_t *handle, status_t status, void *userData)
{
    struct nxp_spi *nxp_spi = (struct nxp_spi *)userData;
    
    if ((kStatus_SPI_Idle == status) && (nxp_spi->spi.parent.cb_table[OS_DEVICE_CB_TYPE_RX].cb != OS_NULL))
    {
        os_interrupt_enter();
        nxp_spi->spi.parent.cb_table[OS_DEVICE_CB_TYPE_RX].cb(&nxp_spi->spi.parent, 0);
        os_interrupt_leave();
    }
}

void nxp_spi_transfer_callback(SPI_Type *base, spi_master_handle_t *handle, status_t status, void *userData)
{
    struct nxp_spi *nxp_spi = (struct nxp_spi *)userData;
    
    if ((kStatus_SPI_Idle == status) && (nxp_spi->spi.parent.cb_table[OS_DEVICE_CB_TYPE_RX].cb != OS_NULL))
    {
        os_interrupt_enter();
        nxp_spi->spi.parent.cb_table[OS_DEVICE_CB_TYPE_RX].cb(&nxp_spi->spi.parent, 0);
        os_interrupt_leave();
    }
}

static os_err_t nxp_spi_configure(struct os_spi_device *device, struct os_spi_configuration *cfg)
{
    struct nxp_spi *nxp_spi = (struct nxp_spi *)device->bus;
    
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    LOG_EXT_I("nxp spi driver don't support config! please use config tool!");
    
    return OS_EOK;
}

static os_uint32_t nxp_spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    status_t status;
    
    struct nxp_spi *nxp_spi = (struct nxp_spi *)device->bus;
    
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(message != OS_NULL);

    struct nxp_sw_spi_cs *cs = device->parent.user_data; 

    if (message->cs_take)
    {
        os_pin_write(cs->pin, PIN_LOW);
    }

    if ((message->recv_buf == OS_NULL) && (message->send_buf == OS_NULL))
    {
        return 0;
    }
    nxp_spi->transXfer.rxData = (uint8_t *)(message->recv_buf); 
    nxp_spi->transXfer.txData = (uint8_t *)(message->send_buf); 
    nxp_spi->transXfer.dataSize = message->length;

    if (nxp_spi->spi_DmaHandle != OS_NULL)
        status = SPI_MasterTransferDMA(nxp_spi->spi_info->spi_base, nxp_spi->spi_DmaHandle, &nxp_spi->transXfer);
    else if (nxp_spi->spi_handle != OS_NULL)
        status = SPI_MasterTransferNonBlocking(nxp_spi->spi_info->spi_base, nxp_spi->spi_handle, &nxp_spi->transXfer);
    else
        status = SPI_MasterTransferBlocking(nxp_spi->spi_info->spi_base, &nxp_spi->transXfer);

    if(message->cs_release)
    {
        os_pin_write(cs->pin, PIN_HIGH);
    }

    if (status != kStatus_Success)
    {
        LOG_EXT_E("%s transfer error : %d\n", nxp_spi->spi.parent.name, status);
        message->length = 0;
    }

    return message->length; 
}

static const struct os_spi_ops nxp_spi_ops = {
    .configure = nxp_spi_configure,
    .xfer      = nxp_spixfer
};

os_err_t os_hw_spi_device_attach(const char *bus_name, const char *device_name, os_base_t pin)
{
    os_err_t ret = OS_EOK;
    
    struct os_spi_device *spi_device = (struct os_spi_device *)os_malloc(sizeof(struct os_spi_device)); 
    OS_ASSERT(spi_device != OS_NULL);
    
    struct nxp_sw_spi_cs *cs_pin = (struct nxp_sw_spi_cs *)os_malloc(sizeof(struct nxp_sw_spi_cs)); 
    OS_ASSERT(cs_pin != OS_NULL);
    
    cs_pin->pin = pin;
    os_pin_mode(pin, PIN_MODE_OUTPUT); 
    os_pin_write(pin, PIN_HIGH); 
    
    ret = os_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)cs_pin); 
    
    return ret; 
}

void nxp_spi_param_cfg(struct nxp_spi *nxp_spi)
{
    switch((os_uint32_t)nxp_spi->spi_info->spi_base)
    {
    case (os_uint32_t)FLEXCOMM0:
        SPI0_CFG_INIT(nxp_spi, 0);
        break;
    case (os_uint32_t)FLEXCOMM1:
        SPI1_CFG_INIT(nxp_spi, 1);
        break;
    case (os_uint32_t)FLEXCOMM2:
        SPI2_CFG_INIT(nxp_spi, 2);
        break;
    case (os_uint32_t)FLEXCOMM3:
        SPI3_CFG_INIT(nxp_spi, 3);
        break;
    case (os_uint32_t)FLEXCOMM4:
        SPI4_CFG_INIT(nxp_spi, 4);
        break;
    case (os_uint32_t)FLEXCOMM5:
        SPI5_CFG_INIT(nxp_spi, 5);
        break;
    case (os_uint32_t)FLEXCOMM6:
        SPI6_CFG_INIT(nxp_spi, 6);
        break;
    case (os_uint32_t)FLEXCOMM7:
        SPI7_CFG_INIT(nxp_spi, 7);
        break;
    case (os_uint32_t)FLEXCOMM8:
        SPI8_CFG_INIT(nxp_spi, 8);
        break;
    default:
        break;
    }
}

static int nxp_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    os_base_t   level;
    
    struct nxp_spi_info *spi_info = (struct nxp_spi_info *)dev->info;

    struct nxp_spi *nxp_spi = os_calloc(1, sizeof(struct nxp_spi));

    OS_ASSERT(nxp_spi);
    
    nxp_spi->spi_info = spi_info;
    nxp_spi_param_cfg(nxp_spi);
    
    struct os_spi_bus *spi = &nxp_spi->spi;

    spi->ops = &nxp_spi_ops;

    level = os_hw_interrupt_disable();
    os_list_add_tail(&nxp_spi_list, &nxp_spi->list);
    os_hw_interrupt_enable(level);
    
    result = os_spi_bus_register(spi, dev->name, &nxp_spi_ops);
    OS_ASSERT(result == OS_EOK);

    return result;

}

OS_DRIVER_INFO nxp_spi_driver = {
    .name   = "SPI_Type",
    .probe  = nxp_spi_probe,
};

OS_DRIVER_DEFINE(nxp_spi_driver, "1");

