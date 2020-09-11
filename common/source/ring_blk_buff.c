/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * Copyright (c) 2006-2018, RT-Thread Development Team.
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
 * @file        ring_blk_buff.c
 *
 * @brief       This function implements a ring block buffer.
 *
 * @details     The ring block buffer(rbb) is the ring buffer which is composed with many blocks. It is different from
 *              the ring buffer. The ring buffer is only composed with chars. The rbb put and get supported zero copies.
 *              So the rbb is very suitable for put block and get block by a certain order. Such as DMA block transmit,
 *              communicate frame send/recv, and so on.
 *
 * @revision
 * Date         Author          Notes
 * 2018-08-25   armink          The first version.
 ***********************************************************************************************************************
 */

#include <os_types.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <os_module.h>
#include <os_list.h>
#include <os_memory.h>
#include <os_hw.h>
#include <os_dbg.h>
#include <ring_blk_buff.h>

static rbb_blk_t *rbb_find_empty_blk_in_set(rbb_ctrl_info_t *rbb)
{
    os_int16_t i;

    OS_ASSERT(rbb);

    for (i = 0; i < rbb->blk_max_num; i++)
    {
        if (RBB_BLK_STATUS_UNUSED == rbb->blk_set[i].status)
        {
            return &rbb->blk_set[i];
        }
    }

    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           Initialize ring block buffer.
 *
 * @attention       When your application need align access, please make the buffer address is aligned.
 *
 * @param[in]       rbb             Ring block buffer control into.
 * @param[in]       buf             The buffer.
 * @param[in]       buf_size        The buffer size.
 * @param[in]       block_set       Block set.
 * @param[in]       blk_max_num     Max block count.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void rbb_init(rbb_ctrl_info_t   *rbb,
              os_uint8_t        *buf,
              os_size_t          buf_size,
              rbb_blk_t         *block_set,
              os_size_t          blk_max_num)
{
    os_size_t i;

    OS_ASSERT(rbb);
    OS_ASSERT(buf);
    OS_ASSERT(block_set);

    rbb->buf         = buf;
    rbb->buf_size    = buf_size;
    rbb->blk_set     = block_set;
    rbb->blk_max_num = blk_max_num;

    os_slist_init(&rbb->blk_list);

    /* Initialize block status */
    for (i = 0; i < blk_max_num; i++)
    {
        block_set[i].status = RBB_BLK_STATUS_UNUSED;
    }

    return;
}
EXPORT_SYMBOL(rbb_init);

#ifdef OS_USING_HEAP
/**
 ***********************************************************************************************************************
 * @brief           Create ring block buffer.
 *
 * @param[in]       buf_size        Buffer size to create.
 * @param[in]       blk_max_num     Max block count.
 *
 * @return          Ring block buffer control info.
 * @retval          OS_NULL         Create ring block buffer failed.
 * @retval          else            Create ring block buffer success.
 ***********************************************************************************************************************
 */
rbb_ctrl_info_t *rbb_create(os_size_t buf_size, os_size_t blk_max_num)
{
    rbb_ctrl_info_t   *rbb;
    os_uint8_t *buf;
    rbb_blk_t  *blk_set;

    rbb = (rbb_ctrl_info_t *)os_malloc(sizeof(rbb_ctrl_info_t));
    if (!rbb)
    {
        return OS_NULL;
    }

    buf = (os_uint8_t *)os_malloc(buf_size);
    if (!buf)
    {
        os_free(rbb);
        return OS_NULL;
    }

    blk_set = (rbb_blk_t *)os_malloc(sizeof(rbb_blk_t) * blk_max_num);
    if (!blk_set)
    {
        os_free(buf);
        os_free(rbb);

        return OS_NULL;
    }

    rbb_init(rbb, buf, buf_size, blk_set, blk_max_num);

    return rbb;
}
EXPORT_SYMBOL(rbb_create);

/**
 ***********************************************************************************************************************
 * @brief           Destroy ring block buffer.
 *
 * @param[in]       rbb             Ring block buffer control info.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void rbb_destroy(rbb_ctrl_info_t *rbb)
{
    OS_ASSERT(rbb);

    os_free(rbb->blk_set);
    os_free(rbb->buf);
    os_free(rbb);

    return;
}
EXPORT_SYMBOL(rbb_destroy);
#endif /* OS_USING_HEAP */

/**
 ***********************************************************************************************************************
 * @brief           Get the size of ring block buffer.
 *
 * @param[in]       rbb             Ring block buffer control info.
 *
 * @return          The buffer size.
 ***********************************************************************************************************************
 */
os_size_t rbb_get_buf_size(rbb_ctrl_info_t *rbb)
{
    OS_ASSERT(rbb);

    return rbb->buf_size;
}
EXPORT_SYMBOL(rbb_get_buf_size);

/**
 ***********************************************************************************************************************
 * @brief           Allocate a block by given size. The block will add to blk_list when allocate success.
 *
 * @attention       When your application need align access, please make the blk_szie is aligned.
 *
 * @param[in]       rbb             Ring block buffer control info.
 * @param[in]       blk_size        Block size
 *
 * @return          Allocated block.
 * @retval          OS_NULL         Allocate failed.
 * @retval          else            Allocate success.
 ***********************************************************************************************************************
 */
rbb_blk_t *rbb_blk_alloc(rbb_ctrl_info_t *rbb, os_size_t blk_size)
{
    os_base_t  level;
    os_size_t  empty1;
    os_size_t  empty2;
    os_size_t  list_len;
    rbb_blk_t *head;
    rbb_blk_t *tail;
    rbb_blk_t *new;

    OS_ASSERT(rbb && blk_size);

    level = os_hw_interrupt_disable();

    new      = rbb_find_empty_blk_in_set(rbb);
    list_len = os_slist_len(&rbb->blk_list);

    if ((list_len < rbb->blk_max_num) && (new != OS_NULL))
    {
        if (list_len > 0)
        {
            head = os_slist_first_entry(&rbb->blk_list, rbb_blk_t, list);
            tail = os_slist_tail_entry(&rbb->blk_list, rbb_blk_t, list);

            if (head->buf <= tail->buf)
            {
                /**
                 *                      head                     tail
                 * +--------------------------------------+-----------------+------------------+
                 * |      empty2     | block1 |   block2  |      block3     |       empty1     |
                 * +--------------------------------------+-----------------+------------------+
                 *                            rbb->buf
                 */
                empty1 = (rbb->buf + rbb->buf_size) - (tail->buf + tail->buf_size);
                empty2 = head->buf - rbb->buf;

                if (empty1 >= blk_size)
                {
                    os_slist_add_tail(&rbb->blk_list, &new->list);
                    new->status   = RBB_BLK_STATUS_INITED;
                    new->buf      = tail->buf + tail->buf_size;
                    new->buf_size = blk_size;
                }
                else if (empty2 >= blk_size)
                {
                    os_slist_add_tail(&rbb->blk_list, &new->list);
                    new->status   = RBB_BLK_STATUS_INITED;
                    new->buf      = rbb->buf;
                    new->buf_size = blk_size;
                }
                else
                {
                    /* No space */
                    new = OS_NULL;
                }
            }
            else
            {
                /**
                 *        tail                                              head
                 * +----------------+-------------------------------------+--------+-----------+
                 * |     block3     |                empty1               | block1 |  block2   |
                 * +----------------+-------------------------------------+--------+-----------+
                 *                            rbb->buf
                 */
                empty1 = head->buf - (tail->buf + tail->buf_size);
                
                if (empty1 >= blk_size)
                {
                    os_slist_add_tail(&rbb->blk_list, &new->list);
                    new->status   = RBB_BLK_STATUS_INITED;
                    new->buf      = tail->buf + tail->buf_size;
                    new->buf_size = blk_size;
                }
                else
                {
                    /* No space */
                    new = OS_NULL;
                }
            }
        }
        else
        {
            /* The list is empty */
            os_slist_add_tail(&rbb->blk_list, &new->list);
            new->status   = RBB_BLK_STATUS_INITED;
            new->buf      = rbb->buf;
            new->buf_size = blk_size;
        }
    }
    else
    {
        new = OS_NULL;
    }

    os_hw_interrupt_enable(level);

    return new;
}
EXPORT_SYMBOL(rbb_blk_alloc);


/**
 ***********************************************************************************************************************
 * @brief           Put a block to ring block buffer.
 *
 * @param[in]       block           The block.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void rbb_blk_put(rbb_blk_t *block)
{
    OS_ASSERT(block);
    OS_ASSERT(block->status == RBB_BLK_STATUS_INITED);

    block->status = RBB_BLK_STATUS_PUT;
    return;
}
EXPORT_SYMBOL(rbb_blk_put);

/**
 ***********************************************************************************************************************
 * @brief           Get a block from the ring block buffer.
 *
 * @param[in]       rbb             The ring block buffer control info.
 *
 * @return          The block.
 * @retval          OS_NULL         Get failed.
 * @retval          else            Get success.
 ***********************************************************************************************************************
 */
rbb_blk_t *rbb_blk_get(rbb_ctrl_info_t *rbb)
{
    os_base_t       level;
    rbb_blk_t       *block;
    os_slist_node_t *node;
    os_bool_t       found;

    OS_ASSERT(rbb);

    if (os_slist_empty(&rbb->blk_list))
    {
        return OS_NULL;
    }
    
    level = os_hw_interrupt_disable();

    found = OS_FALSE;
    block = OS_NULL;
    
    os_slist_for_each(node, &rbb->blk_list) 
    {
        block = os_slist_entry(node, rbb_blk_t, list);
        if (RBB_BLK_STATUS_PUT == block->status)
        {
            block->status = RBB_BLK_STATUS_GET;
            found = OS_TRUE;
            break;
        }
    }

    if (!found)
    {
        block = OS_NULL;
    }

    os_hw_interrupt_enable(level);

    return block;
}
EXPORT_SYMBOL(rbb_blk_get);

/**
 ***********************************************************************************************************************
 * @brief           Return the block size.
 *
 * @param[in]       block           The block.
 *
 * @return          Block size.
 ***********************************************************************************************************************
 */
os_size_t rbb_blk_size(rbb_blk_t *block)
{
    OS_ASSERT(block);

    return block->buf_size;
}
EXPORT_SYMBOL(rbb_blk_size);

/**
 ***********************************************************************************************************************
 * @brief           Return the block buffer.
 *
 * @param[in]       block           The block.
 *
 * @return          Block buffer.
 ***********************************************************************************************************************
 */
os_uint8_t *rbb_blk_buf(rbb_blk_t *block)
{
    OS_ASSERT(block);

    return block->buf;
}
EXPORT_SYMBOL(rbb_blk_buf);

/**
 ***********************************************************************************************************************
 * @brief           Free the block.
 *
 * @param[in]       rbb             The ring block buffer control info.
 * @param[in]       block           The block.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void rbb_blk_free(rbb_ctrl_info_t *rbb, rbb_blk_t *block)
{
    os_base_t level;

    OS_ASSERT(rbb);
    OS_ASSERT(block);
    OS_ASSERT(block->status != RBB_BLK_STATUS_UNUSED);

    level = os_hw_interrupt_disable();

    /* Remove it from rbb block list */
    os_slist_del(&rbb->blk_list, &block->list);
    block->status = RBB_BLK_STATUS_UNUSED;

    os_hw_interrupt_enable(level);

    return;
}
EXPORT_SYMBOL(rbb_blk_free);

