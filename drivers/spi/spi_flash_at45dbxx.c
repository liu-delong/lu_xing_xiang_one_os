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
 * @file        spi_flash_at45dbxx.c
 *
 * @brief       this file implements spi flash at45dbxx related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include "spi_flash_at45dbxx.h"
#include <fal.h>
#include <oneos_config.h>
#include <string.h>
#include <fal_cfg.h>
#include <os_memory.h>

#ifdef OS_USING_SHELL
#include <drv_log.h>
#include <shell.h>
#endif

#define AT45DBXX_BUS_NAME "at45dbxx_spi"
#define AT45DBXX_NAME     "at45dbxx"

/* JEDEC Manufacturer’s ID */
#define MF_ID             (0x1F) /* atmel */
#define DENSITY_CODE_011D (0x02) /* AT45DB011D Density Code : 00010 = 1-Mbit */
#define DENSITY_CODE_021D (0x03) /* AT45DB021D Density Code : 00011 = 2-Mbit */
#define DENSITY_CODE_041D (0x04) /* AT45DB041D Density Code : 00100 = 4-Mbit */
#define DENSITY_CODE_081D (0x05) /* AT45DB081D Density Code : 00101 = 8-Mbit */
#define DENSITY_CODE_161D (0x06) /* AT45DB161D Density Code : 00110 = 16-Mbit */
#define DENSITY_CODE_321D (0x07) /* AT45DB321D Density Code : 00111 = 32-Mbit */
#define DENSITY_CODE_642D (0x08) /* AT45DB642D Density Code : 01000 = 64-Mbit */

struct JEDEC_ID
{
    uint8_t manufacturer_id;  /* Manufacturer ID */
    uint8_t density_code : 5; /* Density Code */
    uint8_t family_code : 3;  /* Family Code */
    uint8_t version_code : 5; /* Product Version Code */
    uint8_t mlc_code : 3;     /* MLC Code */
    uint8_t byte_count;       /* Byte Count */
};

#define AT45DB_BUFFER_1_WRITE                0x84
#define AT45DB_BUFFER_2_WRITE                0x87
#define AT45DB_BUFFER_1_READ                 0xD4
#define AT45DB_BUFFER_2_READ                 0xD6
#define AT45DB_B1_TO_MM_PAGE_PROG_WITH_ERASE 0x83
#define AT45DB_B2_TO_MM_PAGE_PROG_WITH_ERASE 0x86
#define AT45DB_MM_PAGE_TO_B1_XFER            0x53
#define AT45DB_MM_PAGE_TO_B2_XFER            0x55
#define AT45DB_PAGE_ERASE                    0x81
#define AT45DB_SECTOR_ERASE                  0x7C
#define AT45DB_READ_STATE_REGISTER           0xD7
#define AT45DB_MM_PAGE_READ                  0xD2
#define AT45DB_MM_PAGE_PROG_THRU_BUFFER1     0x82
#define AT45DB_CMD_JEDEC_ID                  0x9F

static struct spi_flash_at45dbxx spi_flash_at45dbxx;
os_device_t *at45dbxx_dev = OS_NULL;

static uint8_t at45db_statusregister_read(void)
{
    return os_spi_sendrecv8(spi_flash_at45dbxx.os_spi_device, AT45DB_READ_STATE_REGISTER);
}

static void wait_busy(void)
{
    uint16_t i = 0;
    while (i++ < 10000)
    {
        if (at45db_statusregister_read() & 0x80)
        {
            return;
        }
    }
    LOG_EXT_E("\r\nSPI_FLASH timeout!!!\r\n");
}

static os_err_t at45db_flash_init(os_device_t *dev)
{
    return OS_EOK;
}

static os_err_t at45db_flash_open(os_device_t *dev, os_uint16_t oflag)
{

    return OS_EOK;
}

static os_err_t at45db_flash_close(os_device_t *dev)
{
    return OS_EOK;
}

static os_err_t at45db_flash_control(os_device_t *dev, int cmd, void *args)
{
    OS_ASSERT(dev != OS_NULL);

    if (cmd == OS_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct os_device_blk_geometry *geometry;

        geometry = (struct os_device_blk_geometry *)args;
        if (geometry == OS_NULL)
            return OS_ERROR;

        geometry->bytes_per_sector = 512;
        geometry->sector_count     = 4096;
        geometry->block_size       = 4096; /* block erase: 4k */
    }

    return OS_EOK;
}

