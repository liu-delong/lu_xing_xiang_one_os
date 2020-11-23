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
 * @file        drv_lcd.c
 *
 * @brief       This file implements lcd driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_task.h>

#ifdef BSP_USING_LCD

#include "drv_lcd.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_elcdif.h"
#include "drv_gpio.h"

#define LOG_TAG             "drv.lcd"
#include <drv_log.h>

#if !defined(LCD_WIDTH) || !defined(LCD_HEIGHT)
#error "Please config lcd pixel parameters."
#endif

#if !defined(LCD_HFP) || !defined(LCD_HBP) || !defined(LCD_HSW) || \
    !defined(LCD_VFP) || !defined(LCD_VBP) || !defined(LCD_VSW)
#error "Please config lcd timing parameters."
#endif

#if !defined(LCD_BL_PIN) || !defined(LCD_RST_PIN)
#error "Please config lcd backlight or reset pin."
#endif

struct imxrt_lcd
{
    struct os_device device;
    struct rt_device_graphic_info info;
};

static struct imxrt_lcd lcd;
ALIGN(64) static uint16_t frame_buffer[LCD_HEIGHT][LCD_WIDTH] SECTION("NonCacheable");

static os_err_t imxrt_lcd_init(os_device_t device)
{
    OS_ASSERT(device != OS_NULL);

    os_memset(frame_buffer, 0x00, sizeof(frame_buffer));
    clock_video_pll_config_t pll_config;

    pll_config.loopDivider = 43;
    pll_config.postDivider = 4;
    pll_config.numerator   = 0;
    pll_config.denominator = 0;

    CLOCK_InitVideoPll(&pll_config);

    rt_pin_mode(LCD_RST_PIN, PIN_MODE_OUTPUT); /* LCD_RESET */
    rt_pin_write(LCD_RST_PIN, PIN_LOW);
    os_thread_delay(OS_TICK_PER_SECOND / 100);
    rt_pin_write(LCD_RST_PIN, PIN_HIGH);

    rt_pin_mode (LCD_BL_PIN, PIN_MODE_OUTPUT);  /* LCD_BL */
    rt_pin_write(LCD_BL_PIN, PIN_HIGH);

    /* LCD */
    elcdif_rgb_mode_config_t lcd_config;

    lcd_config.hfp           = LCD_HFP;
    lcd_config.vfp           = LCD_VFP;
    lcd_config.hbp           = LCD_HBP;
    lcd_config.vbp           = LCD_VBP;
    lcd_config.hsw           = LCD_HSW;
    lcd_config.vsw           = LCD_VSW;

    lcd_config.polarityFlags = kELCDIF_DataEnableActiveHigh |
                               kELCDIF_VsyncActiveHigh      |
                               kELCDIF_HsyncActiveLow       |
                               kELCDIF_DriveDataOnRisingClkEdge;

    lcd_config.panelWidth    = LCD_WIDTH;
    lcd_config.panelHeight   = LCD_HEIGHT;
    lcd_config.pixelFormat   = kELCDIF_PixelFormatRGB565;
    lcd_config.dataBus       = kELCDIF_DataBus16Bit;
    lcd_config.bufferAddr    = (uint32_t)frame_buffer;

    ELCDIF_RgbModeInit (LCDIF, &lcd_config);
    ELCDIF_RgbModeStart(LCDIF);

    /* LCD DEVICE */
    lcd.info.width          = LCD_WIDTH;
    lcd.info.height         = LCD_HEIGHT;
    lcd.info.pixel_format   = RTGRAPHIC_PIXEL_FORMAT_RGB565;
    lcd.info.bits_per_pixel = 16;
    lcd.info.framebuffer    = (void *)frame_buffer;

    return OS_EOK;
}

static os_err_t imxrt_lcd_control(os_device_t device, int cmd, void *args)
{
    switch(cmd)
    {
    case RTGRAPHIC_CTRL_RECT_UPDATE:
        break;

    case RTGRAPHIC_CTRL_POWERON:
        rt_pin_write(LCD_BL_PIN, PIN_HIGH);
        break;

    case RTGRAPHIC_CTRL_POWEROFF:
        rt_pin_write(LCD_BL_PIN, PIN_LOW);
        break;

    case RTGRAPHIC_CTRL_GET_INFO:
        os_memcpy(args, &lcd.info, sizeof(lcd.info));
        break;

    case RTGRAPHIC_CTRL_SET_MODE:
        break;
    }

    return OS_EOK;
}

int rt_hw_lcd_init(void)
{
    os_err_t ret;

    lcd.device.type    = OS_DEVICE_TYPE_GRAPHIC;
    lcd.device.init    = imxrt_lcd_init;
    lcd.device.open    = OS_NULL;
    lcd.device.close   = OS_NULL;
    lcd.device.read    = OS_NULL;
    lcd.device.write   = OS_NULL;
    lcd.device.control = imxrt_lcd_control;

    lcd.device.user_data = (void *)&lcd.info;

    ret = os_device_register(&lcd.device, "lcd", RT_DEVICE_FLAG_RDWR);

    return ret;
}

INIT_DEVICE_EXPORT(rt_hw_lcd_init);

#endif /* BSP_USING_LCD */
