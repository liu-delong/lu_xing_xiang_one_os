#include <string.h>
#include <fal/fal.h>
#include <os_memory.h>

static char *fal_ram_base;

static int fal_ram_init(fal_flash_t *flash)
{
    fal_ram_base = os_malloc(OS_FAL_RAM_SIZE);
    OS_ASSERT(fal_ram_base != OS_NULL);

    memset(fal_ram_base, 0xff, OS_FAL_RAM_SIZE);
    
    return 0;
}

static int fal_ram_read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    memcpy(buff, fal_ram_base + page_addr, page_nr);

    return 0;
}

static int fal_ram_write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    memcpy(fal_ram_base + page_addr, buff, page_nr);

    return 0;
}

static int fal_ram_erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    memset(fal_ram_base + page_addr, 0xff, page_nr);

    return 0;
}

FAL_FLASH_DEFINE fal_ram =
{
    .name = "ram_flash",
    .capacity   = OS_FAL_RAM_SIZE,
    .block_size = 1,
    .page_size  = 1,
    .ops  =
    {
        .init        = fal_ram_init,
        .read_page   = fal_ram_read,
        .write_page  = fal_ram_write,
        .erase_block = fal_ram_erase,
    },
};

