#ifdef OS_USING_FAL

#include <fal/fal.h>
#include "esp_partition.h"


static esp_partition_t *esp32_fal_partition = OS_NULL;

static int esp32_flash_read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    int ret = 0;
    if(!esp32_fal_partition)
        return OS_ERROR;

    ret = esp_partition_read(esp32_fal_partition, page_addr * ESP32_FLASH_PAGE_SIZE, buff, page_nr * ESP32_FLASH_PAGE_SIZE);
    if(ret)
        return OS_ERROR;
    else
        return OS_EOK;
}

static int esp32_flash_write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    int ret = 0;
    if(!esp32_fal_partition)
        return OS_ERROR;

    ret = esp_partition_write(esp32_fal_partition, page_addr * ESP32_FLASH_PAGE_SIZE, buff, page_nr * ESP32_FLASH_PAGE_SIZE);
    if(ret)
        return OS_ERROR;
    else
        return OS_EOK;
}

static int esp32_flash_erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int ret = 0;

    if(!esp32_fal_partition)
        return OS_ERROR;

    ret = esp_partition_erase_range(esp32_fal_partition, page_addr * ESP32_FLASH_PAGE_SIZE, page_nr * STM32_FLASH_PAGE_SIZE);
    if(ret)
        return OS_ERROR;
    else
        return OS_EOK;
}

static int esp32_fal_flash_init(fal_flash_t *flash)
{
    esp32_fal_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    if(!esp32_fal_partition)
        return OS_ERROR;

    return OS_EOK;
}

FAL_FLASH_DEFINE esp32_onchip_flash =
{
    .name = "onchip_flash",
    .capacity   = ESP32_FLASH_SIZE,
    .block_size = ESP32_FLASH_BLOCK_SIZE,
    .page_size  = ESP32_FLASH_PAGE_SIZE,
    .ops  =
    {
        .init        = esp32_fal_flash_init,
        .read_page   = esp32_flash_read,
        .write_page  = esp32_flash_write,
        .erase_block = esp32_flash_erase,
    },
};

#endif

