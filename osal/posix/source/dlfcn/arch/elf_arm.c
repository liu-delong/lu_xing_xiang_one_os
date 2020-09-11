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
 * @file        elf_arm.c
 *
 * @brief      Perform relocation  for rel or rela segments for arm architectures
 *             
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_module.h>
#include <elf.h>

#define DBG_EXT_LVL         DBG_EXT_DEBUG
#define DBG_EXT_TAG         "elf"

#include <os_dbg_ext.h>         

int elf_arch_relocate(struct os_module *module, Elf32_Rel *rel, Elf32_Addr sym_val)
{
    Elf32_Addr *where, tmp;
    Elf32_Sword addend, offset;
    os_uint32_t upper, lower, sign, j1, j2;

    where = (Elf32_Addr *)((os_uint8_t *)module->mem_space
                           + rel->r_offset
                           - module->vstart_addr);
    switch (ELF32_R_TYPE(rel->r_info))
    {
    case R_ARM_NONE:
        break;
    case R_ARM_ABS32:
        *where += (Elf32_Addr)sym_val;
        LOG_EXT_D("R_ARM_ABS32: %x -> %x\n",where, *where);
        break;
    case R_ARM_PC24:
    case R_ARM_PLT32:
    case R_ARM_CALL:
    case R_ARM_JUMP24:
        addend = *where & 0x00ffffff;
        if (addend & 0x00800000)
            addend |= 0xff000000;
        tmp = sym_val - (Elf32_Addr)where + (addend << 2);
        tmp >>= 2;
        *where = (*where & 0xff000000) | (tmp & 0x00ffffff);
        LOG_EXT_D("R_ARM_PC24: %x -> %x\n",where, *where);
        break;
    case R_ARM_REL32:
        *where += sym_val - (Elf32_Addr)where;
        LOG_EXT_D("R_ARM_REL32: %x -> %x, sym %x, offset %x\n",where, *where, sym_val, rel->r_offset);
        break;
    case R_ARM_V4BX:
        *where &= 0xf000000f;
        *where |= 0x01a0f000;
        break;

    case R_ARM_GLOB_DAT:
    case R_ARM_JUMP_SLOT:
        *where = (Elf32_Addr)sym_val;
        LOG_EXT_D("R_ARM_JUMP_SLOT: 0x%x -> 0x%x 0x%x\n",where, *where, sym_val);
        break;
#if 0        /* To do */
    case R_ARM_GOT_BREL:
        temp   = (Elf32_Addr)sym_val;
        *where = (Elf32_Addr)&temp;
        LOG_EXT_D("R_ARM_GOT_BREL: 0x%x -> 0x%x 0x%x\n",where, *where, sym_val);
        break;
#endif

    case R_ARM_RELATIVE:
        *where = (Elf32_Addr)sym_val + *where;
        LOG_EXT_D("R_ARM_RELATIVE: 0x%x -> 0x%x 0x%x\n",where, *where, sym_val);
        break;
    case R_ARM_THM_CALL:
    case R_ARM_THM_JUMP24:
        upper  = *(os_uint16_t *)where;
        lower  = *(os_uint16_t *)((Elf32_Addr)where + 2);

        sign   = (upper >> 10) & 1;
        j1     = (lower >> 13) & 1;
        j2     = (lower >> 11) & 1;
        offset = (sign << 24) |
                 ((~(j1 ^ sign) & 1) << 23) |
                 ((~(j2 ^ sign) & 1) << 22) |
                 ((upper & 0x03ff) << 12) |
                 ((lower & 0x07ff) << 1);
        if (offset & 0x01000000)
            offset -= 0x02000000;
        offset += sym_val - (Elf32_Addr)where;

        if (!(offset & 1) ||
            offset <= (os_int32_t)0xff000000 ||
            offset >= (os_int32_t)0x01000000)
        {
            LOG_EXT_E(" Only Thumb addresses allowed\n");

            return -1;
        }

        sign = (offset >> 24) & 1;
        j1   = sign ^ (~(offset >> 23) & 1);
        j2   = sign ^ (~(offset >> 22) & 1);
        *(os_uint16_t *)where = (os_uint16_t)((upper & 0xf800) |
                                              (sign << 10) |
                                              ((offset >> 12) & 0x03ff));
        *(os_uint16_t *)(where + 2) = (os_uint16_t)((lower & 0xd000) |
                                                    (j1 << 13) | (j2 << 11) |
                                                    ((offset >> 1) & 0x07ff));
        upper = *(os_uint16_t *)where;
        lower = *(os_uint16_t *)((Elf32_Addr)where + 2);
        break;
    default:
        return -1;
    }

    return 0;
}

