#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

#include <board.h>

#ifdef __cplusplus
extern "C" {
#endif

int hc32_flash_read(os_uint32_t addr, os_uint8_t *buf, size_t size);
int hc32_flash_write(os_uint32_t addr, const uint8_t *buf, size_t size);
int hc32_flash_erase(os_uint32_t addr, size_t size);

#ifdef __cplusplus
}
#endif

#endif  /* __DRV_FLASH_H__ */
