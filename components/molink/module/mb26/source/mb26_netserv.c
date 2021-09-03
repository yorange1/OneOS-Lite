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
 * @file        mb26_netserv.c
 *
 * @brief       mb26 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mb26_netserv.h"
#include "mb26.h"

#define DBG_EXT_TAG "mb26.netserv"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef MB26_USING_NETSERV_OPS

os_err_t mb26_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CGATT=%d", attach_stat);
}

os_err_t mb26_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+CGATT:", "+CGATT: %hhu", attach_stat) <= 0)
    {
        LOG_EXT_E("Get %s module attach state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t mb26_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%d", reg_n);
}

os_err_t mb26_get_reg(mo_object_t *self, eps_reg_info_t *info)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG: %hhu,%hhu", &info->reg_n, &info->reg_stat) <= 0)
    {
        LOG_EXT_E("Get %s module register state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t mb26_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CGACT=%d,%d", act_stat, cid);
}

os_err_t mb26_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGACT:", "+CGACT: %hhu,%hhu", cid, act_stat) <= 0)
    {
        LOG_EXT_E("Get %s module cgact state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t mb26_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CSQ:", "+CSQ:%hhu,%hhu", rssi, ber) <= 0)
    {
        LOG_EXT_E("Get %s module signal quality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t mb26_get_radio(mo_object_t *self, radio_info_t *radio_info)
{
    at_parser_t *parser       = &self->parser;
    os_uint32_t  buff_len     = 1024;
    const char  *tmp_src_resp = OS_NULL;
    char        *dest_ptr     = OS_NULL;

    memset(radio_info, 0, sizeof(radio_info_t));

    char *resp_buff = malloc(buff_len);

    if (OS_NULL == resp_buff)
    {
        LOG_EXT_E("Module %s get radio info failed, no enough memory", self->name);
        return OS_ENOMEM;
    }
    memset(resp_buff, 0, buff_len);

    at_resp_t resp = {.buff = resp_buff, .buff_size = buff_len, .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+ECSTATUS");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    /* Get DlEarfcn info form resp buff */
    if (at_resp_get_data_by_kw(&resp, "+ECSTATUS: PHY", "+ECSTATUS: PHY, DlEarfcn:%d,", &radio_info->earfcn) <= 0)
    {
        LOG_EXT_E("Get %s module Earfcn failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    tmp_src_resp = at_resp_get_line_by_kw(&resp, "+ECSTATUS: PHY");

    /* Get PCI info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "PCI:");
    if (dest_ptr == OS_NULL)
    {
        LOG_EXT_E("Get %s module PCI failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "PCI:%d,", &radio_info->pci);

    /* Get RSRP info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "RSRP:");
    if (dest_ptr == OS_NULL)
    {
        LOG_EXT_E("Get %s module RSRP failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "RSRP:%d,",&radio_info->rsrp);

    /* Get RSRQ info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "RSRQ:");
    if (dest_ptr == OS_NULL)
    {
        LOG_EXT_E("Get %s module RSRQ failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "RSRQ:%d,",&radio_info->rsrq);

    /* Get SNR info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "SNR:");
    if (dest_ptr == OS_NULL)
    {
        LOG_EXT_E("Get %s module SNR failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "SNR:%d,",&radio_info->snr);

    /* Get ECL info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "CeLevel:");
    if (dest_ptr == OS_NULL)
    {
        LOG_EXT_E("Get %s module ECL failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "CeLevel:%d,",&radio_info->ecl);

    tmp_src_resp = at_resp_get_line_by_kw(&resp, "+ECSTATUS: RRC");
    /* Get CellId info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "CellId:");
    if (dest_ptr == OS_NULL)
    {
        LOG_EXT_E("Get %s module CellId failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "CellId:%s",radio_info->cell_id);

__exit:

    if(resp_buff != OS_NULL)
    {
        free(resp_buff);
    }

    return result;
}

#endif /* MB26_USING_NETSERV_OPS */
