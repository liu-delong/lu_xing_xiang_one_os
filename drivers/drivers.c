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
 * @file        drivers.c
 *
 * @brief       this file implements timer related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <drivers.h>

#ifdef OS_USING_DEVICE_OPS
const static struct os_device_ops dummy_ops =
{
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL
};
#endif

void os_device_default(struct os_device *device, enum os_device_type type)
{
    memset(device, 0, sizeof(struct os_device));
    
#ifdef OS_USING_DEVICE_OPS
    device->ops = &dummy_ops;
#endif

    device->type = type;
}

void calc_mult_shift(os_uint32_t *mult, os_uint32_t *shift, os_uint32_t from, os_uint32_t to, os_uint32_t max_from)
{
    os_uint64_t tmp;
    os_uint32_t sft, sftacc = 32;

    tmp = ((os_uint64_t)max_from * from) >> 32;
    while (tmp)
    {
        tmp >>= 1;
        sftacc--;
    }

    for (sft = 32; sft > 0; sft--)
    {
        tmp = (os_uint64_t)to << sft;
        tmp += from / 2;
        tmp /= from;
        
        if ((tmp >> sftacc) == 0)
            break;
    }
    
    *mult = tmp;
    *shift = sft;
}

static const unsigned short crc_ta[16] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
};

unsigned short crc16(unsigned short init_crc, void *buff, int len)
{
    unsigned short crc = init_crc;
    unsigned char *ptr = buff;
    unsigned char da;

    while (len-- > 0)
    {
        da = crc >> 12;
        crc <<= 4;
        crc ^= crc_ta[da^(*ptr>>4)];
        
        da = crc >> 12;
        crc <<= 4;
        crc ^= crc_ta[da^(*ptr&0x0f)];

        ptr++;
    }

    return crc;
}

void hex_dump(unsigned char *buff, int count)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (i % 16 == 0)
            os_kprintf("%08x: ", i);
    
        os_kprintf("%02x ", buff[i]);

        if (i % 8 == 7)
            os_kprintf(" ");

        if (i % 16 == 15)
            os_kprintf("\r\n");
    }

    os_kprintf("\r\n");
}

/* find first set */
int os_ffs(os_uint32_t value)
{
    int i;

    for (i = 31; i >= 0; i--)
    {
        if (value & (1ul << 31))
            break;

        value <<= 1;
    }

    return i;
}

/* find last set */
int os_fls(os_uint32_t value)
{
    int i;

    for (i = 0; i < 32; i++)
    {
        if (value & 1)
            break;

        value >>= 1;
    }

    return i;
}

#ifdef OS_USING_CONSOLE
os_err_t os_console_init(void)
{
    os_console_set_device(OS_CONSOLE_DEVICE_NAME);
    return 0;
}

OS_INIT_EXPORT(os_console_init, "0.end.1");
#endif

