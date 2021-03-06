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
 * @file        drv_spi.c
 *
 * @brief       This file implements spi driver for nxp.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <drv_log.h>

#include <board.h>
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_memory.h>
#include <string.h>
#include <drv_i2c.h>

struct nxp_i2c
{
    struct os_i2c_bus_device i2c;
    struct nxp_i2c_info *i2c_info;
    i2c_master_transfer_t transXfer;
    os_uint32_t clk_src;
    struct os_i2c_msg *msg;
    os_uint16_t msg_pos;


    IRQn_Type irqn;
    i2c_master_handle_t *i2c_handle;
    i2c_master_dma_handle_t *i2c_DmaHandle;

    os_list_node_t list;
    os_sem_t sem;
};

static os_list_node_t nxp_i2c_list = OS_LIST_INIT(nxp_i2c_list);

void nxp_i2c_irq_callback(struct nxp_i2c *nxp_i2c)
{

}

I2C_IRQHandler_DEFINE(0);
I2C_IRQHandler_DEFINE(1);
I2C_IRQHandler_DEFINE(2);
I2C_IRQHandler_DEFINE(3);
I2C_IRQHandler_DEFINE(4);
I2C_IRQHandler_DEFINE(5);
I2C_IRQHandler_DEFINE(6);
I2C_IRQHandler_DEFINE(7);
I2C_IRQHandler_DEFINE(8);

void nxp_i2c_dma_callback(I2C_Type *base, i2c_master_dma_handle_t *handle, status_t status, void *userData)
{
    struct nxp_i2c *nxp_i2c = (struct nxp_i2c *)userData;
    
    if (kStatus_Success == status)
    {
        os_interrupt_enter();
        os_sem_post(&nxp_i2c->sem);
        os_interrupt_leave();
    }
}

void nxp_i2c_transfer_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
    struct nxp_i2c *nxp_i2c = (struct nxp_i2c *)userData;
    
    if (kStatus_Success == status)
    {
        os_interrupt_enter();
        os_sem_post(&nxp_i2c->sem);
        os_interrupt_leave();
    }
}

