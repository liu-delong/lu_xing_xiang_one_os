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
 * @file        at_resp.c
 *
 * @brief       Implement AT response object functions
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "at_resp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DBG_EXT_TAG "at.resp"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

/**
 ***********************************************************************************************************************
 * @brief           Initializes an instance of AT parser response
 *
 * @param[in]       resp            A pointer to AT Parser response instance
 * @param[in]       name            The name of AT Parser response instance
 * @param[in]       buff_size       The maximum response buffer size
 * @param[in]       line_num        The number of setting response lines
 *                                  = 0: the response data will auto return when received 'OK' or 'ERROR'
 *                                  != 0: the response data will return when received setting lines number data
 * @param[in]       timeout         The maximum response time
 *
 * @return          Returns the result of an initialization operation
 * @retval          OS_EOK          Successfully
 * @retval          OS_ENOMEM       Apply for buffer memory failure
 ***********************************************************************************************************************
 */
os_err_t at_resp_init(at_resp_t *resp, const char *name, os_size_t buff_size, os_size_t line_num, os_int32_t timeout)
{
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(name != OS_NULL);
    OS_ASSERT(buff_size > 0);

    resp->buff = (char *)calloc(1, buff_size);
    if (resp->buff == OS_NULL)
    {
        LOG_EXT_E("AT create response object failed! No memory for response buffer!");
        return OS_ENOMEM;
    }

    os_sem_init(&(resp->resp_notice), name, 0, OS_IPC_FLAG_FIFO);

    resp->max_buff_size = buff_size;
    resp->line_num      = line_num;
    resp->timeout       = timeout;

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy an instance of AT parser response
 *
 * @param[in]       resp            An instance of AT Parser response to be destroyed
 ***********************************************************************************************************************
 */
os_err_t at_resp_deinit(at_resp_t *resp)
{
    OS_ASSERT(resp != OS_NULL);

    os_sem_deinit(&(resp->resp_notice));

    if (resp->buff != OS_NULL)
    {
        free(resp->buff);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Set an AT parser response instance information
 *
 * @param[in]       resp            A pointer to AT Parser response instance
 * @param[in]       buff_size       The maximum response buffer size
 * @param[in]       line_num        The number of setting response lines
 *                                  = 0: the response data will auto return when received 'OK' or 'ERROR'
 *                                  != 0: the response data will return when received setting lines number data
 * @param[in]       timeout         The maximum response time
 *
 * @return          Return operation result
 * @retval          OS_EOK          Set successfully
 * @retval          OS_ENOMEM       Set failed
 ***********************************************************************************************************************
 */
os_err_t at_resp_set(at_resp_t *resp, os_size_t buff_size, os_size_t line_num, os_int32_t timeout)
{
    OS_ASSERT(resp != OS_NULL);

    if (resp->max_buff_size != buff_size)
    {
        resp->max_buff_size = buff_size;

        char *temp = resp->buff;

        resp->buff = (char *)realloc(resp->buff, buff_size);
        if (OS_NULL == resp->buff)
        {
            free(temp);
            resp->max_buff_size = 0;
            LOG_EXT_E("No memory for realloc response buffer size(%d).", buff_size);
            return OS_ENOMEM;
        }
    }

    resp->line_num = line_num;
    resp->timeout  = timeout;

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Set an AT parser response instance default information
 *
 * @param[in]       resp            A pointer to AT Parser response instance
 *
 * @return          Return operation result, @see at_resp_set
 ***********************************************************************************************************************
 */
os_err_t at_resp_reset(at_resp_t *resp)
{
    return at_resp_set(resp, AT_RESP_BUFF_SIZE_DEF, AT_RESP_LINE_NUM_DEF, os_tick_from_ms(AT_RESP_TIMEOUT_DEF));
}

/**
 ***********************************************************************************************************************
 * @brief           Get one line form AT parser response buffer by line number
 *
 * @param[in]       resp            A pointer to AT Parser response instance
 * @param[in]       resp_line       The line number of response data, , start from '1'
 *
 * @return          Return string of one line data
 * @retval          != OS_NULL      Operate successfully
 * @retval          == OS_NULL      Operate failed
 ***********************************************************************************************************************
 */
const char *at_resp_get_line(at_resp_t *resp, os_size_t resp_line)
{
    OS_ASSERT(resp);

    char *resp_buf = resp->buff;

    if (resp_line > resp->line_counts || resp_line < 1)
    {
        LOG_EXT_E("AT response get line failed! Input response line(%d) error!", resp_line);
        return OS_NULL;
    }

    for (int line_num = 1; line_num <= resp->line_counts; line_num++)
    {
        if (resp_line == line_num)
        {
            char *resp_line_buf = resp_buf;

            return resp_line_buf;
        }

        resp_buf += strlen(resp_buf) + 1;
    }

    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           Get one line form AT parser response buffer by keyword
 *
 * @param[in]       resp            A pointer to AT Parser response instance
 * @param[in]       keyword         The keyword of response data
 *
 * @return          Return string of one line data
 * @retval          != OS_NULL      Operate successfully
 * @retval          == OS_NULL      Operate failed
 ***********************************************************************************************************************
 */
const char *at_resp_get_line_by_kw(at_resp_t *resp, const char *keyword)
{
    OS_ASSERT(resp);
    OS_ASSERT(keyword);

    char *resp_buf = resp->buff;

    for (int line_num = 1; line_num <= resp->line_counts; line_num++)
    {
        if (strstr(resp_buf, keyword))
        {
            char *resp_line_buf = resp_buf;

            return resp_line_buf;
        }

        resp_buf += strlen(resp_buf) + 1;
    }

    return OS_NULL;
}