static os_size_t at45db_flash_read_page_256(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    uint32_t index, nr;
    uint8_t *read_buffer = buffer;
    uint32_t page        = pos;

    nr = size;

    for (index = 0; index < nr; index++)
    {
        uint8_t  send_buffer[8];
        uint32_t i;

        for (i = 0; i < sizeof(send_buffer); i++)
        {
            send_buffer[i] = 0;
        }

        send_buffer[0] = AT45DB_MM_PAGE_READ;
        send_buffer[1] = (uint8_t)(page >> 7);
        send_buffer[2] = (uint8_t)(page << 1);

        os_spi_send_then_recv(spi_flash_at45dbxx.os_spi_device, send_buffer, 8, read_buffer, 256);
        read_buffer += 256;
        page++;
    }

    return size;
}

static os_size_t at45db_flash_read_page_512(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    uint32_t index, nr;
    uint8_t *read_buffer = buffer;
    uint32_t page        = pos;

    nr = size;

    for (index = 0; index < nr; index++)
    {
        uint8_t  send_buffer[8];
        uint32_t i;

        for (i = 0; i < sizeof(send_buffer); i++)
        {
            send_buffer[i] = 0;
        }

        send_buffer[0] = AT45DB_MM_PAGE_READ;
        send_buffer[1] = (uint8_t)(page >> 6);
        send_buffer[2] = (uint8_t)(page << 2);

        os_spi_send_then_recv(spi_flash_at45dbxx.os_spi_device, send_buffer, 8, read_buffer, 512);
        read_buffer += 512;
        page++;
    }

    return size;
}

static os_size_t at45db_flash_read_page_1024(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    uint32_t index, nr;
    uint8_t *read_buffer = buffer;
    uint32_t page        = pos;

    nr = size;

    for (index = 0; index < nr; index++)
    {
        uint8_t  send_buffer[8];
        uint32_t i;

        for (i = 0; i < sizeof(send_buffer); i++)
        {
            send_buffer[i] = 0;
        }

        send_buffer[0] = AT45DB_MM_PAGE_READ;
        send_buffer[1] = (uint8_t)(page >> 5);
        send_buffer[2] = (uint8_t)(page << 3);

        os_spi_send_then_recv(spi_flash_at45dbxx.os_spi_device, send_buffer, 8, read_buffer, 1024);
        read_buffer += 1024;
        page++;
    }

    return size;
}

static os_size_t at45db_flash_write_page_256(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_uint32_t    index, nr;
    const uint8_t *write_buffer = buffer;
    uint32_t       page         = pos;

    nr = size;

    for (index = 0; index < nr; index++)
    {
        uint8_t send_buffer[4];

        send_buffer[0] = AT45DB_MM_PAGE_PROG_THRU_BUFFER1;
        send_buffer[1] = (uint8_t)(page >> 7);
        send_buffer[2] = (uint8_t)(page << 1);
        send_buffer[3] = 0;

        os_spi_send_then_send(spi_flash_at45dbxx.os_spi_device, send_buffer, 4, write_buffer, 256);

        write_buffer += 256;
        page++;

        wait_busy();
    }

    return size;
}

static os_size_t at45db_flash_write_page_512(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_uint32_t    index, nr;
    const uint8_t *write_buffer = buffer;
    uint32_t       page         = pos;

    nr = size;

    for (index = 0; index < nr; index++)
    {
        uint8_t send_buffer[4];

        send_buffer[0] = AT45DB_MM_PAGE_PROG_THRU_BUFFER1;
        send_buffer[1] = (uint8_t)(page >> 6);
        send_buffer[2] = (uint8_t)(page << 2);
        send_buffer[3] = 0;

        os_spi_send_then_send(spi_flash_at45dbxx.os_spi_device, send_buffer, 4, write_buffer, 512);

        write_buffer += 512;
        page++;

        wait_busy();
    }

    return size;
}

static os_size_t at45db_flash_write_page_1024(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_uint32_t    index, nr;
    const uint8_t *write_buffer = buffer;
    uint32_t       page         = pos;

    nr = size;

    for (index = 0; index < nr; index++)
    {
        uint8_t send_buffer[4];

        send_buffer[0] = AT45DB_MM_PAGE_PROG_THRU_BUFFER1;
        send_buffer[1] = (uint8_t)(page >> 5);
        send_buffer[2] = (uint8_t)(page << 3);
        send_buffer[3] = 0;

        os_spi_send_then_send(spi_flash_at45dbxx.os_spi_device, send_buffer, 4, write_buffer, 1024);

        write_buffer += 1024;
        page++;

        wait_busy();
    }

    return size;
}

