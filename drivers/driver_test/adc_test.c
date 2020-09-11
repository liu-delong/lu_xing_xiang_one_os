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
 * @file        adc_test.c
 *
 * @brief       The test file for adc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <shell.h>

int adc_sample(int argc, char **argv)
{
    os_uint8_t  channel;
    os_int32_t  adc_databuf;
    os_err_t    ret = OS_EOK;

    os_device_t *adc_dev;
    
    if (argc != 3)
    {
        os_kprintf("usage: adc_sample read <channel>\r\n");
        os_kprintf("       adc_sample read 0 \r\n");
        return -1;
    }
    
    channel  = strtol(argv[2], OS_NULL, 0);

    /* find device */
    adc_dev = os_device_find("adc");
    if (adc_dev == OS_NULL)
    {
        os_kprintf("adc device not find! \r\n");
        return -1;
    }
    
    /* open adc */
    ret = os_device_open(adc_dev, OS_DEVICE_OFLAG_RDONLY);
    if (ret != OS_EOK)
    {
        os_kprintf("adc device cannot open! \r\n");
        return -1;
    }

    ret = os_device_read(adc_dev, channel, &adc_databuf, sizeof(adc_databuf));
    if (ret == sizeof(adc_databuf))
    {
        os_kprintf("adc channle: %d voltage value: %d\n",channel, adc_databuf);
    }
    else
    {
        os_kprintf("adc read failed %d.\r\n", ret);
    }

    os_device_close(adc_dev);
    return ret;
}

SH_CMD_EXPORT(adc_sample, adc_sample, "test set adc");