/**
 ***********************************************************************************************************************
 * @brief           Get a continuous block to queue by given size.
 *
 * @details         The implementation details are as follows:
 *
 *                          tail                         head
 *                  +------------------+---------------+--------+----------+--------+
 *                  |      block3      |  empty1       | block1 |  block2  |fragment|
 *                  +------------------+------------------------+----------+--------+
 *                                                     |<-- return_size -->|    |
 *                                                     |<--- queue_data_len --->|
 *
 *                          tail                          head
 *                  +------------------+---------------+--------+----------+--------+
 *                  |      block3      |  empty1       | block1 |  block2  |fragment|
 *                  +------------------+------------------------+----------+--------+
 *                                                     |<-- return_size -->|              out of len(b1+b2+b3)    |
 *                                                     |<-------------------- queue_data_len -------------------->|

 * @param[in]       rbb             Ring block buffer control info.
 * @param[in]       queue_data_len  The max queue data size, and the return size must less then it.
 * @param[in]       blk_queue       Continuous block queue
 *
 * @return          The block queue data total size.
 ***********************************************************************************************************************
 */
os_size_t rbb_blk_queue_get(rbb_ctrl_info_t *rbb, os_size_t queue_data_len, rbb_blk_queue_t *blk_queue)
{
    os_base_t        level;
    os_size_t        data_total_size;
    os_slist_node_t *node;
    rbb_blk_t       *last_block;
    rbb_blk_t       *block;

    OS_ASSERT(rbb);
    OS_ASSERT(blk_queue);

    if (os_slist_empty(&rbb->blk_list))
    {
        return 0;
    }
    
    level = os_hw_interrupt_disable();

    data_total_size = 0;
    last_block      = OS_NULL;

    os_slist_for_each(node, &rbb->blk_list) 
    {
        if (!last_block)
        {
            last_block = os_slist_entry(node, rbb_blk_t, list);
            if (RBB_BLK_STATUS_PUT == last_block->status)
            {
                /* Save the first put status block to queue */
                blk_queue->blocks  = last_block;
                blk_queue->blk_num = 0;
            }
            else
            {
                /* The first block must be put status */
                last_block = OS_NULL;
                continue;
            }
        }
        else
        {
            block = os_slist_entry(node, rbb_blk_t, list);

            /*
             * These following conditions will break the loop:
             * 1. The current block is not put status
             * 2. The last block and current block is not continuous
             * 3. The data_total_size will out of range
             */
            if ((block->status != RBB_BLK_STATUS_PUT)
                || (last_block->buf > block->buf)
                || (data_total_size + block->buf_size > queue_data_len))
            {
                break;
            }
            
            /* Backup last block */
            last_block = block;
        }
        
        /* Remove current block */
        os_slist_del(&rbb->blk_list, &last_block->list);
        data_total_size += last_block->buf_size;
        last_block->status = RBB_BLK_STATUS_GET;
        blk_queue->blk_num++;
    }

    os_hw_interrupt_enable(level);

    return data_total_size;
}
EXPORT_SYMBOL(rbb_blk_queue_get);

