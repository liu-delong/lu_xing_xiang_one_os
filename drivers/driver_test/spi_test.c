
#include <stdint.h>
#include <oneos_config.h>
#include <spi.h>
#include <drv_spi.h>
#include <drv_gpio.h>

#ifdef OS_USING_SHELL
#include <drv_log.h>
#include <shell.h>
#endif

#define AT45DBXX_BUS_NAME "at45dbxx_spi"

#define AT45DB_CMD_JEDEC_ID                  0x9F

struct JEDEC_ID
{
    uint8_t manufacturer_id;  /* Manufacturer ID */
    uint8_t density_code : 5; /* Density Code */
    uint8_t family_code : 3;  /* Family Code */
    uint8_t version_code : 5; /* Product Version Code */
    uint8_t mlc_code : 3;     /* MLC Code */
    uint8_t byte_count;       /* Byte Count */
};

#define BSP_AT45DBXX_SPI_CS GET_PIN(B, 12)

void spi_test(void)
{
    const char *spi_device_name = "spi1";

    struct os_spi_device *os_spi_device;
    struct JEDEC_ID      *JEDEC_ID;

    os_hw_spi_device_attach(spi_device_name, AT45DBXX_BUS_NAME, BSP_AT45DBXX_SPI_CS);

    os_spi_device = (struct os_spi_device *)os_device_find(AT45DBXX_BUS_NAME);
    if (os_spi_device == OS_NULL)
    {
        LOG_EXT_E("spi device %s not found!\r\n", AT45DBXX_BUS_NAME);
        return ;
    }

    /* config spi */
    {
        struct os_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode       = OS_SPI_MODE_0 | OS_SPI_MSB; /* SPI Compatible Modes 0 and 3 */
        cfg.max_hz     = 6000000;                   /* Atmel RapidS Serial Interface: 66MHz Maximum Clock Frequency */
        os_spi_configure(os_spi_device, &cfg);
    }

    /* read JEDEC ID */
        uint8_t cmd;
        uint8_t id_recv[6];
        JEDEC_ID = (struct JEDEC_ID *)id_recv;

        cmd = AT45DB_CMD_JEDEC_ID;
        os_spi_send_then_recv(os_spi_device, &cmd, 1, id_recv, 6);
        if (JEDEC_ID->manufacturer_id != 0x1F || JEDEC_ID->family_code != 0x01)
        {
            LOG_EXT_E("Manufacturer ID or Memory Type error!\r\n");
            LOG_EXT_E("JEDEC Read-ID Data : %02X %02X %02X\r\n", id_recv[0], id_recv[1], id_recv[2]);
            return;
        }

}
    
SH_CMD_EXPORT(spi_test, spi_test, "spi_test");

