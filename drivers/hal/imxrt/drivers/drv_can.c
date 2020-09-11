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
 * @file        drv_can.c
 *
 * @brief       This file implements can driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <os_task.h>
#ifdef BSP_USING_CAN

#include <os_device.h>
#include "drv_can.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_flexcan.h"

#define LOG_TAG    "drv.can"
#include <drv_log.h>

#if defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL
    #error "Please don't define 'FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL'!"
#endif

#define RX_MB_COUNT     32
static flexcan_frame_t frame[RX_MB_COUNT];    /* one frame buffer per RX MB */
static os_uint32_t filter_mask = 0;

enum
{
#ifdef BSP_USING_CAN1
    CAN1_INDEX,
#endif
#ifdef BSP_USING_CAN2
    CAN2_INDEX,
#endif
};

struct imxrt_can
{
    char *name;
    CAN_Type *base;
    IRQn_Type irqn;
    flexcan_handle_t handle;
    struct rt_can_device can_dev;
};

struct imxrt_can flexcans[] =
{
#ifdef BSP_USING_CAN1
    {
        .name = "can1",
        .base = CAN1,
        .irqn = CAN1_IRQn,
    },
#endif
#ifdef BSP_USING_CAN2
    {
        .name = "can2",
        .base = CAN2,
        .irqn = CAN2_IRQn,
    },
#endif
};

uint32_t GetCanSrcFreq(void)
{
    uint32_t freq;

    freq = (CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 6) / (CLOCK_GetDiv(kCLOCK_CanDiv) + 1U);

    return freq;
}

static void flexcan_callback(CAN_Type *base, flexcan_handle_t *handle, status_t status, uint32_t result, void *userData)
{
    struct imxrt_can *can;
    flexcan_mb_transfer_t rxXfer;

    can = (struct imxrt_can *)userData;

    switch (status)
    {
    case kStatus_FLEXCAN_RxIdle:
        rt_hw_can_isr(&can->can_dev, RT_CAN_EVENT_RX_IND | result << 8);
        rxXfer.frame = &frame[result - 1];
        rxXfer.mbIdx = result;
        FLEXCAN_TransferReceiveNonBlocking(can->base, &can->handle, &rxXfer);
        break;

    case kStatus_FLEXCAN_TxIdle:
        rt_hw_can_isr(&can->can_dev, RT_CAN_EVENT_TX_DONE | (63 - result) << 8);
        break;

    case kStatus_FLEXCAN_WakeUp:

    case kStatus_FLEXCAN_ErrorStatus:
        if ((result >= 47) && (result <= 63))
        {
            rt_hw_can_isr(&can->can_dev, RT_CAN_EVENT_TX_FAIL | (63 - result) << 8);
        }
        break;

    case kStatus_FLEXCAN_TxSwitchToRx:
        break;

    default:
        break;
    }
}

