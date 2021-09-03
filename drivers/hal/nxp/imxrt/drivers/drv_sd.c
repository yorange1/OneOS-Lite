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
 * @file        drv_sd.c
 *
 * @brief       This file provides sd card device register.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <os_memory.h>
#include <block/block_device.h>
#include <dlog.h>
#include <drv_sdio_common.h>
#include <drv_sd.h>
#include "fsl_sd.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.sd"
#include <drv_log.h>

#define DATA_BLOCK_COUNT (62333952U)

typedef struct os_imxrt_sd_device
{
    os_blk_device_t blk_dev;
    sd_card_t sd;
}os_imxrt_sd_device_t;

static os_err_t os_imxrt_sd_init(os_imxrt_sd_device_t *sd_dev, os_imxrt_sdio_sd_cd_param_t *sd_cd_param)
{
    sd_card_t *card = &sd_dev->sd;
    if (os_imxrt_sdio_config(card, OS_SDIO_TYPE_SD, sd_cd_param) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "sdio_config fail");
        return OS_ERROR;
    }
    
    if (SD_Init(card) != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "SD card init fail");
        return OS_ERROR;
    }
    os_imxrt_sdio_init_end();
    return OS_EOK;
}

static int os_imxrt_sdio_read_block(os_blk_device_t *blk, os_uint32_t block_addr, os_uint8_t *buff, os_uint32_t block_nr)
{
    OS_ASSERT(blk != OS_NULL);

    struct os_imxrt_sd_device *sd_dev  = (struct os_imxrt_sd_device *)blk;

    if (kStatus_Success != SD_ReadBlocks(&sd_dev->sd, buff, block_addr, block_nr))
    {
        LOG_E(DRV_EXT_TAG, "read addr: %d, count: %d", block_addr, block_nr);
        return OS_ERROR;
    }

    return OS_EOK;
}

static int os_imxrt_sdio_write_block(os_blk_device_t *blk, os_uint32_t block_addr, const os_uint8_t *buff, os_uint32_t block_nr)
{
    OS_ASSERT(blk != OS_NULL);

    struct os_imxrt_sd_device *sd_dev  = (struct os_imxrt_sd_device *)blk;

    if (kStatus_Success != SD_EraseBlocks(&sd_dev->sd, block_addr, block_nr))
    {
        LOG_E(DRV_EXT_TAG, "erase addr: %d, count: %d", block_addr, block_nr);
        return OS_ERROR;
    }

    if (kStatus_Success != SD_WriteBlocks(&sd_dev->sd, buff, block_addr, block_nr))
    {
        LOG_E(DRV_EXT_TAG, "write addr: %d, count: %d", block_addr, block_nr);
        return OS_ERROR;
    }

    return OS_EOK;
}

const static struct os_blk_ops os_imxrt_sdio_blk_ops = {
    .read_block   = os_imxrt_sdio_read_block,
    .write_block  = os_imxrt_sdio_write_block,
};

static int os_imxrt_sd_device_init(void)
{
    int ret;
    os_uint8_t sd_index = 0;
    char sd_device_name[] = "sd0";

    ret = os_imxrt_sdio_index_get(OS_SDIO_TYPE_SD, &sd_index);
    if (ret != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "cannot find sd config param!");
        return OS_ERROR;
    }

    for(;;)
    {
        struct os_imxrt_sd_device *sd_dev = os_calloc(1, sizeof(struct os_imxrt_sd_device));
        if (sd_dev == OS_NULL)
        {
            LOG_E(DRV_EXT_TAG, "failed!");
            return OS_ENOMEM;
        }
        os_imxrt_sdio_sd_cd_param_t *sd_cd_param = os_calloc(1, sizeof(os_imxrt_sdio_sd_cd_param_t));
        if (sd_cd_param == OS_NULL)
        {
            os_free(sd_dev);
            LOG_E(DRV_EXT_TAG, "failed!");
            return OS_ENOMEM;
        }
        
        os_imxrt_sdio_cfg_param_t *cfg_param = &sdio_cfg_param[sd_index];
        
        sd_cd_param->sd_cd = OS_NULL;
        sd_cd_param->userData = OS_NULL;
        sd_cd_param->cfg_param = cfg_param;
        ret = os_imxrt_sd_init(sd_dev, sd_cd_param);
        if (ret != OS_EOK)
        {
            os_free(sd_dev);
            os_free(sd_cd_param);
            LOG_E(DRV_EXT_TAG, "os_imxrt_sd_init failed!");
            return OS_ERROR;
        }
        
        sd_dev->blk_dev.geometry.block_size = FSL_SDMMC_DEFAULT_BLOCK_SIZE;
        sd_dev->blk_dev.geometry.capacity   = DATA_BLOCK_COUNT * FSL_SDMMC_DEFAULT_BLOCK_SIZE;
        sd_dev->blk_dev.blk_ops = &os_imxrt_sdio_blk_ops;
        
        block_device_register(&sd_dev->blk_dev, sd_device_name);
        sd_device_name[2]++;
        sd_index++;
        ret = os_imxrt_sdio_index_get(OS_SDIO_TYPE_SD, &sd_index);
        if (ret != OS_EOK)
        {
            break;
        }
    }

    LOG_I(DRV_EXT_TAG, "sd init success!");

    return OS_EOK;
}

OS_DEVICE_INIT(os_imxrt_sd_device_init, OS_INIT_SUBLEVEL_LOW);

