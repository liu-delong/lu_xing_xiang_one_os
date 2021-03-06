// Copyright 2015-2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdint.h>
#include <string.h>

#include "esp_attr.h"
#include "esp_err.h"

#include "rom/ets_sys.h"
#include "rom/uart.h"
#include "rom/rtc.h"
#include "rom/cache.h"

#include "soc/cpu.h"
#include "soc/rtc.h"
#include "soc/dport_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/timer_group_reg.h"

#include "driver/rtc_io.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/portmacro.h"

#include "tcpip_adapter.h"

#include "esp_heap_alloc_caps.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_task.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_spi_flash.h"
#include "esp_ipc.h"
#include "esp_crosscore_int.h"
#include "esp_dport_access.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "esp_newlib.h"
#include "esp_brownout.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"
#include "esp_phy_init.h"
#include "esp_cache_err_int.h"
#include "esp_coexist.h"
#include "esp_panic.h"
#include "esp_core_dump.h"
#include "esp_app_trace.h"
#include "esp_clk.h"
#include "trax.h"

#define STRINGIFY(s) STRINGIFY2(s)
#define STRINGIFY2(s) #s

void start_cpu0(void) __attribute__((weak, alias("start_cpu0_default")));
void start_cpu0_default(void) IRAM_ATTR;
#if !CONFIG_FREERTOS_UNICORE
static void IRAM_ATTR call_start_cpu1();
void start_cpu1(void) __attribute__((weak, alias("start_cpu1_default")));
void start_cpu1_default(void) IRAM_ATTR;
static bool app_cpu_started = false;
#endif //!CONFIG_FREERTOS_UNICORE

static void do_global_ctors(void);
static void main_task(void* args);
extern void app_main(void);

extern int _bss_start;
extern int _bss_end;
extern int _rtc_bss_start;
extern int _rtc_bss_end;
extern int _init_start;
extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);
extern volatile int port_xSchedulerRunning[2];

static const char* TAG = "cpu_start";

/*
 * We arrive here after the bootloader finished loading the program from flash. The hardware is mostly uninitialized,
 * and the app CPU is in reset. We do have a stack, so we can do the initialization in C.
 */

void IRAM_ATTR call_start_cpu0()
{
#if CONFIG_FREERTOS_UNICORE
    RESET_REASON rst_reas[1];
#else
    RESET_REASON rst_reas[2];
#endif
    cpu_configure_region_protection();

    //Move exception vectors to IRAM
    asm volatile (\
                  "wsr    %0, vecbase\n" \
                  ::"r"(&_init_start));

    rst_reas[0] = rtc_get_reset_reason(0);

#if !CONFIG_FREERTOS_UNICORE
    rst_reas[1] = rtc_get_reset_reason(1);
#endif

    // from panic handler we can be reset by RWDT or TG0WDT
    if (rst_reas[0] == RTCWDT_SYS_RESET || rst_reas[0] == TG0WDT_SYS_RESET
#if !CONFIG_FREERTOS_UNICORE
        || rst_reas[1] == RTCWDT_SYS_RESET || rst_reas[1] == TG0WDT_SYS_RESET
#endif
    ) {
        esp_panic_wdt_stop();
    }

    //Clear BSS. Please do not attempt to do any complex stuff (like early logging) before this.
    memset(&_bss_start, 0, (&_bss_end - &_bss_start) * sizeof(_bss_start));

    /* Unless waking from deep sleep (implying RTC memory is intact), clear RTC bss */
    if (rst_reas[0] != DEEPSLEEP_RESET) {
        memset(&_rtc_bss_start, 0, (&_rtc_bss_end - &_rtc_bss_start) * sizeof(_rtc_bss_start));
    }

    ESP_EARLY_LOGI(TAG, "Pro cpu up.");

#if !CONFIG_FREERTOS_UNICORE
    ESP_EARLY_LOGI(TAG, "Starting app cpu, entry point is %p", call_start_cpu1);
    //Flush and enable icache for APP CPU
    Cache_Flush(1);
    Cache_Read_Enable(1);
    esp_cpu_unstall(1);
    // Enable clock and reset APP CPU. Note that OpenOCD may have already
    // enabled clock and taken APP CPU out of reset. In this case don't reset
    // APP CPU again, as that will clear the breakpoints which may have already
    // been set.
    if (!DPORT_GET_PERI_REG_MASK(DPORT_APPCPU_CTRL_B_REG, DPORT_APPCPU_CLKGATE_EN)) {
        DPORT_SET_PERI_REG_MASK(DPORT_APPCPU_CTRL_B_REG, DPORT_APPCPU_CLKGATE_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_APPCPU_CTRL_C_REG, DPORT_APPCPU_RUNSTALL);
        DPORT_SET_PERI_REG_MASK(DPORT_APPCPU_CTRL_A_REG, DPORT_APPCPU_RESETTING);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_APPCPU_CTRL_A_REG, DPORT_APPCPU_RESETTING);
    }
    ets_set_appcpu_boot_addr((uint32_t)call_start_cpu1);

    while (!app_cpu_started) {
        ets_delay_us(100);
    }
