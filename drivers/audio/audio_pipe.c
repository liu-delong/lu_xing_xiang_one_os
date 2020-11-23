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
 * @file        audio_pipe.c
 *
 * @brief        This file provides aduio pipe functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_list.h>
#include "drv_cfg.h"
#include "audio_pipe.h"

extern void     os_schedule(void);
extern os_err_t os_task_resume(os_task_t *task);
extern os_err_t os_task_suspend(os_task_t *task);

static void _os_pipe_resume_writer(struct os_audio_pipe *pipe)
{
    if (!os_list_empty(&pipe->suspended_write_list))
    {
        os_task_t *task;

        OS_ASSERT(pipe->flag & OS_PIPE_FLAG_BLOCK_WR);

        /* get suspended task */
        task = os_list_entry(pipe->suspended_write_list.next, struct os_task, task_list);

        /* resume the write task */
        os_task_resume(task);

        os_schedule();
    }
}

static os_size_t os_pipe_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_uint32_t           level;
    os_task_t *           task;
    struct os_audio_pipe *pipe;
    os_size_t             read_nbytes;

    pipe = (struct os_audio_pipe *)dev;
    OS_ASSERT(pipe != OS_NULL);

    if (!(pipe->flag & OS_PIPE_FLAG_BLOCK_RD))
    {
        level       = os_hw_interrupt_disable();
        read_nbytes = rb_ring_buff_get(&(pipe->ringbuffer), buffer, size);

        /* if the ringbuffer is empty, there won't be any writer waiting */
        if (read_nbytes)
            _os_pipe_resume_writer(pipe);

        os_hw_interrupt_enable(level);

        return read_nbytes;
    }

    task = os_task_self();

    do
    {
        level       = os_hw_interrupt_disable();
        read_nbytes = rb_ring_buff_get(&(pipe->ringbuffer), buffer, size);
        if (read_nbytes == 0)
        {
            os_task_suspend(task);
            /* waiting on suspended read list */
            os_list_add(&(pipe->suspended_read_list), &(task->task_list));
            os_hw_interrupt_enable(level);

            os_schedule();
        }
        else
        {
            _os_pipe_resume_writer(pipe);
            os_hw_interrupt_enable(level);
            break;
        }
    } while (read_nbytes == 0);

    return read_nbytes;
}

static void _os_pipe_resume_reader(struct os_audio_pipe *pipe)
{
    struct os_device_cb_info *info = &pipe->parent.cb_table[OS_DEVICE_CB_TYPE_RX];
    if (info->cb != OS_NULL)
    {
        info->size = rb_ring_buff_data_len(&pipe->ringbuffer);
        info->cb(&pipe->parent, info);
    }

    if (!os_list_empty(&pipe->suspended_read_list))
    {
        os_task_t *task;

        OS_ASSERT(pipe->flag & OS_PIPE_FLAG_BLOCK_RD);

        /* get suspended task */
        task = os_list_entry(pipe->suspended_read_list.next, struct os_task, task_list);

        /* resume the read task */
        os_task_resume(task);

        os_schedule();
    }
}

static os_size_t os_pipe_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_uint32_t           level;
    os_task_t *           task;
    struct os_audio_pipe *pipe;
    os_size_t             write_nbytes;

    pipe = (struct os_audio_pipe *)dev;
    OS_ASSERT(pipe != OS_NULL);

    if ((pipe->flag & OS_PIPE_FLAG_FORCE_WR) || !(pipe->flag & OS_PIPE_FLAG_BLOCK_WR))
    {
        level = os_hw_interrupt_disable();

        if (pipe->flag & OS_PIPE_FLAG_FORCE_WR)
            write_nbytes = rb_ring_buff_put_force(&(pipe->ringbuffer), buffer, size);
        else
            write_nbytes = rb_ring_buff_put(&(pipe->ringbuffer), buffer, size);

        _os_pipe_resume_reader(pipe);

        os_hw_interrupt_enable(level);

        return write_nbytes;
    }

    task = os_task_self();

    do
    {
        level        = os_hw_interrupt_disable();
        write_nbytes = rb_ring_buff_put(&(pipe->ringbuffer), buffer, size);
        if (write_nbytes == 0)
        {
            /* pipe full, waiting on suspended write list */
            os_task_suspend(task);
            /* waiting on suspended read list */
            os_list_add(&(pipe->suspended_write_list), &(task->task_list));
            os_hw_interrupt_enable(level);

            os_schedule();
        }
        else
        {
            _os_pipe_resume_reader(pipe);
            os_hw_interrupt_enable(level);
            break;
        }
    } while (write_nbytes == 0);

    return write_nbytes;
}

