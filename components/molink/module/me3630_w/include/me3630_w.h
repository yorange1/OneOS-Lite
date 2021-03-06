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
 * @file        me3630_w.h
 *
 * @brief       me3630_w module api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-1   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ME3630_W_H__
#define __ME3630_W_H__

#include "mo_object.h"

#ifdef ME3630_W_USING_GENERAL_OPS
#include "me3630_w_general.h"
#endif

#ifdef ME3630_W_USING_IFCONFIG_OPS
#include "me3630_w_ifconfig.h"
#endif

#ifdef ME3630_W_USING_NETCONN_OPS
#include "me3630_w_netconn.h"
#endif

#ifdef ME3630_W_USING_NETSERV_OPS
#include "me3630_w_netserv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_ME3630_W

#ifndef ME3630_W_NAME
#define ME3630_W_NAME "me3630_w"
#endif

#ifndef ME3630_W_DEVICE_NAME
#define ME3630_W_DEVICE_NAME "uart2"
#endif

#ifndef ME3630_W_RECV_BUFF_LEN
#define ME3630_W_RECV_BUFF_LEN 512
#endif

#ifndef ME3630_W_NETCONN_NUM
#define ME3630_W_NETCONN_NUM 5
#endif

typedef struct mo_me3630_w
{
    mo_object_t parent;
#ifdef ME3630_W_USING_NETCONN_OPS
    mo_netconn_t netconn[ME3630_W_NETCONN_NUM];

    os_int32_t curr_connect;
    os_event_t netconn_evt;
    os_mutex_t netconn_lock;
    void *netconn_data;
#endif /* ME3630_W_USING_NETCONN_OPS */
} mo_me3630_w_t;

mo_object_t *module_me3630_w_create(const char *name, void *parser_config);
os_err_t module_me3630_w_destroy(mo_object_t *self);

#endif /* MOLINK_USING_ME3630_W */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ME3630_W_H__ */
