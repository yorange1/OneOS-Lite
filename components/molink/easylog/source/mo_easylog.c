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
 * @file        mo_easylog.c
 *
 * @brief       Record molink network information to the flash
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <os_task.h>

#ifdef OS_USING_SHELL
#include <shell.h>
#endif

#ifdef NET_USING_MOLINK
#include <mo_api.h>

#ifdef MOLINK_USING_EASYLOG
#include "mo_easylog.h"

#ifdef OS_USING_RTC
#include <rtc.h>
#endif

static mo_easylog_t *easylog_obj = OS_NULL;
static mo_object_t  *def_module  = OS_NULL;

static os_err_t mo_easylog_info_update(const os_uint8_t *user_data, os_size_t data_len);
static os_err_t mo_easylog_get_sys_time(void);
static os_err_t mo_easylog_get_imei(void);
static os_err_t mo_easylog_get_imsi(void);
static os_err_t mo_easylog_get_iccid(void);
static os_err_t mo_easylog_get_csq(void);
static os_err_t mo_easylog_get_radio_info(void);
static void     mo_easylog_init_user_data(const os_uint8_t *data, os_size_t len);
static os_uint8_t mo_easylog_calculate_tx_checksum(os_uint8_t *data, os_uint32_t len);
static mo_easylog_network_status_t mo_easylog_get_network_status(mo_object_t *module, mo_easylog_t *e_obj);


static mo_easylog_network_status_t mo_easylog_get_network_status(mo_object_t *module, mo_easylog_t *e_obj)
{
    mo_easylog_network_status_t net_status = NETWORK_STATUS_OK;

    os_err_t   reslut = OS_ERROR;
    os_uint8_t status = 0;
    os_uint8_t rssi   = 0;
    os_uint8_t ber    = 0;

    char tmp[MO_IMSI_LEN + 1] = {0};

    if (OS_NULL == module)
    {
        os_kprintf("%s [%d]: module obj is OS_NULL\r\n", __func__, __LINE__);
        net_status = NETWORK_STATUS_UNDEF;
        goto __exit;
    }

    for (os_int8_t i = 0; i < 3; i++)
    {
        reslut = mo_at_test(module);
        if (OS_EOK == reslut)
        {
            break;
        }
        os_task_msleep(1000);
    }

    if (OS_EOK != reslut)
    {
        os_kprintf("%s [%d]: AT test failed\r\n", __func__, __LINE__);
        net_status = NETWORK_AT_TEST_ERROR;
        goto __exit;
    }

    if ((OS_EOK != mo_get_cfun(module, &status)) || (0 == status))
    {
        os_kprintf("%s [%d]: cfun status error\r\n", __func__, __LINE__);
        net_status = NETWORK_CFUN_STATUS_ERROR;
        goto __exit;
    }

    if (OS_EOK != mo_get_imsi(module, tmp, MO_IMSI_LEN))
    {
        os_kprintf("%s [%d]: SIM card status error\r\n", __func__, __LINE__);
        net_status = NETWORK_SIM_CARD_ERROR;
        goto __exit;
    }

    if ((OS_EOK != mo_get_attach(module, &status)) || (0 == status))
    {
        os_kprintf("%s [%d]: attach status error\r\n", __func__, __LINE__);
        net_status = NETWORK_ATTACH_STATUS_ERROR;
        goto __exit;
    }

    eps_reg_info_t reg_info;

    if (OS_EOK != mo_get_reg(module, &reg_info))
    {
        os_kprintf("%s [%d]: get network cereg status error\r\n", __func__, __LINE__);
        net_status = NETWORK_CEREG_STATUS_ERROR;
        goto __exit;
    }

    if ((1 == reg_info.reg_stat) || (5 == reg_info.reg_stat))
    {
        net_status = NETWORK_STATUS_OK;
    }
    else
    {
        os_kprintf("%s [%d]: network cereg status error\r\n", __func__, __LINE__);
        net_status = NETWORK_CEREG_STATUS_ERROR;
        goto __exit;
    }

    if (OS_EOK != mo_get_csq(module, &rssi, &ber))
    {
        os_kprintf("%s [%d]: get csq failed\r\n", __func__, __LINE__);
        goto __exit;
    }
    else
    {
        if (rssi < MO_EASTLOG_CSQ_WARNING_LV)
        {
            os_kprintf("%s [%d]: warning -- signal quality is very bad\r\n", __func__, __LINE__);
            net_status = NETWORK_CSQ_WARNING;
            goto __exit;
        }
    }

__exit:

    return net_status;
}

