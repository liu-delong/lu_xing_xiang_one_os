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
 * @brief       This file implements SPI driver for FM33A0xx.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include "board.h"
#include "drv_gpio.h"
#include "drv_spi.h"
#include <drv_log.h>

#ifdef OS_USING_SPI

#if !defined(BSP_USING_SPI1) && !defined(BSP_USING_SPI2)
#error "Please define at least one BSP_USING_SPIx"
/* this driver can be disabled at menuconfig → RT-Thread Components → Device Drivers */
#endif

enum
{
#ifdef BSP_USING_SPI1
    SPI1_INDEX,
#endif
#ifdef BSP_USING_SPI2
    SPI2_INDEX,
#endif
};

static struct fm_spi_config spi_config[] =
{
#ifdef BSP_USING_SPI1
    SPI1_BUS_CONFIG,
#endif

#ifdef BSP_USING_SPI2
    SPI2_BUS_CONFIG,
#endif
};

static struct fm_spi spi_bus_obj[sizeof(spi_config) / sizeof(spi_config[0])];

static os_err_t fm_spi_init(struct fm_spi *spi_drv, struct os_spi_configuration *cfg)
{
    OS_ASSERT(spi_drv != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    os_uint32_t spi_clk_hz;
    FM_HandleType *spi_handle = &spi_drv->handle;

    if (cfg->mode & OS_SPI_CPHA)
    {
        spi_handle->Init.CPHA = SPIx_SPICR1_CPHA_SECOND;;
    }
    else
    {
        spi_handle->Init.CPHA = SPIx_SPICR1_CPHA_FIRST;
    }

    if (cfg->mode & OS_SPI_CPOL)
    {
        spi_handle->Init.CPHOL = SPIx_SPICR1_CPHOL_HIGH;
    }
    else
    {
        spi_handle->Init.CPHOL = SPIx_SPICR1_CPHOL_LOW;
    }

    if (cfg->mode & OS_SPI_NO_CS)
    {
        spi_handle->Init.SSNSEN = SPIx_SPICR2_SSNSEN_SOFT;
    }
    else
    {
        spi_handle->Init.SSNSEN = SPIx_SPICR2_SSNSEN_SOFT;
    }

    if (cfg->mode & OS_SPI_MSB)
    {
        spi_handle->Init.LSBF = SPIx_SPICR1_LSBF_MSB;
    }
    else
    {
        spi_handle->Init.LSBF = SPIx_SPICR1_LSBF_LSB;
    }

    spi_clk_hz = RCHFCLKCFG*1000*1000;
    if (cfg->max_hz >= spi_clk_hz / 2)
    {
        spi_handle->Init.BAUD_RATE = SPIx_SPICR1_BAUD_PCLK_2;
    }
    else if (cfg->max_hz >= spi_clk_hz / 4)
    {
        spi_handle->Init.BAUD_RATE = SPIx_SPICR1_BAUD_PCLK_4;
    }
    else if (cfg->max_hz >= spi_clk_hz / 8)
    {
        spi_handle->Init.BAUD_RATE = SPIx_SPICR1_BAUD_PCLK_8;
    }
    else if (cfg->max_hz >= spi_clk_hz / 16)
    {
        spi_handle->Init.BAUD_RATE = SPIx_SPICR1_BAUD_PCLK_16;
    }
    else if (cfg->max_hz >= spi_clk_hz / 32)
    {
        spi_handle->Init.BAUD_RATE = SPIx_SPICR1_BAUD_PCLK_32;
    }
    else if (cfg->max_hz >= spi_clk_hz / 64)
    {
        spi_handle->Init.BAUD_RATE = SPIx_SPICR1_BAUD_PCLK_64;
    }
    else if (cfg->max_hz >= spi_clk_hz / 128)
    {
        spi_handle->Init.BAUD_RATE = SPIx_SPICR1_BAUD_PCLK_128;
    }
    else
    {
        /*  min prescaler 256 */
        spi_handle->Init.BAUD_RATE = SPIx_SPICR1_BAUD_PCLK_256;
    }

    LOG_EXT_D("sys freq: %d, SPI limiting freq: %d, BaudRatePrescaler: %d\r\n",
               spi_clk_hz,
               cfg->max_hz,
               spi_handle->Init.BAUD_RATE);

    /* init SPI */
    SPI_Master_Init(spi_handle->Instance, &spi_handle->Init);
    SPIx_SPICR2_SPIEN_Setable(spi_handle->Instance, ENABLE);

    LOG_EXT_D("%s init done\r\n", spi_drv->config->bus_name);
    return OS_EOK;
}

os_uint32_t fm_SPI_Receive(SPIx_Type *spi, os_uint8_t *pData, os_uint16_t Size, os_uint32_t Timeout)
{
    os_uint32_t i;

    for (i = 0; i < Size; i++)
    {
        pData[i] = SPI_RW_Byte(spi, 0xFF);
    }

    return OS_EOK;
}

os_uint32_t fm_SPI_Transmit(SPIx_Type *spi, os_uint8_t *pData, os_uint16_t Size, os_uint32_t Timeout)
{
    os_uint32_t i;

    for (i = 0; i < Size; i++)
    {
        SPI_RW_Byte(spi, pData[i]);
    }

    return OS_EOK;
}

os_uint32_t fm_SPI_TransmitReceive(SPIx_Type *spi, os_uint8_t *pTxData, os_uint8_t *pRxData, os_uint16_t Size,  os_uint32_t Timeout)
{
    os_uint32_t i;

    for (i = 0; i < Size; i++)
    {
        pRxData[i] = SPI_RW_Byte(spi, pTxData[i]);
    }

    return OS_EOK;
}


static os_uint32_t spi_xfer(struct os_spi_device *device, struct os_spi_message *message)
{
    os_uint32_t state;
    os_size_t         message_length, already_send_length;
    os_uint16_t       send_length;
    os_uint8_t       *recv_buf;
    const os_uint8_t *send_buf;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(device->bus->parent.user_data != OS_NULL);
    OS_ASSERT(message != OS_NULL);

    struct fm_spi  *spi_drv    = os_container_of(device->bus, struct fm_spi, spi_bus);
    FM_HandleType *spi_handle = &spi_drv->handle;
    struct fm_hw_spi_cs *cs    = device->parent.user_data;

    if (message->cs_take)
    {
        SPI_SSN_Set_Low(spi_handle->Instance);
        GPIO_ResetBits(cs->GPIOx, cs->GPIO_Pin);
    }

    LOG_EXT_D("%s sendbuf: %X, recvbuf: %X, length: %d\r\n",
               spi_drv->config->bus_name,
               (os_uint32_t)message->send_buf,
               (os_uint32_t)message->recv_buf,
               message->length);

    message_length = message->length;
    recv_buf       = message->recv_buf;
    send_buf       = message->send_buf;
    while (message_length)
    {
        /* the HAL library use os_uint16_t to save the data length */
        if (message_length > 65535)
        {
            send_length    = 65535;
            message_length = message_length - 65535;
        }
        else
        {
            send_length    = message_length;
            message_length = 0;
        }

        /* calculate the start address */
        already_send_length = message->length - send_length - message_length;
        send_buf            = (os_uint8_t *)message->send_buf + already_send_length;
        recv_buf            = (os_uint8_t *)message->recv_buf + already_send_length;

        /* start once data exchange in DMA mode */
        if (message->send_buf && message->recv_buf)
        {
            state = fm_SPI_TransmitReceive(spi_handle->Instance, (os_uint8_t *)send_buf, 	(os_uint8_t *)recv_buf, send_length, 1000);
        }
        else if (message->send_buf)
        {
            state = fm_SPI_Transmit(spi_handle->Instance, (os_uint8_t *)send_buf, send_length, 1000);
        }
        else
        {
            memset((os_uint8_t *)recv_buf, 0xff, send_length);
            state = fm_SPI_Receive(spi_handle->Instance, (os_uint8_t *)recv_buf, send_length, 1000);
        }

        if (state != OS_EOK)
        {
            LOG_EXT_D("spi transfer error : %d\r\n", state);
            message->length   = 0;
        }
        else
        {
            LOG_EXT_D("%s transfer done\r\n", spi_drv->config->bus_name);
        }

        /* For simplicity reasons, this example is just waiting till the end of the
           transfer, but application may perform other tasks while transfer operation
           is ongoing. */
        while (SPIx_SPIIF_BUSY_Chk(spi_handle->Instance) == SET);
    }

    if (message->cs_release)
    {
        SPI_SSN_Set_High(spi_handle->Instance);
        GPIO_SetBits(cs->GPIOx, cs->GPIO_Pin);
    }

    return message->length;
}

static os_err_t spi_configure(struct os_spi_device *device,
                              struct os_spi_configuration *configuration)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(configuration != OS_NULL);

    struct fm_spi *spi_drv =  os_container_of(device->bus, struct fm_spi, spi_bus);
    spi_drv->cfg = configuration;

    return fm_spi_init(spi_drv, configuration);
}

