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
//c lib include
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
//nordic sdk include
#include <nrf_drv_spi.h>
#include <nrfx_spim.h>
#include <nrf_delay.h>
#include <time.h>
#include <nrf_gpio.h>
//app include
#include "app_drv_spi_flash.h"
#include "gpio_common.h"
//#include "spi_flash_log.h"
#define SPI_INSTANCE 0 /**< SPI instance index. */

#define SPI_FLASH_FREE (true)
#define SPI_FLASH_BUSY (false)
static volatile bool sg_spi_trans_done; //the flag for transfer complete
static volatile bool sg_spi_is_free = SPI_FLASH_FREE;
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE); /**< SPI instance. */

#define FLASH_PAGE_SIZE 	(256)
#define FLASH_DATA_NULL 	(0x00)
#define FLASH_DATA_READ 	(0xFF)
#define FLASH_BLOCK_BIT 	(0x00ff0000)
#define FLASH_SECTOR_BIT 	(0x0000ff00)
#define FLASH_PAGE_BIT 		(0x000000ff)
#define NRF_FLASH_HEAD_LEN 	(4)

#define SPI_FLASH_OPERAT_PAGE_SIZE 	(255)
#define SPI_FLASH_RX_BUFF_SIZE 		(256)
#define SPIFLASH_WRITE_BUSYBIT 		(0x01)

#define SPIFLASH_CHIPID_READ_CMD 	(0x90)
#define SPIFLASH_READ_DATA_CMD 		(0x03)
#define SPIFLASH_WRITE_ENABLE_CMD 	(0x06)
#define SPIFLASH_WRITE_DISABLE_CMD 	(0x04)
#define SPIFLASH_PAGE_ERASE_CMD 	(0x81)
#define SPIFLASH_SECTOR_ERASE_CMD 	(0x20)
#define SPIFLASH_PAGE_PROGRAM_CMD 	(0x02)
#define SPIFLASH_READSR_CMD 		(0x05)
static uint8_t sg_spi_rx_buf[SPI_FLASH_RX_BUFF_SIZE]; /**< RX buffer. */

typedef struct st_spi_flash_ret_code
{
    em_spi_flash_error_t ret_code;
    uint8_t 			*trans_data;
} st_spi_flash_ret_code_t;

static inline bool spi_flash_trans_flag_get(void)
{
    return sg_spi_trans_done;
}
static inline void spi_flash_trans_flag_set(bool is_trans_done)
{
    sg_spi_trans_done = is_trans_done;
}
static inline bool spi_flash_is_free_get(void)
{
    return sg_spi_is_free;
}
static inline void spi_flash_is_free_set(bool is_free)
{
    sg_spi_is_free = is_free;
}

static inline uint8_t *spi_flash_rx_data_get(void)
{
    return sg_spi_rx_buf;
}
/**
* @brief: the function for spi event handler
*
* @param:
* @return:
* @note:
*/
static void spi_event_handler(nrf_drv_spi_evt_t const *p_event,
                              void *p_context)
{
    spi_flash_trans_flag_set(true);
}
/**
* @brief: the function for read one byte
*
* @param:
* @return:
* @note:
*/
static uint32_t spi_flash_read_one_byte(uint8_t *buf)
{
    uint8_t len = 1;
    uint32_t err_id;
    uint32_t ret_code = SPI_FLASH_ERROR_OK;
    uint8_t tx_buf = FLASH_DATA_READ;
    uint8_t *rx_buf = buf;

    spi_flash_trans_flag_set(false);
    if ((err_id = nrf_drv_spi_transfer(&spi, &tx_buf, len, rx_buf, len)) == NRFX_SUCCESS)
    {
        while (!spi_flash_trans_flag_get())
            ;
    }
    else
    {
        os_kprintf("spi flash read one byte error id:%d", err_id);
        ret_code = SPI_FLASH_ERROR_READ_FAILED;
    }
    return ret_code;
}

