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
 * @file        posix_mmap.c
 *
 * @brief       This file implements the posix mmap functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <oneos_config.h>
#include <os_task.h>
#include <vfs_posix.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    uint8_t *mem;

    if (addr)
    {
        mem = addr;
    }
    else
    {
        mem = (uint8_t *)malloc(length);
    }

    if (mem)
    {
        off_t cur;
        size_t read_bytes;

        cur = lseek(fd, 0, SEEK_SET);

        lseek(fd, offset, SEEK_SET);
        
        read_bytes = read(fd, mem, length);
        if (read_bytes != length)
        {
            if (addr == OS_NULL)
            {
                /* read failed */
                free(mem);
                mem = OS_NULL;
            }
        }
        
        lseek(fd, cur, SEEK_SET);

        return mem;
    }

    errno = ENOMEM;

    return MAP_FAILED;
}

int munmap(void *addr, size_t length)
{
    if (addr)
    {
        free(addr);
        return 0;
    }

    return -1;
}