static const struct os_spi_ops stm_spi_ops =
{
    .configure = spi_configure,
    .xfer = spi_xfer,
};

void fm_spi_hw_init(void)
{
    RCC_PERCLK_SetableEx(PDCCLK, ENABLE);
#ifdef BSP_USING_SPI1
    RCC_PERCLK_SetableEx(SPI1CLK, ENABLE);

    AltFunIO(GPIOB, GPIO_Pin_12, 2);
    AltFunIO(GPIOB, GPIO_Pin_13, 0);
    AltFunIO(GPIOB, GPIO_Pin_14, 0);
    AltFunIO(GPIOB, GPIO_Pin_15, 0);

#endif

#ifdef BSP_USING_SPI2
    RCC_PERCLK_SetableEx(SPI2CLK, ENABLE);

    AltFunIO(GPIOC, GPIO_Pin_6, 2);
    AltFunIO(GPIOC, GPIO_Pin_7, 0);
    AltFunIO(GPIOC, GPIO_Pin_8, 0);
    AltFunIO(GPIOC, GPIO_Pin_9, 0);

#endif

}

static int os_hw_spi_bus_init(void)
{
    os_err_t result;

    fm_spi_hw_init();

    for (int i = 0; i < sizeof(spi_config) / sizeof(spi_config[0]); i++)
    {
        spi_bus_obj[i].config = &spi_config[i];
        spi_bus_obj[i].spi_bus.parent.user_data = &spi_config[i];
        spi_bus_obj[i].handle.Instance = spi_config[i].Instance;

        result = os_spi_bus_register(&spi_bus_obj[i].spi_bus, spi_config[i].bus_name, &stm_spi_ops);
        OS_ASSERT(result == OS_EOK);

        LOG_EXT_D("%s bus init done", spi_config[i].bus_name);
    }

    return result;
}

