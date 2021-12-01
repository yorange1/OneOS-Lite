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
 * @file        air720uh.c
 *
 * @brief       air720uh module link kit netconn api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "air720uh_netconn.h"
#include "air720uh.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "air720uh_netconn"
#define MO_LOG_LVL MO_LOG_INFO
#include "mo_log.h"

#define SEND_DATA_MAX_SIZE (1460)

#define AIR720_NETCONN_MQ_NAME "air720uh_nc_mq"

#ifndef AIR720_NETCONN_MQ_MSG_SIZE
#define AIR720_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef AIR720_NETCONN_MQ_MSG_MAX
#define AIR720_NETCONN_MQ_MSG_MAX (5)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define AIR720UH_EVENT_SEND_OK    (1L << 0)
#define AIR720UH_EVENT_SEND_FAIL  (1L << 1)
#define AIR720UH_EVENT_RECV_OK    (1L << 2)
#define AIR720UH_EVENT_CONN_OK    (1L << 3)
#define AIR720UH_EVENT_CONN_FAIL  (1L << 4)
#define AIR720UH_EVENT_CLOSE_OK   (1L << 5)
#define AIR720UH_EVENT_CLOSE_FAIL (1L << 6)
#define AIR720UH_EVENT_DOMAIN_OK  (1L << 7)
#define AIR720UH_EVENT_LINK_OK    (1L << 8)
#define AIR720UH_EVENT_LINK_FAIL  (1L << 9)

#define AIR720UH_MULTI_CONN_ENABLE  (1)
#define AIR720UH_MULTI_CONN_DISABLE (0)

#ifdef AIR720UH_USING_NETCONN_OPS

static os_err_t air720uh_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t air720uh_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *air720uh_netconn_alloc(mo_object_t *module)
{
    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    for (int i = 0; i < AIR720UH_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == air720uh->netconn[i].stat)
        {
            air720uh->netconn[i].connect_id = i;
            return &air720uh->netconn[i];
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);
    return OS_NULL;
}

static mo_netconn_t *air720uh_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id || AIR720UH_NETCONN_NUM <= connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    return &air720uh->netconn[connect_id];
}

os_err_t air720uh_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    info->netconn_array = air720uh->netconn;
    info->netconn_nums = sizeof(air720uh->netconn) / sizeof(air720uh->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *air720uh_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    air720uh_lock(&air720uh->netconn_lock);

    mo_netconn_t *netconn = air720uh_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        air720uh_unlock(&air720uh->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(AIR720_NETCONN_MQ_NAME, AIR720_NETCONN_MQ_MSG_SIZE, AIR720_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s data queue create failed, no enough memory.", module->name);
        air720uh_unlock(&air720uh->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    air720uh_unlock(&air720uh->netconn_lock);

    return netconn;
}

static os_err_t air720uh_netconn_close(mo_object_t *module, mo_netconn_t *netconn)
{
    os_uint32_t event;
    mo_air720uh_t *air720uh;
    at_parser_t *parser;
    char at_cmd[20] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num = 1,
                      .timeout = 10 * AT_RESP_TIMEOUT_DEF};
    os_err_t result = OS_ERROR;

    event = SET_EVENT(netconn->connect_id, AIR720UH_EVENT_CLOSE_OK | AIR720UH_EVENT_CLOSE_FAIL);
    air720uh = os_container_of(module, mo_air720uh_t, parent);
    os_event_recv(&air720uh->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);

    parser = &module->parser;
    sprintf(at_cmd, "AT+CIPCLOSE=%d\r\n", netconn->connect_id);
    result = at_parser_send(parser, at_cmd, 15);
    if (result < 0)
    {
        goto __exit;
    }

    result = os_event_recv(&air720uh->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait close event timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&air720uh->netconn_evt,
                           AIR720UH_EVENT_CLOSE_OK | AIR720UH_EVENT_CLOSE_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait close result timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    if (event & AIR720UH_EVENT_CONN_FAIL)
    {
        ERROR("Module %s netconn id %d close failed!", module->name, netconn->connect_id);
    }
__exit:

    return result;
}

os_err_t air720uh_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    DEBUG("Module %s in %d netconnn status", module->name, netconn->stat);

    switch (netconn->stat)
    {
    case NETCONN_STAT_CONNECT:
        air720uh_netconn_close(module, netconn);
        break;
    default:
        /* add handler when we need */
        break;
    }

    if (netconn->stat != NETCONN_STAT_NULL)
    {
        mo_netconn_mq_destroy(netconn->mq);
        netconn->mq = OS_NULL;
    }

    INFO("Module %s netconnn id %d destroyed", module->name, netconn->connect_id);

    netconn->connect_id = -1;
    netconn->stat = NETCONN_STAT_NULL;
    netconn->type = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);

    return OS_EOK;
}

#ifdef AIR720UH_USING_DNS
os_err_t air720uh_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != domain_name);
    OS_ASSERT(OS_NULL != addr);

    at_parser_t *parser = &self->parser;
    char resp_buff[256] = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * OS_TICK_PER_SECOND};

    mo_air720uh_t *air720uh = os_container_of(self, mo_air720uh_t, parent);

    os_event_recv(&air720uh->netconn_evt,
                  AIR720UH_EVENT_DOMAIN_OK,
                  OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                  OS_NO_WAIT,
                  OS_NULL);

    air720uh->netconn_data = addr;

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CDNSGIP=\"%s\"", domain_name);
    if (result < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    result = os_event_recv(&air720uh->netconn_evt,
                           AIR720UH_EVENT_DOMAIN_OK,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);

__exit:
    air720uh->netconn_data = OS_NULL;
    return result;
}
#endif

