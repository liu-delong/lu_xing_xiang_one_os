#ifndef __BSP_H__
#define __BSP_H__

#include "hal_platform.h"
#include "hal_pinmux_define.h"
#include "hal_gpio.h"
#include "drv_pinmux.h"
#include "hal_uart.h"
#include "serial.h"
#include "hal_spi_master.h"

#ifdef BSP_USING_UART

struct mt2625_uart
{
    const char *name;
    hal_uart_port_t uart_port;
    hal_gpio_pin_t uart_tx_pin;
    hal_gpio_pin_t uart_rx_pin;

    uint8_t uart_tx_pinmux;
    uint8_t uart_rx_pinmux;
    uint8_t       hdmatx;
    
    struct os_serial_device serial;
};

typedef enum
{
  HAL_UART_STATE_RESET             = 0x00U,   /*!< Peripheral is not initialized
                                                   Value is allowed for gState and RxState */
  HAL_UART_STATE_READY             = 0x20U,   /*!< Peripheral Initialized and ready for use
                                                   Value is allowed for gState and RxState */
  HAL_UART_STATE_BUSY              = 0x24U,   /*!< an internal process is ongoing
                                                   Value is allowed for gState only */
  HAL_UART_STATE_BUSY_TX           = 0x21U,   /*!< Data Transmission process is ongoing
                                                   Value is allowed for gState only */
  HAL_UART_STATE_BUSY_RX           = 0x22U,   /*!< Data Reception process is ongoing
                                                   Value is allowed for RxState only */
  HAL_UART_STATE_BUSY_TX_RX        = 0x23U,   /*!< Data Transmission and Reception process is ongoing
                                                   Not to be used for neither gState nor RxState.
                                                   Value is result of combination (Or) between gState and RxState values */
  HAL_UART_STATE_TIMEOUT           = 0xA0U,   /*!< Timeout state
                                                   Value is allowed for gState only */
  HAL_UART_STATE_ERROR             = 0xE0U    /*!< Error
                                                   Value is allowed for gState only */
} HAL_UART_StateTypeDef;


typedef struct __UART_HandleTypeDef
{
    struct mt2625_uart  *mt_uart;                /*!< UART registers base address        */
    hal_uart_config_t         Init;                     /*!< UART communication parameters      */    
    uint8_t                  *pRxBuffPtr;              /*!< Pointer to UART Rx transfer Buffer */
    __IO uint16_t                 RxXferSize;               /*!< UART Rx Transfer size              */
    __IO uint16_t            RxXferCount;              /*!< UART Rx Transfer Counter           */
    __IO HAL_UART_StateTypeDef    RxState;             
}UART_HandleTypeDef;
#endif

#ifdef OS_USING_RTC
typedef struct __RTC_HandleTypeDef
{
    void               *Instance;  
}RTC_HandleTypeDef;
#endif

#ifdef OS_USING_WDG
typedef struct __WDG_HandleTypeDef
{
    void      *Instance;  /*!< Register base address */
} WDG_HandleTypeDef;
#endif

#ifdef OS_USING_PWM
typedef struct
{
    hal_pwm_channel_t  pwmx;
    os_uint8_t pwm_clock;
    os_uint32_t pwm_period;
    os_uint32_t total_count;
}PWM_HandleTypeDef;
#endif

#ifdef OS_USING_ADC
typedef struct __ADC_HandleTypeDef
{
  hal_gpio_pin_t    gpio_pio;              /*!< Register base address */
  os_uint32_t        channel;
}ADC_HandleTypeDef;
#endif


#ifdef OS_USING_TIMER_DRIVER
typedef struct __TIM_HandleTypeDef
{
    hal_gpt_port_t gpt_port;
    //os_uint32_t        channel;
}TIM_HandleTypeDef;
#endif

#ifdef OS_USING_SPI
typedef struct __SPI_PIN_INFO_S
{
    os_uint32_t pin_num;
    os_uint32_t pin_num_set;
}SPI_PIN_INFO_S;

typedef struct __SPI_HW_INFO_S
{
    SPI_PIN_INFO_S spi_cs;
    SPI_PIN_INFO_S spi_sck;
    SPI_PIN_INFO_S spi_miso;
    SPI_PIN_INFO_S spi_mosi;
}SPI_HW_INFO_S;

typedef struct __SPI_HandleTypeDef
{
    hal_spi_master_port_t master_port;
    hal_spi_master_config_t spi_config;
    SPI_HW_INFO_S spi_hw;
    os_uint8_t spi_dma;
}SPI_HandleTypeDef;
#endif

#endif /* __BSP_H__ */


