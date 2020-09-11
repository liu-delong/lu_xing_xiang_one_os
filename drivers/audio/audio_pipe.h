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
 * @file        audio_pipe.h
 *
 * @brief       This file implements audio pipe related definitions and declarations.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __AUDIO_PIPE_H__
#define __AUDIO_PIPE_H__

/**
 * Pipe Device
 */
#include <os_task.h>
#include <os_device.h>
#include "drv_cfg.h"
#include "ring_buff.h"

#ifndef OS_PIPE_BUFSZ
#define PIPE_BUFSZ 512
#else
#define PIPE_BUFSZ OS_PIPE_BUFSZ
#endif

/* portal device */
struct os_audio_portal_device
{
    struct os_device  parent;
    struct os_device *write_dev;
    struct os_device *read_dev;
};

enum os_audio_pipe_flag
{
    /* both read and write won't block */
    OS_PIPE_FLAG_NONBLOCK_RDWR = 0x00,
    /* read would block */
    OS_PIPE_FLAG_BLOCK_RD = 0x01,
    /* write would block */
    OS_PIPE_FLAG_BLOCK_WR = 0x02,
    /* write to this pipe will discard some data when the pipe is full.
     * When this flag is set, OS_PIPE_FLAG_BLOCK_WR will be ignored since write
     * operation will always be success. */
    OS_PIPE_FLAG_FORCE_WR = 0x04,
};

struct os_audio_pipe
{
    struct os_device parent;

    /* ring buffer in pipe device */
    struct rb_ring_buff ringbuffer;

    os_int32_t flag;

    /* suspended list */
    os_list_node_t suspended_read_list;
    os_list_node_t suspended_write_list;

    struct os_audio_portal_device *write_portal;
    struct os_audio_portal_device *read_portal;
};

#define PIPE_CTRL_GET_SPACE 0x14 /**< get the remaining size of a pipe device */

os_err_t os_audio_pipe_init(struct os_audio_pipe *pipe,
                      const char *name,
                      os_int32_t flag,
                      os_uint8_t *buf,
                      os_size_t size);
  
os_err_t os_audio_pipe_detach(struct os_audio_pipe *pipe);

#ifdef OS_USING_HEAP
os_err_t os_audio_pipe_create(const char *name, os_int32_t flag, os_size_t size);
void     os_audio_pipe_destroy(struct os_audio_pipe *pipe);
#endif
#endif
