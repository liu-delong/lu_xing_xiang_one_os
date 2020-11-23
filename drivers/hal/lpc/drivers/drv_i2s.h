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
 * @file        drv_i2c.h
 *
 * @brief       This file implements i2c driver for nxp.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRVI2S_H__
#define DRVI2S_H__

#include <drv_common.h>

#define I2S_IRQHandler_DEFINE(__index)                                                      \
void I2S_##__index##_FLEXCOMM_IRQHANDLER(void)                                              \
{                                                                                           \
    struct nxp_i2s *nxp_i2s;                                                                \
    os_list_for_each_entry(nxp_i2s, &nxp_i2s_list, struct nxp_i2s, list)                    \
    {                                                                                       \
        if (nxp_i2s->i2s_info->i2s_base == (I2S_Type *)FLEXCOMM##__index)                   \
        {                                                                                   \
            break;                                                                          \
        }                                                                                   \
    }                                                                                       \
    if (nxp_i2s->i2s_info->i2s_base != (I2S_Type *)FLEXCOMM##__index)                       \
        return;                                                                             \
    nxp_i2s_irq_callback(nxp_i2s);                                                          \
    SDK_ISR_EXIT_BARRIER;                                                                   \
}

#define I2S_IRQ_INIT(_NXP_I2S_, __index)                                                    \
        _NXP_I2S_->clk_src = I2S##__index##_CLOCK_SOURCE;                                   \
        _NXP_I2S_->irqn = I2S##__index##_FLEXCOMM_IRQN;
    
#define I2S_NBK_INIT(_NXP_I2S_, __index)                                                    \
        _NXP_I2S_->clk_src = I2S##__index##_CLOCK_SOURCE;                                   \
        _NXP_I2S_->i2s_handle = &I2S##__index##_handle;                                     \
        _NXP_I2S_->i2s_handle->completionCallback = nxp_i2s_transfer_callback;                        \
        _NXP_I2S_->i2s_handle->userData = _NXP_I2S_;                                        \
    
#define I2S_DMA_INIT(_NXP_I2S_, __index)                                                    \
        _NXP_I2S_->clk_src = I2S##__index##_CLOCK_SOURCE;                                   \
        _NXP_I2S_->i2s_DmaHandle = &I2S##__index##_DMA_Handle;                          \
        _NXP_I2S_->i2s_DmaHandle->completionCallback = nxp_i2s_dma_callback;                          \
        _NXP_I2S_->i2s_DmaHandle->userData = _NXP_I2S_;                                     \
    
#define I2S_POL_INIT(_NXP_I2S_, __index) _NXP_I2S_->clk_src = I2S##__index##_CLOCK_SOURCE;
    
#define I2S_NULL_INIT(_NXP_I2S_, __index) return;
        
#if defined(I2S0_FLEXCOMM_IRQN)
#define I2S0_CFG_INIT I2S_IRQ_INIT
#elif defined(I2S0_RX_BUFFER_SIZE) && defined(I2S0_TX_BUFFER_SIZE)
#define I2S0_CFG_INIT I2S_NBK_INIT
#elif defined(I2S0_RX_DMA_CHANNEL) || defined(I2S0_TX_DMA_CHANNEL)
#if defined(I2S0_TX_DMA_CHANNEL)
#define I2S0_DMA_Handle I2S0_Tx_DMA_Handle
#else
#define I2S0_DMA_Handle I2S0_Rx_DMA_Handle
#endif
#define I2S0_CFG_INIT I2S_DMA_INIT
#else
#define I2S0_CFG_INIT I2S_NULL_INIT
#endif

#if defined(I2S1_FLEXCOMM_IRQN)
#define I2S1_CFG_INIT I2S_IRQ_INIT
#elif defined(I2S1_RX_BUFFER_SIZE) || defined(I2S1_TX_BUFFER_SIZE)
#define I2S1_CFG_INIT I2S_NBK_INIT
#elif defined(I2S1_RX_DMA_CHANNEL) || defined(I2S1_TX_DMA_CHANNEL)
#if defined(I2S6_TX_DMA_CHANNEL)
#define I2S1_DMA_Handle I2S1_Tx_DMA_Handle
#else
#define I2S1_DMA_Handle I2S1_Rx_DMA_Handle
#endif
#define I2S1_CFG_INIT I2S_DMA_INIT
#else
#define I2S1_CFG_INIT I2S_NULL_INIT
#endif
    
#if defined(I2S2_FLEXCOMM_IRQN)
#define I2S2_CFG_INIT I2S_IRQ_INIT
#elif defined(I2S2_RX_BUFFER_SIZE) && defined(I2S2_TX_BUFFER_SIZE)
#define I2S2_CFG_INIT I2S_NBK_INIT
#elif defined(I2S2_RX_DMA_CHANNEL) || defined(I2S2_TX_DMA_CHANNEL)
#if defined(I2S2_TX_DMA_CHANNEL)
#define I2S2_DMA_Handle I2S2_Tx_DMA_Handle
#else
#define I2S2_DMA_Handle I2S2_Rx_DMA_Handle
#endif
#define I2S2_CFG_INIT I2S_DMA_INIT
#else
#define I2S2_CFG_INIT I2S_NULL_INIT
#endif
    
#if defined(I2S3_FLEXCOMM_IRQN)
#define I2S3_CFG_INIT I2S_IRQ_INIT
#elif defined(I2S3_RX_BUFFER_SIZE) || defined(I2S3_TX_BUFFER_SIZE)
#define I2S3_CFG_INIT I2S_NBK_INIT
#elif defined(I2S3_RX_DMA_CHANNEL) || defined(I2S3_TX_DMA_CHANNEL)
#if defined(I2S3_TX_DMA_CHANNEL)
#define I2S3_DMA_Handle I2S3_Tx_DMA_Handle
#else
#define I2S3_DMA_Handle I2S3_Rx_DMA_Handle
#endif
#define I2S3_CFG_INIT I2S_DMA_INIT
#else
#define I2S3_CFG_INIT I2S_NULL_INIT
#endif
    
#if defined(I2S4_FLEXCOMM_IRQN)
#define I2S4_CFG_INIT I2S_IRQ_INIT
#elif defined(I2S4_RX_BUFFER_SIZE) || defined(I2S4_TX_BUFFER_SIZE)
#define I2S4_CFG_INIT I2S_NBK_INIT
#elif defined(I2S4_RX_DMA_CHANNEL) || defined(I2S4_TX_DMA_CHANNEL)
#if defined(I2S4_TX_DMA_CHANNEL)
#define I2S4_DMA_Handle I2S4_Tx_DMA_Handle
#else
#define I2S4_DMA_Handle I2S4_Rx_DMA_Handle
#endif
#define I2S4_CFG_INIT I2S_DMA_INIT
#else
#define I2S4_CFG_INIT I2S_NULL_INIT
#endif
    
#if defined(I2S5_FLEXCOMM_IRQN)
#define I2S5_CFG_INIT I2S_IRQ_INIT
#elif defined(I2S5_RX_BUFFER_SIZE) || defined(I2S5_TX_BUFFER_SIZE)
#define I2S5_CFG_INIT I2S_NBK_INIT
#elif defined(I2S5_RX_DMA_CHANNEL) || defined(I2S5_TX_DMA_CHANNEL)
#if defined(I2S5_TX_DMA_CHANNEL)
#define I2S5_DMA_Handle I2S5_Tx_DMA_Handle
#else
#define I2S5_DMA_Handle I2S5_Rx_DMA_Handle
#endif
#define I2S5_CFG_INIT I2S_DMA_INIT
#else
#define I2S5_CFG_INIT I2S_NULL_INIT
#endif
    
#if defined(I2S6_FLEXCOMM_IRQN)
#define I2S6_CFG_INIT I2S_IRQ_INIT
#elif defined(I2S6_RX_BUFFER_SIZE) || defined(I2S6_TX_BUFFER_SIZE)
#define I2S6_CFG_INIT I2S_NBK_INIT
#elif defined(I2S6_RX_DMA_CHANNEL) || defined(I2S6_TX_DMA_CHANNEL)
#if defined(I2S6_TX_DMA_CHANNEL)
#define I2S6_DMA_Handle I2S6_Tx_DMA_Handle
#else
#define I2S6_DMA_Handle I2S6_Rx_DMA_Handle
#endif
#define I2S6_CFG_INIT I2S_DMA_INIT
#else
#define I2S6_CFG_INIT I2S_NULL_INIT
#endif
    
#if defined(I2S7_FLEXCOMM_IRQN)
#define I2S7_CFG_INIT I2S_IRQ_INIT
#elif defined(I2S7_RX_BUFFER_SIZE) || defined(I2S7_TX_BUFFER_SIZE)
#define I2S7_CFG_INIT I2S_NBK_INIT
#elif defined(I2S7_RX_DMA_CHANNEL) || defined(I2S7_TX_DMA_CHANNEL)
#if defined(I2S7_TX_DMA_CHANNEL)
#define I2S7_DMA_Handle I2S7_Tx_DMA_Handle
#else
#define I2S7_DMA_Handle I2S7_Rx_DMA_Handle
#endif
#define I2S7_CFG_INIT I2S_DMA_INIT
#else
#define I2S7_CFG_INIT I2S_NULL_INIT
#endif
    
#if defined(I2S8_FLEXCOMM_IRQN)
#define I2S8_CFG_INIT I2S_IRQ_INIT
#elif defined(I2S8_RX_BUFFER_SIZE) || defined(I2S8_TX_BUFFER_SIZE)
#define I2S8_CFG_INIT I2S_NBK_INIT
#elif defined(I2S8_RX_DMA_CHANNEL) || defined(I2S8_TX_DMA_CHANNEL)
#if defined(I2S8_TX_DMA_CHANNEL)
#define I2S8_DMA_Handle I2S8_Tx_DMA_Handle
#else
#define I2S8_DMA_Handle I2S8_Rx_DMA_Handle
#endif
#define I2S8_CFG_INIT I2S_DMA_INIT
#else
#define I2S8_CFG_INIT I2S_NULL_INIT
#endif

struct nxp_i2s_info {
    I2S_Type *i2s_base;
    const i2s_config_t *i2s_config;
};

#endif
