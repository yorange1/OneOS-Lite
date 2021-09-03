/**
* @copyright Copyright (c) 2018 CMCC-CMIOT-SZ
*
* @file: app_drv_spi_flash.c
* @brief:
* @version: 0.1
*
* @author: huzhou (huzhou@cmiot.chinamobile.com)
* @date: 2019-2-19
* @history:
* version  date | author | content
* 0.1 |2019.2.19 | huzhou | init
*
*
*
*/
#ifndef APP_DRV_SPI_FLASH_H_
#define APP_DRV_SPI_FLASH_H_
#include <stdint.h>
#include <stdbool.h>
typedef enum em_spi_flash_error
{
    SPI_FLASH_ERROR_OK = 0,
    SPI_FLASH_ERROR_INIT,
    SPI_FLASH_ERROR_READ_FAILED,
    SPI_FLASH_ERROR_WRITE_FAILED,
    SPI_FLASH_ERROR_ERASE_SECTOR_FAILED,
    SPI_FLASH_ERROR_OPERA_BUSY,
    SPI_FLASH_ERROR_MALLOC_TX_BUFFER_FAILED,
    SPI_FLASH_ERROR_CHIP_ID_READ_FAILED,
    SPI_FLASH_ERROR_WRITE_DATA, //after write data to read it error
} em_spi_flash_error_t;

/**
* @brief: the function for device chip id read
*
* @param:
* @return:
* @note:tx_buf value 000000H/000001H just the manufacturer id is not in the same order as the device id
*/
uint32_t app_drv_spi_flash_chip_id_read(uint8_t *pbuf);
/**
* @brief: the function for erase page
*
* @param: page_number
* @return:
* @note:only be used on the flash that supports page erase
*/
uint32_t app_drv_spi_flash_erase_page(uint16_t page_number);
/**
* @brief: the function for erase sector
*
* @param: sector_number
* @return:
* @note:The minimum erasure for GD25Q16 unit is sector 4K
	addr:block:sector:page 24bit
*/
uint32_t app_drv_spi_flash_erase_sector(uint16_t sector_number);
/**
* @brief: the function for write one page
*
* @param: pbuffer
* @return:
* @note:
*/
uint32_t app_drv_spi_flash_write_page(const uint8_t *pbuffer, uint32_t write_addr, uint16_t data_len);
/**
* @brief: the function for write spi flash data
*
* @param:
* @return:
* @note:
*/
uint32_t app_drv_spi_flash_write(const uint8_t *pbuffer, uint32_t write_addr, uint32_t data_len);
/**
* @brief: the function for read spi flash data
*
* @param: pbuffer:the read data buffer
* @param: read_addr:flash read address
* @return:
* @note:
*/
uint32_t app_drv_spi_flash_read(uint8_t *pbuffer, uint32_t read_addr, uint32_t read_bytes_num);
/**
* @brief: the function for spi init
*
* @param:
* @return:
* @note:
*/
void app_drv_spi_init(void);
/**
* @brief: the function for spi uninit
*
* @param:
* @return:
* @note:
*/
void app_drv_spi_uninit(void);
/**
* @brief: the function for spi uninit
*
* @param:
* @retval true     spi was already initialized.
* @retval false    spi was uninitialized
* @note:
*/
bool app_drv_spi_is_init(void);

/**
* @brief: the function for spi test
*
* @param:
* @return:
* @note:
*/
void app_drv_spi_test(void);

#endif
