static const struct nxp_usb_info usb_info = {};
OS_HAL_DEVICE_DEFINE("USB_Type", "usb", usb_info);

static const struct nxp_adc_info adc0_info = {ADC0_PERIPHERAL, &ADC0_config};
OS_HAL_DEVICE_DEFINE("ADC_Type", "adc0", adc0_info);

static const struct nxp_crc_engine_info crc_engine_info = {CRC_ENGINE_PERIPHERAL, &CRC_ENGINE_config};
OS_HAL_DEVICE_DEFINE("CRC_ENGINE_Type", "crc_engine", crc_engine_info);

static const struct nxp_ctimer_info ctimer0_info = {CTIMER0_PERIPHERAL, &CTIMER0_config};
OS_HAL_DEVICE_DEFINE("CTIMER_Type", "ctimer0", ctimer0_info);

static const struct nxp_ctimer_info ctimer1_info = {CTIMER1_PERIPHERAL, &CTIMER1_config};
OS_HAL_DEVICE_DEFINE("CTIMER_Type", "ctimer1", ctimer1_info);

static const struct nxp_ctimer_info ctimer2_info = {CTIMER2_PERIPHERAL, &CTIMER2_config};
OS_HAL_DEVICE_DEFINE("CTIMER_Type", "ctimer2", ctimer2_info);

static const struct nxp_ctimer_info ctimer3_info = {CTIMER3_PERIPHERAL, &CTIMER3_config};
OS_HAL_DEVICE_DEFINE("CTIMER_Type", "ctimer3", ctimer3_info);

static const struct nxp_ctimer_info ctimer4_info = {CTIMER4_PERIPHERAL, &CTIMER4_config};
OS_HAL_DEVICE_DEFINE("CTIMER_Type", "ctimer4", ctimer4_info);

static const struct nxp_i2c_info i2c1_info = {I2C1_PERIPHERAL, &I2C1_config};
OS_HAL_DEVICE_DEFINE("I2C_Type", "i2c1", i2c1_info);

static const struct nxp_i2s_info i2s7_info = {I2S7_PERIPHERAL, &I2S7_config};
OS_HAL_DEVICE_DEFINE("I2S_Type", "i2s7", i2s7_info);

static const struct nxp_rtc_info rtc_info = {RTC_PERIPHERAL, OS_NULL};
OS_HAL_DEVICE_DEFINE("RTC_Type", "rtc", rtc_info);

static const struct nxp_spi_info spi0_info = {SPI0_PERIPHERAL, &SPI0_config};
OS_HAL_DEVICE_DEFINE("SPI_Type", "spi0", spi0_info);

static const struct nxp_spi_info spi3_info = {SPI3_PERIPHERAL, &SPI3_config};
OS_HAL_DEVICE_DEFINE("SPI_Type", "spi3", spi3_info);

static const struct nxp_spi_info spi6_info = {SPI6_PERIPHERAL, &SPI6_config};
OS_HAL_DEVICE_DEFINE("SPI_Type", "spi6", spi6_info);

static const struct nxp_spi_info spi8_info = {SPI8_PERIPHERAL, &SPI8_config};
OS_HAL_DEVICE_DEFINE("SPI_Type", "spi8", spi8_info);

static const struct nxp_usart_info usart2_info = {USART2_PERIPHERAL, &USART2_config};
OS_HAL_DEVICE_DEFINE("USART_Type", "usart2", usart2_info);

static const struct nxp_usart_info usart4_info = {USART4_PERIPHERAL, &USART4_config};
OS_HAL_DEVICE_DEFINE("USART_Type", "usart4", usart4_info);

static const struct nxp_usart_info usart5_info = {USART5_PERIPHERAL, &USART5_config};
OS_HAL_DEVICE_DEFINE("USART_Type", "usart5", usart5_info);

static const struct nxp_utick_info utick0_info = {UTICK0_PERIPHERAL, OS_NULL};
OS_HAL_DEVICE_DEFINE("UTICK_Type", "utick0", utick0_info);

static const struct nxp_wwdt_info wwdt_info = {WWDT_PERIPHERAL, &WWDT_config};
OS_HAL_DEVICE_DEFINE("WWDT_Type", "wwdt", wwdt_info);

