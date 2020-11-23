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

#include <drv_spi.h>

struct hc32_spi spis[] = {

#ifdef  BSP_USING_SPI0
	{
		.spi_base = M0P_SPI0,
		.reset = ResetMskSpi0,
		.peripheral = SysctrlPeripheralSpi0,
		.port_cs = SPI0_CS_PORT,
		.pin_cs = SPI0_CS_PIN,
		.port_sck = SPI0_SCK_PORT,
		.pin_sck = SPI0_SCK_PIN,
		.port_miso = SPI0_MISO_PORT,
		.pin_miso = SPI0_MISO_PIN,
		.port_mosi = SPI0_MOSI_PORT,
		.pin_mosi = SPI0_MOSI_PIN,
		.gpio_af = SPI0_GPIO_AF,
		.name = "spi0",
	},
#endif

#ifdef  BSP_USING_SPI1
	{
		.spi_base = M0P_SPI1,
		.reset = ResetMskSpi1,
		.peripheral = SysctrlPeripheralSpi1,
		.port_cs = SPI1_CS_PORT,
		.pin_cs = SPI1_CS_PIN,
		.port_sck = SPI1_SCK_PORT,
		.pin_sck = SPI1_SCK_PIN,
		.port_miso = SPI1_MISO_PORT,
		.pin_miso = SPI1_MISO_PIN,
		.port_mosi = SPI1_MOSI_PORT,
		.pin_mosi = SPI1_MOSI_PIN,
		.gpio_af = SPI1_GPIO_AF,
		.name = "spi1",
	},
#endif
};


static os_err_t hc32_spi_init(struct hc32_spi *spi_drv)
{
	stc_spi_cfg_t  SpiInitStruct;
	struct os_spi_configuration *cfg;
	uint32_t u32Pclk;

	cfg = spi_drv->cfg;

	OS_ASSERT(spi_drv != OS_NULL);
	OS_ASSERT(cfg != OS_NULL);

	Reset_RstPeripheral0(spi_drv->reset);

	if (cfg->mode & OS_SPI_SLAVE)
	{
		SpiInitStruct.enSpiMode = SpiMskSlave;
	}
	else
	{
		SpiInitStruct.enSpiMode = SpiMskMaster;
	}

	if (cfg->mode & OS_SPI_CPHA)
	{
		SpiInitStruct.enCPHA    = SpiMskCphasecond;
	}
	else
	{
		SpiInitStruct.enCPHA    = SpiMskCphafirst;
	}

	if (cfg->mode & OS_SPI_CPOL)
	{
		SpiInitStruct.enCPOL    = SpiMskcpolhigh;
	}
	else
	{
		SpiInitStruct.enCPOL    = SpiMskcpollow;
	}

	u32Pclk = Sysctrl_GetPClkFreq();

	if (cfg->max_hz >= u32Pclk / 2)
	{
		SpiInitStruct.enPclkDiv = SpiClkMskDiv2;
	}
	else if (cfg->max_hz >= u32Pclk / 4)
	{
		SpiInitStruct.enPclkDiv = SpiClkMskDiv4;
	}
	else if (cfg->max_hz >= u32Pclk / 8)
	{
		SpiInitStruct.enPclkDiv = SpiClkMskDiv8;
	}
	else if (cfg->max_hz >= u32Pclk / 16)
	{
		SpiInitStruct.enPclkDiv = SpiClkMskDiv16;
	}
	else if (cfg->max_hz >= u32Pclk / 32)
	{
		SpiInitStruct.enPclkDiv = SpiClkMskDiv32;
	}
	else if (cfg->max_hz >= u32Pclk / 64)
	{
		SpiInitStruct.enPclkDiv = SpiClkMskDiv64;
	}
	else if (cfg->max_hz >= u32Pclk / 128)
	{
		SpiInitStruct.enPclkDiv = SpiClkMskDiv128;
	}
	else
	{
		return OS_ERROR;
	}

	LOG_EXT_D("sys freq: %d, pclk2 freq: %d, SPI limiting freq: %d, BaudRatePrescaler: %d",        
			Sysctrl_GetPClkFreq(),
			u32Pclk,
			cfg->max_hz,
			SpiInitStruct.enPclkDiv);

	if(Spi_Init(spi_drv->spi_base, &SpiInitStruct) != Ok)
	{
		return OS_EIO;
	}

	LOG_EXT_D("%s init done", spi_drv->name);
	return OS_EOK;
}