/**
* @brief: the function for write one byte
*
* @param: Dat:the data to write
* @return:
* @note: only for the write enable/disable
*/
static uint32_t spi_flash_write_one_byte(uint8_t dat)
{
    uint8_t len = 1;
    uint8_t tx_buf = dat;
    uint32_t err_id;
    uint32_t ret_code = SPI_FLASH_ERROR_OK;
    uint8_t *rx_buf = spi_flash_rx_data_get();

    memset(rx_buf, 0, SPI_FLASH_RX_BUFF_SIZE);
    spi_flash_trans_flag_set(false);
    //	SPI_PIN_CS_LOW;
    if ((err_id = nrf_drv_spi_transfer(&spi, &tx_buf, len, rx_buf, len)) == NRFX_SUCCESS)
    {
        while (!spi_flash_trans_flag_get())
            ;
    }
    else
    {
        os_kprintf("spi flash write one byte error id:%d", err_id);
        ret_code = SPI_FLASH_ERROR_WRITE_FAILED;
    }
    //	SPI_PIN_CS_HIGH;
    return ret_code;
}

/**
* @brief: the function for write enable
*
* @param:
* @return:
* @note:
*/
static uint32_t spi_flash_write_enable(void)
{
    return spi_flash_write_one_byte(SPIFLASH_WRITE_ENABLE_CMD);
}

/**
* @brief: the function for write enable
*
* @param:
* @return:
* @note:
*/
static uint32_t spi_flash_write_disable(void)
{
    return spi_flash_write_one_byte(SPIFLASH_WRITE_DISABLE_CMD);
}

static uint8_t spi_flash_read_status_reg(void)
{
    uint8_t ret = 0;

    spi_flash_write_one_byte(SPIFLASH_READSR_CMD);
    spi_flash_read_one_byte(&ret);
    //	os_kprintf("status:%d",retValue);
    return ret;
}

static void spi_flash_wait_busy(void)
{
    while ((spi_flash_read_status_reg() & SPIFLASH_WRITE_BUSYBIT) == SPIFLASH_WRITE_BUSYBIT)
        ; //status register bit 0 is the bit for flash busy

    os_kprintf("STATUS:%x", spi_flash_read_status_reg());
}
/**
* @brief: the function for device chip id read
*
* @param:
* @return:
* @note:tx_buf value 000000H/000001H just the manufacturer id is not in the same order as the device id
*/
uint32_t app_drv_spi_flash_chip_id_read(uint8_t *pbuf)
{
    uint8_t *rx_buf = spi_flash_rx_data_get();
    uint8_t tx_buf[NRF_FLASH_HEAD_LEN];
    uint8_t rlen = 8;
    uint32_t ret_code = SPI_FLASH_ERROR_OK;
    tx_buf[0] = SPIFLASH_CHIPID_READ_CMD; //SPIFLASH_CHIPID_READ_CMD;    //0x90
    tx_buf[1] = FLASH_DATA_NULL;
    tx_buf[2] = FLASH_DATA_NULL;
    tx_buf[3] = FLASH_DATA_NULL;

    if (spi_flash_is_free_get() == SPI_FLASH_FREE)
    {
        spi_flash_is_free_set(SPI_FLASH_BUSY);
        spi_flash_trans_flag_set(false);
        if (nrf_drv_spi_transfer(&spi, tx_buf, NRF_FLASH_HEAD_LEN, rx_buf, rlen) == NRFX_SUCCESS)
        {
            while (!spi_flash_trans_flag_get())
                ;
            memcpy(pbuf, &rx_buf[4], 4);
            os_kprintf("CHIP ID:%x%x%x%x", pbuf[0], pbuf[1], pbuf[2], pbuf[3]);
        }
        else
        {
            ret_code = SPI_FLASH_ERROR_CHIP_ID_READ_FAILED;
            os_kprintf("spi flash read chip id error");
        }
        spi_flash_is_free_set(SPI_FLASH_FREE);
    }
    else
    {
        os_kprintf("spi flash is busy,please wait");
        ret_code = SPI_FLASH_ERROR_OPERA_BUSY;
    }

    return ret_code;
}

