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
 * @file        drv_sdio_common.c
 *
 * @brief       This file provides sdio-usdhc bus common config.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_memory.h>

#include <fsl_sd.h>
#include <fsl_sdio.h>
#include <fsl_mmc.h>

#include <drv_sdio_common.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.sdio_common"
#include <drv_log.h>

#if defined(OS_USDHC1_DMA_BUFF_SIZE)
AT_NONCACHEABLE_SECTION_ALIGN(uint32_t os_usdhc1_dma_buf[OS_USDHC1_DMA_BUFF_SIZE], 4);
#endif
#if defined(OS_USDHC2_DMA_BUFF_SIZE)
AT_NONCACHEABLE_SECTION_ALIGN(uint32_t os_usdhc2_dma_buf[OS_USDHC2_DMA_BUFF_SIZE], 4);
#endif
#if defined(OS_USDHC3_DMA_BUFF_SIZE)
AT_NONCACHEABLE_SECTION_ALIGN(uint32_t os_usdhc3_dma_buf[OS_USDHC3_DMA_BUFF_SIZE], 4);
#endif

static os_uint8_t os_sdio_param_num = sizeof(sdio_cfg_param) / sizeof(sdio_cfg_param[0]);

os_imxrt_sdio_pin_param_t sdio_pwr_param;
os_imxrt_sdio_pin_param_t sdio_irq_param;

os_uint32_t os_imxrt_sdio_clock_config(void)
{
    CLOCK_InitSysPll(&sysPllConfig_BOARD_BootClockRUN);
    /*configure system pll PFD0 fractional divider to 24, output clock is 528MHZ * 18 / 24 = 396 MHZ*/
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 24U);
    /* Configure USDHC clock source and divider */
    CLOCK_SetDiv(kCLOCK_Usdhc1Div, 1U); /* USDHC clock root frequency maximum: 198MHZ */
    CLOCK_SetMux(kCLOCK_Usdhc1Mux, 1U);

    return 396000000 / 2;
}

bool os_imxrt_sdio_init_get_detectstatus(void)
{
    if (sdio_irq_param.status)
    {
        if (os_pin_read(sdio_irq_param.pin) == sdio_irq_param.level)
            return OS_TRUE;
        else
            return OS_FALSE;
    }
    
    return OS_TRUE;
}

bool os_imxrt_sdio_get_detectstatus(os_imxrt_sdio_cfg_param_t *cfg_param)
{
    if (cfg_param->irq_pin)
    {
        if ((cfg_param->sdio_type == OS_SDIO_TYPE_SD) || (cfg_param->sdio_type == OS_SDIO_TYPE_SDIO))
        {
            if (os_pin_read(cfg_param->irq_pin) == cfg_param->insert_level)
                return OS_TRUE;
            else
                return OS_FALSE;
        }
    }
    return OS_FALSE;
}

void os_imxrt_sdio_irq_cb(void *args)
{
    os_imxrt_sdio_sd_cd_param_t *sd_cd_parama = (os_imxrt_sdio_sd_cd_param_t *)args;
    if (sd_cd_parama->sd_cd != OS_NULL)
    {
        sd_cd_parama->sd_cd(os_imxrt_sdio_get_detectstatus(sd_cd_parama->cfg_param), sd_cd_parama->userData);
    }
}

os_err_t os_imxrt_sdio_detectinit(sd_detect_card_t *s_cd, os_imxrt_sdio_sd_cd_param_t *sd_cd_parama)
{
    /* install card detect callback */
    s_cd->cdDebounce_ms = sd_cd_parama->cfg_param->card_detect_debounce_delay_ms;
    s_cd->type          = sd_cd_parama->cfg_param->cd_type;
    s_cd->cardDetected  = os_imxrt_sdio_init_get_detectstatus;
    s_cd->callback      = sd_cd_parama->sd_cd;
    s_cd->userData      = sd_cd_parama->userData;
    
    sdio_irq_param.pin = sd_cd_parama->cfg_param->irq_pin;
    sdio_irq_param.level = sd_cd_parama->cfg_param->insert_level;
    sdio_irq_param.status = OS_TRUE;
    if (sd_cd_parama->cfg_param->irq_pin)
    {
        os_pin_mode(sd_cd_parama->cfg_param->irq_pin, PIN_MODE_INPUT);
        os_pin_attach_irq(sd_cd_parama->cfg_param->irq_pin, sd_cd_parama->cfg_param->intterupt_type, os_imxrt_sdio_irq_cb, sd_cd_parama);
        os_pin_irq_enable(sd_cd_parama->cfg_param->irq_pin, OS_TRUE);
        if (os_pin_read(sd_cd_parama->cfg_param->irq_pin) == sd_cd_parama->cfg_param->insert_level)
        {
            if (sd_cd_parama->sd_cd != NULL)
            {
                sd_cd_parama->sd_cd(true, sd_cd_parama->userData);
            }
            return OS_EOK;
        }
        LOG_E(DRV_EXT_TAG, "sdio device cannot find!");
        return OS_ERROR;
    }
    
    LOG_I(DRV_EXT_TAG, "sdio irq_pin not config!");
    return OS_EOK;
}