static os_err_t can_cfg(struct rt_can_device *can_dev, struct can_configure *cfg)
{
    struct imxrt_can *can;
    flexcan_config_t config;
    os_uint32_t res = OS_EOK;
    flexcan_rx_mb_config_t mbConfig;
    flexcan_mb_transfer_t rxXfer;
    os_uint8_t i, mailbox;

    OS_ASSERT(can_dev != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    can = (struct imxrt_can *)can_dev->parent.user_data;
    OS_ASSERT(can != OS_NULL);

    FLEXCAN_GetDefaultConfig(&config);
    config.baudRate = cfg->baud_rate;
    config.maxMbNum = 64;               /* all series have 64 MB */
    config.enableIndividMask = true;    /* one filter per MB */
    switch (cfg->mode)
    {
    case RT_CAN_MODE_NORMAL:
        /* default mode */
        break;
    case RT_CAN_MODE_LISEN:
        break;
    case RT_CAN_MODE_LOOPBACK:
        config.enableLoopBack = true;
        break;
    case RT_CAN_MODE_LOOPBACKANLISEN:
        break;
    }
    FLEXCAN_Init(can->base, &config, GetCanSrcFreq());
    FLEXCAN_TransferCreateHandle(can->base, &can->handle, flexcan_callback, can);
    /* init RX_MB_COUNT RX MB to default status */
    mbConfig.format = kFLEXCAN_FrameFormatStandard;  /* standard ID */
    mbConfig.type = kFLEXCAN_FrameTypeData;          /* data frame */
    mbConfig.id = FLEXCAN_ID_STD(0);                 /* default ID is 0 */
    for (i = 0; i < RX_MB_COUNT; i++)
    {
        /* the used MB index from 1 to RX_MB_COUNT */
        mailbox = i + 1;
        /* all ID bit in the filter is "don't care" */
        FLEXCAN_SetRxIndividualMask(can->base, mailbox, FLEXCAN_RX_MB_STD_MASK(0, 0, 0));
        FLEXCAN_SetRxMbConfig(can->base, mailbox, &mbConfig, true);
        /* one frame buffer per MB */
        rxXfer.frame = &frame[i];
        rxXfer.mbIdx = mailbox;
        FLEXCAN_TransferReceiveNonBlocking(can->base, &can->handle, &rxXfer);
    }

    return res;
}

static os_err_t can_control(struct rt_can_device *can_dev, int cmd, void *arg)
{
    struct imxrt_can *can;
    os_uint32_t argval, mask;
    os_uint32_t res = OS_EOK;
    flexcan_rx_mb_config_t mbConfig;
    struct rt_can_filter_config  *cfg;
    struct rt_can_filter_item *item;
    os_uint8_t i, count, index;

    OS_ASSERT(can_dev != OS_NULL);

    can = (struct imxrt_can *)can_dev->parent.user_data;
    OS_ASSERT(can != OS_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_SET_INT:
        argval = (os_uint32_t) arg;
        if (argval == RT_DEVICE_FLAG_INT_RX)
        {
            mask = kFLEXCAN_RxWarningInterruptEnable;
        }
        else if (argval == RT_DEVICE_FLAG_INT_TX)
        {
            mask = kFLEXCAN_TxWarningInterruptEnable;
        }
        else if (argval == RT_DEVICE_CAN_INT_ERR)
        {
            mask = kFLEXCAN_ErrorInterruptEnable;
        }
        FLEXCAN_EnableInterrupts(can->base, mask);
        NVIC_SetPriority(can->irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
        EnableIRQ(can->irqn);
        break;
    case RT_DEVICE_CTRL_CLR_INT:
        /* each CAN device have one IRQ number. */
        DisableIRQ(can->irqn);
        break;
    case RT_CAN_CMD_SET_FILTER:
        cfg = (struct rt_can_filter_config *)arg;
        item = cfg->items;
        count = cfg->count;

        if (filter_mask == 0xffffffff)
        {
            LOG_E("%s filter is full!\n", can->name);
            res = OS_ERROR;
            break;
        }
        else if (filter_mask == 0)
        {
            /* deinit all init RX MB */
            for (i = 0; i < RX_MB_COUNT; i++)
            {
                FLEXCAN_SetRxMbConfig(can->base, i + 1, OS_NULL, false);
            }
        }

        while (count)
        {
            if (item->ide)
            {
                mbConfig.format = kFLEXCAN_FrameFormatExtend;
                mbConfig.id = FLEXCAN_ID_EXT(item->id);
                mask = FLEXCAN_RX_MB_EXT_MASK(item->mask, 0, 0);
            }
            else
            {
                mbConfig.format = kFLEXCAN_FrameFormatStandard;
                mbConfig.id = FLEXCAN_ID_STD(item->id);
                mask = FLEXCAN_RX_MB_STD_MASK(item->mask, 0, 0);
            }

            if (item->rtr)
            {
                mbConfig.type = kFLEXCAN_FrameTypeRemote;
            }
            else
            {
                mbConfig.type = kFLEXCAN_FrameTypeData;
            }

            /* user does not specify hdr index,set hdr from RX MB 1 */
            if (item->hdr == -1)
            {

                for (i = 0; i < 32; i++)
                {
                    if (!(filter_mask & (1 << i)))
                    {
                        index = i;
                        break;
                    }
                }
            }
            else    /* use user specified hdr */
            {
                if (filter_mask & (1 << item->hdr))
                {
                    res = OS_ERROR;
                    LOG_E("%s hdr%d filter already set!\n", can->name, item->hdr);
                    break;
                }
                else
                {
                    index = item->hdr;
                }
            }

            /* RX MB index from 1 to 32,hdr index 0~31 map RX MB index 1~32. */
            FLEXCAN_SetRxIndividualMask(can->base, index + 1, mask);
            FLEXCAN_SetRxMbConfig(can->base, index + 1, &mbConfig, true);
            filter_mask |= 1 << index;

            item++;
            count--;
        }

        break;

    case RT_CAN_CMD_SET_BAUD:
        res = OS_ERROR;
        break;
    case RT_CAN_CMD_SET_MODE:
        res = OS_ERROR;
        break;

    case RT_CAN_CMD_SET_PRIV:
        res = OS_ERROR;
        break;
    case RT_CAN_CMD_GET_STATUS:
        FLEXCAN_GetBusErrCount(can->base, (os_uint8_t *)(&can->can_dev.status.snderrcnt), (os_uint8_t *)(&can->can_dev.status.rcverrcnt));
        os_memcpy(arg, &can->can_dev.status, sizeof(can->can_dev.status));
        break;
    default:
        res = OS_ERROR;
        break;
    }

    return res;
}

static int can_send(struct rt_can_device *can_dev, const void *buf, os_uint32_t boxno)
{
    struct imxrt_can *can;
    struct rt_can_msg *msg;
    status_t ret;
    flexcan_frame_t frame;
    flexcan_mb_transfer_t txXfer;
    os_uint8_t sendMB;

    OS_ASSERT(can_dev != OS_NULL);
    OS_ASSERT(buf != OS_NULL);

    can = (struct imxrt_can *)can_dev->parent.user_data;
    msg = (struct rt_can_msg *) buf;
    OS_ASSERT(can != OS_NULL);
    OS_ASSERT(msg != OS_NULL);

    /* use the last 16 MB to send msg */
    sendMB = 63 - boxno;
    FLEXCAN_SetTxMbConfig(can->base, sendMB, true);

    if (RT_CAN_STDID == msg->ide)
    {
        frame.id = FLEXCAN_ID_STD(msg->id);
        frame.format = kFLEXCAN_FrameFormatStandard;
    }
    else if (RT_CAN_EXTID == msg->ide)
    {
        frame.id = FLEXCAN_ID_EXT(msg->id);
        frame.format = kFLEXCAN_FrameFormatExtend;
    }

    if (RT_CAN_DTR == msg->rtr)
    {
        frame.type = kFLEXCAN_FrameTypeData;
    }
    else if (RT_CAN_RTR == msg->rtr)
    {
        frame.type = kFLEXCAN_FrameTypeRemote;
    }

    frame.length = msg->len;
    frame.dataByte0 = msg->data[0];
    frame.dataByte1 = msg->data[1];
    frame.dataByte2 = msg->data[2];
    frame.dataByte3 = msg->data[3];
    frame.dataByte4 = msg->data[4];
    frame.dataByte5 = msg->data[5];
    frame.dataByte6 = msg->data[6];
    frame.dataByte7 = msg->data[7];

    txXfer.mbIdx = sendMB;
    txXfer.frame = &frame;

    ret = FLEXCAN_TransferSendNonBlocking(can->base, &can->handle, &txXfer);
    switch (ret)
    {
    case kStatus_Success:
        ret = OS_EOK;
        break;
    case kStatus_Fail:
        ret = OS_ERROR;
        break;
    case kStatus_FLEXCAN_TxBusy:
        ret = OS_EBUSY;
        break;
    }
    
    return ret;
}

static int can_recv(struct rt_can_device *can_dev, void *buf, os_uint32_t boxno)
{
    struct imxrt_can *can;
    struct rt_can_msg *pmsg;
    os_uint8_t index;

    OS_ASSERT(can_dev != OS_NULL);

    can = (struct imxrt_can *)can_dev->parent.user_data;
    pmsg = (struct rt_can_msg *) buf;
    OS_ASSERT(can != OS_NULL);

    index = boxno - 1;

    if (frame[index].format == kFLEXCAN_FrameFormatStandard)
    {
        pmsg->ide = RT_CAN_STDID;
        pmsg->id = frame[index].id >> CAN_ID_STD_SHIFT;
    }
    else
    {
        pmsg->ide = RT_CAN_EXTID;
        pmsg->id = frame[index].id >> CAN_ID_EXT_SHIFT;
    }

    if (frame[index].type == kFLEXCAN_FrameTypeData)
    {
        pmsg->rtr = RT_CAN_DTR;
    }
    else if (frame[index].type == kFLEXCAN_FrameTypeRemote)
    {
        pmsg->rtr = RT_CAN_RTR;
    }
    pmsg->hdr = index;      /* one hdr filter per MB */
    pmsg->len = frame[index].length;
    pmsg->data[0] = frame[index].dataByte0;
    pmsg->data[1] = frame[index].dataByte1;
    pmsg->data[2] = frame[index].dataByte2;
    pmsg->data[3] = frame[index].dataByte3;
    pmsg->data[4] = frame[index].dataByte4;
    pmsg->data[5] = frame[index].dataByte5;
    pmsg->data[6] = frame[index].dataByte6;
    pmsg->data[7] = frame[index].dataByte7;

    return 0;
}

static struct rt_can_ops imxrt_can_ops =
{
    .configure    = can_cfg,
    .control      = can_control,
    .sendmsg      = can_send,
    .recvmsg      = can_recv,
};

int rt_hw_can_init(void)
{
    int i;
    os_err_t ret = OS_EOK;
    struct can_configure config = CANDEFAULTCONFIG;

    config.privmode = 0;
    config.ticks = 50;
    config.sndboxnumber = 16;             /* send Mailbox count */
    config.msgboxsz = RX_MB_COUNT;        /* RX msg buffer count */
#ifdef RT_CAN_USING_HDR
    config.maxhdr = RX_MB_COUNT;          /* filter count,one filter per MB */
#endif

    for (i = 0; i < sizeof(flexcans) / sizeof(flexcans[0]); i++)
    {
        flexcans[i].can_dev.config = config;
        ret = rt_hw_can_register(&flexcans[i].can_dev, flexcans[i].name, &imxrt_can_ops, &flexcans[i]);
    }

    return ret;
}
INIT_BOARD_EXPORT(rt_hw_can_init);

#endif /* BSP_USING_CAN */
