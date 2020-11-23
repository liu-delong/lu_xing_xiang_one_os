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
 * @file        drv_i2c.c
 *
 * @brief       This file implements i2c driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <os_hw.h>
#include <os_device.h>
#include <os_irq.h>
#include <os_memory.h>
#include <os_dbg_ext.h>
#include "drv_i2c.h"

static struct hc32_i2c i2cs[] = {

#ifdef  BSP_USING_I2C0
	{
		.i2c_base = M0P_I2C0,
		.mode = I2cMasterMode,
		.slave_addr = 0,
		.scl_port = I2C0_SCL_PORT,
		.scl_pin = I2C0_SCL_PIN,
		.sda_port = I2C0_SDA_PORT,
		.sda_pin = I2C0_SDA_PIN,
		.peripheral = SysctrlPeripheralI2c0,
		.gpio_af = I2C0_GPIO_AF,
	},
#endif

#ifdef  BSP_USING_I2C1
	{
		.i2c_base = M0P_I2C1,
		.mode = I2cMasterMode,
		.slave_addr = 0,
		.scl_port = I2C1_SCL_PORT,
		.scl_pin = I2C1_SCL_PIN,
		.sda_port = I2C1_SDA_PORT,
		.sda_pin = I2C1_SDA_PIN,
		.peripheral = SysctrlPeripheralI2c1,
		.gpio_af = I2C1_GPIO_AF,
	},
#endif
};


void hal_i2c_init(M0P_I2C_TypeDef *i2c,en_sysctrl_peripheral_gate_t peripheral)
{
	stc_i2c_cfg_t stcI2cCfg;

	DDL_ZERO_STRUCT(stcI2cCfg);                           

	Sysctrl_SetPeripheralGate(peripheral,TRUE);

	stcI2cCfg.u32Pclk = Sysctrl_GetPClkFreq();            
	stcI2cCfg.u32Baud = 100000;                         
	stcI2cCfg.enMode = I2cMasterMode;                     
	stcI2cCfg.u8SlaveAddr = 0x00;                          
	stcI2cCfg.bGc = FALSE;                                
	I2C_Init(i2c,&stcI2cCfg);    

}


en_result_t hc_hal_i2c_read(M0P_I2C_TypeDef* I2CX,uint8_t u8Addr,uint8_t *pu8Data,uint32_t u32Len)
{
	en_result_t enRet = Error;
	uint32_t u32i=0,u8State;

	I2C_SetFunc(I2CX,I2cStart_En);

	while(1)
	{
		while(0 == I2C_GetIrq(I2CX))
		{;}
		u8State = I2C_GetState(I2CX);
		switch(u8State)
		{
			case 0x08:                                   
				I2C_ClearFunc(I2CX,I2cStart_En);
				I2C_WriteByte(I2CX,(u8Addr<<1)|0x01);
				break;
			case 0x18:                                   
				I2C_WriteByte(I2CX,0);                   
				break;
			case 0x28:                                    
				I2C_SetFunc(I2CX,I2cStart_En);
				break;
			case 0x10:                                   
				I2C_ClearFunc(I2CX,I2cStart_En);
				I2C_WriteByte(I2CX,(u8Addr<<1)|0x01);
				break;
			case 0x40:                                    
				if(u32Len>1)
				{
					I2C_SetFunc(I2CX,I2cAck_En);
				}
				break;
			case 0x50:                                   
				pu8Data[u32i++] = I2C_ReadByte(I2CX);
				if(u32i==u32Len-1)
				{
					I2C_ClearFunc(I2CX,I2cAck_En);        
				}
				break;
			case 0x58:                                   
				pu8Data[u32i++] = I2C_ReadByte(I2CX);
				I2C_SetFunc(I2CX,I2cStop_En);            
				break;    
			case 0x38:                                    
				I2C_SetFunc(I2CX,I2cStart_En);           
				break;
			case 0x48:                                   
				I2C_SetFunc(I2CX,I2cStop_En);
				I2C_SetFunc(I2CX,I2cStart_En);
				break;
			default:                                      
				I2C_SetFunc(I2CX,I2cStart_En);            
				break;
		}
		I2C_ClearIrq(I2CX);                              
		if(u32i==u32Len)                                  
		{
			break;
		}
	}
	enRet = Ok;
	return enRet;
}

en_result_t hc_hal_i2c_write(M0P_I2C_TypeDef* I2CX,uint8_t u8Addr,uint8_t *pu8Data,uint32_t u32Len)
{
	en_result_t enRet = Error;
	uint32_t u32i=0,u8State;
	I2C_SetFunc(I2CX,I2cStart_En);
	while(1)
	{
		while(0 == I2C_GetIrq(I2CX))
		{;}
		u8State = I2C_GetState(I2CX);
		switch(u8State)
		{
			case 0x08:                          
				I2C_ClearFunc(I2CX,I2cStart_En);
				I2C_WriteByte(I2CX,(u8Addr<<1));  
				break;
			case 0x18:                                
			case 0x28:                                 
				I2C_WriteByte(I2CX,pu8Data[u32i++]);
				break;
			case 0x20:                                
			case 0x38:                                
				I2C_SetFunc(I2CX,I2cStart_En);        
				break;
			case 0x30:                                 
				I2C_SetFunc(I2CX,I2cStop_En);         
				break;
			default:
				break;
		}            
		if(u32i>u32Len)
		{
			I2C_SetFunc(I2CX,I2cStop_En);    
			I2C_ClearIrq(I2CX);
			break;
		}		
		I2C_ClearIrq(I2CX);  
	}
	enRet = Ok;
	return enRet;
}


