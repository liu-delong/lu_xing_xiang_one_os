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
 * @file        drv_hwtimer.c
 *
 * @brief       This file implements timer driver for FM33A0xx.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_irq.h>
#include "drv_hwtimer.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.hwtimer"
#include <drv_log.h>

#ifdef OS_USING_HWTIMER
enum
{
#ifdef OS_USING_BSTIMER1
    BSTIMER1_INDEX,
#endif
#ifdef OS_USING_BSTIMER2
    BSTIMER2_INDEX,
#endif
};

struct fm_hwtimer
{
    os_hwtimer_t      time_device;
    FM_BTIMHandleTypeDef tim_handle;
    IRQn_Type         tim_irqn;
    char             *name;
};

static struct fm_hwtimer fm_hwtimer_obj[] =
{
#ifdef OS_USING_BSTIMER1
    USING_BSTIMER1_CONFIG,
#endif

#ifdef OS_USING_BSTIMER2
    USING_BSTIMER2_CONFIG,
#endif

};

static void fm_btm_cfg(struct os_hwtimer_device *timer, os_uint16_t freq)
{
    FM_BTIMHandleTypeDef    *Btim             = OS_NULL;
    struct fm_hwtimer *tim_device      = OS_NULL;
    BTIM_InitTypeDef *init_para;

    OS_ASSERT(timer != OS_NULL);

    Btim  = (FM_BTIMHandleTypeDef *)timer->parent.user_data;
    tim_device = (struct fm_hwtimer *)timer;
    init_para = &Btim->Init;

    init_para->sig_src_para.GRP1SEL = BTIMx_BTCFG1_GRP1SEL_APBCLK;
    init_para->sig_src_para.GRP2SEL = BTIMx_BTCFG1_GRP2SEL_RTCOUT2;

    init_para->sig_src_para.CNTSRC = BTIMx_BTCR2_SIG2SEL_GROUP1;
    init_para->sig_src_para.CAPSRC = BTIMx_BTCR2_SIG1SEL_GROUP2;

    init_para->sig_src_para.CNTHSEL = BTIMx_BTCR2_CNTHSEL_CNTL; /* 16-bit counter */
    init_para->sig_src_para.SRCSEL = BTIMx_BTCR2_SRCSEL_WITHOUT_DIR;

    init_para->ctrl_para.PRESCALE = ((os_uint32_t)RCHFCLKCFG*1000*1000 / 100000) - 1;
    /* 10000 - 1;  */
    init_para->ctrl_para.PRESETL = 0;   /* Low counter preset number */
    init_para->ctrl_para.PRESETH = 0;  /* High-end counter preset number */

    init_para->ctrl_para.LOADL = freq&0xFF;     /* Load register with low compare value */
    init_para->ctrl_para.LOADH = freq>>8;       /* Load the high-order comparison value into the register. Assuming that it is 1000, it should be 1ms */

    init_para->ctrl_para.EDGESEL = BTIMx_BTCR1_EDGESEL_POS; 
    init_para->ctrl_para.MODE = BTIMx_BTCR1_MODE_8BITS_TIM_CNT; 


    init_para->sig_src_para.RTCSEL1 = BTIMx_BTCFG1_RTCSEL1_RTC_32768;
    init_para->sig_src_para.RTCSEL2 = BTIMx_BTCFG1_RTCSEL2_RTC_32768;
    init_para->sig_src_para.INSEL1 = BTIMx_BTCFG2_INSEL1_UART_RX0;
    init_para->sig_src_para.INSEL2 = BTIMx_BTCFG2_INSEL2_UART_RX3;
    init_para->sig_src_para.EXSEL1 = BTIMx_BTCFG2_EXSEL1_BT_IN0;
    init_para->sig_src_para.EXSEL2 = BTIMx_BTCFG2_EXSEL2_BT_IN0;

    init_para->cap_para.CAPONCE = BTIMx_BTCR1_CAPONCE_SINGLE;
    init_para->cap_para.CAPMOD = BTIMx_BTCR1_CAPMOD_PAUSE_PERIOD;
    init_para->cap_para.CAPCLR = BTIMx_BTCR1_CAPCLR_CAP_CNT_CLR;

    init_para->dir_para.DIREN = DISABLE;
    init_para->dir_para.DIRPO = BTIMx_BTCR2_DIRPO_NO_ANTI;

    init_para->out_para.OUTCNT = 0;
    init_para->out_para.OUTCLR = BTIMx_BTOCR_OUTCLR_CLR;
    init_para->out_para.OUTINV = BTIMx_BTOCR_OUTINV_NOT_ANTI;
    init_para->out_para.OUTMOD = BTIMx_BTOCR_OUTMOD_PULSE;
    init_para->out_para.OUTSEL = BTIMx_BTOCR_OUTSEL_CMPH;

    BTIMx_Init(Btim->Instance, init_para);

    /* Enable LPTIMER peripheral interrupt */
    NVIC_DisableIRQ(tim_device->tim_irqn);
    NVIC_SetPriority(tim_device->tim_irqn, FM_IRQ_PRI_BSTIME);
    NVIC_EnableIRQ(tim_device->tim_irqn);
}