static os_err_t at45dbxx_init(const char *flash_device_name)
{
    const char *spi_device_name = BSP_AT45DBXX_SPI_BUS;

    struct os_spi_device *os_spi_device;
    struct JEDEC_ID      *JEDEC_ID;
    struct os_device_ops *at45dbxx_ops;

    os_hw_spi_device_attach(spi_device_name, AT45DBXX_BUS_NAME, BSP_AT45DBXX_SPI_CS);

    os_spi_device = (struct os_spi_device *)os_device_find(AT45DBXX_BUS_NAME);
    if (os_spi_device == OS_NULL)
    {
        LOG_EXT_E("spi device %s not found!\r\n", AT45DBXX_BUS_NAME);
        return OS_ENOSYS;
    }
    spi_flash_at45dbxx.os_spi_device = os_spi_device;

    /* config spi */
    {
        struct os_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode       = OS_SPI_MODE_0 | OS_SPI_MSB; /* SPI Compatible Modes 0 and 3 */
        cfg.max_hz     = 66000000;                   /* Atmel RapidS Serial Interface: 66MHz Maximum Clock Frequency */
        os_spi_configure(spi_flash_at45dbxx.os_spi_device, &cfg);
    }

    /* read JEDEC ID */
    {
        uint8_t cmd;
        uint8_t id_recv[6];
        JEDEC_ID = (struct JEDEC_ID *)id_recv;

        cmd = AT45DB_CMD_JEDEC_ID;
        os_spi_send_then_recv(spi_flash_at45dbxx.os_spi_device, &cmd, 1, id_recv, 6);

        /**< 1FH = Atmel */
        /**< 001 = Atmel DataFlash */
        if (JEDEC_ID->manufacturer_id != 0x1F || JEDEC_ID->family_code != 0x01)
        {
            LOG_EXT_E("Manufacturer ID or Memory Type error!\r\n");
            LOG_EXT_E("JEDEC Read-ID Data : %02X %02X %02X\r\n", id_recv[0], id_recv[1], id_recv[2]);
            return OS_ENOSYS;
        }

        if (JEDEC_ID->density_code == DENSITY_CODE_011D)
        {
            /**< AT45DB011D Density Code : 00010 = 1-Mbit */
            LOG_EXT_D("AT45DB011D detection\r\n");
            spi_flash_at45dbxx.geometry.bytes_per_sector = 256;      /* Page Erase (256 Bytes) */
            spi_flash_at45dbxx.geometry.sector_count     = 512;      /* 1-Mbit / 8 / 256 = 512 */
            spi_flash_at45dbxx.geometry.block_size       = 1024 * 2; /* Block Erase (2-Kbytes) */
        }
        else if (JEDEC_ID->density_code == DENSITY_CODE_021D)
        {
            /**< AT45DB021D Density Code : 00011 = 2-Mbit */
            LOG_EXT_D("AT45DB021D detection\r\n");
            spi_flash_at45dbxx.geometry.bytes_per_sector = 256;      /* Page Erase (256 Bytes) */
            spi_flash_at45dbxx.geometry.sector_count     = 512 * 2;  /* 2-Mbit / 8 / 256 = 1024 */
            spi_flash_at45dbxx.geometry.block_size       = 1024 * 2; /* Block Erase (2-Kbytes) */
        }
        else if (JEDEC_ID->density_code == DENSITY_CODE_041D)
        {
            /**< AT45DB041D Density Code : 00100 = 4-Mbit */
            LOG_EXT_D("AT45DB041D detection\r\n");
            spi_flash_at45dbxx.geometry.bytes_per_sector = 256;      /* Page Erase (256 Bytes) */
            spi_flash_at45dbxx.geometry.sector_count     = 512 * 4;  /* 4-Mbit / 8 / 256 = 2048 */
            spi_flash_at45dbxx.geometry.block_size       = 1024 * 2; /* Block Erase (2-Kbytes) */
        }
        else if (JEDEC_ID->density_code == DENSITY_CODE_081D)
        {
            /**< AT45DB081D Density Code : 00101 = 8-Mbit */
            LOG_EXT_D("AT45DB081D detection\r\n");
            spi_flash_at45dbxx.geometry.bytes_per_sector = 256;      /* Page Erase (256 Bytes) */
            spi_flash_at45dbxx.geometry.sector_count     = 512 * 8;  /* 8-Mbit / 8 / 256 = 4096 */
            spi_flash_at45dbxx.geometry.block_size       = 1024 * 2; /* Block Erase (2-Kbytes) */
        }
        else if (JEDEC_ID->density_code == DENSITY_CODE_161D)
        {
            /**< AT45DB161D Density Code : 00110 = 16-Mbit */
            LOG_EXT_D("AT45DB161D detection\r\n");
            spi_flash_at45dbxx.geometry.bytes_per_sector = 512;      /* Page Erase (512 Bytes) */
            spi_flash_at45dbxx.geometry.sector_count     = 256 * 16; /* 16-Mbit / 8 / 512 = 4096 */
            spi_flash_at45dbxx.geometry.block_size       = 1024 * 4; /* Block Erase (4-Kbytes) */
        }
        else if (JEDEC_ID->density_code == DENSITY_CODE_321D)
        {
            /**< AT45DB321D Density Code : 00111 = 32-Mbit */
            LOG_EXT_D("AT45DB321D detection\r\n");
            spi_flash_at45dbxx.geometry.bytes_per_sector = 512;      /* Page Erase (512 Bytes) */
            spi_flash_at45dbxx.geometry.sector_count     = 256 * 32; /* 32-Mbit / 8 / 512 = 8192 */
            spi_flash_at45dbxx.geometry.block_size       = 1024 * 4; /* Block Erase (4-Kbytes) */
        }
        else if (JEDEC_ID->density_code == DENSITY_CODE_642D)
        {
            /**< AT45DB642D Density Code : 01000 = 64-Mbit */
            LOG_EXT_D("AT45DB642D detection\r\n");
            spi_flash_at45dbxx.geometry.bytes_per_sector = 1024;     /* Page Erase (1 Kbyte) */
            spi_flash_at45dbxx.geometry.sector_count     = 128 * 64; /* 64-Mbit / 8 / 1024 = 8192 */
            spi_flash_at45dbxx.geometry.block_size       = 1024 * 8; /* Block Erase (8 Kbytes) */
        }
        else
        {
            LOG_EXT_E("Memory Capacity error!\r\n");
            return OS_ENOSYS;
        }
    }

    /* register device */	
	at45dbxx_ops = os_calloc(1, sizeof(struct os_device_ops));

	if (at45dbxx_ops == OS_NULL)
	{
		LOG_EXT_E("os_calloc failed!\r\n");
		return OS_ENOMEM;
	}
	
    spi_flash_at45dbxx.flash_device.type    = OS_DEVICE_TYPE_SPIDEVICE;
	at45dbxx_ops->init = at45db_flash_init;
	at45dbxx_ops->open = at45db_flash_open;
	at45dbxx_ops->close = at45db_flash_close;
	at45dbxx_ops->control = at45db_flash_control;

    if (JEDEC_ID->density_code == DENSITY_CODE_642D)
    {
		at45dbxx_ops->read = at45db_flash_read_page_1024;
		at45dbxx_ops->write = at45db_flash_write_page_1024;
    }
    else if (JEDEC_ID->density_code == DENSITY_CODE_161D || JEDEC_ID->density_code == DENSITY_CODE_321D)
    {
		at45dbxx_ops->read = at45db_flash_read_page_512;
		at45dbxx_ops->write = at45db_flash_write_page_512;
    }
    else
    {
        at45dbxx_ops->read  = at45db_flash_read_page_256;
        at45dbxx_ops->write = at45db_flash_write_page_256;		
    }

	spi_flash_at45dbxx.flash_device.ops = at45dbxx_ops;

    /* no private */
    spi_flash_at45dbxx.flash_device.user_data = OS_NULL;

    os_device_register(&spi_flash_at45dbxx.flash_device,
                       flash_device_name,
                       OS_DEVICE_FLAG_RDWR | OS_DEVICE_FLAG_STANDALONE);

    return OS_EOK;
}

