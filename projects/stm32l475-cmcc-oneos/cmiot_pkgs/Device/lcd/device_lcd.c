#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "usr_lcd.h"

/***********************屏幕状态*********************/
#define OFF                 ((uint32_t)0x00000000)
#define ON                  ((uint32_t)0x00000001)


const mp_obj_type_t mp_lcd_type;

typedef struct _device_lcd_obj_t {
    mp_obj_base_t base;
    device_info_t *lcd_device;
}device_lcd_obj_t;

mp_obj_t mp_lcd_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    device_info_t * lcd = mpycall_device_find("lcd");
    if (NULL == lcd)
    {
        mp_raise_ValueError("lcd can not find!\n");
    }
    device_lcd_obj_t *self = m_new_obj(device_lcd_obj_t);
    self->base.type = &mp_lcd_type;
    self->lcd_device = lcd;
    return (mp_obj_t) self;
}

STATIC mp_obj_t mp_lcd_init(mp_obj_t self)
{
    int ret = 0;
    device_lcd_obj_t *lcd = (device_lcd_obj_t *)self;
	  if (NULL != lcd->lcd_device->ops && NULL != lcd->lcd_device->ops->open)
		{
			  ret = lcd->lcd_device->ops->open(lcd->lcd_device, 0);
				if (0 != ret)
				{
						mp_raise_ValueError("lcd init failed!\n");
				}
		
		}
    
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mplcd_init_obj, mp_lcd_init);

STATIC mp_obj_t mp_lcd_deinit (mp_obj_t self)
{
    int ret = 0;
    device_lcd_obj_t *lcd = (device_lcd_obj_t *)self;
    if (NULL != lcd->lcd_device->ops && NULL != lcd->lcd_device->ops->close)
    {
        ret = lcd->lcd_device->ops->close(lcd->lcd_device);
        if (0 != ret)
        {
            mp_raise_ValueError("lcd_deinit failed!\n");
        }
    }
    else
    {
        printf("lcd deinit function!\n");
    }
    
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mplcd_deinit_obj, mp_lcd_deinit);

STATIC mp_obj_t mp_lcd_light(size_t n_args, const mp_obj_t *args) 
{
	  int ret = 0;
    if (1 == n_args)
    {
        device_lcd_obj_t *lcd = (device_lcd_obj_t *)args[0];
				if (NULL != lcd->lcd_device->ops && NULL != lcd->lcd_device->ops->ioctl)
				{
					ret = lcd->lcd_device->ops->ioctl(lcd->lcd_device, IOCTL_LCD_LIGHT, NULL);
				}
				else
				{
					mp_raise_ValueError("lcd_light failed!\n");
				}
        return MP_OBJ_NEW_SMALL_INT(ret);
    }
    else if (2 == n_args)
    {
        mp_int_t light = mp_obj_get_int(args[1]);
			  device_lcd_obj_t *lcd = (device_lcd_obj_t *)args[0];
				if (NULL != lcd->lcd_device->ops && NULL != lcd->lcd_device->ops->ioctl)
				{
					ret = lcd->lcd_device->ops->ioctl(lcd->lcd_device, IOCTL_LCD_LIGHT, &light);
					if (0 != ret)
					{
						mp_raise_ValueError("lcd_light failed!\n");
					}
				}
				else
				{
					mp_raise_ValueError("lcd_light failed!\n");
				}
        return mp_const_none;
    }
    else
    {
        mp_raise_ValueError("args can be void or one arg only!\n");
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mplcd_light_obj, 1, 2, mp_lcd_light);

//此函数与华大oled函数区别是底层函数只支持全屏清除
STATIC mp_obj_t mp_lcd_clear(size_t n_args, const mp_obj_t *args)
{
    mp_int_t x = 0,y = 0,w = 0,h = 0;
    mp_int_t ret = 0;
    if(1 == n_args)
    {
        device_lcd_obj_t *lcd = (device_lcd_obj_t *)args[0];
        if (NULL != lcd->lcd_device->ops)
        {
            ret = lcd->lcd_device->ops->ioctl(lcd->lcd_device, IOCTL_LCD_CLEAR, NULL);
            if (0 != ret)
            {
                mp_raise_ValueError("oled_init failed!\n");
            }
        }
        else
        {
             printf("clear the whole screen!\n");
        }
    }
    else if (5 == n_args)
    {
        x = mp_obj_get_int(args[1]);
        y = mp_obj_get_int(args[2]);
        w = mp_obj_get_int(args[3]);
        h = mp_obj_get_int(args[4]);

        printf("x = %d\n y = %d\n  w = %d\n  h = %d\n", x, y, w, h);
        device_lcd_obj_t *lcd = (device_lcd_obj_t *)args[0];
        if (NULL != lcd->lcd_device->ops)
        {
            ret = lcd->lcd_device->ops->ioctl(lcd->lcd_device, IOCTL_LCD_CLEAR, (void *)args);
            if (0 != ret)
            {
                mp_raise_ValueError("oled_init failed!\n");
            }
        }
        else
        {
             printf("clear the rect screen  OLED_CLEAR_RECT!\n");
        }
        
    }
    else
    {
         mp_raise_ValueError("arg num is not 4 in one list or arg num is 0!\n");
    }
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mplcd_clear_obj, 0,5 , mp_lcd_clear);

STATIC mp_obj_t device_lcd_pixel_helper \
    (mp_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) 
{
	  int ret = 0;
	  lcd_fill_arg_t lcd_data = {0};
    if (0 == n_args)
    {
        mp_raise_ValueError("examples: oled.pixel(1,2, w= 3,h = 4, col = oled.BLUE)\n");
    }
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_w, MP_ARG_REQUIRED |MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_h, MP_ARG_REQUIRED |MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_col, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };


    lcd_data.x = mp_obj_get_int(pos_args[0]);
    lcd_data.y = mp_obj_get_int(pos_args[1]);
    if ((lcd_data.x > LCD_W) || (lcd_data.x < 0))
    {
        mp_raise_ValueError("x range is 0 ~ LCD_W!\n");
    }
    if ((lcd_data.y > LCD_H) || (lcd_data.x < 0))
    {
        mp_raise_ValueError("y range is 0 ~ LCD_W!\n");
    }

    struct {
        mp_arg_val_t w;
        mp_arg_val_t h; 
        mp_arg_val_t col;
    } args;
    mp_arg_parse_all(n_args - 2, pos_args + 2, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);

    lcd_data.w = args.w.u_int;
    lcd_data.h = args.h.u_int;

    lcd_data.color = BLUE;
    if (args.col.u_obj != mp_const_none)
    {
        lcd_data.color = mp_obj_get_int(args.col.u_obj);
    }
		
		device_lcd_obj_t *lcd = (device_lcd_obj_t *)self;
		if (NULL != lcd->lcd_device->ops)
		{
				ret = lcd->lcd_device->ops->ioctl(lcd->lcd_device, IOCTL_LCD_PIXEL, &lcd_data);
				if (0 != ret)
				{
						mp_raise_ValueError("lcd_init failed!\n");
				}
		}
		else
		{
				 mp_raise_ValueError("fill the rect screen  failed!\n");
		}
		
		
    return mp_const_none;
}

