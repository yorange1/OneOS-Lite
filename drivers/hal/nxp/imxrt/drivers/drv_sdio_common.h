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
 * @file        drv_sdio_common.h
 *
 * @brief       This file provides sdio-usdhc bus common config.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRV_SDIO_COMMON_H__
#define DRV_SDIO_COMMON_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <pin.h>

#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "fsl_common.h"
#include "fsl_sdmmc_host.h"
#include "fsl_sdmmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
    OS_SDIO_TYPE_SD = 1,
    OS_SDIO_TYPE_SDIO,
    OS_SDIO_TYPE_MMC
}os_imxrt_sdio_type;

typedef struct os_imxrt_sdio_cfg_param
{
    USDHC_Type         *host_baseaddr;
    IRQn_Type           host_irq;
    os_uint32_t         host_irq_priority;
    os_uint8_t          host_cache_ctl;
    os_uint8_t          cd_type;
    os_uint32_t         card_detect_debounce_delay_ms;
    os_uint32_t         maxfrq;
    os_uint32_t         block_size[kSDIO_FunctionNum7 + 1];
    sd_io_voltage_t     io_voltage;
    os_imxrt_sdio_type  sdio_type;
    os_base_t           pwr_pin;
    os_int32_t          pwr_pin_level;
    os_base_t           irq_pin;
    os_uint32_t         intterupt_type;
    os_int32_t          insert_level;
    os_uint32_t        *buf;
    os_uint32_t         buf_size;
}os_imxrt_sdio_cfg_param_t;

typedef struct os_imxrt_sdio_sd_cd_param
{
    sd_cd_t sd_cd;
    void *userData;
    os_imxrt_sdio_cfg_param_t *cfg_param;
}os_imxrt_sdio_sd_cd_param_t;

typedef struct os_pwr_param
{
    os_bool_t   status;
    os_base_t   pin;
    os_int32_t  level;
}os_imxrt_sdio_pin_param_t;

#if defined(OS_USDHC1_DMA_BUFF_SIZE)
extern uint32_t os_usdhc1_dma_buf[OS_USDHC1_DMA_BUFF_SIZE];
#endif
#if defined(OS_USDHC2_DMA_BUFF_SIZE)
extern uint32_t os_usdhc2_dma_buf[OS_USDHC2_DMA_BUFF_SIZE];
#endif
#if defined(OS_USDHC3_DMA_BUFF_SIZE)
extern uint32_t os_usdhc3_dma_buf[OS_USDHC3_DMA_BUFF_SIZE];
#endif

#if defined(OS_USING_SDIO_NXP)
static os_imxrt_sdio_cfg_param_t sdio_cfg_param[] =
{
#if defined(OS_USDHC1_ENABLE)
    {
    .host_baseaddr                  = USDHC1,
    .host_irq                       = USDHC1_IRQn,
    .host_irq_priority              = 5,
    .host_cache_ctl                 = kSDMMCHOST_CacheControlRWBuffer,
    .cd_type                        = kSD_DetectCardByGpioCD,
    .card_detect_debounce_delay_ms  = 100,
#if defined(OS_USDHC1_SD)
    .maxfrq                         = MMC_CLOCK_HS200,
#elif defined(OS_USDHC1_SDIO)
    .maxfrq                         = SD_CLOCK_50MHZ,
#elif defined(OS_USDHC1_MMC)
    .maxfrq                         = MMC_CLOCK_HS200,
#endif
    .io_voltage.type                = kSD_IOVoltageCtrlByHost,
    .io_voltage.func                = OS_NULL,
#if defined(OS_USDHC1_SD)
    .sdio_type                      = OS_SDIO_TYPE_SD,
#elif defined(OS_USDHC1_SDIO)
    .sdio_type                      = OS_SDIO_TYPE_SDIO,
    .block_size[kSDIO_FunctionNum0] = OS_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum1] = OS_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum2] = OS_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum3] = OS_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum4] = OS_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum5] = OS_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum6] = OS_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum7] = OS_SDIO_USDHC1_BLOCK_SIZE;
#elif defined(OS_USDHC1_MMC)
    .sdio_type                      = OS_SDIO_TYPE_MMC,
#endif
#if defined(OS_USDHC1_PWR_EN) && defined(OS_USDHC1_PWR_PIN)
    .pwr_pin                        = OS_USDHC1_PWR_PIN,
    .pwr_pin_level                  = OS_USDHC1_PWR_PIN_LEVEL,
#else
    .pwr_pin                        = 0,
#endif
#if defined(OS_USDHC1_IRQ_EN) && defined(OS_USDHC1_IRQ_PIN)
    .irq_pin                        = OS_USDHC1_IRQ_PIN,
    .intterupt_type                 = PIN_IRQ_MODE_RISING_FALLING,
    .insert_level                   = OS_USDHC1_IRQ_PIN_LEVEL,
#else
    .irq_pin                        = NULL,
#endif
    .buf_size                       = OS_USDHC1_DMA_BUFF_SIZE,
    .buf                            = (os_uint32_t *)os_usdhc1_dma_buf,
    },
