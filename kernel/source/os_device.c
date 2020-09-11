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
 * @file        os_device.c
 *
 * @brief       This file implements the device functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-27   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_device.h>
#include <os_memory.h>
#include <os_errno.h>
#include <os_module.h>
#include <os_hw.h>
#include <os_irq.h>
#include <os_dbg.h>
#include <os_object.h>
#include <os_task.h>
#include <os_util.h>
#include "os_util_internal.h"
#include "os_kernel_internal.h"

#ifdef OS_USING_DEVICE

#define DEV_TAG         "DEV"

#ifdef OS_USING_DEVICE_OPS
#define device_init     (dev->ops->init)
#define device_open     (dev->ops->open)
#define device_close    (dev->ops->close)
#define device_read     (dev->ops->read)
#define device_write    (dev->ops->write)
#define device_control  (dev->ops->control)
#else
#define device_init     (dev->init)
#define device_open     (dev->open)
#define device_close    (dev->close)
#define device_read     (dev->read)
#define device_write    (dev->write)
#define device_control  (dev->control)
#endif

/**
 ***********************************************************************************************************************
 * @brief           This function registers a device and places it on the list of device object.
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       name            Pointer to device name string.
 * @param[in]       flag            Flags of device.
 *
 * @return          Regist result.
 * @retval          OS_EOK          Successful.
 * @retval          OS_EINVAL       Fail.
 ***********************************************************************************************************************
 */
os_err_t os_device_register(os_device_t *dev, const char *name, os_uint16_t flags)
{
    if (OS_NULL == dev)
    {
        return OS_EINVAL;
    }

    /* Check for name null or duplicate names */
    if ((OS_NULL == name) || (OS_NULL != os_device_find(name)))
    {
        return OS_EINVAL;
    }

    os_object_init(&dev->parent, OS_OBJECT_DEVICE, name, OS_TRUE);

    dev->flag      = flags;
    dev->ref_count = 0;
    dev->open_flag = 0;

#ifdef OS_USING_POSIX
    dev->fops = OS_NULL;
    os_waitqueue_init(&dev->wait_queue);
#endif

    return OS_EOK;
}
EXPORT_SYMBOL(os_device_register);

/**
 ***********************************************************************************************************************
 * @brief           This function will remove a previously registered device.
 *
 * @param[in]       dev             The descriptor of device control block.
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
os_err_t os_device_unregister(os_device_t *dev)
{
    OS_ASSERT(OS_NULL != dev);
    OS_ASSERT(OS_OBJECT_DEVICE == os_object_get_type(&dev->parent));
    OS_ASSERT(OS_TRUE == os_object_is_static(&dev->parent));

    os_object_deinit(&dev->parent);

    return OS_EOK;
}
EXPORT_SYMBOL(os_device_unregister);

/**
 ***********************************************************************************************************************
 * @brief           Find device by name on the device object list .
 *
 * @details         This function will find device by name on the device object list.
 *
 * @param[in]       name            Pointer to device name string.
 * 
 * @return          On success, return a device control block descriptor; on error, OS_NULL is returned.
 * @retval          not OS_NULL     Return a task control block descriptor.
 * @retval          OS_NULL         No task to be found.
 ***********************************************************************************************************************
 */
os_device_t *os_device_find(const char *name)
{
    os_object_t      *object;
    os_list_node_t   *node;
    os_object_info_t *info;

    os_enter_critical();

    /* Try to find device object. */
    info = os_object_get_info(OS_OBJECT_DEVICE);
    OS_ASSERT(OS_NULL != info);
    
    for (node = info->object_list.next; node != &info->object_list; node = node->next)
    {
        object = os_list_entry(node, os_object_t, list);
        if (0 == os_strncmp(object->name, name, OS_NAME_MAX))
        {
            os_exit_critical();

            return (os_device_t *)object;
        }
    }

    os_exit_critical();

    return OS_NULL;
}
EXPORT_SYMBOL(os_device_find);

/**
 ***********************************************************************************************************************
 * @brief           This function will initialize the device through the initialization function installed on the device.
 *
 * @param[in]       dev             The descriptor of device control block.
 *
 * @return          Initialization result.
 ***********************************************************************************************************************
 */
os_err_t os_device_init(os_device_t *dev)
{
    os_err_t result;

    OS_ASSERT(OS_NULL != dev);

    result = OS_EOK;

    if (OS_NULL != device_init)
    {
        if (!(dev->flag & OS_DEVICE_FLAG_ACTIVATED))
        {
            result = device_init(dev);
            if (OS_EOK != result)
            {
                OS_KERN_LOG(KERN_ERROR, DEV_TAG, "To initialize device:%s failed. The error code is %d",
                            dev->parent.name, 
                            result);
            }
            else
            {
                dev->flag |= OS_DEVICE_FLAG_ACTIVATED;
            }
        }
    }

    return result;
}
EXPORT_SYMBOL(os_device_init);

