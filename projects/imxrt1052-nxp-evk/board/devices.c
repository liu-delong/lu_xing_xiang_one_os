static const struct nxp_adc_info adc1_info = {ADC1_PERIPHERAL, &ADC1_config};
OS_HAL_DEVICE_DEFINE("ADC_Type", "adc1", adc1_info);

static const struct nxp_adc_etc_info adc_etc_info = {ADC_ETC_PERIPHERAL, &ADC_ETC_config};
OS_HAL_DEVICE_DEFINE("ADC_ETC_Type", "adc_etc", adc_etc_info);

static const struct nxp_gpt_info gpt1_info = {GPT1_PERIPHERAL, &GPT1_config};
OS_HAL_DEVICE_DEFINE("GPT_Type", "gpt1", gpt1_info);

static const struct nxp_gpt_info gpt2_info = {GPT2_PERIPHERAL, &GPT2_config};
OS_HAL_DEVICE_DEFINE("GPT_Type", "gpt2", gpt2_info);

static const struct nxp_lpi2c_info lpi2c1_info = {LPI2C1_PERIPHERAL, &LPI2C1_masterConfig};
OS_HAL_DEVICE_DEFINE("LPI2C_Type", "lpi2c1", lpi2c1_info);

static const struct nxp_lpi2c_info lpi2c3_info = {LPI2C3_PERIPHERAL, &LPI2C3_masterConfig};
OS_HAL_DEVICE_DEFINE("LPI2C_Type", "lpi2c3", lpi2c3_info);

static const struct nxp_lpspi_info lpspi1_info = {LPSPI1_PERIPHERAL, &LPSPI1_config};
OS_HAL_DEVICE_DEFINE("LPSPI_Type", "lpspi1", lpspi1_info);

static const struct nxp_lpspi_info lpspi3_info = {LPSPI3_PERIPHERAL, &LPSPI3_config};
OS_HAL_DEVICE_DEFINE("LPSPI_Type", "lpspi3", lpspi3_info);

static const struct nxp_lpuart_info lpuart1_info = {LPUART1_PERIPHERAL, &LPUART1_config};
OS_HAL_DEVICE_DEFINE("LPUART_Type", "lpuart1", lpuart1_info);

static const struct nxp_rtc_info rtc_info = {RTC_PERIPHERAL, &RTC_config};
OS_HAL_DEVICE_DEFINE("RTC_Type", "rtc", rtc_info);

static const struct nxp_rtwdog_info rtwdog_info = {RTWDOG_PERIPHERAL, &RTWDOG_config};
OS_HAL_DEVICE_DEFINE("RTWDOG_Type", "rtwdog", rtwdog_info);

static const struct nxp_sai_info sai1_info = {SAI1_PERIPHERAL, &SAI1_Tx_config, &SAI1_Rx_config};
OS_HAL_DEVICE_DEFINE("SAI_Type", "sai1", sai1_info);

static const struct nxp_trng_info trng1_info = {TRNG1_PERIPHERAL, &TRNG1_config};
OS_HAL_DEVICE_DEFINE("TRNG_Type", "trng1", trng1_info);

static const struct nxp_wdog_info wdog1_info = {WDOG1_PERIPHERAL, &WDOG1_config};
OS_HAL_DEVICE_DEFINE("WDOG_Type", "wdog1", wdog1_info);

