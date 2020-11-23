#include "board.h"
#include <drv_gpio.h>

#define LOG os_kprintf

const led_t led_table[] = 
{
    {GET_PIN(C, 7), PIN_LOW},
    {GET_PIN(C, 8), PIN_LOW},
};

const int led_table_size = sizeof(led_table) / sizeof(led_table[0]);
