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
 * @file        rthw.h
 *
 * @brief       RT-Thread adaper macro definition of hardware header file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __RT_HW_H__
#define __RT_HW_H__

#include <rtconfig.h>
#include <rtdef.h>
#include <os_hw.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Some macros define */
#ifndef HWREG32
#define HWREG32(x)          (*((volatile rt_uint32_t *)(x)))
#endif

#ifndef HWREG16
#define HWREG16(x)          (*((volatile rt_uint16_t *)(x)))
#endif

#ifndef HWREG8
#define HWREG8(x)           (*((volatile rt_uint8_t *)(x)))
#endif

#ifndef RT_CPU_CACHE_LINE_SZ
#define RT_CPU_CACHE_LINE_SZ	32
#endif

enum RT_HW_CACHE_OPS
{
    RT_HW_CACHE_FLUSH      = 0x01,
    RT_HW_CACHE_INVALIDATE = 0x02,
};

#define rt_hw_cpu_icache_enable     os_hw_cpu_icache_enable
#define rt_hw_cpu_icache_disable    os_hw_cpu_icache_disable
#define rt_hw_cpu_icache_status     os_hw_cpu_icache_status
#define rt_hw_cpu_icache_ops        os_hw_cpu_icache_ops

#define rt_hw_cpu_dcache_enable     os_hw_cpu_dcache_enable
#define rt_hw_cpu_dcache_disable    os_hw_cpu_dcache_disable
#define rt_hw_cpu_dcache_status     os_hw_cpu_dcache_status
#define rt_hw_cpu_dcache_ops        os_hw_cpu_dcache_ops

extern void      rt_hw_cpu_reset(void);
extern void      rt_hw_cpu_shutdown(void);

extern rt_base_t rt_hw_interrupt_disable(void);
extern void      rt_hw_interrupt_enable(rt_base_t level);

extern void      rt_hw_console_output(const char *str);
extern void      rt_hw_show_memory(rt_uint32_t addr, rt_uint32_t size);

extern void      rt_hw_us_delay(rt_uint32_t us);

#define RT_DEFINE_SPINLOCK(x)  
#define RT_DECLARE_SPINLOCK(x)    rt_ubase_t x

#define rt_hw_spin_lock(lock)     *(lock) = rt_hw_interrupt_disable()
#define rt_hw_spin_unlock(lock)   rt_hw_interrupt_enable(*(lock))

#ifdef __cplusplus
}
#endif

#endif /* __RT_HW_H__ */

