#include <board.h>

static void user_task(void *parameter)
{
    int i = 0;

    for (i = 0; i < led_table_size; i++)
    {
        os_pin_mode(led_table[i].pin, PIN_MODE_OUTPUT);
    }

    while (1)
    {
        for (i = 0; i < led_table_size; i++)
        {
            os_pin_write(led_table[i].pin, led_table[i].active_level);
            os_task_msleep(200);

            os_pin_write(led_table[i].pin, !led_table[i].active_level);
            os_task_msleep(200);
        }
    }
}

int main(void)
{
    os_task_t *task;

    task = os_task_create("user", user_task, NULL, 512, 3, 5);
    OS_ASSERT(task);    
    os_task_startup(task);
    
    return 0;
}