static int fal_at45dbxx_init(fal_flash_t *flash)
{
    const char *flash_device_name = AT45DBXX_NAME;

    if (at45dbxx_init(flash_device_name) != OS_EOK)
    {
        LOG_EXT_E("%s init failed!\r\n",flash_device_name);
    }
    else
    {
        at45dbxx_dev = os_device_find(flash_device_name);
        OS_ASSERT(at45dbxx_dev != NULL);

        os_device_open(at45dbxx_dev, OS_DEVICE_FLAG_RDWR);
    }
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           read flash
 *
 * @attention       only be read by sector. ensure the size of the written data to prevent data loss.
 *                  use to test read(only one sector).
 *
 * @param[in]       offset          offset address
 * @param[in]       buf             pointer of data to write
 * @param[in]       size            size
 *
 * @return          Return_result_description
 * @retval          size            size
 ***********************************************************************************************************************
 */
static int at45dbxx_read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    uint32_t    page       = (page_addr * MCU_SPI_FLASH_SECTOR_SIZE) / spi_flash_at45dbxx.geometry.bytes_per_sector;
    os_uint16_t sector_num = (page_nr * MCU_SPI_FLASH_SECTOR_SIZE - 1) / spi_flash_at45dbxx.geometry.bytes_per_sector + 1;

    os_device_read(at45dbxx_dev, page, buff, sector_num);

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           write flash
 *
 * @attention       only be written by sector. ensure the size of the written data to prevent data loss.
 *                  use to test write(only one sector),so use a data buffer to prevent data out of bounds.
 *
 * @param[in]       offset          offset address
 * @param[in]       buf             pointer of data to write
 * @param[in]       size            size
 *
 * @return          Return_result_description
 * @retval          size            size
 ***********************************************************************************************************************
 */
static int at45dbxx_write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    os_uint16_t intex        = 0;
    os_uint16_t count        = 0;
    os_uint8_t *write_buffer = OS_NULL;
    os_uint32_t page         = (page_addr * MCU_SPI_FLASH_SECTOR_SIZE) / spi_flash_at45dbxx.geometry.bytes_per_sector;
    os_uint16_t sector_num   = (page_nr * MCU_SPI_FLASH_SECTOR_SIZE - 1) / spi_flash_at45dbxx.geometry.bytes_per_sector + 1;

    write_buffer = calloc(1, spi_flash_at45dbxx.geometry.bytes_per_sector);
    memset(write_buffer, 0xFF, spi_flash_at45dbxx.geometry.bytes_per_sector);

    for (intex = 0; intex < sector_num; intex++)
    {
        for (count = 0; count < page_nr * MCU_SPI_FLASH_SECTOR_SIZE; count++)
        {
            write_buffer[count] = buff[count];
        }
        os_device_write(at45dbxx_dev, page + intex, write_buffer, 1);
    }

    free(write_buffer);
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           erase flash
 *
 * @attention       this function is not recommended. The chip will be automatically erased when writing.
 *                  use to test write(only one sector).
 *
 * @param[in]       offset          offset address
 * @param[in]       size            size
 *
 * @return          Return_result_description
 * @retval          size            size
 ***********************************************************************************************************************
 */
static int at45dbxx_erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    os_uint16_t intex        = 0;
    os_uint8_t *write_buffer = OS_NULL;
    os_uint32_t page         = (page_addr * MCU_SPI_FLASH_SECTOR_SIZE) / spi_flash_at45dbxx.geometry.bytes_per_sector;
    os_uint16_t sector_num   = (page_nr * MCU_SPI_FLASH_SECTOR_SIZE - 1) / spi_flash_at45dbxx.geometry.bytes_per_sector + 1;

    write_buffer = calloc(1, spi_flash_at45dbxx.geometry.bytes_per_sector);
    memset(write_buffer, 0xFF, spi_flash_at45dbxx.geometry.bytes_per_sector);

    for (intex = 0; intex < sector_num; intex++)
    {
        os_device_write(at45dbxx_dev, page + intex, write_buffer, 1);
    }
    free(write_buffer);
    return 0;
}

FAL_FLASH_DEFINE sfud_flash0 =
{
    .name = OS_EXTERN_FLASH_NAME,
    .capacity   = MCU_SPI_FLASH_SIZE,
    .block_size = MCU_SPI_FLASH_SECTOR_SIZE,
    .page_size  = MCU_SPI_FLASH_SECTOR_SIZE,
    .ops  =
    {
        .init        = fal_at45dbxx_init,
        .read_page   = at45dbxx_read,
        .write_page  = at45dbxx_write,
        .erase_block = at45dbxx_erase,
    },
};