#ifdef OS_USING_RTC
static os_err_t mo_easylog_get_sys_time(void)
{
    time_t sys_time;
    char   tmp[32] = {0};

    if (OS_NULL == easylog_obj)
    {
        os_kprintf("easylog obj has not been created, get system time failed\r\n");
        return OS_ERROR;
    }

    sys_time = rtc_get();
    /* delete week info */
    snprintf(tmp, sizeof(tmp), "%s", (char *)(ctime(&sys_time) + 4));
    memmove(easylog_obj->sys_time, tmp, strlen(tmp));

    return OS_EOK;
}
#endif /* OS_USING_RTC */

#ifdef MOLINK_EASYLOG_USING_IMEI
static os_err_t mo_easylog_get_imei(void)
{
    if (OS_EOK != mo_get_imei(def_module, easylog_obj->imei, MO_IMEI_LEN))
    {
        os_kprintf("%s [%d]: get imei info error\r\n", __func__, __LINE__);
        return OS_ERROR;
    }

    return OS_EOK;
}
#endif /* MOLINK_EASYLOG_USING_IMEI */

#ifdef MOLINK_EASYLOG_USING_IMSI
static os_err_t mo_easylog_get_imsi(void)
{
    if (OS_EOK != mo_get_imsi(def_module, easylog_obj->imsi, MO_IMSI_LEN))
    {
        os_kprintf("%s [%d]: get imsi info error\r\n", __func__, __LINE__);
        return OS_ERROR;
    }

    return OS_EOK;
}
#endif /* MOLINK_EASYLOG_USING_IMSI */

#ifdef MOLINK_EASYLOG_USING_ICCID
static os_err_t mo_easylog_get_iccid(void)
{
    if (OS_EOK != mo_get_iccid(def_module, easylog_obj->iccid, MO_ICCID_LEN))
    {
        os_kprintf("%s [%d]: get iccid info error\r\n", __func__, __LINE__);
        return OS_ERROR;
    }

    return OS_EOK;
}
#endif /* MOLINK_EASYLOG_USING_ICCID */

#ifdef MOLINK_EASYLOG_USING_CSQ
static os_err_t mo_easylog_get_csq(void)
{
    if (OS_EOK != mo_get_csq(def_module, &easylog_obj->csq_rssi, &easylog_obj->csq_ber))
    {
        os_kprintf("%s [%d]: get CSQ info error\r\n", __func__, __LINE__);
        return OS_ERROR;
    }

    return OS_EOK;
}
#endif /* MOLINK_EASYLOG_USING_CSQ */

#ifdef MOLINK_EASYLOG_USING_RADIOINFO
static os_err_t mo_easylog_get_radio_info(void)
{
    radio_info_t ra_info;

    if (OS_EOK != mo_get_radio(def_module, &ra_info))
    {
        os_kprintf("%s [%d]: get radio info error\r\n", __func__, __LINE__);
        return OS_ERROR;
    }

#ifdef MOLINK_EASYLOG_USING_PCI
    easylog_obj->pci = ra_info.pci;
#endif

#ifdef MOLINK_EASYLOG_USING_EARFCN
    easylog_obj->earfcn = ra_info.earfcn;
#endif

#ifdef MOLINK_EASYLOG_USING_CELLID
    if (strlen(ra_info.cell_id) > 0)
    {
        memmove(easylog_obj->cell_id, ra_info.cell_id, strlen(ra_info.cell_id));
    }
#endif

#ifdef MOLINK_EASYLOG_USING_RSRP
    easylog_obj->rsrp = ra_info.rsrp;
#endif

#ifdef MOLINK_EASYLOG_USING_RSRQ
    easylog_obj->rsrq = ra_info.rsrq;
#endif

#ifdef MOLINK_EASYLOG_USING_SNR
    easylog_obj->snr = ra_info.snr;
#endif

#ifdef MOLINK_EASYLOG_USING_TX_PWR
    easylog_obj->tx_power = ra_info.signal_power;
#endif

#ifdef MOLINK_EASYLOG_USING_ECL
    easylog_obj->ecl = ra_info.ecl;
#endif

    return OS_EOK;
}
#endif /* MOLINK_EASYLOG_USING_RADIOINFO */

#ifdef MOLINK_EASYLOG_USING_USER_DATA
static void mo_easylog_init_user_data(const os_uint8_t *data, os_size_t len)
{
    if (OS_NULL == data)
    {
        os_kprintf("%s [%d]: easylog init user data: data is OS_NULL\r\n", __func__, __LINE__);
        return;
    }

    if ((0 == len) || (len > MO_EASYLOG_USER_DATA_LEN))
    {
        os_kprintf("Easylog init user data len[%d] error, max %d\r\n", len, MO_EASYLOG_USER_DATA_LEN);
        return;
    }
    else
    {
        memcpy(easylog_obj->user_data, data, len);
    }

    return;
}
#endif /* MOLINK_EASYLOG_USING_USER_DATA */