uint32_t app_drv_spi_flash_erase_page(uint16_t page_number)
{
    uint8_t tx_buf[NRF_FLASH_HEAD_LEN];
    uint32_t ret_code = SPI_FLASH_ERROR_OK;
    uint8_t *rx_buf = spi_flash_rx_data_get();

    tx_buf[0] = SPIFLASH_PAGE_ERASE_CMD;
    tx_buf[1] = page_number >> 8;
    tx_buf[2] = page_number >> 4;
    tx_buf[3] = page_number << 4;

    if (spi_flash_is_free_get() == SPI_FLASH_FREE)
    {
        spi_flash_is_free_set(SPI_FLASH_BUSY);
        os_kprintf("spi flash erase page");
        if ((ret_code = spi_flash_write_enable()) != SPI_FLASH_ERROR_OK)
        {
            os_kprintf("spi flash write enable for page erase error:%d", ret_code);
            return ret_code;
        }

        spi_flash_trans_flag_set(false);
        if ((ret_code = nrf_drv_spi_transfer(&spi, tx_buf, NRF_FLASH_HEAD_LEN, rx_buf, 0)) == NRFX_SUCCESS)
        {
            while (!spi_flash_trans_flag_get())
                ;
            spi_flash_wait_busy();
            ret_code = SPI_FLASH_ERROR_OK;
        }
        else
        {
            os_kprintf("spi flash page erase failed:%d", ret_code);
            ret_code = SPI_FLASH_ERROR_ERASE_SECTOR_FAILED;
        }

        spi_flash_write_disable();
        spi_flash_is_free_set(SPI_FLASH_FREE);
    }
    else
    {
        os_kprintf("spi flash is busy");
        ret_code = SPI_FLASH_ERROR_OPERA_BUSY;
    }
    return ret_code;
}
/**
* @brief: the function for erase sector
*
* @param:
* @return:
* @note:The minimum erasure for GD25Q16 unit is sector 4K
	addr:block:sector:page 24bit
*/
uint32_t app_drv_spi_flash_erase_sector(uint16_t sector_number)
{

    uint8_t tx_buf[NRF_FLASH_HEAD_LEN];
    uint32_t ret_code = SPI_FLASH_ERROR_OK;
    uint8_t *rx_buf = spi_flash_rx_data_get();

    tx_buf[0] = SPIFLASH_SECTOR_ERASE_CMD;
    tx_buf[1] = sector_number >> 4;
    tx_buf[2] = sector_number << 4;
    tx_buf[3] = FLASH_DATA_NULL;

    if (spi_flash_is_free_get() == SPI_FLASH_FREE)
    {
        spi_flash_is_free_set(SPI_FLASH_BUSY);
        os_kprintf("spi flash erase sector");
        if ((ret_code = spi_flash_write_enable()) != SPI_FLASH_ERROR_OK)
        {
            os_kprintf("spi flash write enable for erase error:%d", ret_code);
            return ret_code;
        }

        spi_flash_trans_flag_set(false);
        if ((ret_code = nrf_drv_spi_transfer(&spi, tx_buf, NRF_FLASH_HEAD_LEN, rx_buf, 0)) == NRFX_SUCCESS)
        {
            while (!spi_flash_trans_flag_get())
                ;
            spi_flash_wait_busy();
            ret_code = SPI_FLASH_ERROR_OK;
        }
        else
        {
            os_kprintf("spi flash write failed:%d", ret_code);
            ret_code = SPI_FLASH_ERROR_ERASE_SECTOR_FAILED;
        }

        spi_flash_write_disable();
        spi_flash_is_free_set(SPI_FLASH_FREE);
    }
    else
    {
        os_kprintf("spi flash is busy");
        ret_code = SPI_FLASH_ERROR_OPERA_BUSY;
    }
    return ret_code;
}