static void timer_init(struct os_hwtimer_device *timer, os_uint32_t state)
{
    FM_BTIMHandleTypeDef    *Btim             = OS_NULL;

    OS_ASSERT(timer != OS_NULL);

    if (state)
    {
        Btim  = (FM_BTIMHandleTypeDef *)timer->parent.user_data;

        RCC_PERCLK_SetableEx( Btim->periph_def, ENABLE );	

        fm_btm_cfg(timer, 10000 - 1);

    }
}

static os_err_t timer_start(os_hwtimer_t *timer, os_uint32_t t, os_hwtimer_mode_t opmode)
{
    os_err_t           result = OS_EOK;
    FM_BTIMHandleTypeDef *Btim    = OS_NULL;

    OS_ASSERT(timer != OS_NULL);

    Btim= (FM_BTIMHandleTypeDef *)timer->parent.user_data;

    BTIMx_BTPRESET_PRESETH_Set(Btim->Instance, 0);
    BTIMx_BTPRESET_PRESETL_Set(Btim->Instance, 0);

    BTIMx_BTLOADH_Write(Btim->Instance, ((t - 1) >> 8) & 0xFF);
    BTIMx_BTLOADL_Write(Btim->Instance, (t - 1) & 0xFF);

    if (opmode == HWTIMER_MODE_ONESHOT)
    {
        /* set timer to single mode */
        BTIMx_BTCR1_CAPONCE_Set(Btim->Instance, BTIMx_BTCR1_CAPONCE_SINGLE);
    }
    else
    {
        BTIMx_BTCR1_CAPONCE_Set(Btim->Instance, BTIMx_BTCR1_CAPONCE_CONTINUE);
    }

    /* Enable BTIMER compare value interrupt */
    BTIMx_BTIE_CMPLIE_Setable(Btim->Instance, ENABLE); 
    BTIMx_BTIE_CMPHIE_Setable(Btim->Instance, ENABLE);

    /* The BTIMER module writes the initial counter value and the target counter value */
    BTIMx_BTLOADCR_LHEN_Setable(Btim->Instance, ENABLE);
    BTIMx_BTLOADCR_LLEN_Setable(Btim->Instance, ENABLE);

    BTIMx_BTCR1_CHEN_Setable(Btim->Instance, ENABLE);
    BTIMx_BTCR1_CLEN_Setable(Btim->Instance, ENABLE);

    return result;
}

static void timer_stop(os_hwtimer_t *timer)
{
    FM_BTIMHandleTypeDef *Btim = OS_NULL;

    OS_ASSERT(timer != OS_NULL);

    Btim = (FM_BTIMHandleTypeDef *)timer->parent.user_data;

    /* stop timer */
    BTIMx_BTCR1_CHEN_Setable(Btim->Instance, DISABLE);
    BTIMx_BTCR1_CLEN_Setable(Btim->Instance, DISABLE);

    /* set tim cnt */
    BTIMx_BTPRESET_PRESETH_Set(Btim->Instance, 0);
    BTIMx_BTPRESET_PRESETL_Set(Btim->Instance, 0);

}