/**
  * Attach the spi device to SPI bus, this function must be used after initialization.
  */
os_err_t os_hw_spi_device_attach(const char *bus_name, const char *device_name, os_uint16_t pin_num)
{
    os_err_t result;
    struct os_spi_device *spi_device;
    struct fm_hw_spi_cs *cs_pin;

    OS_ASSERT(bus_name != OS_NULL);
    OS_ASSERT(device_name != OS_NULL);

    /* attach the device to spi bus*/
    spi_device = (struct os_spi_device *)os_malloc(sizeof(struct os_spi_device));
    OS_ASSERT(spi_device != OS_NULL);

    cs_pin = (struct fm_hw_spi_cs *)os_malloc(sizeof(struct fm_hw_spi_cs));
    OS_ASSERT(cs_pin != OS_NULL);

    cs_pin->GPIOx    = PIN_BASE(pin_num);
    cs_pin->GPIO_Pin = PIN_OFFSET(pin_num);

    /* initialize the cs pin && select the slave*/
    AltFunIO(cs_pin->GPIOx, cs_pin->GPIO_Pin, 2);
    GPIO_ResetBits(cs_pin->GPIOx, cs_pin->GPIO_Pin);

    result = os_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)cs_pin);

    if (result != OS_EOK)
    {
        LOG_EXT_D("%s attach to %s faild, %d\n", device_name, bus_name, result);
    }

    OS_ASSERT(result == OS_EOK);

    LOG_EXT_D("%s attach to %s done", device_name, bus_name);

    return result;
}

int os_hw_spi_init(void)
{
    return os_hw_spi_bus_init();
}


OS_BOARD_INIT(os_hw_spi_init);

#endif /* OS_USING_SPI */

