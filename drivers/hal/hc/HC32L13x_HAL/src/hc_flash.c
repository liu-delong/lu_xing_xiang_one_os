/******************************************************************************
*Copyright(C)2018, Huada Semiconductor Co.,Ltd All rights reserved.
*
* This software is owned and published by:
* Huada Semiconductor Co.,Ltd("HDSC").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with HDSC
* components. This software is licensed by HDSC to be adapted only
* for use in systems utilizing HDSC components. HDSC shall not be
* responsible for misuse or illegal use of this software for devices not
* supported herein. HDSC is providing this software "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the software.
*
* Disclaimer:
* HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS),
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
* WARRANTY OF NONINFRINGEMENT.
* HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
* SAVINGS OR PROFITS,
* EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
* FROM, THE SOFTWARE.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Disclaimer and Copyright notice must be
* included with each copy of this software, whether used in part or whole,
* at all times.
*/

/** \file flash.c
 **
 ** Common API of flash.
 ** @link flashGroup Some description @endlink
 **
 **   - 2018-05-08
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc_flash.h"
/**
 *******************************************************************************
 ** \addtogroup FlashGroup
 ******************************************************************************/
//@{

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define FLASH_END_ADDR              (0x0000FFFFu)
#define FLASH_BYPASS()              {M0P_FLASH->BYPASS_f.BYSEQ = 0x5A5A;M0P_FLASH->BYPASS_f.BYSEQ = 0xA5A5;}
#define FLASH_IE_TRUE               (0x03)
#define FLASH_IE_FALSE              (0x00)   
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
/**
 ******************************************************************************
 ** \brief FLASH OP
 ** 
 ** Flash 操作控制数据类型重定�?
 ******************************************************************************/
typedef enum en_flash_op
{
    Read        = 0u,           ///<读配置�?
    Program     = 1u,           ///<编程配置�?
    SectorErase = 2u,           ///<扇区擦除配置�?
    ChipErase   = 3u,           ///<全片擦除配置�?
} en_flash_op_t;

/**
 ******************************************************************************
 ** \brief FLASH LOCK
 ** 
 ** Flash 加解锁数据类型重定义 
 ******************************************************************************/
typedef enum en_flash_lock
{
    LockAll   = 0x00000000u,      ///<全片加锁  
    UnlockAll = (int)0xFFFFFFFFu,      ///<全片解锁
} en_flash_lock_t;

/**
 ******************************************************************************
 ** \brief FLASH 编程时间参数配置
 ** 
 ** FLASH编程时间参数配置数据类型重定�?(4MHz)
 ******************************************************************************/
typedef enum en_flash_prgtimer
{
    Tnvs      = 0x20u,
    Tpgs      = 0x17u,
    Tprog     = 0x1Bu,
    Tserase   = 0x4650u,
    Tmerase   = 0x222E0u,
    Tprcv     = 0x18u,
    Tsrcv     = 0xF0u,
    Tmrcv     = 0x3E8u,
} en_flash_prgtimer_t;

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static func_ptr_t pfnFlashCallback = NULL;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 *****************************************************************************
 ** \brief Flash中断服务函数
 **
 **
 ** \param [in] u8Param        == 0
 **                                      
 *****************************************************************************/
void EfRam_IRQHandler(uint8_t u8Param)
{   
    pfnFlashCallback();
}

/**
 *****************************************************************************
 ** \brief Flash中断标志获取
 **
 **
 ** \param [in]  enFlashIntType          Flash中断类型
 ** 
 ** \retval TRUE or FALSE                                      
 *****************************************************************************/
boolean_t Flash_GetIntFlag(en_flash_int_type_t enFlashIntType)
{
    boolean_t bRetVal = FALSE;
    
    switch (enFlashIntType)
    {
        case FlashPCInt:
            bRetVal = M0P_FLASH->IFR_f.IF0 ? TRUE : FALSE;
            break;
        case FlashSlockInt:
            bRetVal = M0P_FLASH->IFR_f.IF1 ? TRUE : FALSE;
            break;
        default:
            bRetVal = FALSE;
            break;
    }
    
    return bRetVal;
}

/**
 *****************************************************************************
 ** \brief Flash中断标志清除
 **
 **
 ** \param [in]  enFlashIntType          Flash中断类型
 ** 
 ** \retval Ok or Error                                      
 *****************************************************************************/
