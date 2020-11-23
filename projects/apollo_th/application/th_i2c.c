
#include <os_memory.h>
#include "i2c.h"
#include "th_i2c.h"
#include "os_errno.h"

extern void *os_memset(void *buff, os_uint8_t val, os_size_t count);


#define POLYNOMIAL  0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001

struct os_i2c_client *SHT3X_i2c = OS_NULL;

#define BSP_SHT3X_I2C_BUS_NAME "soft_i2c1"
#define BSP_SHT3X_I2C_ADDR 0x45

int SHT3X_I2C_init(void)
{
#if 0
    I2C0_pin_config();
    I2C0_init();
    SHT3X_Init(0x45);	//Set sensor i2c slave addr = 0x44;

#else
    SHT3X_i2c = (struct os_i2c_client *)os_calloc(1, sizeof(struct os_i2c_client));
    if (SHT3X_i2c == OS_NULL)
    {
        return -1;
    }

    SHT3X_i2c->bus = (struct os_i2c_bus_device *)os_device_find(BSP_SHT3X_I2C_BUS_NAME);
    if (SHT3X_i2c->bus == OS_NULL)
    {
        return -2;
    }

    SHT3X_i2c->client_addr = BSP_SHT3X_I2C_ADDR;
#endif
    
    return 0;
}


/****************************************************************************
* Function: EndianChange
* Description: 字节序反转
* Param: *value:被反转数据指针
* retval: 0:success,-1:failed
*****************************************************************************/
int EndianChange(void *value)
{
	unsigned char valueSize = sizeof(value);
	char *p = 0, *p1 = 0, temp = 0;
	
	if(valueSize >8 || (valueSize%2 != 0))
	{
		return -1;
	}
	
	p = (char*)value;
	p1 = p + valueSize-1;
	
	while(p < p1)
	{
		temp = *p;
		*p = *p1;
		*p1 = temp;
		p++;
		p1--;
	}
	return 0;
}

//-----------------------------------------------------------------------------
static os_uint8_t SHT3X_CalcCrc(os_uint8_t data[], os_uint8_t nbrOfBytes)
{
  os_uint8_t bit;        // bit mask
  os_uint8_t crc = 0xFF; // calculated checksum
  os_uint8_t byteCtr;    // byte counter

  // calculates 8-Bit checksum with given polynomial
  for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
  {
    crc ^= (data[byteCtr]);
    for(bit = 8; bit > 0; --bit)
    {
      if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
      else           crc = (crc << 1);
    }
  }

  return crc;
}

//-----------------------------------------------------------------------------
static etError SHT3X_CheckCrc(os_uint8_t data[], os_uint8_t nbrOfBytes, os_uint8_t checksum)
{
  os_uint8_t crc;     // calculated checksum

  // calculates 8-Bit checksum
  crc = SHT3X_CalcCrc(data, nbrOfBytes);

  // verify checksum
  if(crc != checksum) return CHECKSUM_ERROR;
  else                return NO_ERROR;
}

//-----------------------------------------------------------------------------
static float SHT3X_CalcTemperature(os_uint16_t rawValue)
{
  // calculate temperature [癈]
  // T = -45 + 175 * rawValue / (2^16-1)
  return 175.0f * (float)rawValue / 65535.0f - 45.0f;
}

//-----------------------------------------------------------------------------
static float SHT3X_CalcHumidity(os_uint16_t rawValue)
{
  // calculate relative humidity [%RH]
  // RH = rawValue / (2^16-1) * 100
  return 100.0f * (float)rawValue / 65535.0f;
}


//-----------------------------------------------------------------------------
etError SHT3X_GetTempAndHumiClkStretch(float* temperature, float* humidity,
                                       etRepeatability repeatability,
                                       os_uint8_t timeout)
{
	etError		error = NO_ERROR;;        // error code
	os_uint16_t    rawValueTemp = 0; // temperature raw value from sensor
	os_uint16_t    rawValueHumi = 0; // humidity raw value from sensor
	os_uint32_t	WriteCmd = 0;
	os_uint8_t    	ReadBuf[6];
        
	os_memset(ReadBuf, 0, 6);
	switch(repeatability)
	{
		case	REPEATAB_LOW:
				WriteCmd = CMD_MEAS_CLOCKSTR_L;
				break;

		case 	REPEATAB_MEDIUM:
				WriteCmd = CMD_MEAS_CLOCKSTR_M;
				break;

		case 	REPEATAB_HIGH:
				WriteCmd = CMD_MEAS_CLOCKSTR_H;
				break;

		default:
				error = PARM_ERROR;
				break;
	}

	EndianChange(&WriteCmd);
	WriteCmd = WriteCmd >> 16;

#if 1
        os_err_t ret;
        ret = os_i2c_client_read(SHT3X_i2c, WriteCmd, 2, ReadBuf, 6);
        if (OS_EOK == ret)
        {
            error = NO_ERROR;
        }
#else        
	if(1 == hal_I2C_Write(0, _i2cAddress, &WriteCmd, 2))
	{
		if(1 == hal_I2C_Read(0, _i2cAddress, (uint32_t*)ReadBuf, 6))
		{
			error = NO_ERROR;
		}
	}
	else
	{
		error = TIMEOUT_ERROR;
	}
#endif

	//am_hal_iom_i2c_write(0, _i2cAddress, &WriteCmd, 2, AM_HAL_IOM_RAW);
	//am_hal_iom_i2c_read(0, _i2cAddress, (uint32_t*)ReadBuf, 8, AM_HAL_IOM_RAW);

	if(error == NO_ERROR)
	{
		error = SHT3X_CheckCrc(ReadBuf, 2, ReadBuf[2]);
	}

	if(error == NO_ERROR)
	{
		error = SHT3X_CheckCrc(&ReadBuf[3], 2, ReadBuf[5]);
	}
	//am_util_stdio_printf("ReadBuf:%s\n",ReadBuf);

	// if no error, calculate temperature in 癈 and humidity in %RH
	if(error == NO_ERROR)
	{
		rawValueTemp = ReadBuf[0]<<8|ReadBuf[1];
		rawValueHumi = ReadBuf[3]<<8|ReadBuf[4];

		*temperature = SHT3X_CalcTemperature(rawValueTemp);
		*humidity = SHT3X_CalcHumidity(rawValueHumi);
	}

	return error;
}