/**
 ***********************************************************************************************************************
 * @brief           Get all block length on block queue.
 *
 * @param[in]       blk_queue       The block queue.
 *
 * @return          Total length.
 ***********************************************************************************************************************
 */
os_size_t rbb_blk_queue_len(rbb_blk_queue_t *blk_queue)
{
    os_size_t i;
    os_size_t data_total_size;

    OS_ASSERT(blk_queue);

    data_total_size = 0;
    for (i = 0; i < blk_queue->blk_num; i++)
    {
        data_total_size += blk_queue->blocks[i].buf_size;
    }

    return data_total_size;
}
EXPORT_SYMBOL(rbb_blk_queue_len);

/**
 ***********************************************************************************************************************
 * @brief           Return the block queue buffer.
 *
 * @param[in]       blk_queue       The block queue.
 *
 * @return          Block queue buffer.
 ***********************************************************************************************************************
 */
os_uint8_t *rbb_blk_queue_buf(rbb_blk_queue_t *blk_queue)
{
    OS_ASSERT(blk_queue);
    
    return blk_queue->blocks[0].buf;
}
EXPORT_SYMBOL(rbb_blk_queue_buf);

/**
 ***********************************************************************************************************************
 * @brief           Free the block queue.
 *
 * @param[in]       rbb             Ring block buffer control info.
 * @param[in]       blk_queue       The block queue.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void rbb_blk_queue_free(rbb_ctrl_info_t *rbb, rbb_blk_queue_t *blk_queue)
{
    os_size_t i;

    OS_ASSERT(rbb);
    OS_ASSERT(blk_queue);

    for (i = 0; i < blk_queue->blk_num; i++)
    {
        rbb_blk_free(rbb, &blk_queue->blocks[i]);
    }

    return;
}
EXPORT_SYMBOL(rbb_blk_queue_free);

/**
 ***********************************************************************************************************************
 * @brief           The put status and buffer continuous blocks can be make a block queue.
 *                  This function will return the length which from next can be make block queue.
 *
 * @param[in]       rbb             Ring block buffer control info.
 *
 * @return          The next can be make block queue's length.
 ***********************************************************************************************************************
 */