/**
 ***********************************************************************************************************************
 * @brief           This function will open the device through the open function installed on the device.
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       oflag           The flags for device open.
 *
 * @return          Open result.
 ***********************************************************************************************************************
 */
os_err_t os_device_open(os_device_t *dev, os_uint16_t oflag)
{
    os_err_t result;

    OS_ASSERT(OS_NULL != dev);
    OS_ASSERT(OS_OBJECT_DEVICE == os_object_get_type(&dev->parent));

    result = OS_EOK;

    /* If device is not initialized, initialize it. */
    if (!(dev->flag & OS_DEVICE_FLAG_ACTIVATED))
    {
        if (OS_NULL != device_init)
        {
            result = device_init(dev);
            if (OS_EOK != result)
            {
                OS_KERN_LOG(KERN_ERROR, DEV_TAG, "To initialize device:%s failed. The error code is %d",
                            dev->parent.name, 
                            result);

                return result;
            }
        }

        dev->flag |= OS_DEVICE_FLAG_ACTIVATED;
    }

    /* Device is a stand alone device and opened. */
    if ((dev->flag & OS_DEVICE_FLAG_STANDALONE) && (dev->open_flag & OS_DEVICE_OFLAG_OPEN))
    {
        return OS_EBUSY;
    }

    /* Call device open interface. */
    if (OS_NULL != device_open)
    {
        result = device_open(dev, oflag);
    }
    else
    {
        /* Set open flag. */
        dev->open_flag = (oflag & OS_DEVICE_OFLAG_MASK);
    }

    /* Set open flag. */
    if ((OS_EOK == result) || (OS_ENOSYS == result))
    {
        dev->open_flag |= OS_DEVICE_OFLAG_OPEN;

        dev->ref_count++;
        
        /* Don't let bad things happen silently. If you are bitten by this assert,
         *please set the ref_count to a bigger type.
         */
        OS_ASSERT(0 != dev->ref_count);
    }

    return result;
}
EXPORT_SYMBOL(os_device_open);

/**
 ***********************************************************************************************************************
 * @brief           This function will close the device through the close function installed on the device.
 *
 * @param[in]       dev             The descriptor of device control block.
 *
 * @return          Close result.
 ***********************************************************************************************************************
 */
os_err_t os_device_close(os_device_t *dev)
{
    os_err_t result;

    OS_ASSERT(OS_NULL != dev);
    OS_ASSERT(OS_OBJECT_DEVICE == os_object_get_type(&dev->parent));

    result = OS_EOK;

    if (0 == dev->ref_count)
    {
        return OS_ERROR;
    }

    dev->ref_count--;

    if (0 != dev->ref_count)
    {
        return OS_EOK;
    }

    /* Call device close interface. */
    if (OS_NULL != device_close)
    {
        result = device_close(dev);
    }

    /* Set open flag. */
    if ((OS_EOK == result) || (OS_ENOSYS == result))
    {
        dev->open_flag = OS_DEVICE_OFLAG_CLOSE;
    }

    return result;
}
EXPORT_SYMBOL(os_device_close);

/**
 ***********************************************************************************************************************
 * @brief           This function will read some data from a device. 
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       pos             The position of reading.
 * @param[out]      buffer          The data buffer to save read data.
 * @param[in]       size            The size of buffer.
 * 
 * @return          The actually read size on successful, otherwise negative returned.
 ***********************************************************************************************************************
 */
os_size_t os_device_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    OS_ASSERT(OS_NULL != dev);
    OS_ASSERT(OS_OBJECT_DEVICE == os_object_get_type(&dev->parent));

    if (0 == dev->ref_count)
    {
        os_set_errno(OS_ERROR);
        return 0;
    }

    /* Call device read interface. */
    if (OS_NULL != device_read)
    {
        return device_read(dev, pos, buffer, size);
    }

    /* Set error code. */
    os_set_errno(OS_ENOSYS);

    return 0;
}
EXPORT_SYMBOL(os_device_read);

/**
 ***********************************************************************************************************************
 * @brief           This function will write some data to a device. 
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       pos             The position of written.
 * @param[out]      buffer          The data buffer to be written to device.
 * @param[in]       size            The size of buffer.
 * 
 * @return          The actually written size on successful, otherwise negative returned.
 ***********************************************************************************************************************
 */