#else
    ESP_EARLY_LOGI(TAG, "Single core mode");
    DPORT_CLEAR_PERI_REG_MASK(DPORT_APPCPU_CTRL_B_REG, DPORT_APPCPU_CLKGATE_EN);
#endif

    /* Initialize heap allocator. WARNING: This *needs* to happen *after* the app cpu has booted.
       If the heap allocator is initialized first, it will put free memory linked list items into
       memory also used by the ROM. Starting the app cpu will let its ROM initialize that memory,
       corrupting those linked lists. Initializing the allocator *after* the app cpu has booted
       works around this problem. */
    heap_alloc_caps_init();


    ESP_EARLY_LOGI(TAG, "Pro cpu start user code");
    start_cpu0();
}

#if !CONFIG_FREERTOS_UNICORE

static void wdt_reset_cpu1_info_enable(void)
{
    DPORT_REG_SET_BIT(DPORT_APP_CPU_RECORD_CTRL_REG, DPORT_APP_CPU_PDEBUG_ENABLE | DPORT_APP_CPU_RECORD_ENABLE);
    DPORT_REG_CLR_BIT(DPORT_APP_CPU_RECORD_CTRL_REG, DPORT_APP_CPU_RECORD_ENABLE);
}

void IRAM_ATTR call_start_cpu1()
{
    asm volatile (\
                  "wsr    %0, vecbase\n" \
                  ::"r"(&_init_start));

    ets_set_appcpu_boot_addr(0);
    cpu_configure_region_protection();

#if CONFIG_CONSOLE_UART_NONE
    ets_install_putc1(NULL);
    ets_install_putc2(NULL);
#else // CONFIG_CONSOLE_UART_NONE
    uartAttach();
    ets_install_uart_printf();
    uart_tx_switch(CONFIG_CONSOLE_UART_NUM);
#endif

    wdt_reset_cpu1_info_enable();
    ESP_EARLY_LOGI(TAG, "App cpu up.");
    app_cpu_started = 1;
    start_cpu1();
}
#endif //!CONFIG_FREERTOS_UNICORE

static void intr_matrix_clear(void)
{
    //Clear all the interrupt matrix register
    for (int i = ETS_WIFI_MAC_INTR_SOURCE; i <= ETS_CACHE_IA_INTR_SOURCE; i++) {
        intr_matrix_set(0, i, ETS_INVALID_INUM);
#if !CONFIG_FREERTOS_UNICORE
        intr_matrix_set(1, i, ETS_INVALID_INUM);
#endif
    }
}