#ifdef MOLINK_EASYLOG_USING_CHECKSUM
static os_uint8_t mo_easylog_calculate_tx_checksum(os_uint8_t *data, os_uint32_t len)
{
    os_uint32_t i   = 0;
    os_uint8_t  ret = 0;

    if ((OS_NULL == data) || (len == 0))
    {
        os_kprintf("%s [%d]: data is OS_NULL, or len error\r\n", __func__, __LINE__);
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        ret += *(data++);
    }

    return (~ret);
}

os_uint8_t mo_easylog_rx_data_check(os_uint8_t *data, os_uint32_t len)
{
    os_uint32_t i   = 0;
    os_uint8_t  ret = 0;

    if ((OS_NULL == data) || (len == 0))
    {
        os_kprintf("%s [%d]: data is OS_NULL, or len error\r\n", __func__, __LINE__);
        return 0xFF;
    }

    for (i = 0; i < len; i++)
    {
        ret += *(data++);
    }

    return (ret + 1);
}
#endif /* MOLINK_EASYLOG_USING_CHECKSUM */

static os_err_t mo_easylog_info_update(const os_uint8_t *user_data, os_size_t data_len)
{
#ifdef OS_USING_RTC
    mo_easylog_get_sys_time();
#endif

    mo_easylog_network_status_t status = mo_easylog_get_network_status(def_module, easylog_obj);
    easylog_obj->net_status = (os_uint8_t)status;
    if (NETWORK_AT_TEST_ERROR == status)
    {
        os_kprintf("%s [%d]: easylog info update warning, module cannot communicate with MCU\r\n", __func__, __LINE__);
        goto __flag;
    }

#ifdef MOLINK_EASYLOG_USING_IMEI
    mo_easylog_get_imei();
#endif

#ifdef MOLINK_EASYLOG_USING_IMSI
    mo_easylog_get_imsi();
#endif

#ifdef MOLINK_EASYLOG_USING_ICCID
    mo_easylog_get_iccid();
#endif

#ifdef MOLINK_EASYLOG_USING_CSQ
    mo_easylog_get_csq();
#endif

#ifdef MOLINK_EASYLOG_USING_RADIOINFO
    mo_easylog_get_radio_info();
#endif

__flag:

#ifdef MOLINK_EASYLOG_USING_USER_DATA
    mo_easylog_init_user_data(user_data, data_len);
#endif

#ifdef MOLINK_EASYLOG_USING_CHECKSUM
    /* The checksum field is still 0 When we calculate the checksum */
    easylog_obj->checksum = mo_easylog_calculate_tx_checksum((os_uint8_t *)easylog_obj, sizeof(mo_easylog_t));

    if (0 == mo_easylog_rx_data_check((os_uint8_t *)easylog_obj, sizeof(mo_easylog_t)))
    {
        os_kprintf("%s [%d]: easylog info update success\r\n", __func__, __LINE__);
    }
    else
    {
        return OS_ERROR;
    }
#endif

    return OS_EOK;
}

