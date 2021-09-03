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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for nxp
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "drv_cfg.h"
#include "drv_common.h"

#include "devices.c"

const usdhc_config_t sd_config = {
     .dataTimeout          = 0xFU,     /*!< Data timeout value. */
     .endianMode           = kUSDHC_EndianModeLittle,     /*!< Endian mode. */
     .readWatermarkLevel   = 0x80U,    /*!< Watermark level for DMA read operation. Available range is 1 ~ 128. */
     .writeWatermarkLevel  = 0x80U    /*!< Watermark level for DMA write operation. Available range is 1 ~ 128. */
};
//static const struct nxp_SD_info sd_info = {SDIO_PERIPHERAL, &SD_config};
//OS_HAL_DEVICE_DEFINE("sd_Type", "SD", sd_info);
//static const struct nxp_sdio_info sd_info = {USDHC1_PERIPHERAL, &sd_config};
//OS_HAL_DEVICE_DEFINE("USDHC_Type", "sd1", sd_info);