en_result_t Flash_ClearIntFlag(en_flash_int_type_t enFlashIntType)
{
    en_result_t enResult = Error;

    switch (enFlashIntType)
    {
        case FlashPCInt:
            FLASH_BYPASS();
            M0P_FLASH->ICLR_f.ICLR0 = FALSE;
            enResult = Ok;
            break;
        case FlashSlockInt:
            FLASH_BYPASS();
            M0P_FLASH->ICLR_f.ICLR1 = FALSE;
            enResult = Ok;
            break;
        default:
            enResult = Error;
            break;
    }

    return enResult;
}

/**
 *****************************************************************************
 ** \brief Flash中断使能
 **
 **
 ** \param [in]  enFlashIntType          Flash中断类型
 ** 
 ** \retval Ok or Error                                      
 *****************************************************************************/
en_result_t Flash_EnableIrq (en_flash_int_type_t enFlashIntType)
{
    en_result_t enResult = Error;

    switch (enFlashIntType)
    {
        case FlashPCInt:
            FLASH_BYPASS();
            M0P_FLASH->CR_f.IE |= 0x01;
            enResult = Ok;
            break;
        case FlashSlockInt:
            FLASH_BYPASS();
            M0P_FLASH->CR_f.IE |= 0x02;
            enResult = Ok;
            break;
        default:
            enResult = Error;
            break;
    }

    return enResult;
}

/**
 *****************************************************************************
 ** \brief Flash中断禁止
 **
 **
 ** \param [in]  enFlashIntType          Flash中断类型
 ** 
 ** \retval Ok or Error                                      
 *****************************************************************************/
en_result_t Flash_DisableIrq(en_flash_int_type_t enFlashIntType)
{
    en_result_t enResult = Error;

    switch (enFlashIntType)
    {
        case FlashSlockInt:
            FLASH_BYPASS();
            M0P_FLASH->CR_f.IE &= ~0x02u;
            enResult = Ok;
            break;
        case FlashPCInt:
            FLASH_BYPASS();
            M0P_FLASH->CR_f.IE &= ~0x01u;
            enResult = Ok;
            break;
        default:
            enResult = Error;
            break;
    }

    return enResult;
}

/**
 *****************************************************************************
 ** \brief FLASH 初始化函数——中断服务程序、编程时间配置及低功耗模�?
 **
 ** 该函数用于配置中断服务函数、低功耗模式、根据系统时钟配置FLASH编程时间相关寄存�?
 **
 ** \param [in]  pfnFlashCb       Flash中断服务回调函数[void function(void)]
 ** \param [in]  u8FreqCfg        FLASH编程时钟频率配置(根据HCLK的频率选择配置�?�?
 **                               1      - (0,4]MHz;
 **                               2      - (4,8]MHz;
 **                               4      - (8,16]MHz;
 **                               6      - (16,24]MHz;
 **                               8      - (24,32]MHz（该配置会设置插�?个FLASH等待周期�?
 **                               12     - (32,48]MHz（该配置会设置插�?个FLASH等待周期�?
 **                               other   -  无效�?
 ** \param [in] bDpstbEn          TRUE  - 当系统进入DeepSleep模式，FLASH进入低功耗模�?
 **                               FALSE - 当系统进入DeepSleep模式，FLASH不进入低功耗模�?
 ** 
 ** \retval Ok                    操作成功.
 ** \retval ErrorInvalidParameter 参数无效.
 **
 *****************************************************************************/
en_result_t Flash_Init(func_ptr_t pfnFlashCb, uint8_t u8FreqCfg, boolean_t bDpstbEn)
{
    en_result_t             enResult = Ok;
    
    if ((1  != u8FreqCfg) &&
        (2  != u8FreqCfg) &&
        (4  != u8FreqCfg) &&
        (6  != u8FreqCfg) &&
        (8  != u8FreqCfg) &&
        (12 != u8FreqCfg))
    {
        enResult = ErrorInvalidParameter;
        return (enResult);
    }

    //当系统进入DeepSleep模式时，FLASH模式配置
    FLASH_BYPASS();
    M0P_FLASH->CR_f.DPSTB_EN = bDpstbEn;
    
    //flash时间参数寄存器配�?
    FLASH_BYPASS();
    M0P_FLASH->TNVS_f.TNVS       = Tnvs * u8FreqCfg;
    FLASH_BYPASS();
    M0P_FLASH->TPGS_f.TPGS       = Tpgs * u8FreqCfg;
    FLASH_BYPASS();
    M0P_FLASH->TPROG_f.TPROG     = Tprog * u8FreqCfg;
    FLASH_BYPASS();
    M0P_FLASH->TSERASE_f.TSERASE = Tserase * u8FreqCfg;
    FLASH_BYPASS();
    M0P_FLASH->TMERASE_f.TMERASE = Tmerase * u8FreqCfg;
    FLASH_BYPASS();
    M0P_FLASH->TPRCV_f.TPRCV     = Tprcv * u8FreqCfg;
    FLASH_BYPASS();
    M0P_FLASH->TSRCV_f.TSRCV     = Tsrcv * u8FreqCfg;
    FLASH_BYPASS();
    M0P_FLASH->TMRCV_f.TMRCV     = Tmrcv * u8FreqCfg; 
    
    //开启读FLASH等待周期
    if (8 == u8FreqCfg)
    {
        FLASH_BYPASS();
        M0P_FLASH->CR_f.WAIT = 0x01;
    }
    else if(12 == u8FreqCfg)
    {
        FLASH_BYPASS();
        M0P_FLASH->CR_f.WAIT = 0x02;
    }
    else
    {
        FLASH_BYPASS();    
        M0P_FLASH->CR_f.WAIT = 0x00;
    }
      
    pfnFlashCallback = pfnFlashCb;
    
    return (enResult);
}

