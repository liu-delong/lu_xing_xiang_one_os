#include "board.h"
#include "define_all.h"
#include "drv_gpio.h"

const led_t led_table[] =
{
    {GET_PIN(C, 12), PIN_LOW},
#ifdef OS_USING_LED
    {GET_PIN(C, 13), PIN_LOW},
    {GET_PIN(C, 14), PIN_LOW},
    {GET_PIN(D, 7), PIN_LOW},
#endif
};

const int led_table_size = ARRAY_SIZE(led_table);


