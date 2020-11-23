#ifdef OS_USING_FAL

#include <fal/fal.h>

static int fal_flash_init(fal_flash_t *flash)
{
    os_uint32_t status;
    
    LOG_EXT_I("lpc55s69 onchip flash need config in 96/12Mhz to use! \r\n");
    
    status = FLASH_Init(&nxp_flashConfig);
    if (status != kStatus_Success)
    {
        LOG_EXT_E("onchip_flash init failed! \r\n");
    }
    LOG_EXT_I("onchip_flash init success! \r\n");
        
    return 0;
}

static int fal_flash_read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    int count = nxp_flash_read(NXP_FLASH_START_ADRESS + page_addr * NXP_FLASH_PAGE_SIZE, buff, page_nr * NXP_FLASH_PAGE_SIZE);

    return (count == page_nr * NXP_FLASH_PAGE_SIZE) ? 0 : -1;
}

static int fal_flash_write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    int count = nxp_flash_write(NXP_FLASH_START_ADRESS + page_addr * NXP_FLASH_PAGE_SIZE, buff, page_nr * NXP_FLASH_PAGE_SIZE);

    return (count == page_nr * NXP_FLASH_PAGE_SIZE) ? 0 : -1;
}

static int fal_flash_erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int count =  nxp_flash_erase(NXP_FLASH_START_ADRESS + page_addr * NXP_FLASH_PAGE_SIZE, page_nr * NXP_FLASH_PAGE_SIZE);

    return (count == page_nr * NXP_FLASH_PAGE_SIZE) ? 0 : -1;
}

FAL_FLASH_DEFINE nxp_onchip_flash =
{
    .name = "onchip_flash",
    .capacity   = NXP_FLASH_SIZE,
    .block_size = NXP_FLASH_BLOCK_SIZE,
    .page_size  = NXP_FLASH_PAGE_SIZE,
    .ops  =
    {
        .init        = fal_flash_init,
        .read_page   = fal_flash_read,
        .write_page  = fal_flash_write,
        .erase_block = fal_flash_erase,
    },
};

#endif

