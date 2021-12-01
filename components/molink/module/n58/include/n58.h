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
 * @file        n58.h
 *
 * @brief       n58 module api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-30   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __N58_H__
#define __N58_H__

#include "mo_object.h"

#ifdef N58_USING_GENERAL_OPS
#include "n58_general.h"
#endif

#ifdef N58_USING_NETSERV_OPS
#include "n58_netserv.h"
#endif

#ifdef N58_USING_PING_OPS
#include "n58_ping.h"
#endif

#ifdef N58_USING_IFCONFIG_OPS
#include "n58_ifconfig.h"
#endif

#ifdef N58_USING_NETCONN_OPS
#include "n58_netconn.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_N58

#ifndef N58_NAME
#define N58_NAME "n58"
#endif

#ifndef N58_DEVICE_NAME
#define N58_DEVICE_NAME "uart2"
#endif

#ifndef N58_RECV_BUFF_LEN
#define N58_RECV_BUFF_LEN 512
#endif

#ifndef N58_NETCONN_NUM
#define N58_NETCONN_NUM 6
#endif

typedef struct mo_n58
{
    mo_object_t parent;
#ifdef N58_USING_NETCONN_OPS
    mo_netconn_t netconn[N58_NETCONN_NUM];

    os_int32_t curr_connect;
    os_event_t netconn_evt;
    os_mutex_t netconn_lock;
    void *netconn_data;
#endif /* N58_USING_NETCONN_OPS */
} mo_n58_t;

mo_object_t *module_n58_create(const char *name, void *parser_config);
os_err_t module_n58_destroy(mo_object_t *self);

#endif /* MOLINK_USING_N58 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __N58_H__ */