void os_imxrt_sdio_pwrctl(os_bool_t enable)
{
    if (sdio_pwr_param.status)
    {
        if (enable)    
            os_pin_write(sdio_pwr_param.pin, sdio_pwr_param.level);
        else
            os_pin_write(sdio_pwr_param.pin, !sdio_pwr_param.level);
    }
}

void os_imxrt_sdio_pwrctl_init(os_imxrt_sdio_cfg_param_t *cfg_param)
{
    if (cfg_param->pwr_pin)
    {
        os_pin_mode(cfg_param->pwr_pin, PIN_MODE_OUTPUT);
        sdio_pwr_param.pin = cfg_param->pwr_pin;
        sdio_pwr_param.level = cfg_param->pwr_pin_level;
        os_imxrt_sdio_pwrctl(1);
        sdio_pwr_param.status = OS_TRUE;
    }
    else
    {
        sdio_pwr_param.status = OS_FALSE;
    }
}

void os_imxrt_sdio_host_cfg(sdmmchost_t *host, os_imxrt_sdio_cfg_param_t *cfg_param)
{
    memset(cfg_param->buf, 0, cfg_param->buf_size);
    host->dmaDesBuffer = cfg_param->buf;
    host->dmaDesBufferWordsNum = cfg_param->buf_size;

    host->enableCacheControl = cfg_param->host_cache_ctl;
    host->hostController.sourceClock_Hz = os_imxrt_sdio_clock_config();
    host->hostController.base = cfg_param->host_baseaddr;
}

os_err_t os_imxrt_sdio_index_get(os_imxrt_sdio_type sdio_type, os_uint8_t *index)
{
    os_uint8_t i = 0;
    os_uint8_t count = 0;

    if (*index >= os_sdio_param_num)
        return OS_ERROR;
        
    for (i = *index;i < os_sdio_param_num;i++)
    {
        if (sdio_type == sdio_cfg_param[i].sdio_type)
        {
            *index = i;
            return OS_EOK;
        }
    }
    
    return OS_EEMPTY;
}

os_err_t os_imxrt_sdio_config(void *card_t, os_imxrt_sdio_type sdio_type, os_imxrt_sdio_sd_cd_param_t *sd_cd_parama)
{
    sdmmchost_t *host = OS_NULL;
    sd_detect_card_t *s_cd = OS_NULL;

    OS_UNUSED sd_card_t *sd_card = OS_NULL;
    OS_UNUSED sdio_card_t *sdio_card = OS_NULL;
    OS_UNUSED mmc_card_t *mmc_card = OS_NULL;

    if ((sdio_type < OS_SDIO_TYPE_SD) || (sdio_type > OS_SDIO_TYPE_MMC))
    {
        LOG_E(DRV_EXT_TAG, "cannot find sdio bus!");
        return OS_ERROR;
    }

    host = os_calloc(1, sizeof(sdmmchost_t));
    s_cd = os_calloc(1, sizeof(sd_detect_card_t));
    os_imxrt_sdio_pwrctl_init(sd_cd_parama->cfg_param);
    os_imxrt_sdio_host_cfg(host, sd_cd_parama->cfg_param);
    if (os_imxrt_sdio_detectinit(s_cd, sd_cd_parama) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "cannot find sdio device!");
        return OS_ERROR;
    }
    
    switch(sdio_type)
    {
    case OS_SDIO_TYPE_SD:
        sd_card = (sd_card_t *)card_t;
        sd_card->host = host;
        sd_card->usrParam.cd = s_cd;
        sd_card->usrParam.pwr = (sd_pwr_t)os_imxrt_sdio_pwrctl;
        sd_card->usrParam.ioStrength = OS_NULL; 
        sd_card->usrParam.ioVoltage = &sd_cd_parama->cfg_param->io_voltage;
        sd_card->usrParam.maxFreq = sd_cd_parama->cfg_param->maxfrq;
        break;
    case OS_SDIO_TYPE_SDIO:
        sdio_card = (sdio_card_t *)card_t;
        sdio_card->host = host;
        sdio_card->usrParam.cd = s_cd;
        sdio_card->usrParam.pwr = (sd_pwr_t)os_imxrt_sdio_pwrctl;
        sdio_card->usrParam.ioStrength = OS_NULL; 
        sdio_card->usrParam.ioVoltage = &sd_cd_parama->cfg_param->io_voltage;
        sdio_card->usrParam.maxFreq = sd_cd_parama->cfg_param->maxfrq;
        break;
    case OS_SDIO_TYPE_MMC:
        mmc_card = (mmc_card_t *)card_t;
        mmc_card->host = host;
        mmc_card->usrParam.ioStrength = OS_NULL; 
        mmc_card->usrParam.maxFreq = sd_cd_parama->cfg_param->maxfrq;
        break;
    default:
        LOG_E(DRV_EXT_TAG, "cannot find sdio bus!");
        return OS_ERROR;
    }

    NVIC_SetPriority(sd_cd_parama->cfg_param->host_irq, sd_cd_parama->cfg_param->host_irq_priority);
    return OS_EOK;
}

void os_imxrt_sdio_init_end(void)
{
    sdio_pwr_param.status = OS_FALSE;
    sdio_irq_param.status = OS_FALSE;
}