static os_size_t nxp_i2c_master_xfer(struct os_i2c_bus_device *bus, struct os_i2c_msg msgs[], os_uint32_t num)
{
    int i = 0;
    status_t status;
    os_uint8_t transfer_direction;
    os_uint32_t transfer_flag;
    
    struct nxp_i2c *nxp_i2c;
    
    OS_ASSERT(bus != OS_NULL);
    
    nxp_i2c = (struct nxp_i2c *) bus;

    if (num == 1)
    {
        if (msgs[i].flags & OS_I2C_RD)
        {
            status = I2C_MasterStart(nxp_i2c->i2c_info->i2c_base, msgs[i].addr, kI2C_Read);
            if (status != kStatus_Success)
            {
                LOG_EXT_E("i2c start failed!\n");
                return 0;
            }
            status =I2C_MasterReadBlocking(nxp_i2c->i2c_info->i2c_base, msgs[i].buf, msgs[i].len, kI2C_TransferDefaultFlag);
            if (status != kStatus_Success)
            {
                LOG_EXT_E("i2c read failed!\n");
                return 0;
            }
        }
        else
        {
            status = I2C_MasterStart(nxp_i2c->i2c_info->i2c_base, msgs[i].addr, kI2C_Write);
            if (status != kStatus_Success)
            {
                LOG_EXT_E("i2c start failed!\n");
                return 0;
            }
            status = I2C_MasterWriteBlocking(nxp_i2c->i2c_info->i2c_base, msgs[i].buf, msgs[i].len, kI2C_TransferDefaultFlag);
            if (status != kStatus_Success)
            {
                LOG_EXT_E("i2c write failed!\n");
                return 0;
            }
        }
        status = I2C_MasterStop(nxp_i2c->i2c_info->i2c_base);
        if (status != kStatus_Success)
        {
            LOG_EXT_E("i2c stop failed!\n");
            return 0;
        }
        return num;
    }
    
    for (i = 0; i < num; i += 2)
    {
        if ((i % 2) == 0)
        {
            if (msgs[i+1].flags & OS_I2C_RD)
                transfer_direction = kI2C_Read;
            else
                transfer_direction = kI2C_Write;
            
            nxp_i2c->transXfer.slaveAddress = msgs[i].addr;
            nxp_i2c->transXfer.direction = transfer_direction;
            nxp_i2c->transXfer.subaddress = *msgs[i].buf;
            nxp_i2c->transXfer.subaddressSize = msgs[i].len;
            nxp_i2c->transXfer.data = msgs[i+1].buf;
            nxp_i2c->transXfer.dataSize = msgs[i+1].len;
            if (nxp_i2c->i2c_DmaHandle != OS_NULL)
            {
                status = I2C_MasterTransferDMA(nxp_i2c->i2c_info->i2c_base, nxp_i2c->i2c_DmaHandle, &nxp_i2c->transXfer);
                if (status == kStatus_Success)
                {
                    os_sem_wait(&nxp_i2c->sem, OS_IPC_WAITING_FOREVER);
                }
                else
                {
                    LOG_EXT_E("i2c transmit dma failed!\n");
                    return 0;
                }

            }
            else if (nxp_i2c->i2c_handle != OS_NULL)
            {
                status = I2C_MasterTransferNonBlocking(nxp_i2c->i2c_info->i2c_base, nxp_i2c->i2c_handle, &nxp_i2c->transXfer);
                if (status == kStatus_Success)
                {
                    os_sem_wait(&nxp_i2c->sem, OS_IPC_WAITING_FOREVER);
                }
                else
                {
                    LOG_EXT_E("i2c transmit handle failed!\n");
                    return 0;
                }
            }
            else if (nxp_i2c->irqn == 17)
            {
#if 0
                I2C_EnableInterrupts(nxp_i2c->i2c_info->i2c_base, kUSART_TxLevelInterruptEnable | kUSART_TxErrorInterruptEnable);
                I2C_EnableInterrupts(nxp_i2c->i2c_info->i2c_base, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
                status = I2C_MasterStart(nxp_i2c->i2c_info->i2c_base, msgs[i].addr, transfer_direction);
                if (status == kStatus_Success)
                {
                    if (transfer_direction == kI2C_Read)
                        status = I2C_MasterWriteBlocking(nxp_i2c->i2c_info->i2c_base, msgs[i].buf, msgs[i].len, transfer_flag);
                    else
                        status = I2C_MasterReadBlocking(nxp_i2c->i2c_info->i2c_base, msgs[i].buf, msgs[i].len, transfer_flag);
                    
                    if (status == kStatus_Success)
                        os_sem_wait(&nxp_i2c->sem, OS_IPC_WAITING_FOREVER);
                }
#endif
            }
            else
            {
                if (transfer_direction == kI2C_Read)
                {
                    status = I2C_MasterStart(nxp_i2c->i2c_info->i2c_base, nxp_i2c->transXfer.slaveAddress, kI2C_Write);
                    if (status != kStatus_Success)
                    {
                        LOG_EXT_E("i2c start failed!\n");
                        return 0;
                    }
                    status = I2C_MasterWriteBlocking(nxp_i2c->i2c_info->i2c_base, &nxp_i2c->transXfer.subaddress, nxp_i2c->transXfer.subaddressSize, kI2C_TransferNoStopFlag);
                    if (status != kStatus_Success)
                    {
                        LOG_EXT_E("i2c read transmit subaddr failed!\n");
                        return 0;
                    }
                    status = I2C_MasterRepeatedStart(nxp_i2c->i2c_info->i2c_base, nxp_i2c->transXfer.slaveAddress, kI2C_Read);
                    if (status != kStatus_Success)
                    {
                        LOG_EXT_E("i2c restart failed!\n");
                        return 0;
                    }
                    status = I2C_MasterReadBlocking(nxp_i2c->i2c_info->i2c_base, nxp_i2c->transXfer.data, nxp_i2c->transXfer.dataSize, kI2C_TransferDefaultFlag);
                    if (status != kStatus_Success)
                    {
                        LOG_EXT_E("i2c read data failed!\n");
                        return 0;
                    }
                }
                else
                {
                    status = I2C_MasterStart(nxp_i2c->i2c_info->i2c_base, nxp_i2c->transXfer.slaveAddress, kI2C_Write);
                    if (status != kStatus_Success)
                    {
                        LOG_EXT_E("i2c start failed!\n");
                        return 0;
                    }
                    status = I2C_MasterWriteBlocking(nxp_i2c->i2c_info->i2c_base, &nxp_i2c->transXfer.subaddress, nxp_i2c->transXfer.subaddressSize, kI2C_TransferNoStopFlag);
                    if (status != kStatus_Success)
                    {
                        LOG_EXT_E("i2c read transmit subaddr failed!\n");
                        return 0;
                    }
                    status = I2C_MasterWriteBlocking(nxp_i2c->i2c_info->i2c_base, nxp_i2c->transXfer.data, nxp_i2c->transXfer.dataSize, kI2C_TransferDefaultFlag);
                    if (status != kStatus_Success)
                    {
                        LOG_EXT_E("i2c read data failed!\n");
                        return 0;
                    }  
                }

                status = I2C_MasterStop(nxp_i2c->i2c_info->i2c_base);
                if (status != kStatus_Success)
                {
                    LOG_EXT_E("i2c stop failed!\n");
                    return 0;
                }
            }
        }
    }

    return i;
}

static os_size_t nxp_i2c_slave_xfer(struct os_i2c_bus_device *bus, struct os_i2c_msg msgs[], os_uint32_t num)
{
    return 0;
}
static os_err_t nxp_i2c_bus_control(struct os_i2c_bus_device *bus, void *arg)
{
    return OS_ERROR;
}

static const struct os_i2c_bus_device_ops nxp_i2c_ops =
{
    .i2c_transfer       = nxp_i2c_master_xfer,
    .i2c_slave_transfer = nxp_i2c_slave_xfer,
    .i2c_bus_control    = nxp_i2c_bus_control,
};

void nxp_i2c_parse_configs_from_configtool(struct nxp_i2c *nxp_i2c)
{
    struct os_i2c_bus_device *i2c = &nxp_i2c->i2c;
}

void nxp_i2c_param_cfg(struct nxp_i2c *nxp_i2c)
{
    switch((os_uint32_t)nxp_i2c->i2c_info->i2c_base)
    {
    case (os_uint32_t)FLEXCOMM0:
        I2C0_CFG_INIT(nxp_i2c, 0);
        break;
    case (os_uint32_t)FLEXCOMM1:
        I2C1_CFG_INIT(nxp_i2c, 1);
        break;
    case (os_uint32_t)FLEXCOMM2:
        I2C2_CFG_INIT(nxp_i2c, 2);
        break;
    case (os_uint32_t)FLEXCOMM3:
        I2C3_CFG_INIT(nxp_i2c, 3);
        break;
    case (os_uint32_t)FLEXCOMM4:
        I2C4_CFG_INIT(nxp_i2c, 4);
        break;
    case (os_uint32_t)FLEXCOMM5:
        I2C5_CFG_INIT(nxp_i2c, 5);
        break;
    case (os_uint32_t)FLEXCOMM6:
        I2C6_CFG_INIT(nxp_i2c, 6);
        break;
    case (os_uint32_t)FLEXCOMM7:
        I2C7_CFG_INIT(nxp_i2c, 7);
        break;
    case (os_uint32_t)FLEXCOMM8:
        I2C8_CFG_INIT(nxp_i2c, 8);
        break;
    default:
        break;
    }
}

static int nxp_i2c_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    os_base_t   level;
    
    struct nxp_i2c_info *i2c_info = (struct nxp_i2c_info *)dev->info;

    struct nxp_i2c *nxp_i2c = os_calloc(1, sizeof(struct nxp_i2c));

    OS_ASSERT(nxp_i2c);
    
    nxp_i2c->i2c_info = i2c_info;
    nxp_i2c_param_cfg(nxp_i2c);
    
    struct os_i2c_bus_device *i2c = &nxp_i2c->i2c;

    i2c->ops = &nxp_i2c_ops;

    char sem_name[16] = "sem_";
    strcat(sem_name, dev->name);
    os_sem_init(&nxp_i2c->sem, sem_name, 0, OS_IPC_FLAG_FIFO);

    level = os_hw_interrupt_disable();
    os_list_add_tail(&nxp_i2c_list, &nxp_i2c->list);
    os_hw_interrupt_enable(level);

    char dev_name[16] = "hard_";
    strcat(dev_name, dev->name);
    result = os_i2c_bus_device_register(i2c, dev_name, OS_DEVICE_FLAG_RDWR, i2c);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO nxp_i2c_driver = {
    .name   = "I2C_Type",
    .probe  = nxp_i2c_probe,
};

OS_DRIVER_DEFINE(nxp_i2c_driver, "2");

