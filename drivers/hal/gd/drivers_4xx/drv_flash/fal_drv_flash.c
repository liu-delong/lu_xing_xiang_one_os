#ifdef OS_USING_FAL

#include <fal/fal.h>

static int fal_flash_read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    int count = gd32_flash_read(GD32_FLASH_START_ADRESS + page_addr * GD32_FLASH_PAGE_SIZE, buff, page_nr * GD32_FLASH_PAGE_SIZE);

    return (count == page_nr * GD32_FLASH_PAGE_SIZE) ? 0 : -1;
}

static int fal_flash_write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    int count = gd32_flash_write(GD32_FLASH_START_ADRESS + page_addr * GD32_FLASH_PAGE_SIZE, buff, page_nr * GD32_FLASH_PAGE_SIZE);

    return (count == page_nr * GD32_FLASH_PAGE_SIZE) ? 0 : -1;
}

static int fal_flash_erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int count =  gd32_flash_erase(GD32_FLASH_START_ADRESS + page_addr * GD32_FLASH_PAGE_SIZE, page_nr * GD32_FLASH_PAGE_SIZE);

    return (count == page_nr * GD32_FLASH_PAGE_SIZE) ? 0 : -1;
}

FAL_FLASH_DEFINE gd32_onchip_flash =
{
    .name = "onchip_flash",
    .capacity   = GD32_FLASH_SIZE,
    .block_size = GD32_FLASH_BLOCK_SIZE,
    .page_size  = GD32_FLASH_PAGE_SIZE,
    .ops  =
    {
        .init        = NULL,
        .read_page   = fal_flash_read,
        .write_page  = fal_flash_write,
        .erase_block = fal_flash_erase,
    },
};

#endif