#endif
#if defined(OS_USDHC2_ENABLE)
    {
    .host_baseaddr                  = USDHC2,
    .host_irq                       = USDHC2_IRQn,
    .host_irq_priority              = 5,
    .host_cache_ctl                 = kSDMMCHOST_CacheControlRWBuffer,
    .cd_type                        = kSD_DetectCardByGpioCD,
    .card_detect_debounce_delay_ms  = 100,
#if defined(OS_USDHC2_SD)
    .maxfrq                         = MMC_CLOCK_HS200,
#elif defined(OS_USDHC2_SDIO)
    .maxfrq                         = SD_CLOCK_50MHZ,
    .block_size[kSDIO_FunctionNum0] = OS_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum1] = OS_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum2] = OS_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum3] = OS_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum4] = OS_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum5] = OS_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum6] = OS_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum7] = OS_SDIO_USDHC2_BLOCK_SIZE;
#elif defined(OS_USDHC2_MMC)
    .maxfrq                         = MMC_CLOCK_HS200,
#endif
    .io_voltage.type                = kSD_IOVoltageCtrlByHost,
    .io_voltage.func                = OS_NULL,
#if defined(OS_USDHC2_SD)
    .sdio_type                      = OS_SDIO_TYPE_SD,
#elif defined(OS_USDHC2_SDIO)
    .sdio_type                      = OS_SDIO_TYPE_SDIO,
#elif defined(OS_USDHC2_MMC)
    .sdio_type                      = OS_SDIO_TYPE_MMC,
#endif
#if defined(OS_USDHC2_PWR_EN) && defined(OS_USDHC1_PWR_PIN)
    .pwr_pin                        = OS_USDHC2_PWR_PIN,
    .pwr_pin_level                  = OS_USDHC2_PWR_PIN_LEVEL,
#else
    .pwr_pin                        = NULL,
#endif
#if defined(OS_USDHC2_IRQ_EN) && defined(OS_USDHC1_PWR_PIN)
    .irq_pin                        = OS_USDHC2_IRQ_PIN,
    .intterupt_type                 = PIN_IRQ_MODE_RISING_FALLING,
    .insert_level                   = OS_USDHC2_IRQ_PIN_LEVEL,
#else
    .irq_pin                        = NULL,
#endif
    .buf_size                       = OS_USDHC2_DMA_BUFF_SIZE,
    .buf                            = (os_uint32_t *)os_usdhc2_dma_buf,
    },
#endif
#if defined(OS_USDHC3_ENABLE)
    {
    .host_baseaddr                  = USDHC3,
    .host_irq                       = USDHC3_IRQn,
    .host_irq_priority              = 5,
    .host_cache_ctl                 = kSDMMCHOST_CacheControlRWBuffer,
    .cd_type                        = kSD_DetectCardByGpioCD,
    .card_detect_debounce_delay_ms  = 100,
#if defined(OS_USDHC3_SD)
    .maxfrq                         = MMC_CLOCK_HS200,
#elif defined(OS_USDHC3_SDIO)
    .maxfrq                         = SD_CLOCK_50MHZ,
    .block_size[kSDIO_FunctionNum0] = OS_SDIO_USDHC3_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum1] = OS_SDIO_USDHC3_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum2] = OS_SDIO_USDHC3_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum3] = OS_SDIO_USDHC3_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum4] = OS_SDIO_USDHC3_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum5] = OS_SDIO_USDHC3_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum6] = OS_SDIO_USDHC3_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum7] = OS_SDIO_USDHC3_BLOCK_SIZE;
#elif defined(OS_USDHC3_MMC)
    .maxfrq                         = MMC_CLOCK_HS200,
#endif
    .io_voltage.type                = kSD_IOVoltageCtrlByHost,
    .io_voltage.func                = OS_NULL,
#if defined(OS_USDHC3_SD)
    .sdio_type                      = OS_SDIO_TYPE_SD,
#elif defined(OS_USDHC3_SDIO)
    .sdio_type                      = OS_SDIO_TYPE_SDIO,
#elif defined(OS_USDHC3_MMC)
    .sdio_type                      = OS_SDIO_TYPE_MMC,
#endif
#if defined(OS_USDHC3_PWR_EN) && defined(OS_USDHC3_PWR_PIN)
    .pwr_pin                        = OS_USDHC3_PWR_PIN,
    .pwr_pin_level                  = OS_USDHC3_PWR_PIN_LEVEL,
#else
    .pwr_pin                        = NULL,
#endif
#if defined(OS_USDHC3_IRQ_EN) && defined(OS_USDHC3_IRQ_PIN)
    .irq_pin                        = OS_USDHC3_IRQ_PIN,
    .intterupt_type                 = PIN_IRQ_MODE_RISING_FALLING,
    .insert_level                   = OS_USDHC3_IRQ_PIN_LEVEL,
#else
    .irq_pin                        = NULL,
#endif
    .buf_size                       = OS_USDHC3_DMA_BUFF_SIZE,
    .buf                            = (os_uint32_t *)os_usdhc3_dma_buf,
    },
#endif
};
#endif

bool os_imxrt_sdio_get_detectstatus(os_imxrt_sdio_cfg_param_t *cfg_param);
os_err_t os_imxrt_sdio_index_get(os_imxrt_sdio_type sdio_type, os_uint8_t *index);
os_err_t os_imxrt_sdio_config(void *card_t, os_imxrt_sdio_type sdio_type, os_imxrt_sdio_sd_cd_param_t *sd_cd_parama);
void os_imxrt_sdio_init_end(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_SAI_H__ */

