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
 * @file        one_pos_memory_manager.h
 * 
 * @brief       Cache controller
 * 
 * @details     Cache management for location services
 * 
 * @revision
 * Date         Author          Notes
 * 2020-07-16   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __ONE_POS_MEMORY_MANAGER_H__
#define __ONE_POS_MEMORY_MANAGER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "one_pos_types.h"
#include "one_pos_sem.h"

#if defined __OPS_WINDOWS__
#include <malloc.h>
#elif defined __OPS_ONE_OS__
#include <os_memory.h>
#else
#error Undefined platform
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define OPS_BLOCK_NUM_MAX      5                 /* Maximum number of memory blocks(Cannot be zero) */
#define OPS_MEM_DATA_NUM_MAX   5                 /* The maximum number of blocks of data allowed to be added 
                                                  * at one time in a single block in a memory manager */

/**
 ***********************************************************************************************************************
 * @struct      ops_queue_node_t
 *      
 * @brief       Queue node
 ***********************************************************************************************************************
 */
typedef struct ops_queue_node_t
{
    ops_void_t *buffer;                          /* Data for the current node */
    struct ops_queue_node_t *prev;               /* Previous node */
    struct ops_queue_node_t *next;               /* Next node */
}ops_queue_node_t;

/**
 ***********************************************************************************************************************
 * @struct      link_queue_t
 *      
 * @brief       The queue
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_int_t size;                              /* Size of the queue */
    ops_queue_node_t *head;                      /* The head of the queue */
    ops_queue_node_t *tail;                      /* The tail of the queue */
}ops_link_queue_t;

/**
 ***********************************************************************************************************************
 * @struct      queue_node_grp_t
 *      
 * @brief       A group of nodes
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_int_t num;                               /* The number of the node */
    ops_queue_node_t *node[OPS_BLOCK_NUM_MAX];   /* Addresses of all nodes */
}ops_queue_node_grp_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_mem_manager_t
 *      
 * @brief       Memory controller
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_uint_t        block_num_max;            /* The maximum number of memory blocks */
    ops_sem_t        *sem;                      /* A semaphore controlled by reading and writing */
    ops_link_queue_t *queue;                    /* Memory queue */
}ops_mem_manager_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_mem_blocks_t
 *      
 * @brief       A group of blocks
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_int_t   num;                            /* The number of the block */
    ops_void_t *block[OPS_BLOCK_NUM_MAX];       /* Addresses of all blocks */
}ops_mem_block_grp_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_data_info
 *      
 * @brief       Information about the data
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_ushort_t num;                            /* The amount of space taken up by data (in bytes) */
    ops_void_t  *addr;                           /* The address of the data */
}ops_data_info_t;

/**
 ***********************************************************************************************************************
 * @struct      ops_mem_multiple_data
 *      
 * @brief       A group of data
 ***********************************************************************************************************************
 */
typedef struct
{
    ops_ushort_t    num;                          /* Number of data */
    ops_data_info_t data[OPS_MEM_DATA_NUM_MAX];   /* Each piece of data */
}ops_mem_data_grp_t;

ops_mem_manager_t* ops_mem_create(ops_uint_t block_num_max);

ops_err_t ops_mem_destroy(ops_mem_manager_t *manager);

ops_err_t ops_mem_blcok_add(ops_mem_manager_t *manager, ops_void_t *data, ops_uint_t data_num);

ops_err_t ops_mem_get_all_blocks(ops_mem_manager_t *manager, ops_mem_block_grp_t *block_grp);

ops_err_t ops_mem_clear(ops_mem_manager_t *manager);

ops_uint_t ops_mem_get_length(ops_mem_manager_t *manager);

ops_err_t ops_mem_multiple_data_add(ops_mem_manager_t  *manager,
                                    ops_mem_data_grp_t *data_grp);

#ifdef __cplusplus
}
#endif

#endif /* __ONE_POS_MEMORY_MANAGER_H__ */