/**
 *****************************************************************************
 ** \brief FLASH 字节�?
 **
 ** 用于向FLASH写入1字节数据.
 **
 ** \param [in]  u32Addr          Flash地址
 ** \param [in]  u8Data           1字节数据
 ** 
 ** \retval Ok                    写入成功.
 ** \retval ErrorInvalidParameter FLASH地址无效                                
 *****************************************************************************/
en_result_t Flash_WriteByte(uint32_t u32Addr, uint8_t u8Data)
{
    en_result_t             enResult = Ok;    
    
    if (FLASH_END_ADDR < u32Addr)
    {
        enResult = ErrorInvalidParameter;
        return (enResult);
    }
   
    //unlock flash
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK = (uint32_t)UnlockAll;
    
    //busy?
    while (TRUE == M0P_FLASH->CR_f.BUSY)
    {
        ;
    }
    
    //set OP
    FLASH_BYPASS();
    M0P_FLASH->CR_f.OP = Program;
    
    //write data
    *((volatile uint8_t*)u32Addr) = u8Data;
    
    //busy?
    while (TRUE == M0P_FLASH->CR_f.BUSY)
    {
        ;
    }
    
    //lock flash
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK = (uint32_t)LockAll;
    
    return (enResult);
}

/**
 *****************************************************************************
 ** \brief FLASH 半字�?
 **
 ** 用于向FLASH写入半字�?字节）数�?
 **
 ** \param [in]  u32Addr         Flash地址
 ** \param [in]  u16Data        半字�?字节）数�?
 ** 
 ** \retval Ok                    写入成功.
 ** \retval ErrorInvalidParameter FLASH地址无效                                
 *****************************************************************************/
en_result_t Flash_WriteHalfWord(uint32_t u32Addr, uint16_t u16Data)
{
    en_result_t             enResult = Ok;    
    
    if ((FLASH_END_ADDR < u32Addr) || (u32Addr % 2))
    {
        enResult = ErrorInvalidParameter;
        return (enResult);
    }
   
    //unlock flash
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK = (uint32_t)UnlockAll;
    
    //busy?
    while (TRUE == M0P_FLASH->CR_f.BUSY)
    {
        ;
    }
    
    //set OP
    FLASH_BYPASS();
    M0P_FLASH->CR_f.OP = Program;
    
    //write data
    *((volatile uint16_t*)u32Addr) = u16Data;
    
    //busy?
    while (TRUE == M0P_FLASH->CR_f.BUSY)
    {
        ;
    }
    
    //lock flash
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK = (uint32_t)LockAll;
    
    return (enResult);
}

/**
 *****************************************************************************
 ** \brief FLASH 字写
 **
 ** 用于向FLASH写入1个字的数�?
 **
 ** \param [in]  u32Addr         Flash地址
 ** \param [in]  u32Data         1个字数据
 ** 
 ** \retval Ok                    写入成功.
 ** \retval ErrorInvalidParameter FLASH地址无效                                
 *****************************************************************************/
en_result_t Flash_WriteWord(uint32_t u32Addr, uint32_t u32Data)
{
    en_result_t             enResult = Ok;    
    
    if ((FLASH_END_ADDR < u32Addr) || (u32Addr % 4))
    {
        enResult = ErrorInvalidParameter;
        return (enResult);
    }
   
    //unlock flash
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK = (uint32_t)UnlockAll;
    
    //busy?
    while (TRUE == M0P_FLASH->CR_f.BUSY)
    {
        ;
    }
    
    //set OP
    FLASH_BYPASS();
    M0P_FLASH->CR_f.OP = Program;
    
    //write data
    *((volatile uint32_t*)u32Addr) = u32Data;
    
    //busy?
    while (TRUE == M0P_FLASH->CR_f.BUSY)
    {
        ;
    }
    
    //lock flash
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK = LockAll;
    
    return (enResult);
}

