#ifndef __DRV_RTC_H__
#define __DRV_RTC_H__

#ifdef BSP_USING_ONCHIP_RTC

typedef struct 
{
    os_int32_t rtc_year;
}os_rtc_para;

typedef struct
{
    __IO os_uint32_t BAK0;
    __IO os_uint32_t BAK1;
    __IO os_uint32_t BAK2;
    __IO os_uint32_t BAK3;
    __IO os_uint32_t BAK4;
}RTC_BAK_Type;

#define RTC_BAK     ((RTC_BAK_Type *)(RTC_BASE + 0x200))

#define BKUP_REG_DATA 0xA5A5

#endif /* BSP_USING_ONCHIP_RTC */

#endif /* __DRV_RTC_H__ */