/**
* @brief: the function for write one page
*
* @param:
* @return:
* @note:nordic spi driver can be only read and write 251 data,so if the data is 256,we need write it twice
*/
uint32_t app_drv_spi_flash_write_page(const uint8_t *pbuffer, uint32_t write_addr, uint16_t data_len)
{
    uint16_t len = data_len, len_cut = 0, slen;
    uint8_t tx_buf[255];
    uint8_t cmp_rx_buf[256];
    uint8_t *rx_buf = spi_flash_rx_data_get();
    uint32_t ret_code = SPI_FLASH_ERROR_OK, cmp_code = 0;
    uint32_t addr = write_addr;


    if (spi_flash_is_free_get() == SPI_FLASH_FREE)
    {
        spi_flash_is_free_set(SPI_FLASH_BUSY);

        do
        {
            if ((ret_code = spi_flash_write_enable()) != SPI_FLASH_ERROR_OK)
            {
                os_kprintf("spi flash write enable for write data error");
                return ret_code;
            }
            os_kprintf("spi flash write page");
            tx_buf[0] = SPIFLASH_PAGE_PROGRAM_CMD;
            tx_buf[1] = (uint8_t)((addr & FLASH_BLOCK_BIT) >> 16);
            tx_buf[2] = (uint8_t)((addr & FLASH_SECTOR_BIT) >> 8);
            tx_buf[3] = (uint8_t)addr;
            len_cut = (len >= (SPI_FLASH_OPERAT_PAGE_SIZE - NRF_FLASH_HEAD_LEN)) ? SPI_FLASH_OPERAT_PAGE_SIZE :
                      (len + NRF_FLASH_HEAD_LEN);

            slen = len_cut - NRF_FLASH_HEAD_LEN;
            memcpy(&tx_buf[4], pbuffer, slen);

            spi_flash_trans_flag_set(false);
            if (nrf_drv_spi_transfer(&spi, tx_buf, len_cut, rx_buf, 0) == NRFX_SUCCESS)
            {
                while (!spi_flash_trans_flag_get())
                    ;
                spi_flash_wait_busy();
                ret_code = SPI_FLASH_ERROR_OK;
            }
            else
            {
                os_kprintf("spi flash write failed");
                ret_code = SPI_FLASH_ERROR_ERASE_SECTOR_FAILED;
            }
            addr += slen;
            len -= slen;
            pbuffer += slen;
            spi_flash_write_disable();
        }
        while (len > 0);

        spi_flash_is_free_set(SPI_FLASH_FREE);
    }
    else
    {
        os_kprintf("spi flash is busy");
        ret_code = SPI_FLASH_ERROR_OPERA_BUSY;
    }
    pbuffer -= data_len;
    memset(cmp_rx_buf, 0, data_len);
    if (app_drv_spi_flash_read(cmp_rx_buf, write_addr, data_len) == SPI_FLASH_ERROR_OK)
    {
        if ((cmp_code = memcmp(cmp_rx_buf, pbuffer, data_len)) != 0)
        {
            os_kprintf("write data error,cmp code:%d", cmp_code);
            ret_code = SPI_FLASH_ERROR_WRITE_DATA;
        }
    }

    return ret_code;
}

/**
* @brief: the function for write spi flash data
*
* @param:
* @return:
* @note:
*/
uint32_t app_drv_spi_flash_write(const uint8_t *pbuffer, uint32_t write_addr, uint32_t data_len)
{
    //write data len the time
    uint16_t slen = data_len;
    //page data to write
    uint16_t plen;
    const uint8_t *pic_point = pbuffer;
    uint32_t ret_code = SPI_FLASH_ERROR_OK;

    if (spi_flash_is_free_get() == SPI_FLASH_FREE)
    {
        plen = write_addr & FLASH_PAGE_BIT ? FLASH_PAGE_SIZE - (write_addr & FLASH_PAGE_BIT) : FLASH_PAGE_SIZE;

        if (plen >= slen)
        {
            app_drv_spi_flash_write_page(pic_point, write_addr, slen);
        }
        else
        {
            do
            {
                app_drv_spi_flash_write_page(pic_point, write_addr, plen);
                pic_point += plen;
                write_addr += plen;
                slen -= plen;
                if (slen < FLASH_PAGE_SIZE)
                {
                    plen = slen;
                }
                else
                {
                    plen = FLASH_PAGE_SIZE;
                }
            }
            while (slen > 0);
        }
    }
    else
    {
        os_kprintf("spi flash is busy");
        ret_code = SPI_FLASH_ERROR_OPERA_BUSY;
    }
    return ret_code;
}
/**
* @brief: the function for read spi flash data
*
* @param:
* @return:
* @note:
*/
uint32_t app_drv_spi_flash_read(uint8_t *pbuffer, uint32_t read_addr, uint32_t read_bytes_num)
{

    uint16_t len = read_bytes_num, len_cut, slen;
    uint8_t tx_buf[NRF_FLASH_HEAD_LEN];
    uint32_t addr = read_addr;
    uint8_t *pic_point = pbuffer;
    uint8_t *rx_buf = spi_flash_rx_data_get();
    uint32_t ret_code = SPI_FLASH_ERROR_OK;

    if (spi_flash_is_free_get() == SPI_FLASH_FREE)
    {
        spi_flash_is_free_set(SPI_FLASH_BUSY);
        if ((ret_code = spi_flash_write_enable()) != SPI_FLASH_ERROR_OK)
        {
            os_kprintf("spi flash write enable for write data error");
            ;
            return ret_code;
        }
        do
        {
            memset(rx_buf, 0, SPI_FLASH_RX_BUFF_SIZE);
            tx_buf[0] = SPIFLASH_READ_DATA_CMD;
            tx_buf[1] = (uint8_t)((addr & FLASH_BLOCK_BIT) >> 16);
            tx_buf[2] = (uint8_t)((addr & FLASH_SECTOR_BIT) >> 8);
            tx_buf[3] = (uint8_t)addr;

            len_cut = (len >= (SPI_FLASH_OPERAT_PAGE_SIZE - NRF_FLASH_HEAD_LEN)) ? SPI_FLASH_OPERAT_PAGE_SIZE :
                      (len + NRF_FLASH_HEAD_LEN);
            slen = len_cut - NRF_FLASH_HEAD_LEN;

            spi_flash_trans_flag_set(false);

            if ((ret_code = nrf_drv_spi_transfer(&spi, tx_buf, NRF_FLASH_HEAD_LEN, rx_buf, len_cut)) == NRFX_SUCCESS)
            {
                while (!spi_flash_trans_flag_get())
                    ;
                memcpy(pic_point, &rx_buf[NRF_FLASH_HEAD_LEN], slen);
                //                os_kprintf("RECV:");
                //                os_kprintf("%x %d", pic_point[0], len_cut - 4);
                //                for (int i = 0; i < 240; i += 40)
                //                {
                //                    if ((i + 30) < len_cut - 4)
                //                    {
                //                        os_kprintf("%x %x %x,%x", pic_point[i], pic_point[i + 10], pic_point[i + 20], pic_point[i + 30]);
                //                    }
                //                }

                ret_code = SPI_FLASH_ERROR_OK;
            }
            else
            {
                os_kprintf("spi flash read failed:%d,", ret_code);
                ret_code = SPI_FLASH_ERROR_ERASE_SECTOR_FAILED;
            }

            addr += slen;
            len -= slen;
            pic_point += slen;

        }
        while (len > 0);

        spi_flash_write_disable();
        spi_flash_is_free_set(SPI_FLASH_FREE);
    }
    else
    {
        os_kprintf("spi flash is busy");
        ret_code = SPI_FLASH_ERROR_OPERA_BUSY;
    }
    return ret_code;
}