os_size_t os_device_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    OS_ASSERT(OS_NULL != dev);
    OS_ASSERT(OS_OBJECT_DEVICE == os_object_get_type(&dev->parent));

    if (0 == dev->ref_count)
    {
        os_set_errno(OS_ERROR);
        return 0;
    }

    /* Call device write interface */
    if (OS_NULL != device_write)
    {
        return device_write(dev, pos, buffer, size);
    }

    /* Set error code */
    os_set_errno(OS_ENOSYS);

    return 0;
}
EXPORT_SYMBOL(os_device_write);

/**
 ***********************************************************************************************************************
 * @brief           Control device
 *
 * @details         This function control or change the properties of the device.
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       cmd             The command sent to device.
 * @param[in]       arg             Control argments.
 *
 * @return          Control result.
 ***********************************************************************************************************************
 */
os_err_t os_device_control(os_device_t *dev, int cmd, void *arg)
{
    OS_ASSERT(OS_NULL != dev);
    OS_ASSERT(OS_OBJECT_DEVICE == os_object_get_type(&dev->parent));

    /* Call device write interface. */
    if (OS_NULL != device_control)
    {
        return device_control(dev, cmd, arg);
    }

    return OS_ENOSYS;
}
EXPORT_SYMBOL(os_device_control);

/**
 ***********************************************************************************************************************
 * @brief           This function will set the reception indication callback function. This callback function is invoked 
 *                  when this device receives data.
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       rx_ind          The indication callback function.
 *
 * @return          Will only return OS_EOK.
 ***********************************************************************************************************************
 */
os_err_t os_device_set_rx_indicate(os_device_t *dev, os_err_t (*rx_ind)(os_device_t *dev, os_size_t size))
{
    OS_ASSERT(OS_NULL != dev);
    OS_ASSERT(OS_OBJECT_DEVICE == os_object_get_type(&dev->parent));

    dev->rx_indicate = rx_ind;

    return OS_EOK;
}
EXPORT_SYMBOL(os_device_set_rx_indicate);

/**
 ***********************************************************************************************************************
 * @brief           This function will set the indication callback function when device has written data to physical hardware.
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       tx_done         The indication callback function.
 *
 * @return          Will only return OS_EOK.
 ***********************************************************************************************************************
 */
os_err_t os_device_set_tx_complete(os_device_t *dev, os_err_t (*tx_done)(os_device_t *dev, void *buffer))
{
    OS_ASSERT(OS_NULL != dev);
    OS_ASSERT(OS_OBJECT_DEVICE == os_object_get_type(&dev->parent));

    dev->tx_complete = tx_done;

    return OS_EOK;
}
EXPORT_SYMBOL(os_device_set_tx_complete);


#ifdef OS_USING_SHELL

#include <shell.h>

static char *const gs_device_type_str[] =
{
    "Character Device",
    "Block Device",
    "Network Interface",
    "MTD Device",
    "CAN Device",
    "RTC",
    "Sound Device",
    "Graphic Device",
    "I2C Bus",
    "USB Slave Device",
    "USB Host Bus",
    "SPI Bus",
    "SPI Device",
    "SDIO Bus",
    "PM Pseudo Device",
    "Pipe",
    "Portal Device",
    "ClockSource Device",
    "ClockEvent Device",
    "Miscellaneous Device",
    "Sensor Device",
    "Touch Device",
    "Infrared Device",
    "Unknown"
};

static os_err_t sh_list_device_info(os_list_node_t *list)
{
    os_int32_t      maxlen;
    os_device_t    *device;
    os_list_node_t *node;
    const char     *item_title = "device";

    maxlen = os_object_name_maxlen(item_title, list);

    os_kprintf("%-*.s         type         ref count\r\n", maxlen, item_title);
    os_object_split(maxlen);
    os_kprintf(     " -------------------- ----------\r\n");
    
    for (node = list->next; node != list; node = node->next)
    {
        device = (os_device_t *)(os_list_entry(node, os_object_t, list));
        os_kprintf("%-*.*s %-20s %-8d\r\n",
                   maxlen, 
                   OS_NAME_MAX,
                   device->parent.name,
                   (device->type <= OS_DEVICE_TYPE_UNKNOWN) ? gs_device_type_str[device->type] : gs_device_type_str[OS_DEVICE_TYPE_UNKNOWN],
                   device->ref_count);
    }

    return OS_EOK;
}

/**
***********************************************************************************************************************
* @brief           Show all device on the list of device object
*
* @param[in]       argc                argment count
* @param[in]       argv                argment list
*
* @return          Will only return OS_EOK     
***********************************************************************************************************************
*/
os_err_t sh_list_device(os_int32_t argc, char **argv)
{
    os_object_info_t *info;

    info = os_object_get_info(OS_OBJECT_DEVICE);
    return sh_list_device_info(&info->object_list);
}

SH_CMD_EXPORT(list_device, sh_list_device, "list device");
#endif 

#endif

