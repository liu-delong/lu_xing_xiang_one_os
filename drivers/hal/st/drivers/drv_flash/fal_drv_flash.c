#ifdef OS_USING_FAL

#include <fal/fal.h>

static int fal_flash_read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    int count = stm32_flash_read(STM32_FLASH_START_ADRESS + page_addr * STM32_FLASH_PAGE_SIZE, buff, page_nr * STM32_FLASH_PAGE_SIZE);

    return (count == page_nr * STM32_FLASH_PAGE_SIZE) ? 0 : -1;
}

static int fal_flash_write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    int count = stm32_flash_write(STM32_FLASH_START_ADRESS + page_addr * STM32_FLASH_PAGE_SIZE, buff, page_nr * STM32_FLASH_PAGE_SIZE);

    return (count == page_nr * STM32_FLASH_PAGE_SIZE) ? 0 : -1;
}

static int fal_flash_erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int count =  stm32_flash_erase(STM32_FLASH_START_ADRESS + page_addr * STM32_FLASH_PAGE_SIZE, page_nr * STM32_FLASH_PAGE_SIZE);

    return (count == page_nr * STM32_FLASH_PAGE_SIZE) ? 0 : -1;
}

FAL_FLASH_DEFINE stm32_onchip_flash =
{
    .name = "onchip_flash",
    .capacity   = STM32_FLASH_SIZE,
    .block_size = STM32_FLASH_BLOCK_SIZE,
    .page_size  = STM32_FLASH_PAGE_SIZE,
    .ops  =
    {
        .init        = NULL,
        .read_page   = fal_flash_read,
        .write_page  = fal_flash_write,
        .erase_block = fal_flash_erase,
    },
};

#endif

