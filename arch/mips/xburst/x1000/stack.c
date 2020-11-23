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
 * @file        stack.c
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include "../common/mips.h"

register U32 $GP __asm__ ("$28");

os_uint8_t *os_hw_stack_init(void *tentry, void *parameter, os_uint8_t *stack_addr, void *texit)
{
    static os_uint32_t wSR=0;
    static os_uint32_t wGP;

    mips_reg_ctx	*regCtx;
    mips_arg_ctx	*argCtx;
    os_uint32_t i;

    if (wSR == 0)
    {
        wSR = read_c0_status();
        wSR &= 0xfffffffe;
        wSR |= 0x0403;

        wGP = $GP;
    }

    if ((os_uint32_t) stack_addr & 0x7)
    {
        stack_addr = (os_uint8_t *)((os_uint32_t)stack_addr - 4);
    }

    argCtx = (mips_arg_ctx *)((os_uint32_t)stack_addr - sizeof(mips_arg_ctx));
    regCtx = (mips_reg_ctx *)((os_uint32_t)stack_addr - sizeof(mips_arg_ctx) - sizeof(mips_reg_ctx));

    for (i = 0; i < 4; ++i)
    {
        argCtx->args[i] = i;
    }

    for (i = 0; i < 32; ++i)
    {
        regCtx->regs[i] = i;
    }

    regCtx->regs[REG_SP] = (os_uint32_t)stack_addr;
    regCtx->regs[REG_A0] = (os_uint32_t)parameter;
    regCtx->regs[REG_GP] = (os_uint32_t)wGP;
    regCtx->regs[REG_FP] = (os_uint32_t)0x0;
    regCtx->regs[REG_RA] = (os_uint32_t)texit;

    regCtx->CP0DataLO	= 0x00;
    regCtx->CP0DataHI	= 0x00;
    regCtx->CP0Cause	= read_c0_cause();
    regCtx->CP0Status	= wSR;
    regCtx->CP0EPC		= (os_uint32_t)tentry;
    regCtx->CP0BadVAddr= 0x00;

    return (os_uint8_t *)(regCtx);
}