static os_err_t timer_ctrl(os_hwtimer_t *timer, os_uint32_t cmd, void *arg)
{
    os_err_t           result = OS_EOK;
    os_uint32_t freq;
    FM_BTIMHandleTypeDef *Btim = OS_NULL;
    os_uint16_t val;

    OS_ASSERT(timer != OS_NULL);
    OS_ASSERT(arg != OS_NULL);

    Btim = (FM_BTIMHandleTypeDef *)timer->parent.user_data;
    freq      = *((os_uint32_t *)arg);

    val = ((os_uint32_t)RCHFCLKCFG*1000*1000 / freq);

    switch (cmd)
    {
    case HWTIMER_CTRL_FREQ_SET:
    {
        BTIMx_BTPRES_Write(Btim->Instance, val - 1); /* Select frequency division factor */
    }
    break;
    default:
    {
        result = OS_ENOSYS;
    }
    }

    return result;
}

static os_uint32_t timer_counter_get(os_hwtimer_t *timer)
{
    FM_BTIMHandleTypeDef *Btim = OS_NULL;

    OS_ASSERT(timer != OS_NULL);

    Btim = (FM_BTIMHandleTypeDef *)timer->parent.user_data;

    return (BTIMx_BTCNTH_Read(Btim->Instance) << 8) | BTIMx_BTCNTL_Read(Btim->Instance);
}

static const struct os_hwtimer_info _info = TIM_DEV_INFO_CONFIG;

static const struct os_hwtimer_ops _ops =
{
    .init      = timer_init,
    .start     = timer_start,
    .stop      = timer_stop,
    .count_get = timer_counter_get,
    .control   = timer_ctrl,
};

void  BTIM_PeriodElapsedCallback(FM_BTIMHandleTypeDef *htim)
{
#ifdef OS_USING_BSTIMER1
    if (htim->Instance == BTIM1)
    {
        os_device_hwtimer_isr(&fm_hwtimer_obj[BSTIMER1_INDEX].time_device);
    }
#endif

#ifdef OS_USING_BSTIMER2
    if (htim->Instance == BTIM2)
    {
        os_device_hwtimer_isr(&fm_hwtimer_obj[BSTIMER2_INDEX].time_device);
    }
#endif

}
void  BTIM_IRQHandler(FM_BTIMHandleTypeDef *htim)
{
    /* TIM Update event */
    if (BTIMx_BTIF_CMPHIF_Chk(htim->Instance) == SET)
    {
        BTIMx_BTIF_CMPHIF_Clr(htim->Instance);;
        BTIM_PeriodElapsedCallback(htim);
    }

}

#if defined(OS_USING_BSTIMER1)
void BTIM1_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();
    BTIM_IRQHandler(&fm_hwtimer_obj[BSTIMER1_INDEX].tim_handle);
    /* leave interrupt */
    os_interrupt_leave();
}
#endif

#if defined(OS_USING_BSTIMER2)
void BTIM2_IRQHandler(void)
{
    /* enter interrupt */
    os_interrupt_enter();
    BTIM_IRQHandler(&fm_hwtimer_obj[BSTIMER2_INDEX].tim_handle);
    /* leave interrupt */
    os_interrupt_leave();
}
#endif


/**
 ***********************************************************************************************************************
 * @brief           fm_hwtimer_init:register timer device.
 *
 * @param[in]       none
 *
 * @return          Return timer register status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int fm_hwtimer_init(void)
{
    int i      = 0;
    int result = OS_EOK;

    for (i = 0; i < sizeof(fm_hwtimer_obj) / sizeof(fm_hwtimer_obj[0]); i++)
    {
        fm_hwtimer_obj[i].time_device.info = &_info;
        fm_hwtimer_obj[i].time_device.ops  = &_ops;
        if (os_device_hwtimer_register(&fm_hwtimer_obj[i].time_device,
                                       fm_hwtimer_obj[i].name,
                                       &fm_hwtimer_obj[i].tim_handle) == OS_EOK)
        {
            LOG_EXT_D("%s register success", fm_hwtimer_obj[i].name);
        }
        else
        {
            LOG_EXT_E("%s register failed", fm_hwtimer_obj[i].name);
            result = OS_ERROR;
        }
    }

    return result;
}
OS_BOARD_INIT(fm_hwtimer_init);

#endif /* OS_USING_HWTIMER */
