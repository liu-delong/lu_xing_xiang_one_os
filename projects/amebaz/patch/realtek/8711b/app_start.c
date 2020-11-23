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
 * @file        app_start.c
 *
 * @brief       Main APP entry.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ameba_soc.h"
#include "build_info.h"
#include "os_kernel.h"

_WEAK void SVC_Handler(void)
{
}

extern void PendSV_Handler(void);
extern void SysTick_Handler(void);

extern os_int32_t os_startup(void);

/* The Main App entry point */
void APP_Start(void)
{
#if CONFIG_SOC_PS_MODULE
    SOCPS_InitSYSIRQ();
#endif

    InterruptForOSInit((VOID *)SVC_Handler, (VOID *)PendSV_Handler, (VOID *)SysTick_Handler);

#if defined(__ICCARM__)
    __iar_cstart_call_ctors(NULL);
#endif
    /* force SP align to 8 byte not 4 byte (initial SP is 4 byte align) */
    __asm("mov r0, sp\n"
          "bic r0, r0, #7\n"
          "mov sp, r0\n");

    os_startup();
}