os_size_t os_i2c_master_xfer(struct os_i2c_bus_device *bus, struct os_i2c_msg *msgs, os_uint32_t num)
{

	os_err_t result = OS_EOK;
	os_uint32_t msgs_count = 0;
	os_uint32_t msgs_count_cur = 0;
	os_uint32_t msgs_buf_length = 0;
	os_uint16_t dev_addr = 0;
	os_uint16_t i2c_flag = 0;
	os_uint8_t *i2c_data_buf = OS_NULL;
	os_uint8_t *i2c_data_buf_cur = OS_NULL;

	struct hc32_i2c *i2c;

	i2c = os_container_of(bus, struct hc32_i2c, parent);

	while (msgs_count_cur < num)
	{
		msgs_buf_length = msgs[msgs_count_cur].len;
		i2c_flag = msgs[msgs_count_cur].flags;
		msgs_count = msgs_count_cur + 1;
		dev_addr = msgs[msgs_count_cur].addr;
		while (msgs_count < num)
		{
			if ((msgs[msgs_count].flags & OS_I2C_NO_START) && (msgs[msgs_count].addr == msgs[msgs_count_cur].addr) && ((i2c_flag & 0x01) == (msgs[msgs_count].flags & 0x01) ))
			{
				msgs_buf_length += msgs[msgs_count].len;
				msgs_count++;
			}
			else
			{
				break;
			}
		}

		if (msgs_count - msgs_count_cur > 1)
		{
			i2c_data_buf = (os_uint8_t *)os_calloc(1, msgs_buf_length);
			if (i2c_data_buf == OS_NULL)
			{
				LOG_EXT_E("i2c calloc failed! too many message with same condition to merge!");
			}

			i2c_data_buf_cur = i2c_data_buf;

			if ((i2c_flag & 0x01) == OS_I2C_WR)
			{
				while (msgs_count_cur < msgs_count)
				{
					memcpy(i2c_data_buf_cur, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len);
					i2c_data_buf_cur += msgs[msgs_count_cur].len; 
					msgs_count_cur++;
				}
			}

			if (i2c_flag & OS_I2C_RD)
			{
				result = hc_hal_i2c_read(i2c->i2c_base, dev_addr, i2c_data_buf, msgs_buf_length);
				while (msgs_count_cur < msgs_count)
				{
					memcpy(msgs[msgs_count_cur].buf, i2c_data_buf_cur, msgs[msgs_count_cur].len);
					i2c_data_buf_cur += msgs[msgs_count_cur].len; 
					msgs_count_cur++;
				}

			}
			else
			{
				result = hc_hal_i2c_write(i2c->i2c_base, dev_addr, i2c_data_buf, msgs_buf_length);
			}
			os_free(i2c_data_buf);
		}
		else
		{
			if (i2c_flag & OS_I2C_RD)
			{
				result = hc_hal_i2c_read(i2c->i2c_base, dev_addr, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len);
			}
			else
			{
				result = hc_hal_i2c_write(i2c->i2c_base, dev_addr, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len);
			}
			msgs_count_cur++;
		}

		if (result != OS_EOK)
		{
			break;
		}
	}

	if (result == OS_EOK)
	{
		return num;
	}
	else
	{
		return 0;
	}

}

os_err_t os_i2c_bus_control(struct os_i2c_bus_device *bus, void *arg)
{

	return OS_EOK;
}

static const struct os_i2c_bus_device_ops hc32_i2c_ops =
{
	os_i2c_master_xfer,
	OS_NULL,
	os_i2c_bus_control
};


int os_hw_i2c_init(void)
{
	os_uint32_t idx = 0;
	char name[10];
	stc_gpio_cfg_t stcGpioCfg;

	Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);


	for(idx=0;idx<(sizeof(i2cs)/sizeof(i2cs[0]));idx++){

		DDL_ZERO_STRUCT(stcGpioCfg);

		stcGpioCfg.enDir = GpioDirOut;						
		stcGpioCfg.enOD = GpioOdEnable; 				
		stcGpioCfg.enPu = GpioPuEnable; 						
		stcGpioCfg.enPd = GpioPdDisable;								
		Gpio_Init(i2cs[idx].scl_port,i2cs[idx].scl_pin,&stcGpioCfg);				 
		Gpio_Init(i2cs[idx].sda_port,i2cs[idx].sda_pin,&stcGpioCfg);		
		Gpio_SetAfMode(i2cs[idx].scl_port,i2cs[idx].scl_pin,i2cs[idx].gpio_af);		
		Gpio_SetAfMode(i2cs[idx].sda_port,i2cs[idx].sda_pin,i2cs[idx].gpio_af);

		hal_i2c_init(i2cs[idx].i2c_base,i2cs[idx].peripheral);

		i2cs[idx].parent.ops = &hc32_i2c_ops;
		os_snprintf(name, sizeof(name) - 1, "i2c%d", idx);
		os_i2c_bus_device_register(&(i2cs[idx].parent), (const char *)name,OS_DEVICE_FLAG_RDWR,&(i2cs[idx].parent));
	}


	return 0;
}

OS_DEVICE_INIT(os_hw_i2c_init);