os_size_t rbb_next_blk_queue_len(rbb_ctrl_info_t *rbb)
{
    os_base_t        level;
    os_size_t        data_len;
    os_slist_node_t *node;
    rbb_blk_t       *last_block;
    rbb_blk_t       *block;

    OS_ASSERT(rbb);

    if (os_slist_empty(&rbb->blk_list))
    {
        return 0;
    }
    
    level = os_hw_interrupt_disable();

    data_len   = 0;
    last_block = OS_NULL;
    
    os_slist_for_each(node, &rbb->blk_list)
    {
        if (!last_block)
        {
            last_block = os_slist_entry(node, rbb_blk_t, list);
            if (last_block->status != RBB_BLK_STATUS_PUT)
            {
                /* The first block must be put status */
                last_block = OS_NULL;
                continue;
            }
        }
        else
        {
            block = os_slist_entry(node, rbb_blk_t, list);
            /*
             * These following conditions will break the loop:
             * 1. the current block is not put status
             * 2. the last block and current block is not continuous
             */
            if ((block->status != RBB_BLK_STATUS_PUT) || (last_block->buf > block->buf))
            {
                break;
            }
            
            /* Backup last block */
            last_block = block;
        }
        
        data_len += last_block->buf_size;
    }

    os_hw_interrupt_enable(level);

    return data_len;
}
EXPORT_SYMBOL(rbb_next_blk_queue_len);

