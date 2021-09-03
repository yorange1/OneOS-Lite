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
 * @file        uart_dma.c
 *
 * @brief       This file provides uart dma functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _DRIVERS_SOFT_DMA_H_
#define _DRIVERS_SOFT_DMA_H_

#include "string.h"
#include <os_stddef.h>
#include <os_timer.h>
#include <os_memory.h>
#include <drv_cfg.h>

#define HARD_DMA_MODE_NORMAL        0
#define HARD_DMA_MODE_CIRCULAR      1

#define SOFT_DMA_IRQ_HALF    (1 << 0)
#define SOFT_DMA_IRQ_FULL    (1 << 1)
#define SOFT_DMA_IRQ_TIMEOUT (1 << 2)

typedef struct soft_dma soft_dma_t;

#define SOFT_DMA_TIMEOUT_INVALID    0

typedef struct hard_info
{
    os_uint32_t mode;               /* HARD_DMA_MODE_CIRCULAR */
    os_uint32_t max_size;           /* 64 * 1024 */
    os_uint32_t soft_data_timeout;  /* SOFT_DMA_TIMEOUT_INVALID us */
    os_uint32_t soft_half_timeout;  /* SOFT_DMA_TIMEOUT_INVALID us */
}hard_dma_info_t;

typedef struct hard_dma_ops
{
    os_uint32_t (*get_index)(soft_dma_t *dma);
    os_err_t (*dma_start)(soft_dma_t *dma, void *buff, os_uint32_t size);
    os_err_t (*dma_stop)(soft_dma_t *dma);
}hard_dma_ops_t;

typedef struct soft_dma_callbacks
{
    void (*dma_half_callback)(soft_dma_t *dma);
    void (*dma_full_callback)(soft_dma_t *dma);
    void (*dma_timeout_callback)(soft_dma_t *dma);
}soft_dma_callbacks_t;

typedef struct dma_ring
{
    os_uint8_t *buff;
    int size;
    int head;
    int tail;
    int count;
}dma_ring_t;

typedef struct soft_dma_rx_buffer
{
    os_uint32_t  flag;
    
    dma_ring_t   buff;
    
    os_uint8_t  *hard_buffers[2];
    dma_ring_t   hard_buff;
}soft_dma_rx_buffer_t;

typedef struct soft_dma
{
    hard_dma_info_t         hard_info;
    hard_dma_ops_t          ops;
    soft_dma_callbacks_t    cbs;
    
    os_timer_t             *rx_timeout_timer;
    os_timer_t             *rx_half_timer;
    soft_dma_rx_buffer_t   *buffer;
    
    os_uint32_t             irq_status;
    os_uint32_t             irq_mask;
}soft_dma_t;

os_err_t soft_dma_init(soft_dma_t *dma);
os_err_t soft_dma_deinit(soft_dma_t *dma);
os_err_t soft_dma_start_recv(soft_dma_t *dma, os_uint8_t *buff, os_uint32_t size);
os_err_t soft_dma_stop_recv(soft_dma_t *dma);

void soft_dma_irq_mask_set(soft_dma_t *dma, os_uint32_t mask);
void soft_dma_irq_mask_clr(soft_dma_t *dma, os_uint32_t mask);

void soft_dma_half_irq(soft_dma_t *dma);
void soft_dma_full_irq(soft_dma_t *dma);
void soft_dma_timeout_irq(soft_dma_t *dma);

#endif // _DRIVERS_SOFT_DMA_H_
