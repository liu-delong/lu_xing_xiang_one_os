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
 * @file        rtc_test.c
 *
 * @brief       The test file for rtc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_device.h>
#include <os_errno.h>
#include <sys/time.h>
#include <rtc/rtc.h>
#include <drv_log.h>

#ifdef OS_USING_SHELL
#include <shell.h>
#endif

static int rtc_set_test(int argc, char *argv[])
{
    os_err_t ret = OS_EOK;
    time_t   now;

    /* Set date */
    ret = set_date(2020, 6, 1);
    if (ret != OS_EOK)
    {
        LOG_EXT_E("set RTC date failed %d", (int)ret);
        return ret;
    }
    os_task_mdelay(200);

    /* Set time */
    ret = set_time(9, 30, 0);
    if (ret != OS_EOK)
    {
        LOG_EXT_E("set RTC time failed %d", (int)ret);
        return ret;
    }

    os_task_mdelay(3000);

    /* Get time */
    now = rtc_get();
    os_kprintf("%s\n", ctime(&now));
    return ret;
}

SH_CMD_EXPORT(rtc_set_test, rtc_set_test, "set rtc time");

static void rtc_get_test(int argc, char *argv[])
{
    time_t now;

    /* Get time */
    now = rtc_get();

    os_kprintf("%s\n", ctime(&now));
}
SH_CMD_EXPORT(rtc_get_test, rtc_get_test, "get rtc time");