/**
* @brief: the function for spi init
*
* @param:
* @return:
* @note:
*/
void app_drv_spi_init(void)
{
    uint8_t err_code = SPI_FLASH_ERROR_OK;
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

    if (app_drv_spi_is_init())
    {
        os_kprintf("spi flash already init!!");
        return;
    }
    spi_config.ss_pin = SPI_CS_PIN_NUM;
    spi_config.miso_pin = SPI_SO_PIN_NUM;
    spi_config.mosi_pin = SPI_SI_PIN_NUM;
    spi_config.sck_pin = SPI_CLK_PIN_NUM;
    err_code = nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
    if (err_code == NRFX_SUCCESS || err_code == NRFX_ERROR_INVALID_STATE)
    {
        os_kprintf("spi flash init sucessed!!");
    }
    else
    {
        os_kprintf("spi flash init error, driver id: %d", err_code);
        err_code = SPI_FLASH_ERROR_INIT;
    }

}
bool app_drv_spi_is_init()
{
    return nrfx_spim_is_init(&spi.u.spim);
}
/**
* @brief: the function for spi uninit
*
* @param:
* @return:
* @note:if want to use spi flash again,you need to reinitialize it
*/
void app_drv_spi_uninit(void)
{
    if (app_drv_spi_is_init())
    {
        nrf_drv_spi_uninit(&spi);
        nrf_gpio_cfg_default(SPI_CLK_PIN_NUM);
        nrf_gpio_cfg_default(SPI_SI_PIN_NUM);
        nrf_gpio_cfg_default(SPI_SO_PIN_NUM);
        nrf_gpio_cfg_default(SPI_CS_PIN_NUM);
    }

}

/**
* @brief: the function for spi test
*
* @param:
* @return:
* @note:
*/
void app_drv_spi_test()
{
    uint8_t id[8];
    uint8_t recv_buf[256], send_buf[] = "helloworldn";
    app_drv_spi_init();
    app_drv_spi_flash_chip_id_read(id);
    app_drv_spi_flash_erase_sector(0);
    //    app_drv_spi_flash_erase_page(0);
    nrf_delay_ms(100);
    //    app_drv_spi_flash_read(recv_buf,0,12);
    uint8_t len = 12;
    app_drv_spi_flash_write_page(send_buf, 0, len);
    memset(recv_buf, 0, 256);
    app_drv_spi_flash_read(recv_buf, 0, len);
    uint32_t cmp_code = memcmp(recv_buf, send_buf, len);
    os_kprintf("read = %s,%d\r\n", recv_buf, cmp_code);


    //    app_drv_spi_uninit();
}