os_err_t air720uh_check_link_status(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_uint32_t link_status = 0;
    os_err_t result;
    os_int32_t mode;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    os_event_recv(&air720uh->netconn_evt,
                  AIR720UH_EVENT_LINK_OK | AIR720UH_EVENT_LINK_FAIL,
                  OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                  OS_NO_WAIT,
                  OS_NULL);

    result = at_parser_exec_cmd(parser, &resp, "AT+CIPSTATUS");
    if (result < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    result = os_event_recv(&air720uh->netconn_evt,
                           AIR720UH_EVENT_LINK_OK | AIR720UH_EVENT_LINK_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           5 * OS_TICK_PER_SECOND,
                           &link_status);

    if (AIR720UH_EVENT_LINK_OK != link_status)
    { /* active link */
        at_parser_exec_cmd(parser, &resp, "AT+CIPSHUT");

        result = at_parser_exec_cmd(parser, &resp, "AT+CIPMUX?");
        if (result != OS_EOK)
        {
            return result;
        }

        mode = 0;
        if (at_resp_get_data_by_kw(&resp, "+CIPMUX:", "+CIPMUX:%d", &mode) <= 0)
        {
            return result;
        }

        if (AIR720UH_MULTI_CONN_DISABLE == mode)
        {
            result = at_parser_exec_cmd(parser, &resp, "AT+CIPMUX=1");
        }

        if (result != OS_EOK)
        {
            return result;
        }

        result = at_parser_exec_cmd(parser, &resp, "AT+CSTT");
        if (result != OS_EOK)
        {
            return result;
        }

        result = at_parser_exec_cmd(parser, &resp, "AT+CIICR");
        if (result != OS_EOK)
        {
            return result;
        }

        at_parser_exec_cmd(parser, &resp, "AT+CIFSR");
    }

__exit:
    return result;
}

os_err_t air720uh_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    os_uint32_t event;
    os_err_t result;
    at_parser_t *parser = &module->parser;
    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    air720uh_lock(&air720uh->netconn_lock);
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};
    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);
    event = SET_EVENT(netconn->connect_id, AIR720UH_EVENT_CONN_OK | AIR720UH_EVENT_CONN_FAIL);
    os_event_recv(&air720uh->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);

    result = OS_EOK;

    switch (netconn->type)
    {
#ifdef AIR720UH_USING_TCP
    case NETCONN_TYPE_TCP:
        result =
            at_parser_exec_cmd(parser, &resp, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d", netconn->connect_id, remote_ip, port);
        break;
#endif

#ifdef AIR720UH_USING_UDP
    case NETCONN_TYPE_UDP:
        result =
            at_parser_exec_cmd(parser, &resp, "AT+CIPSTART=%d,\"UDP\",\"%s\",%d", netconn->connect_id, remote_ip, port);
        break;
#endif

    default:
        ERROR("Module %s connect to %s:%d protocol %d failed!", module->name, remote_ip, port, netconn->type);
        result = OS_ERROR;
        goto __exit;
    }

    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&air720uh->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait connect event timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&air720uh->netconn_evt,
                           AIR720UH_EVENT_CONN_OK | AIR720UH_EVENT_CONN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait connect result timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    if (event & AIR720UH_EVENT_CONN_FAIL)
    {
        result = OS_ERROR;
        ERROR("Module %s netconn id %d conect failed!", module->name, netconn->connect_id);
    }

__exit:
    if (OS_EOK == result)
    {
        ip_addr_copy(netconn->remote_ip, addr);
        netconn->remote_port = port;
        netconn->stat = NETCONN_STAT_CONNECT;

        DEBUG("Module %s connect to %s:%d successfully!", module->name, remote_ip, port);
    }
    else
    {
        ERROR("Module %s connect to %s:%d failed!", module->name, remote_ip, port);
    }
    air720uh_unlock(&air720uh->netconn_lock);

    return result;
}