void start_cpu0_default(void)
{
    esp_setup_syscall_table();
//Enable trace memory and immediately start trace.
#if CONFIG_ESP32_TRAX
#if CONFIG_ESP32_TRAX_TWOBANKS
    trax_enable(TRAX_ENA_PRO_APP);
#else
    trax_enable(TRAX_ENA_PRO);
#endif
    trax_start_trace(TRAX_DOWNCOUNT_WORDS);
#endif
    esp_clk_init();
    intr_matrix_clear();

#ifndef CONFIG_CONSOLE_UART_NONE
    uart_div_modify(CONFIG_CONSOLE_UART_NUM, (rtc_clk_apb_freq_get() << 4) / CONFIG_CONSOLE_UART_BAUDRATE);
#endif

#if CONFIG_BROWNOUT_DET
    esp_brownout_init();
#endif


    rtc_gpio_force_hold_dis_all();
    esp_setup_time_syscalls();
    esp_vfs_dev_uart_register();
    esp_reent_init(_GLOBAL_REENT);

    _GLOBAL_REENT->_stdin  = (FILE*) &__sf_fake_stdin;
    _GLOBAL_REENT->_stdout = (FILE*) &__sf_fake_stdout;
    _GLOBAL_REENT->_stderr = (FILE*) &__sf_fake_stderr;


	ESP_EARLY_LOGE(TAG, "run os_startup!");
	extern void entry(void);
	entry();
 	xPortStartScheduler();
    //vTaskStartScheduler();

#if CONFIG_ESP32_APPTRACE_ENABLE
    esp_err_t err = esp_apptrace_init();
    if (err != ESP_OK) {
        ESP_EARLY_LOGE(TAG, "Failed to init apptrace module on CPU0 (%d)!", err);
    }
#endif
#if CONFIG_SYSVIEW_ENABLE
    SEGGER_SYSVIEW_Conf();
#endif
    do_global_ctors();
#if CONFIG_INT_WDT
    esp_int_wdt_init();
#endif
#if CONFIG_TASK_WDT
    esp_task_wdt_init();
#endif
    esp_cache_err_int_init();
    esp_crosscore_int_init();
    esp_ipc_init();
#ifndef CONFIG_FREERTOS_UNICORE
    esp_dport_access_int_init();
#endif
    spi_flash_init();
    /* init default OS-aware flash access critical section */
    spi_flash_guard_set(&g_flash_guard_default_ops);

#if CONFIG_ESP32_ENABLE_COREDUMP
    esp_core_dump_init();
#endif
	
	vTaskStartScheduler();
    xTaskCreatePinnedToCore(&main_task, "main",
            ESP_TASK_MAIN_STACK, NULL,
            ESP_TASK_MAIN_PRIO, NULL, 0);
    ESP_LOGI(TAG, "Starting scheduler on PRO CPU.");
    vTaskStartScheduler();
}

#if !CONFIG_FREERTOS_UNICORE
void start_cpu1_default(void)
{
#if CONFIG_ESP32_TRAX_TWOBANKS
    trax_start_trace(TRAX_DOWNCOUNT_WORDS);
#endif
#if CONFIG_ESP32_APPTRACE_ENABLE
    esp_err_t err = esp_apptrace_init();
    if (err != ESP_OK) {
        ESP_EARLY_LOGE(TAG, "Failed to init apptrace module on CPU1 (%d)!", err);
    }
#endif
    // Wait for FreeRTOS initialization to finish on PRO CPU
    while (port_xSchedulerRunning[0] == 0) {
        ;
    }
    //Take care putting stuff here: if asked, FreeRTOS will happily tell you the scheduler
    //has started, but it isn't active *on this CPU* yet.
    esp_cache_err_int_init();
    esp_crosscore_int_init();
    esp_dport_access_int_init();

    ESP_EARLY_LOGI(TAG, "Starting scheduler on APP CPU.");
    xPortStartScheduler();
}
#endif //!CONFIG_FREERTOS_UNICORE

static void do_global_ctors(void)
{
    void (**p)(void);
    for (p = &__init_array_end - 1; p >= &__init_array_start; --p) {
        (*p)();
    }
}
void app_main()
{
    while(1)
        ;;
}
static void main_task(void* args)
{
    // Now that the application is about to start, disable boot watchdogs
    REG_CLR_BIT(TIMG_WDTCONFIG0_REG(0), TIMG_WDT_FLASHBOOT_MOD_EN_S);
    REG_CLR_BIT(RTC_CNTL_WDTCONFIG0_REG, RTC_CNTL_WDT_FLASHBOOT_MOD_EN);
    //Enable allocation in region where the startup stacks were located.
    heap_alloc_enable_nonos_stack_tag();
    app_main();
    vTaskDelete(NULL);
}