static os_err_t os_pipe_control(os_device_t *dev, int cmd, void *args)
{
    struct os_audio_pipe *pipe;

    pipe = (struct os_audio_pipe *)dev;

    if (cmd == PIPE_CTRL_GET_SPACE && args)
        *(os_size_t *)args = rb_ring_buff_space_len(&pipe->ringbuffer);
    return OS_EOK;
}

const static struct os_device_ops audio_pipe_ops =
{
    OS_NULL, 
    OS_NULL, 
    OS_NULL, 
    os_pipe_read, 
    os_pipe_write, 
    os_pipe_control
};

/**
 ***********************************************************************************************************************
 * @brief           This function will initialize a pipe device and put it under control of
 * resource management.
 *
 * @param[in]       pipe       the pipe device
 * @param[in]       name       the name of pipe device
 * @param[in]       flag       the attribute of the pipe device
 * @param[in]       buf        the buffer of pipe device
 * @param[in]       size       the size of pipe device buffer
 *
 * @return          os_err_t
 * @retval          OS_EOK     run successfully
 * @retval          OS_EINVAL  Failed
 ***********************************************************************************************************************
 */
os_err_t os_audio_pipe_init(struct os_audio_pipe *pipe,
                      const char *name,
                      os_int32_t flag,
                      os_uint8_t *buf,
                      os_size_t size)
{
    OS_ASSERT(pipe);
    OS_ASSERT(buf);

    /* initialize suspended list */
    os_list_init(&pipe->suspended_read_list);
    os_list_init(&pipe->suspended_write_list);

    /* initialize ring buffer */
    rb_ring_buff_init(&pipe->ringbuffer, buf, size);

    pipe->flag = flag;

    /* create pipe */
    pipe->parent.type = OS_DEVICE_TYPE_PIPE;
    pipe->parent.ops = &audio_pipe_ops;

    return os_device_register(&(pipe->parent), name, OS_DEVICE_FLAG_RDWR);
}

os_err_t os_audio_pipe_detach(struct os_audio_pipe *pipe)
{
    return os_device_unregister(&pipe->parent);
}

#ifdef OS_USING_HEAP
os_err_t os_audio_pipe_create(const char *name, os_int32_t flag, os_size_t size)
{
    os_uint8_t *          rb_memptr = OS_NULL;
    struct os_audio_pipe *pipe      = OS_NULL;

    /* get aligned size */
    size = OS_ALIGN_UP(size, OS_ALIGN_SIZE);
    pipe = (struct os_audio_pipe *)os_calloc(1, sizeof(struct os_audio_pipe));
    if (pipe == OS_NULL)
        return OS_ENOMEM;

    /* create ring buffer of pipe */
    rb_memptr = os_malloc(size);
    if (rb_memptr == OS_NULL)
    {
        os_free(pipe);
        return OS_ENOMEM;
    }

    return os_audio_pipe_init(pipe, name, flag, rb_memptr, size);
}

void os_audio_pipe_destroy(struct os_audio_pipe *pipe)
{
    if (pipe == OS_NULL)
        return;

    /* un-register pipe device */
    os_audio_pipe_detach(pipe);

    /* release memory */
    os_free(pipe->ringbuffer.buff);
    os_free(pipe);

    return;
}

#endif