os_err_t mo_easylog_info_printf(mo_easylog_t *easylog_obj)
{
    if (OS_NULL == easylog_obj)
    {
        os_kprintf("test easylog info printf: easylog obj or avgflash_obj is OS_NULL\r\n");
        return OS_ERROR;
    }

    os_kprintf("\r\nMolink easylog info:\r\n");
    os_kprintf("Easylog  size: %d bytes, upload: %d\r\n", easylog_obj->len, easylog_obj->upload_flag);

#ifdef OS_USING_RTC
    os_kprintf("Sys      time: %s\r\n", &(easylog_obj->sys_time[0]));
#endif

    os_kprintf("Network  info: net_status[%d]", easylog_obj->net_status);

#ifdef MOLINK_EASYLOG_USING_CSQ
    os_kprintf(", csq_rssi[%d], csq_ber[%d]\r\n", easylog_obj->csq_rssi, easylog_obj->csq_ber);
#endif

#ifdef MOLINK_EASYLOG_USING_IMEI
    os_kprintf("imei     info: %s\r\n", easylog_obj->imei);
#endif

#ifdef MOLINK_EASYLOG_USING_IMSI
    os_kprintf("imsi     info: %s\r\n", easylog_obj->imsi);
#endif

#ifdef MOLINK_EASYLOG_USING_ICCID
    os_kprintf("iccid    info: %s\r\n", easylog_obj->iccid);
#endif

#ifdef MOLINK_EASYLOG_USING_RADIOINFO
    os_kprintf("Radio    info: ");
#endif

#ifdef MOLINK_EASYLOG_USING_PCI
    os_kprintf("pci[%d], ", easylog_obj->pci);
#endif

#ifdef MOLINK_EASYLOG_USING_EARFCN
    os_kprintf("earfcn[%d], ", easylog_obj->earfcn);
#endif

#ifdef MOLINK_EASYLOG_USING_CELLID
    os_kprintf("cell_id[%s], ", easylog_obj->cell_id);
#endif

#ifdef MOLINK_EASYLOG_USING_RSRP
    os_kprintf("rsrp[%d], ", easylog_obj->rsrp);
#endif

#ifdef MOLINK_EASYLOG_USING_RSRQ
    os_kprintf("rsrq[%d], ", easylog_obj->rsrq);
#endif

#ifdef MOLINK_EASYLOG_USING_SNR
    os_kprintf("snr[%d], ", easylog_obj->snr);
#endif

#ifdef MOLINK_EASYLOG_USING_TX_PWR
    os_kprintf("tx_power[%d], ", easylog_obj->tx_power);
#endif

#ifdef MOLINK_EASYLOG_USING_ECL
    os_kprintf("ecl[%d]\r\n", easylog_obj->ecl);
#endif

#ifdef MOLINK_EASYLOG_USING_USER_DATA
    os_kprintf("user data len[%d], context in hex format:\r\n", MO_EASYLOG_USER_DATA_LEN);
    for (os_int16_t i = 0; i < MO_EASYLOG_USER_DATA_LEN; i++)
    {
        os_kprintf("%02x ", easylog_obj->user_data[i]);
        if (0 == ((i + 1) % 32))
        {
            os_kprintf("\r\n");
        }
    }
    os_kprintf("\r\n");
#endif

#ifdef MOLINK_EASYLOG_USING_CHECKSUM
    if (0 == mo_easylog_rx_data_check((os_uint8_t *)easylog_obj, sizeof(mo_easylog_t)))
    {
        os_kprintf("easylog checksum [0x%02x], correct\r\n", easylog_obj->checksum);
    }
    else
    {
        os_kprintf("easylog checksum [0x%02x], error\r\n", easylog_obj->checksum);
    }
#endif

    return OS_EOK;
}

mo_easylog_t *mo_easylog_create(const os_uint8_t *user_data, os_size_t data_len)
{
    os_uint16_t size   = 0;
    os_err_t    result = OS_EOK;

    def_module = mo_get_default();
    if (OS_NULL == def_module)
    {
        os_kprintf("%s-%d: no default module obj. Create module obj first.\r\n", __func__, __LINE__);
        return OS_NULL;
    }

    size = sizeof(mo_easylog_t);

    easylog_obj = (mo_easylog_t *)os_malloc(size);
    if (OS_NULL == easylog_obj)
    {
        os_kprintf("Create %s easylog instance failed, no enough memory.\r\n", def_module->name);
        return OS_NULL;
    }

    memset(easylog_obj, 0, size);
    easylog_obj->upload_flag = EASYLOG_NOT_UPLOAD;
    easylog_obj->len         = size;

    if(OS_EOK != mo_easylog_info_update(user_data, data_len))
    {
        os_kprintf("Molink easylog info update fail\r\n");
    }
    else
    {
        os_kprintf("Molink easylog info update success\r\n");
    }

#ifdef MOLINK_EASYLOG_INFO_PRINTF
    mo_easylog_info_printf(easylog_obj);
#endif

    return easylog_obj;
}

os_err_t mo_easylog_destory(mo_easylog_t *easylog)
{
    if (OS_NULL == easylog)
    {
        os_kprintf("%s [%d]: destory easylog failed, easylog_obj is NULL\r\n", __func__, __LINE__);
        return OS_ERROR;
    }

    if (OS_NULL != easylog)
    {
        os_free(easylog);
        easylog_obj = OS_NULL;
    }

    def_module  = OS_NULL;

    return OS_EOK;
}

#ifdef OS_USING_SHELL
static void mo_easylog_test_info_printf(void)
{
    os_uint8_t test_data[MO_EASYLOG_USER_DATA_LEN] = {0, 1, 2, 3, 4, 5, 6, 7};

    mo_easylog_t *easylog = mo_easylog_create(test_data, MO_EASYLOG_USER_DATA_LEN);

    if (OS_NULL == easylog)
    {
        os_kprintf("Mo easylog printf test fail.\r\n");
        return;
    }

#ifndef MOLINK_EASYLOG_INFO_PRINTF
    mo_easylog_info_printf(easylog);
#endif
    mo_easylog_destory(easylog);

    return;
}
SH_CMD_EXPORT(easylog_printf, mo_easylog_test_info_printf, "mo easylog printf test");
#endif

#endif /* MOLINK_USING_EASYLOG */

#endif /* NET_USING_MOLINK */