/**
 *****************************************************************************
 ** \brief FLASH 扇区擦除
 **
 ** FLASH 扇区擦除.
 **
 ** \param [in]  u32SectorAddr    所擦除扇区内的地址
 ** 
 ** \retval Ok                    擦除成功.
 ** \retval ErrorInvalidParameter FLASH地址无效                                
 *****************************************************************************/
en_result_t Flash_SectorErase(uint32_t u32SectorAddr)
{

    en_result_t             enResult = Ok;    
    
    if (FLASH_END_ADDR < u32SectorAddr)
    {
        enResult = ErrorInvalidParameter;
        return (enResult);
    }
   
    //unlock flash
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK = (uint32_t)UnlockAll;
    
    //busy?
    while (TRUE == M0P_FLASH->CR_f.BUSY)
    {
        ;
    }
    
    //set OP
    FLASH_BYPASS();
    M0P_FLASH->CR_f.OP = SectorErase;
    
    //write data
    *((volatile uint8_t*)u32SectorAddr) = 0;
    
    //busy?
    while (TRUE == M0P_FLASH->CR_f.BUSY)
    {
        ;
    }
    
    //lock flash
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK = LockAll;
    
    return (enResult);
}

/**
 *****************************************************************************
 ** \brief FLASH 全片擦除
 **
 ** FLASH 全片擦除.
 **
 ** 
 ** \retval Ok              擦除成功.
 **                              
 *****************************************************************************/
en_result_t Flash_ChipErase(void)
{

    en_result_t             enResult = Ok;    
   
    //unlock flash
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK = (uint32_t)UnlockAll;
    
    //busy?
    while (TRUE == M0P_FLASH->CR_f.BUSY)
    {
        ;
    }
    
    //set OP
    FLASH_BYPASS();
    M0P_FLASH->CR_f.OP = ChipErase;
    
    //write data
    *((volatile uint8_t*)0) = 0;
    
    //busy?
    while (TRUE == M0P_FLASH->CR_f.BUSY)
    {
        ;
    }
    
    //lock flash
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK = LockAll;
    
    return (enResult);
}

/**
 *****************************************************************************
 ** \brief FLASH 编程保护加锁
 **
 ** \param [in]  enFlashSector    加锁范围选择枚举
 ** 
 ** \retval Ok                    加锁成功
 ** \retval ErrorInvalidParameter 参数错误                                
 *****************************************************************************/
en_result_t Flash_Lock(en_flash_sector_lock_t enFlashSector)
{
    en_result_t             enResult = Ok;    
   
    FLASH_BYPASS();
     M0P_FLASH->SLOCK_f.SLOCK &= ~(uint32_t)enFlashSector;

    return enResult;
}

/**
 *****************************************************************************
 ** \brief FLASH 编程保护解锁
 **
 ** \param [in]  enFlashSector    解锁范围选择枚举
 ** 
 ** \retval Ok                    解锁成功
 ** \retval ErrorInvalidParameter 参数错误                                
 *****************************************************************************/
en_result_t Flash_Unlock(en_flash_sector_lock_t enFlashSector)
{
    en_result_t             enResult = Ok;    
   
    FLASH_BYPASS();
    M0P_FLASH->SLOCK_f.SLOCK |= (uint32_t)enFlashSector;

    return enResult;
}

/**
 *****************************************************************************
 ** \brief FLASH 读等待周期设�?
 **
 ** \param [in]  enWaitCycle  插入FLASH读等待周期数枚举类型
 ** 
 ** \retval Ok                    解锁成功
 ** \retval ErrorInvalidParameter 参数错误                                
 *****************************************************************************/
en_result_t Flash_WaitCycle(en_flash_waitcycle_t enWaitCycle)
{
    en_result_t enResult = Ok;
    
    //插入FLASH读等待周�?
    M0P_SYSCTRL->PERI_CLKEN_f.FLASH = 1;
    M0P_FLASH->BYPASS_f.BYSEQ = 0x5A5A;
    M0P_FLASH->BYPASS_f.BYSEQ = 0xA5A5;
    if (0 == enWaitCycle)
    {
        M0P_FLASH->CR_f.WAIT = 0;
    }
    else if(1 == enWaitCycle)
    {
        M0P_FLASH->CR_f.WAIT = 1;
    }
    else
    {
        M0P_FLASH->CR_f.WAIT = 2;
    }
    
    return enResult;
}


//@} // FlashGroup

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