static os_err_t spi_configure(struct os_spi_device *device, struct os_spi_configuration *configuration)
{
	OS_ASSERT(device != OS_NULL);
	OS_ASSERT(configuration != OS_NULL);

	struct hc32_spi *spi_drv = os_container_of(device->bus, struct hc32_spi, spi_bus);
	spi_drv->cfg              = configuration;

	return hc32_spi_init(spi_drv);
}

static os_uint32_t spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
	en_result_t state;
	os_size_t         message_length, already_send_length;
	os_uint32_t       send_length;
	os_uint8_t       *recv_buf;
	const os_uint8_t *send_buf;

	OS_ASSERT(device != OS_NULL);
	OS_ASSERT(device->bus != OS_NULL);
	OS_ASSERT(message != OS_NULL);

	struct hc32_spi  *spi_drv    = os_container_of(device->bus, struct hc32_spi, spi_bus);
	struct hc32_hw_spi_cs *cs    = device->parent.user_data;

	if (message->cs_take)
	{
		if(cs->cs_pin_ref == OS_TRUE)
		{			
			/*must called,otherwise spi driver will loop*/
			Spi_SetCS(spi_drv->spi_base, FALSE);

			Gpio_WriteOutputIO(cs->GPIOx,cs->GPIO_Pin,FALSE);			
		}
		else
		{
			Spi_SetCS(spi_drv->spi_base, FALSE);
		}
	}

	LOG_EXT_D("%s transfer prepare and start", spi_drv->name);
	LOG_EXT_D("%s sendbuf: %X, recvbuf: %X, length: %d",
			spi_drv->name,
			(uint32_t)message->send_buf,
			(uint32_t)message->recv_buf,
			message->length);

	message_length = message->length;
	recv_buf       = message->recv_buf;
	send_buf       = message->send_buf;
	while (message_length)
	{
		/* the HC library use uint32 to save the data length */
		if (message_length > (uint32_t)(~0))
		{
			send_length    = (uint32_t)(~0);
			message_length = message_length - (uint32_t)(~0);
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
			memset((uint8_t *)recv_buf, 0xff, send_length);

			state = Spi_SendBuf(spi_drv->spi_base, (uint8_t *)send_buf, send_length);

			if(state != Ok)
				goto done;

			state = Spi_ReceiveBuf(spi_drv->spi_base, (uint8_t *)recv_buf, send_length);
		}
		else if (message->send_buf)
		{
			state = Spi_SendBuf(spi_drv->spi_base, (uint8_t *)send_buf, send_length);
		}
		else
		{
			memset((uint8_t *)recv_buf, 0xff, send_length);
			state = Spi_ReceiveBuf(spi_drv->spi_base, (uint8_t *)recv_buf, send_length);
		}
done:
		if (state != Ok)
		{
			LOG_EXT_I("spi transfer error : %d", state);
			message->length   = 0;
		}


		else
		{
			LOG_EXT_D("%s transfer done", spi_drv->name);
		}

	}

	if (message->cs_release)
	{
		if(cs->cs_pin_ref == OS_TRUE)
		{

			/*must called,otherwise spi driver will loop*/			
			Spi_SetCS(spi_drv->spi_base, TRUE);

			Gpio_WriteOutputIO(cs->GPIOx,cs->GPIO_Pin,TRUE);
		}
		else
		{
			Spi_SetCS(spi_drv->spi_base, TRUE);
		}
	}

	return message->length;
}




