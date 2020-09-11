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
 * @brief       This file implements SPI driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_memory.h>
#include <drv_gpio.h>
#include <drv_common.h>
#include <drv_spi.h>
#include <string.h>
#include "hal_spi_master_internal.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.spi"
#include <drv_log.h>

struct mt2625_spi_cs
{
    uint16_t cs_pin;      /* hal_gpio_pin_t */
    uint16_t cs_pin_func; /* defined in hal_pinmux_define.h */
};


struct mt_spi_device
{
    os_uint32_t pin;
    char *bus_name;
    char *device_name;
};

struct mt_spi
{
    struct os_spi_bus spi_bus;
    SPI_HandleTypeDef           *hspi;
    struct os_spi_configuration *cfg;

    os_list_node_t list;
};

static os_list_node_t mt_spi_list = OS_LIST_INIT(mt_spi_list);


static void mt_user_spi_callback (hal_spi_master_callback_event_t event, void *user_data)
{
    if (HAL_SPI_MASTER_EVENT_SEND_FINISHED == event) 
    {
        
    } 
    else if (HAL_SPI_MASTER_EVENT_RECEIVE_FINISHED == event) 
    {
        
    }
}


static os_err_t mt_spi_init(struct mt_spi *spi_drv, struct os_spi_configuration *cfg)
{
    OS_ASSERT(spi_drv != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    hal_spi_master_status_t ret = HAL_SPI_MASTER_STATUS_ERROR;
    SPI_HandleTypeDef *spi_handle = spi_drv->hspi;
    OS_ASSERT(spi_handle != OS_NULL);
    
    if (cfg->mode & OS_SPI_SLAVE)
    {
        LOG_EXT_E("[%s]-[%d], ret[%d]\r\n", __FILE__, __LINE__, ret);
        return OS_EOK;
    }
    else
    {
        spi_handle->spi_config.slave_port = HAL_SPI_MASTER_SLAVE_0;
    }

    if (cfg->mode & OS_SPI_CPHA)
    {
        spi_handle->spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE1;
    }
    else
    {
        spi_handle->spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE0;
    }

    if (cfg->mode & OS_SPI_CPOL)
    {
        spi_handle->spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY1;
    }
    else
    {
        spi_handle->spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY0;
    }

    if (((cfg->max_hz) >= HAL_SPI_MASTER_CLOCK_MIN_FREQUENCY) &&
            ((cfg->max_hz) <= HAL_SPI_MASTER_CLOCK_MAX_FREQUENCY))
    {
        spi_handle->spi_config.clock_frequency = cfg->max_hz;
    }
    else
    {
        LOG_EXT_E("[%s]-[%d], clock_frequency error, [%d <= clk <= %d]\r\n", __FILE__, __LINE__, 
            HAL_SPI_MASTER_CLOCK_MIN_FREQUENCY, HAL_SPI_MASTER_CLOCK_MAX_FREQUENCY);
        return OS_EIO;
    }

    if (cfg->mode & OS_SPI_MSB)
    {
        spi_handle->spi_config.bit_order = HAL_SPI_MASTER_MSB_FIRST;
    }
    else
    {
        spi_handle->spi_config.bit_order = HAL_SPI_MASTER_MSB_FIRST;
    }

    ret = hal_spi_master_init(spi_handle->master_port, &spi_handle->spi_config);
    if (HAL_SPI_MASTER_STATUS_OK != ret) 
    {
        LOG_EXT_E("[%s]-[%d], ret[%d]\r\n", __FILE__, __LINE__, ret);
        return OS_EIO;
    }

    if (spi_handle->spi_dma == 1)
    {
        ret = hal_spi_master_register_callback(spi_handle->master_port ,mt_user_spi_callback, (void *)spi_handle->master_port);
        if (HAL_SPI_MASTER_STATUS_OK != ret) 
        {
            LOG_EXT_E("[%s]-[%d], ret[%d]\r\n", __FILE__, __LINE__, ret);
            return OS_EIO;
        }
    }

    return OS_EOK;
}

static uint32_t mt_spi_hw_init(SPI_HandleTypeDef *hspi)
{
    uint32_t ret = 0;
    SPI_HW_INFO_S *spi_hw = &(hspi->spi_hw);
    OS_ASSERT(spi_hw != OS_NULL);
    
    //ret = hal_gpio_init(spi_hw->spi_cs.pin_num);
    ret |= hal_gpio_init(spi_hw->spi_miso.pin_num);
    ret |= hal_gpio_init(spi_hw->spi_mosi.pin_num);
    ret |= hal_gpio_init(spi_hw->spi_sck.pin_num);

    //ret |= hal_pinmux_set_function(spi_hw->spi_cs.pin_num,   spi_hw->spi_cs.pin_num_set);   // Set the pin to be used as CS signal of SPI.
    ret |= hal_pinmux_set_function(spi_hw->spi_miso.pin_num,   spi_hw->spi_miso.pin_num_set);  // Set the pin to be used as SCK signal of SPI.
    ret |= hal_pinmux_set_function(spi_hw->spi_mosi.pin_num,   spi_hw->spi_mosi.pin_num_set); // Set the pin to be used as MOSI signal of SPI.
    ret |= hal_pinmux_set_function(spi_hw->spi_sck.pin_num,   spi_hw->spi_sck.pin_num_set); // Set the pin to be used as MISO signal of SPI.
        
    return ret;
}

static os_err_t spi_configure(struct os_spi_device *device, struct os_spi_configuration *configuration)
{
    uint32_t ret = 0;
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(configuration != OS_NULL);
    struct mt_spi *spi_drv = os_container_of(device->bus, struct mt_spi, spi_bus);
    spi_drv->cfg              = configuration;
    
    ret = mt_spi_hw_init(spi_drv->hspi);
    if (ret != 0)
    {
        LOG_EXT_E("[%s]-[%d], ret[%d]\r\n", __FILE__, __LINE__, ret);
        return OS_ERROR;
    }

    return mt_spi_init(spi_drv, configuration);
}

static os_uint32_t mt_spi_send_recv_polling(hal_spi_master_port_t master_port, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, os_uint8_t dma_flag)
{
    hal_spi_master_send_and_receive_config_t spi_config;
    hal_spi_master_status_t ret;

    spi_config.send_length = Size;
    spi_config.send_data = pTxData;

    spi_config.receive_length = Size;
    spi_config.receive_buffer = pRxData;

    if (dma_flag == 1)
    {
        ret = hal_spi_master_send_and_receive_dma(master_port, &spi_config);
    }
    else
    {
        ret = hal_spi_master_send_and_receive_polling(master_port, &spi_config);
    }
    if (HAL_SPI_MASTER_STATUS_OK != ret) {
        LOG_EXT_E("[%s]-[%d], ret[%d]\r\n", __FILE__, __LINE__, ret);
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_uint32_t mt_spi_send_polling(hal_spi_master_port_t master_port, uint8_t *pTxData, uint16_t Size, os_uint8_t dma_flag)
{
    hal_spi_master_status_t ret;

    if (dma_flag == 1)
    {
        ret = hal_spi_master_send_dma(master_port, pTxData, Size);
    }
    else
    {
        ret = hal_spi_master_send_polling(master_port, pTxData, Size);
    }
    
    if (HAL_SPI_MASTER_STATUS_OK != ret) {
        LOG_EXT_E("[%s]-[%d], ret[%d]\r\n", __FILE__, __LINE__, ret);
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_uint32_t mt_spi_recv_polling(hal_spi_master_port_t master_port, uint8_t *pRxData, uint16_t Size, os_uint8_t dma_flag)
{
    hal_spi_master_send_and_receive_config_t spi_config;
    hal_spi_master_status_t ret;

    spi_config.send_length = 0;
    spi_config.send_data = pRxData;

    spi_config.receive_length = Size;
    spi_config.receive_buffer = pRxData;

    if (dma_flag == 1)
    {
        ret = hal_spi_master_send_and_receive_dma(master_port, &spi_config);
    }
    else
    {
        ret = hal_spi_master_send_and_receive_polling(master_port, &spi_config);
    }
    
    if (HAL_SPI_MASTER_STATUS_OK != ret) {
        LOG_EXT_E("[%s]-[%d], ret[%d]\r\n", __FILE__, __LINE__, ret);
        return OS_ERROR;
    }

    return OS_EOK;
}


static os_uint32_t spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    os_size_t         message_length, already_send_length;
    os_uint16_t       send_length;
    os_uint8_t       *recv_buf;
    const os_uint8_t *send_buf;
    hal_spi_master_status_t ret = HAL_SPI_MASTER_STATUS_ERROR;
    hal_spi_master_running_status_t running_status = HAL_SPI_MASTER_BUSY;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(message != OS_NULL);
    struct mt2625_spi_cs *cs = device->parent.user_data;

    struct mt_spi  *spi_drv    = os_container_of(device->bus, struct mt_spi, spi_bus);
    SPI_HandleTypeDef *spi_handle = spi_drv->hspi;
    OS_ASSERT(spi_drv != OS_NULL);
    OS_ASSERT(spi_handle != OS_NULL);

    LOG_EXT_D("master_port[%d] transfer prepare and start", spi_drv->hspi->master_port);
    LOG_EXT_D("sendbuf: 0x%p, recvbuf: 0x%p, length: %d",
              (uint32_t)message->send_buf,
              (uint32_t)message->recv_buf,
              message->length);
    
    if (message->cs_take)
    {
        hal_gpio_set_output(cs->cs_pin, HAL_GPIO_DATA_LOW);
    }

    message_length = message->length;
    recv_buf       = message->recv_buf;
    send_buf       = message->send_buf;
    
    while (message_length)
    {
        /* the HAL library use uint16 to save the data length */
        if (message_length > HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE)
        {
            send_length    = HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE;
            message_length = message_length - HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE;
        }
        else
        {
            send_length    = message_length;
            message_length = 0;
        }

        /* calculate the start address */
        already_send_length = message->length - send_length - message_length;
        send_buf  = (os_uint8_t *)message->send_buf + already_send_length;
        recv_buf  = (os_uint8_t *)message->recv_buf + already_send_length;

        /* start once data exchange in DMA mode */
        if (message->send_buf && message->recv_buf)
        {
            ret = mt_spi_send_recv_polling(spi_handle->master_port, (uint8_t *)send_buf, (uint8_t *)recv_buf, send_length, spi_handle->spi_dma);
        }
        else if (message->send_buf)
        {
            ret = mt_spi_send_polling(spi_handle->master_port, (uint8_t *)send_buf, send_length, spi_handle->spi_dma);
        }
        else  if (message->recv_buf)
        {
            ret = mt_spi_recv_polling(spi_handle->master_port, (uint8_t *)recv_buf, send_length, spi_handle->spi_dma);
        }

        if (ret != OS_EOK)
        {
            LOG_EXT_E("spi transfer error : %d", ret);
            message->length   = 0;
            running_status = HAL_SPI_MASTER_IDLE;
        }
        else
        {
            LOG_EXT_D("master_port[%d] transfer prepare and start", spi_drv->hspi->master_port);
        }

        /* For simplicity reasons, this example is just waiting till the end of the
           transfer, but application may perform other tasks while transfer operation
           is ongoing. */
        while (running_status != HAL_SPI_MASTER_IDLE)
        {
            ret = hal_spi_master_get_running_status(spi_handle->master_port, &running_status);
            if (HAL_SPI_MASTER_STATUS_OK != ret)
            {
                LOG_EXT_I("spi transfer error : %d", ret);
                running_status = HAL_SPI_MASTER_IDLE;
            }
        }

    }
    
    if (message->cs_release)
    {
        hal_gpio_set_output(cs->cs_pin, HAL_GPIO_DATA_HIGH);
    }


    return message->length;
}

static const struct os_spi_ops mt_spi_ops = {
    .configure = spi_configure,
    .xfer      = spixfer,
};

os_err_t os_hw_spi_device_attach(const char *bus_name, const char *device_name, os_base_t cs_pin)
{
    OS_ASSERT(bus_name != OS_NULL);
    OS_ASSERT(device_name != OS_NULL);

    os_err_t result;
    struct os_spi_device *spi_device;
    struct mt2625_spi_cs *cs_pin_ops;

    os_pin_mode(cs_pin, PIN_MODE_OUTPUT);

    /* attach the device to spi bus*/
    spi_device = (struct os_spi_device *)os_malloc(sizeof(struct os_spi_device));
    OS_ASSERT(spi_device != OS_NULL);

    cs_pin_ops = (struct mt2625_spi_cs *)os_malloc(sizeof(struct mt2625_spi_cs));
    OS_ASSERT(cs_pin_ops != OS_NULL);

    cs_pin_ops->cs_pin = cs_pin;

    result = os_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)cs_pin_ops);
    if (result != OS_EOK)
    {
        LOG_EXT_E("%s attach to %s faild, %d\n", device_name, bus_name, result);
        os_free(spi_device);
        os_free(cs_pin_ops);
    }

    LOG_EXT_D("%s attach to %s done", device_name, bus_name);

    return result;
}

static int mt_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    os_err_t    result  = 0;
    os_base_t   level;

    struct mt_spi *st_spi = os_calloc(1, sizeof(struct mt_spi));

    OS_ASSERT(st_spi);

    st_spi->hspi = (SPI_HandleTypeDef *)dev->info;

    struct os_spi_bus *spi_bus = &st_spi->spi_bus;

    level = os_hw_interrupt_disable();
    os_list_add_tail(&mt_spi_list, &st_spi->list);
    os_hw_interrupt_enable(level);

    result = os_spi_bus_register(spi_bus, dev->name, &mt_spi_ops);
    OS_ASSERT(result == OS_EOK);

    LOG_EXT_D("%s bus init done", dev->name);

    return result;
}

OS_DRIVER_INFO mt_spi_driver = {
    .name   = "SPI_HandleTypeDef",
    .probe  = mt_spi_probe,
};

OS_DRIVER_DEFINE(mt_spi_driver, "1");

