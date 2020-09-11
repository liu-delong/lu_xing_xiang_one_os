#include <board.h>
#include <drv_cfg.h>
#include <os_hw.h>
#include <os_memory.h>
#include <drv_log.h>
#include <string.h>
#include <stdlib.h>
#include <fal/fal.h>

#define AT24CXX_EEPROM_SIZE (256)

struct os_i2c_client *at24cxx_i2c;

static int fal_at24cxx_init(fal_flash_t *flash)
{
    at24cxx_i2c = os_calloc(1, sizeof(struct os_i2c_client));
    if (at24cxx_i2c == OS_NULL)
    {
        LOG_EXT_E("AT24CXX i2c malloc failed.");
        return -1;
    }

    at24cxx_i2c->bus = (struct os_i2c_bus_device *)os_device_find(BSP_AT24CXX_I2C_BUS_NAME);
    if (at24cxx_i2c->bus == OS_NULL)
    {
        LOG_EXT_E("AT24CXX i2c invalid.");
        return -2;
    }

    at24cxx_i2c->client_addr = BSP_AT24CXX_I2C_ADDR;

    return 0;
}

static int fal_at24cxx_read_page(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    os_uint32_t offset = page_addr * flash->page_size;
    os_uint32_t size   = page_nr * flash->page_size;

    return os_i2c_client_read(at24cxx_i2c, offset, 1, buff, size);
}

static int fal_at24cxx_write_page(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    int ret;

    os_uint32_t offset = page_addr * flash->page_size;
    os_uint32_t size   = page_nr * flash->page_size;

    while (size > 0)
    {
        ret = os_i2c_client_write(at24cxx_i2c, offset, 1, (os_uint8_t *)buff, flash->page_size);

        if (ret != OS_EOK)
            return OS_EIO;
        
        offset += flash->page_size;
        buff   += flash->page_size;
        size   -= flash->page_size;

        os_task_msleep(5);
    }

    return OS_EOK;
}

static int fal_at24cxx_erase_block(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int i, ret;
    os_uint8_t buff[8];

    memset(buff, 0xff, sizeof(buff));

    for (i = 0; i < page_nr; i++, page_addr++)
    {
        ret = fal_at24cxx_write_page(flash, page_addr, buff, 1);

        if (ret != OS_EOK)
            return OS_EIO;
    }

    return OS_EOK;
}

FAL_FLASH_DEFINE at24cxx_eeprom =
{
    .name = "at24cxx_eeprom",
    .capacity   = AT24CXX_EEPROM_SIZE,
    .block_size = 8,
    .page_size  = 8,
    .ops  =
    {
        .init        = fal_at24cxx_init,
        .read_page   = fal_at24cxx_read_page,
        .write_page  = fal_at24cxx_write_page,
        .erase_block = fal_at24cxx_erase_block,
    },
};

