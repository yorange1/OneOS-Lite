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
 * @file        mo_easylog.h
 *
 * @brief       The modules network log functions header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __MO_EASYLOG_H__
#define __MO_EASYLOG_H__

#include <os_task.h>
#include <os_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NET_USING_MOLINK
#include <mo_api.h>

#ifdef MOLINK_USING_EASYLOG

#define MO_EASYLOG_SYS_TIME_LEN   (23)
#define MO_EASTLOG_CSQ_WARNING_LV (10) /* Alarm threshold for poor signal quality, define by user */

#ifndef MO_EASYLOG_USER_DATA_LEN
#define MO_EASYLOG_USER_DATA_LEN (8) /* The size defined by user application, default value 8 */
#endif

typedef enum mo_easylog_upload_flag
{
    EASYLOG_NOT_UPLOAD = 0,
    EASYLOG_UPLOAD,
    EASYLOG_NOT_UPLOAD_UNDEF
} mo_easylog_upload_flag_t;

typedef enum mo_easylog_network_status
{
    NETWORK_STATUS_OK = 0,
    NETWORK_AT_TEST_ERROR,
    NETWORK_CFUN_STATUS_ERROR,
    NETWORK_SIM_CARD_ERROR,
    NETWORK_ATTACH_STATUS_ERROR,
    NETWORK_CEREG_STATUS_ERROR,
    NETWORK_CSQ_WARNING,
    NETWORK_STATUS_UNDEF = 0xFF,
} mo_easylog_network_status_t;

typedef struct mo_easylog
{
    os_uint8_t upload_flag;
    os_uint16_t len;
    os_uint8_t net_status;
#ifdef OS_USING_RTC
    char sys_time[MO_EASYLOG_SYS_TIME_LEN + 1]; /* "Jan  1 01:07:59 2000"--2000年01月01日01时07分59秒 */
#endif
#ifdef MOLINK_EASYLOG_USING_IMEI
    char imei[MO_IMEI_LEN + 1];
#endif
#ifdef MOLINK_EASYLOG_USING_IMSI
    char imsi[MO_IMSI_LEN + 1];
#endif
#ifdef MOLINK_EASYLOG_USING_ICCID
    char iccid[MO_ICCID_LEN + 1];
#endif
#ifdef MOLINK_EASYLOG_USING_CSQ
    os_uint8_t csq_rssi;
    os_uint8_t csq_ber;
#endif
#ifdef MOLINK_EASYLOG_USING_PCI
    os_int32_t pci;
#endif
#ifdef MOLINK_EASYLOG_USING_EARFCN
    os_int32_t earfcn;
#endif
#ifdef MOLINK_EASYLOG_USING_CELLID
    char cell_id[CELL_ID_MAX_LEN + 1];
#endif
#ifdef MOLINK_EASYLOG_USING_RSRP
    os_int32_t rsrp;
#endif
#ifdef MOLINK_EASYLOG_USING_RSRQ
    os_int32_t rsrq;
#endif
#ifdef MOLINK_EASYLOG_USING_SNR
    os_int32_t snr;
#endif
#ifdef MOLINK_EASYLOG_USING_TX_PWR
    os_int32_t tx_power;
#endif
#ifdef MOLINK_EASYLOG_USING_ECL
    os_int32_t ecl;
#endif
#ifdef MOLINK_EASYLOG_USING_USER_DATA
    os_uint8_t user_data[MO_EASYLOG_USER_DATA_LEN];
#endif
#ifdef MOLINK_EASYLOG_USING_CHECKSUM
    os_uint8_t checksum;
#endif
} mo_easylog_t;

#ifdef MOLINK_EASYLOG_USING_CHECKSUM
os_uint8_t mo_easylog_calculate_rx_checksum(os_uint8_t *data, os_uint32_t len);
#endif

mo_easylog_t *mo_easylog_create(const os_uint8_t *user_data, os_size_t data_len);
os_err_t mo_easylog_destory(mo_easylog_t *easylog);
os_err_t mo_easylog_info_printf(mo_easylog_t *easylog_obj);
#endif /* MOLINK_USING_EASYLOG */

#endif /* NET_USING_MOLINK */

#ifdef __cplusplus
}
#endif

#endif /* __MO_EASYLOG_H__ */
