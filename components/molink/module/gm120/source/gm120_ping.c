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
 * @file        gm120_ping.c
 *
 * @brief       gm120 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "gm120_ping.h"
#include <os_mb.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "gm120_ping"
#define MO_LOG_LVL MO_LOG_INFO
#include "mo_log.h"

#define GM120_MIN_PING_PKG_LEN (1)
#define GM120_MAX_PING_PKG_LEN (1400)
#define GM120_MIN_PING_TIMEOUT (1)
#define GM120_MAX_PING_TIMEOUT (65534)

#define MB_MAX_MAILS  5
#define TEST_NAME_MAX 16

#ifdef GM120_USING_PING_OPS

static os_mb_t *mb_dynamic;

static void urc_ping_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    char tmp_str[5] = {0};
    char *tmp;

    tmp = strstr(data, "time:");
    tmp = tmp + 5;
    for (int i = 0;; i++)
    {
        if ('m' == tmp[i])
        {
            break;
        }

        tmp_str[i] = tmp[i];
    }

    os_mb_send(mb_dynamic, (os_uint32_t)atoi(tmp_str), OS_WAIT_FOREVER);
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "statistics:", .suffix = "\r\n", .func = urc_ping_func},
};

os_err_t gm120_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser = &self->parser;
    os_err_t result = OS_EOK;
    os_uint32_t req_time = 0;
    os_int16_t ttl = -1;
    os_uint32_t timeout_s = timeout / 1000; /* Milliseconds convert to seconds */
    os_ubase_t recv_data = 0;

    if (parser == OS_NULL)
    {
        ERROR("GM120 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < GM120_MIN_PING_PKG_LEN) || (len > GM120_MAX_PING_PKG_LEN))
    {
        ERROR("GM120 ping: ping package len[%d] is out of range[%d, %d].",
              len,
              GM120_MIN_PING_PKG_LEN,
              GM120_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout_s < GM120_MIN_PING_TIMEOUT) || (timeout_s > GM120_MAX_PING_TIMEOUT))
    {
        ERROR("GM120 ping: ping timeout %us is out of range[%ds, %ds].",
              timeout_s,
              GM120_MIN_PING_TIMEOUT,
              GM120_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    /* Set netconn urc table */
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    mb_dynamic = os_mb_create("mailbox_dynamic", MB_MAX_MAILS);
    if (!mb_dynamic)
    {
        WARN("mailbox_dynamic_sample mailbox create ERR");
        return OS_ERROR;
    }
    DEBUG("GM120 ping: %s, len: %d, timeout: %us", host, len, timeout_s);

    char resp_buff[256] = {0};

    at_resp_t at_resp = {.buff = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .timeout = (5 + timeout_s) * OS_TICK_PER_SECOND};

    /* Send commond "AT+MPING=<domain name> or ip addr" and wait response */
    /* Exec AT+MPING="www.baidu.com",2,1,64 and return: 0 183.232.231.172: bytes = 36 time = 96(ms), TTL = 255 */

    if (at_parser_exec_cmd(parser, &at_resp, "AT+NPING=%s,%u,1,%d,%d", host, len, timeout_s, timeout_s + 1) < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }
    if (OS_EOK != os_mb_recv(mb_dynamic, &recv_data, (6 + timeout_s) * OS_TICK_PER_SECOND))
    {
        result = OS_ERROR;
        goto __exit;
    }

    req_time = recv_data;

    if (0 != req_time)
    {
        inet_aton(host, &(resp->ip_addr));
        resp->data_len = len;
        resp->ttl = ttl;
        resp->time = req_time;
    }
    else
    {
        result = OS_ERROR;
    }

__exit:
    os_mb_destroy(mb_dynamic);
    return result;
}

#endif /* GM120_USING_PING_OPS */
