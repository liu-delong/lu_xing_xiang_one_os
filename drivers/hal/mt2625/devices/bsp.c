
#include "bsp.h"
#if 1
#ifdef BSP_USING_UART

#ifdef BSP_USING_UART0
#ifdef BSP_UART0_TXRX_USING_DMA
#define UART0_DMA_FLAG   1
#else
#define UART0_DMA_FLAG   0
#endif
static struct mt2625_uart uart_obj_sets0 = 
{
    .name = "uart0",
    .uart_port = HAL_UART_0,
    .uart_tx_pinmux = DRV_FUNC_UART0_TX,
    .uart_rx_pinmux = DRV_FUNC_UART0_RX,
    .uart_tx_pin = DRV_GPIO_UART0_TX,
    .uart_rx_pin = DRV_GPIO_UART0_RX,
    .hdmatx = UART0_DMA_FLAG,
};

UART_HandleTypeDef huart0 =
{
    .mt_uart = &uart_obj_sets0,
};
#endif


#ifdef BSP_USING_UART1
#ifdef BSP_UART1_TXRX_USING_DMA
#define UART1_DMA_FLAG   1
#else
#define UART1_DMA_FLAG   0
#endif

static struct mt2625_uart uart_obj_sets1 = 
{
    .name = "uart1",
    .uart_port = HAL_UART_1,
    .uart_tx_pinmux = DRV_FUNC_UART1_TX,
    .uart_rx_pinmux = DRV_FUNC_UART1_RX,
    .uart_tx_pin = DRV_GPIO_UART1_TX,
    .uart_rx_pin = DRV_GPIO_UART1_RX,
    .hdmatx = UART1_DMA_FLAG,
};

UART_HandleTypeDef huart1 =
{
    .mt_uart = &uart_obj_sets1,
};
#endif

#ifdef BSP_USING_UART2
#ifdef BSP_UART2_TXRX_USING_DMA
#define UART2_DMA_FLAG   1
#else
#define UART2_DMA_FLAG   0
#endif

static struct mt2625_uart uart_obj_sets2 = 
{
    .name = "uart2",
    .uart_port = HAL_UART_2,
    .uart_tx_pinmux = DRV_FUNC_UART2_TX,
    .uart_rx_pinmux = DRV_FUNC_UART2_RX,
    .uart_tx_pin = DRV_GPIO_UART2_TX,
    .uart_rx_pin = DRV_GPIO_UART2_RX,
    .hdmatx = UART2_DMA_FLAG,
};

UART_HandleTypeDef huart2 =
{
    .mt_uart = &uart_obj_sets2,
};
#endif

#ifdef BSP_USING_UART3
#ifdef BSP_UART3_TXRX_USING_DMA
#define UART3_DMA_FLAG   1
#else
#define UART3_DMA_FLAG   0
#endif

static struct mt2625_uart uart_obj_sets3 = 
{
    .name = "uart3",
    .uart_port = HAL_UART_3,      
    .uart_tx_pinmux = DRV_FUNC_UART3_TX,
    .uart_rx_pinmux = DRV_FUNC_UART3_RX,
    .uart_tx_pin = DRV_GPIO_UART3_TX,
    .uart_rx_pin = DRV_GPIO_UART3_RX,
    .hdmatx = UART3_DMA_FLAG,
};

UART_HandleTypeDef huart3 =
{
    .mt_uart = &uart_obj_sets3,
};
#endif

#endif /* BSP_USING_UART */
#endif

#ifdef OS_USING_RTC
RTC_HandleTypeDef hrtc;
#endif

#ifdef  OS_USING_WDG
WDG_HandleTypeDef hwdg;
#endif

#ifdef  OS_USING_PWM
#ifdef OS_USING_PWM0
PWM_HandleTypeDef hpwm0 = 
{
    .pwmx = HAL_PWM_0,
    .pwm_clock = OS_PWM0_CLOCK
};
#endif

#ifdef OS_USING_PWM1
PWM_HandleTypeDef hpwm1 = 
{
    .pwmx = HAL_PWM_1,
    .pwm_clock = OS_PWM1_CLOCK
};
#endif

#ifdef OS_USING_PWM2
PWM_HandleTypeDef hpwm2 = 
{
    .pwmx = HAL_PWM_2,
    .pwm_clock = OS_PWM2_CLOCK
};
#endif

#ifdef OS_USING_PWM3
PWM_HandleTypeDef hpwm3 = 
{
    .pwmx = HAL_PWM_3,
    .pwm_clock = OS_PWM3_CLOCK
};
#endif

#endif


#ifdef OS_USING_ADC
#ifdef OS_USING_ADC0 
ADC_HandleTypeDef hadc0 = 
{
    .gpio_pio = HAL_GPIO_30,
    .channel = HAL_ADC_CHANNEL_0
};
#endif

#ifdef OS_USING_ADC1
ADC_HandleTypeDef hadc1 = 
{
    .gpio_pio = HAL_GPIO_31,
    .channel = HAL_ADC_CHANNEL_1
};
#endif

#ifdef OS_USING_ADC2
ADC_HandleTypeDef hadc2 = 
{
    .gpio_pio = HAL_GPIO_32,
    .channel = HAL_ADC_CHANNEL_2
};
#endif

#ifdef OS_USING_ADC3
ADC_HandleTypeDef hadc3 = 
{
    .gpio_pio = HAL_GPIO_33,
    .channel = HAL_ADC_CHANNEL_3
};
#endif

#ifdef OS_USING_ADC4
ADC_HandleTypeDef hadc4 = 
{
    .gpio_pio = HAL_GPIO_34,
    .channel = HAL_ADC_CHANNEL_4
};
#endif

#endif /* OS_USING_ADC */

#ifdef OS_USING_TIMER_DRIVER
TIM_HandleTypeDef htim3 =
{
    .gpt_port = HAL_GPT_3
};

#ifdef OS_USING_HWTIMER1
TIM_HandleTypeDef htim1 =
{
    .gpt_port = HAL_GPT_1
};
#endif

#ifdef OS_USING_HWTIMER2
TIM_HandleTypeDef htim2 =
{
    .gpt_port = HAL_GPT_2
};
#endif

#endif  /* OS_USING_TIMER_DRIVER */

#ifdef OS_USING_SPI

#ifdef BSP_USING_SPI0

#ifdef BSP_SPI0_TXRX_USING_DMA
#define SPI0_TXRX_FLAG  1
#else
#define SPI0_TXRX_FLAG  0
#endif

SPI_HandleTypeDef hspi0 = 
{
    .master_port = HAL_SPI_MASTER_0,
    .spi_hw.spi_cs.pin_num = HAL_GPIO_8,
    .spi_hw.spi_cs.pin_num_set = HAL_GPIO_8_GPIO8,

    .spi_hw.spi_sck.pin_num = HAL_GPIO_11,
    .spi_hw.spi_sck.pin_num_set = HAL_GPIO_11_SPI_MST0_SCK,

    .spi_hw.spi_miso.pin_num = HAL_GPIO_9,
    .spi_hw.spi_miso.pin_num_set = HAL_GPIO_9_SPI_MST0_MISO,

    .spi_hw.spi_mosi.pin_num = HAL_GPIO_10,
    .spi_hw.spi_mosi.pin_num_set = HAL_GPIO_10_SPI_MST0_MOSI,
    .spi_dma = SPI0_TXRX_FLAG,
};
#endif

#endif /* OS_USING_SPI */