STATIC mp_obj_t mp_lcd_pixel(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    return device_lcd_pixel_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(mplcd_pixel_obj, 1, mp_lcd_pixel);

STATIC mp_obj_t device_lcd_text_helper \
    (mp_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) 
{
	  int ret = 0;
	  lcd_text_arg_t  lcd_text_data = {0};
		
		const char *str = mp_obj_str_get_str(pos_args[0]);
		lcd_text_data.str = m_new(char, strlen(str) + 1);
		strncpy(lcd_text_data.str, str, strlen(str));
		lcd_text_data.str[strlen(str)] = '\0';
    lcd_text_data.x= mp_obj_get_int(pos_args[1]);
    lcd_text_data.y = mp_obj_get_int(pos_args[2]);
	  lcd_text_data.size = mp_obj_get_int(pos_args[3]);

    if ((lcd_text_data.x > LCD_W) || (lcd_text_data.x < 0))
    {
        mp_raise_ValueError("x range is 0 ~ 127!\n");
    }
    if ((lcd_text_data.y > LCD_H) || (lcd_text_data.y < 0))
    {
        mp_raise_ValueError("y range is 0 ~ 63!\n");
    }

    device_lcd_obj_t *lcd = (device_lcd_obj_t *)self;
    if (NULL != lcd->lcd_device->ops)
    {
				ret = lcd->lcd_device->ops->ioctl(lcd->lcd_device, IOCTL_LCD_TEXT, &lcd_text_data);
				if (0 != ret)
				{
						mp_raise_ValueError("oled_init failed!\n");
				}
    }
    else
    {
         mp_raise_ValueError("text the  screen  failed!\n");
    }

    return mp_const_none;
}

STATIC mp_obj_t mp_lcd_text(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    return device_lcd_text_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(mplcd_text_obj, 1, mp_lcd_text);

STATIC mp_obj_t device_lcd_color_helper \
    (mp_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) 
{
	  int ret = 0;
	  lcd_color_arg_t  lcd_color_data = {0};
		
    lcd_color_data.fore_color = mp_obj_get_int(pos_args[0]);
    lcd_color_data.back_color = mp_obj_get_int(pos_args[1]);

    device_lcd_obj_t *lcd = (device_lcd_obj_t *)self;
    if (NULL != lcd->lcd_device->ops)
    {
				ret = lcd->lcd_device->ops->ioctl(lcd->lcd_device, IOCTL_LCD_COLOR, &lcd_color_data);
				if (0 != ret)
				{
						mp_raise_ValueError("oled set color  failed!\n");
				}
    }
    else
    {
        mp_raise_ValueError("oled set color  failed!\n");
    }

    return mp_const_none;
}

STATIC mp_obj_t mp_lcd_color(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    return device_lcd_color_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(mplcd_color_obj, 1, mp_lcd_color);


STATIC mp_obj_t mp_lcd_fill(mp_obj_t self, mp_obj_t color)
{
	  int ret = 0;
    mp_int_t lcd_color = mp_obj_get_int(color);
	  //这里设置的不全，等调试的时候再加上
	  #if 0
    if ((WHITE != lcd_color) && (BLACK != lcd_color))
    {
        mp_raise_ValueError("color error!\n");
    }
	  #endif
		device_lcd_obj_t *lcd = (device_lcd_obj_t *)self;
		if (NULL != lcd->lcd_device->ops && NULL != lcd->lcd_device->ops->ioctl)
		{
			ret = lcd->lcd_device->ops->ioctl(lcd->lcd_device, IOCTL_LCD_FILL, &lcd_color);
			if (0 != ret)
			{
				mp_raise_ValueError("lcd_light failed!\n");
			}
		}
		else
		{
			mp_raise_ValueError("lcd_light failed!\n");
		}
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mplcd_fill_obj, mp_lcd_fill);



STATIC const mp_rom_map_elem_t mp_module_lcd_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), 	MP_ROM_PTR(&mplcd_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), 	MP_ROM_PTR(&mplcd_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), 	MP_ROM_PTR(&mplcd_clear_obj) },
		{ MP_ROM_QSTR(MP_QSTR_light), MP_ROM_PTR(&mplcd_light_obj) },
		{ MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&mplcd_fill_obj) },
		{ MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&mplcd_clear_obj) },
		{ MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&mplcd_pixel_obj) },
		{ MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&mplcd_text_obj) },
		{ MP_ROM_QSTR(MP_QSTR_color), MP_ROM_PTR(&mplcd_color_obj) },
		
		{ MP_ROM_QSTR(MP_QSTR_ON), MP_ROM_INT(ON) },
    { MP_ROM_QSTR(MP_QSTR_OFF), MP_ROM_INT(OFF) },
		
		{ MP_ROM_QSTR(MP_QSTR_WHITE), MP_ROM_INT(WHITE) },
		{ MP_ROM_QSTR(MP_QSTR_BLACK), MP_ROM_INT(BLACK) },
		{ MP_ROM_QSTR(MP_QSTR_BLUE), MP_ROM_INT(BLUE) },
		{ MP_ROM_QSTR(MP_QSTR_BRED), MP_ROM_INT(BRED) },
		{ MP_ROM_QSTR(MP_QSTR_GRED), MP_ROM_INT(GRED) },
		{ MP_ROM_QSTR(MP_QSTR_GBLUE), MP_ROM_INT(GBLUE) },
		{ MP_ROM_QSTR(MP_QSTR_RED), MP_ROM_INT(RED) },
		{ MP_ROM_QSTR(MP_QSTR_MAGENTA), MP_ROM_INT(MAGENTA) },
		{ MP_ROM_QSTR(MP_QSTR_GREEN), MP_ROM_INT(GREEN) },
		{ MP_ROM_QSTR(MP_QSTR_CYAN), MP_ROM_INT(CYAN) },
		{ MP_ROM_QSTR(MP_QSTR_YELLOW), MP_ROM_INT(YELLOW) },
		{ MP_ROM_QSTR(MP_QSTR_BROWN), MP_ROM_INT(BROWN) },
		{ MP_ROM_QSTR(MP_QSTR_BRRED), MP_ROM_INT(BRRED) },
		{ MP_ROM_QSTR(MP_QSTR_GRAY), MP_ROM_INT(GRAY) },
		{ MP_ROM_QSTR(MP_QSTR_GRAY175), MP_ROM_INT(GRAY175) },
		{ MP_ROM_QSTR(MP_QSTR_GRAY151), MP_ROM_INT(GRAY151) },
		{ MP_ROM_QSTR(MP_QSTR_GRAY187), MP_ROM_INT(GRAY187) },
		{ MP_ROM_QSTR(MP_QSTR_GRAY240), MP_ROM_INT(GRAY240) },
		
};

STATIC MP_DEFINE_CONST_DICT(device_lcd_locals_dict, mp_module_lcd_globals_table);

const mp_obj_type_t mp_lcd_type =
{
    { &mp_type_type },
    .name = MP_QSTR_LCD,
    .make_new = mp_lcd_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_lcd_locals_dict,
};