/*do not use cs_pin selected by user*/
os_err_t os_hw_spi_device_attach(const char *bus_name, const char *device_name, os_base_t cs_pin)
{	
	struct hc32_hw_spi_cs *__cs_pin = OS_NULL;
	os_device_t *bus = OS_NULL;
	struct os_spi_bus *spi_bus = OS_NULL;
	struct hc32_spi *spi_drv = OS_NULL;
	os_err_t                result;
	struct os_spi_device   *spi_device;
	stc_gpio_cfg_t GPIO_Cfg;

	OS_ASSERT(bus_name != OS_NULL);
	OS_ASSERT(device_name != OS_NULL);

	__cs_pin = (struct hc32_hw_spi_cs *)os_malloc(sizeof(struct hc32_hw_spi_cs));
	OS_ASSERT(__cs_pin != OS_NULL);

	__cs_pin->GPIOx = PIN_BASE(cs_pin);
	__cs_pin->GPIO_Pin = PIN_OFFSET(cs_pin);

	/* attach the device to spi bus*/
	spi_device = (struct os_spi_device *)os_malloc(sizeof(struct os_spi_device));
	OS_ASSERT(spi_device != OS_NULL);

	result = os_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)__cs_pin);

	if (result != OS_EOK)
	{
		LOG_EXT_E("%s attach to %s faild, %d\n", device_name, bus_name, result);
	}

	OS_ASSERT(result == OS_EOK);

	bus = os_device_find(bus_name);

	if (bus != OS_NULL && bus->type == OS_DEVICE_TYPE_SPIBUS)
	{
		spi_bus = os_container_of(bus, struct os_spi_bus, parent);
		spi_drv = (struct hc32_spi *)spi_bus;

		if(__cs_pin->GPIOx != spi_drv->port_cs || __cs_pin->GPIO_Pin != spi_drv->pin_cs)
		{
			__cs_pin->cs_pin_ref = OS_TRUE;

			Gpio_SetAfMode(__cs_pin->GPIOx, __cs_pin->GPIO_Pin,GpioAf0);

			GPIO_Cfg.enCtrlMode = GpioAHB;
			GPIO_Cfg.enDrv = GpioDrvH;
			GPIO_Cfg.enDir = GpioDirOut;
			GPIO_Cfg.enPd = GpioPdDisable;
			GPIO_Cfg.enPu = GpioPuDisable;
			Gpio_Init(__cs_pin->GPIOx, __cs_pin->GPIO_Pin, &GPIO_Cfg);

			Gpio_WriteOutputIO(__cs_pin->GPIOx, __cs_pin->GPIO_Pin,TRUE);

			goto done;
		}
	}

	__cs_pin->cs_pin_ref = OS_FALSE;

done:

	LOG_EXT_D("%s attach to %s done", device_name, bus_name);

	return result;


}

static const struct os_spi_ops hc32_spi_ops = {
	.configure = spi_configure,
	.xfer      = spixfer,
};

static inline void __os_hw_spi_init(struct hc32_spi *spi_drv)
{	
	stc_gpio_cfg_t GpioInitStruct;

	Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);
	Sysctrl_SetPeripheralGate(spi_drv->peripheral,TRUE);

	DDL_ZERO_STRUCT(GpioInitStruct);

	GpioInitStruct.enDrv = GpioDrvH;
	GpioInitStruct.enDir = GpioDirOut;   

	Gpio_Init(spi_drv->port_cs, spi_drv->pin_cs, &GpioInitStruct);
	Gpio_SetAfMode(spi_drv->port_cs, spi_drv->pin_cs, spi_drv->gpio_af);

	Gpio_Init(spi_drv->port_sck, spi_drv->pin_sck, &GpioInitStruct);            
	Gpio_SetAfMode(spi_drv->port_sck, spi_drv->pin_sck, spi_drv->gpio_af);

	Gpio_Init(spi_drv->port_mosi, spi_drv->pin_mosi, &GpioInitStruct);           
	Gpio_SetAfMode(spi_drv->port_mosi, spi_drv->pin_mosi, spi_drv->gpio_af);

	GpioInitStruct.enDir = GpioDirIn;                          
	Gpio_Init(spi_drv->port_miso, spi_drv->pin_miso, &GpioInitStruct);            
	Gpio_SetAfMode(spi_drv->port_miso, spi_drv->pin_miso, spi_drv->gpio_af);
}


int os_hw_spi_init(void)
{
	os_err_t result  = OS_ERROR;
	os_uint32_t idx = 0;
	struct os_spi_bus *spi_bus;


	for(idx=0;idx<(sizeof(spis)/sizeof(spis[0]));idx++)
	{
		spi_bus = &(spis[idx].spi_bus);

		__os_hw_spi_init(&(spis[idx]));

		result = os_spi_bus_register(spi_bus, spis[idx].name, &hc32_spi_ops);

		if(result != OS_EOK)
		{
			LOG_EXT_D("%s bus init failed",spis[idx].name);
			break;
		}
	}

	LOG_EXT_D("spi bus init done");

	return result;
}

OS_DEVICE_INIT(os_hw_spi_init);