os_size_t air720uh_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser = &module->parser;
    os_err_t result = OS_EOK;
    os_size_t sent_size = 0;
    os_size_t curr_size = 0;
    os_uint32_t event = 0;

    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    at_parser_exec_lock(parser);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    at_parser_set_end_mark(parser, ">", 1);

    while (sent_size < size)
    {
        if (netconn->stat != NETCONN_STAT_CONNECT)
        {
            ERROR("Module %s netconn %d isn't in the CONNECT state, send data fail", parser->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        if (size - sent_size < SEND_DATA_MAX_SIZE)
        {
            curr_size = size - sent_size;
        }
        else
        {
            curr_size = SEND_DATA_MAX_SIZE;
        }

        result = at_parser_exec_cmd(parser, &resp, "AT+CIPSEND=%d,%d", netconn->connect_id, curr_size);
        if (result != OS_EOK)
        {
            goto __exit;
        }

        if (at_parser_send(parser, data + sent_size, curr_size) != curr_size)
        {
            result = OS_ERROR;
            ERROR("Drv or hardware send data fail, some data is missing.\r\n");
            goto __exit;
        }

        result = os_event_recv(&air720uh->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               10 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&air720uh->netconn_evt,
                               AIR720UH_EVENT_SEND_OK | AIR720UH_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & AIR720UH_EVENT_SEND_FAIL)
        {
            ERROR("Module %s connect id %d send failed!", module->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        sent_size += curr_size;

        os_task_msleep(300);
    }

__exit:

    at_parser_set_end_mark(parser, OS_NULL, 0);

    at_parser_exec_unlock(parser);

    if (result != OS_EOK)
    {
        ERROR("Module %s connect id %d send %d bytes data failed", module->name, netconn->connect_id, size);
        return 0;
    }

    return sent_size;
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);
    os_int32_t connect_id = -1;

    connect_id = data[0] - '0';
    if (strstr(data, "SEND OK"))
    {
        os_event_send(&air720uh->netconn_evt, SET_EVENT(connect_id, AIR720UH_EVENT_SEND_OK));
    }
    else if (strstr(data, "SEND FAIL"))
    {
        os_event_send(&air720uh->netconn_evt, SET_EVENT(connect_id, AIR720UH_EVENT_SEND_FAIL));
    }
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;

    sscanf(data, "%d,CLOSED", &connect_id);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = air720uh_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    WARN("Module %s receive close urc data of connect %d", module->name, connect_id);

    mo_netconn_pasv_close_notice(netconn);

    return;
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size = 0;

    sscanf(data, "+RECEIVE,%d,%d:", &connect_id, &data_size);

    os_int32_t timeout = data_size > 10 ? data_size : 10;

    INFO("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = air720uh_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    char *recv_buff = os_calloc(1, data_size);
    char temp_buff[8] = {0};

    if (recv_buff == OS_NULL)
    {
        /* read and clean the coming data */
        ERROR("Calloc recv buff %d bytes fail, no enough memory", data_size);
        os_size_t temp_size = 0;

        while (temp_size < data_size)
        {
            if (data_size - temp_size > sizeof(temp_buff))
            {
                at_parser_recv(parser, temp_buff, sizeof(temp_buff), timeout);
            }
            else
            {
                at_parser_recv(parser, temp_buff, data_size - temp_size, timeout);
            }
            temp_size += sizeof(temp_buff);
        }
        return;
    }

    at_parser_recv(parser, temp_buff, 2, timeout);

    if (at_parser_recv(parser, recv_buff, data_size, timeout) != data_size)
    {
        ERROR("Module %s netconnt id %d recv %d bytes data failed!", module->name, netconn->connect_id, data_size);
        os_free(recv_buff);

        return;
    }

    mo_netconn_data_recv_notice(netconn, recv_buff, data_size);

    return;
}

static void urc_recv_dns(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    int j = 0;

    for (int i = 0; i < size; i++)
    {
        if (*(data + i) == '.')
            j++;
    }

    /* There would be several dns result, we just pickup one */
    if ((j > 2) && (j % 3 == 2))
    {
        char recvip[IPADDR_MAX_STR_LEN + 1] = {0};
        sscanf(data, "+CDNSGIP: 1,%*[^,],\"%[^\"]\"", recvip);
        recvip[IPADDR_MAX_STR_LEN] = '\0';
        inet_aton(recvip, (ip_addr_t *)air720uh->netconn_data);
        os_event_send(&air720uh->netconn_evt, AIR720UH_EVENT_DOMAIN_OK);
    }
    else
    {
        ERROR("Module %s gethostbyname failed!", module->name);
    }
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    os_int32_t connect_id = -1;
    connect_id = data[0] - '0';
    if (strstr(data, "CONNECT OK"))
    {
        os_event_send(&air720uh->netconn_evt, SET_EVENT(connect_id, AIR720UH_EVENT_CONN_OK));
    }
    else
    {
        os_event_send(&air720uh->netconn_evt, SET_EVENT(connect_id, AIR720UH_EVENT_CONN_FAIL));
    }
}

static void urc_destory_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    os_int32_t connect_id = -1;
    connect_id = data[0] - '0';
    if (strstr(data, "CLOSE OK"))
    {
        os_event_send(&air720uh->netconn_evt, SET_EVENT(connect_id, AIR720UH_EVENT_CLOSE_OK));
    }
    else
    {
        os_event_send(&air720uh->netconn_evt, SET_EVENT(connect_id, AIR720UH_EVENT_CLOSE_FAIL));
    }
}

static void urc_link_status_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_air720uh_t *air720uh = os_container_of(module, mo_air720uh_t, parent);

    if (strstr(data, "STATE: IP STATUS") || strstr(data, "STATE: IP PROCESSING"))
    {
        os_event_send(&air720uh->netconn_evt, AIR720UH_EVENT_LINK_OK);
    }
    else
    {
        os_event_send(&air720uh->netconn_evt, AIR720UH_EVENT_LINK_FAIL);
    }
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "", .suffix = "SEND OK\r\n", .func = urc_send_func},
    {.prefix = "", .suffix = "SEND FAIL\r\n", .func = urc_send_func},
    {.prefix = "+RECEIVE", .suffix = ":", .func = urc_recv_func},
    {.prefix = "", .suffix = ",CLOSED\r\n", .func = urc_close_func},
    {.prefix = "+CDNSGIP:", .suffix = "\r\n", .func = urc_recv_dns},
    {.prefix = "", .suffix = "CONNECT OK\r\n", .func = urc_connect_func},
    {.prefix = "", .suffix = "CLOSE OK\r\n", .func = urc_destory_func},
    {.prefix = "STATE: ", .suffix = "\r\n", .func = urc_link_status_func},
};

os_err_t air720uh_netconn_init(mo_air720uh_t *module)
{
    at_parser_t *parser = &(module->parent.parser);
    os_err_t result;
    os_int32_t mode;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    resp.line_num = 1;
    at_parser_exec_cmd(parser, &resp, "AT+CIPSHUT");

    resp.line_num = 0;
    result = at_parser_exec_cmd(parser, &resp, "AT+CIPMUX?");
    if (result != OS_EOK)
    {
        return result;
    }

    mode = 0;
    if (at_resp_get_data_by_kw(&resp, "+CIPMUX:", "+CIPMUX:%d", &mode) <= 0)
    {
        return result;
    }

    if (AIR720UH_MULTI_CONN_DISABLE == mode)
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CIPMUX=1");
    }

    if (result != OS_EOK)
    {
        return result;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+CSTT");
    if (result != OS_EOK)
    {
        return result;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+CIICR");
    if (result != OS_EOK)
    {
        return result;
    }

    resp.line_num = 1;
    at_parser_exec_cmd(parser, &resp, "AT+CIFSR");

    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < AIR720UH_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    return result;
}

#endif /* AIR720UH_USING_NETCONN_OPS */
