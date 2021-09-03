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
#include "soft_dma.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.soft_dma"
#include <drv_log.h>

static os_uint8_t *soft_dma_normal_buffer_init(soft_dma_t *dma, os_uint8_t *buff, os_uint32_t size)
{
    return OS_NULL;
}

static os_uint8_t *soft_dma_circular_buffer_init(soft_dma_t *dma, os_uint8_t *buff, os_uint32_t size)
{
    /* buffer */
    dma->buffer = os_calloc(1, sizeof(soft_dma_rx_buffer_t));
    if (dma->buffer == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "dma->buffer call failed!");
        return OS_NULL;
    }

    /* soft buffer */
    dma->buffer->buff.buff = buff;
    dma->buffer->buff.size = size;

    /* hard buffer */
    if (size <= dma->hard_info.max_size)
    {
        dma->buffer->hard_buff.size = size;
        dma->buffer->hard_buff.buff = buff;
    }
    else
    {
        dma->buffer->hard_buff.size = min(128, size);
        dma->buffer->hard_buff.buff = os_calloc(1, dma->buffer->hard_buff.size);
    }

    OS_ASSERT(dma->buffer->hard_buff.buff != OS_NULL);
    
    return dma->buffer->hard_buff.buff;
}

os_uint8_t *soft_dma_buffer_init(soft_dma_t *dma, os_uint8_t *buff, os_uint32_t size)
{
    OS_ASSERT(dma->buffer == OS_NULL);
    
    if (dma->hard_info.mode == HARD_DMA_MODE_NORMAL)
    {
        return soft_dma_normal_buffer_init(dma, buff, size);
    }
    else if (dma->hard_info.mode == HARD_DMA_MODE_CIRCULAR)
    {
        return soft_dma_circular_buffer_init(dma, buff, size);
    }
    
    return OS_NULL;
}

os_err_t soft_dma_buffer_deinit(soft_dma_t *dma)
{
    if (dma->hard_info.mode == HARD_DMA_MODE_NORMAL)
    {
        os_free(dma->buffer->hard_buffers[0]);
        os_free(dma->buffer);
    }
    else if (dma->hard_info.mode == HARD_DMA_MODE_CIRCULAR)
    {
        if (dma->buffer->hard_buffers)
        {
            os_free(dma->buffer->hard_buffers);
        }
        os_free(dma->buffer);
    }
    return OS_EOK;
}

static void copy_line_to_ring(dma_ring_t *ring, os_uint8_t *line, int count)
{
    int copy_count;

    if (ring->head + count <= ring->size)
    {
        memcpy(ring->buff + ring->head, line, count);
        ring->head += count;
    }
    else
    {
        copy_count = ring->size - ring->head;
        count -= copy_count;

        memcpy(ring->buff + ring->head, line, copy_count);
        ring->head = 0;
        
        memcpy(ring->buff, line + copy_count, count);
        ring->head = count;
    }
}

static void copy_ring_to_ring(dma_ring_t *dst, dma_ring_t *src, int count)
{
    int copy_count;
    
    if (src->tail + count <= src->size)
    {
        copy_line_to_ring(dst, src->buff + src->tail, count);
        src->tail += count;
    }
    else
    {
        copy_count = src->size - src->tail;
        count -= copy_count;

        copy_line_to_ring(dst, src->buff + src->tail, copy_count);
        src->tail = 0;
        
        copy_line_to_ring(dst, src->buff, count);
        src->tail = count;
    }
}

static void copy_hardbuff_to_dst(soft_dma_t *dma, os_uint32_t head)
{
    os_uint32_t count;
    
    if (head >= dma->buffer->hard_buff.tail)
    {
        count = head - dma->buffer->hard_buff.tail;
    }
    else
    {
        count = head - dma->buffer->hard_buff.tail + dma->buffer->hard_buff.size;
    }

    OS_ASSERT(count > 0);

    copy_ring_to_ring(&dma->buffer->buff, &dma->buffer->hard_buff, count);
}

os_err_t soft_dma_updata_buffer(soft_dma_t *dma, os_uint32_t head)
{
    if (dma->hard_info.mode == HARD_DMA_MODE_NORMAL)
    {
        dma->irq_status = 0;
        return OS_EOK;
    }
    else if (dma->hard_info.mode == HARD_DMA_MODE_CIRCULAR)
    {
        /* update irq status */
        os_uint32_t last_head = dma->buffer->hard_buff.head;

        if (last_head < dma->buffer->buff.size / 2)
        {
            if (head >= dma->buffer->buff.size / 2)
            {
                dma->irq_status |= SOFT_DMA_IRQ_HALF;
            }
        }
        else
        {
            if (head < last_head)
            {
                dma->irq_status |= SOFT_DMA_IRQ_FULL;
            }
        }

        /* copy data */
        if (dma->buffer->hard_buff.buff == dma->buffer->buff.buff)
        {
            dma->buffer->buff.head = head;
        }
        else
        {
            copy_hardbuff_to_dst(dma, head);
        }

        dma->buffer->hard_buff.head = head;
        
        return OS_EOK;
    }
    
    return OS_ERROR;
}

os_err_t soft_dma_normal_full_updata_buffer(soft_dma_t *dma, os_uint32_t count)
{
    copy_hardbuff_to_dst(dma, count);
    
    if (dma->buffer->hard_buff.buff == dma->buffer->hard_buffers[0])
        dma->buffer->hard_buff.buff = dma->buffer->hard_buffers[1];
    else
        dma->buffer->hard_buff.buff = dma->buffer->hard_buffers[0];
    return OS_EOK;
}

os_err_t soft_dma_circular_full_updata_buffer(soft_dma_t *dma, os_uint32_t count)
{
    if (dma->buffer->hard_buffers == OS_NULL)
    {
        dma->irq_status |= SOFT_DMA_IRQ_FULL;
    }
    else
    {
        copy_hardbuff_to_dst(dma, count);
    }
    return OS_EOK;
}

os_err_t soft_dma_full_updata_buffer(soft_dma_t *dma, os_uint32_t count)
{
    if (dma->hard_info.mode == HARD_DMA_MODE_NORMAL)
    {
        return soft_dma_normal_full_updata_buffer(dma, count);
    }
    else if (dma->hard_info.mode == HARD_DMA_MODE_CIRCULAR)
    {
        return soft_dma_circular_full_updata_buffer(dma, count);
    }
    return OS_ERROR;
}

static void soft_dma_half_timer_callback(void *hard_info)
{
    soft_dma_t *dma = (soft_dma_t *)hard_info;
    
    soft_dma_half_irq(dma);
}

static void soft_dma_timeout_timer_callback(void *hard_info)
{
    soft_dma_t *dma = (soft_dma_t *)hard_info;
    
    soft_dma_timeout_irq(dma);
}

static OS_UNUSED os_err_t soft_dma_timout_timer_stop(soft_dma_t *dma)
{
    if (dma->rx_timeout_timer)
        return os_timer_stop(dma->rx_timeout_timer);
    return OS_EINVAL;
}

static os_err_t soft_dma_rx_half_timer_init(soft_dma_t *dma)
{
    dma->rx_half_timer = os_calloc(1, sizeof(os_timer_t));
    if (dma->rx_half_timer)
    {
        LOG_E(DRV_EXT_TAG, "dma->rx_half_timer call memory fialed!");
        return OS_ERROR;
    }
    os_timer_init(dma->rx_half_timer, 
              OS_NULL,
              soft_dma_timeout_timer_callback,
              dma, 
              dma->hard_info.soft_half_timeout,
              OS_TIMER_FLAG_ONE_SHOT);
    return OS_EOK;
}

static os_err_t soft_dma_rx_timout_timer_init(soft_dma_t *dma)
{
    dma->rx_timeout_timer = os_calloc(1, sizeof(os_timer_t));
    if (dma->rx_timeout_timer)
    {
        LOG_E(DRV_EXT_TAG, "dma->rx_timeout_timer call memory fialed!");
        return OS_ERROR;
    }
    os_timer_init(dma->rx_timeout_timer, 
              OS_NULL,
              soft_dma_half_timer_callback,
              dma, 
              dma->hard_info.soft_data_timeout,
              OS_TIMER_FLAG_PERIODIC);
    
    return OS_EOK;
}

static os_err_t soft_dma_normal_init(soft_dma_t *dma)
{
    if (dma->hard_info.soft_data_timeout != SOFT_DMA_TIMEOUT_INVALID)
    {
        if (dma->ops.get_index == OS_NULL)
        {
            LOG_E(DRV_EXT_TAG, "get_index and dma_half_callback must support!");
            return OS_ERROR;
        }
        if (soft_dma_rx_timout_timer_init(dma) != OS_EOK)
            return OS_ERROR;
    }
    return OS_EOK;
}

static os_err_t soft_dma_circular_init(soft_dma_t *dma)
{
    if (dma->hard_info.soft_data_timeout != SOFT_DMA_TIMEOUT_INVALID)
    {
        if (soft_dma_rx_timout_timer_init(dma) != OS_EOK)
            return OS_ERROR;
    }
    
    if (dma->hard_info.soft_half_timeout != SOFT_DMA_TIMEOUT_INVALID)
    {
        if (soft_dma_rx_half_timer_init(dma) != OS_EOK)
            return OS_ERROR;
    }
    
    return OS_EOK;
}

os_err_t soft_dma_init(soft_dma_t *dma)
{
    OS_ASSERT(dma->ops.get_index != OS_NULL);
    OS_ASSERT(dma->ops.dma_start != OS_NULL);
    OS_ASSERT(dma->ops.dma_stop != OS_NULL);
    OS_ASSERT(dma->cbs.dma_half_callback != OS_NULL);
    OS_ASSERT(dma->cbs.dma_full_callback != OS_NULL);
    OS_ASSERT(dma->cbs.dma_timeout_callback != OS_NULL);

    if (dma->hard_info.mode == HARD_DMA_MODE_NORMAL)
    {
        return soft_dma_normal_init(dma);
    }
    else if (dma->hard_info.mode == HARD_DMA_MODE_CIRCULAR)
    {
        return soft_dma_circular_init(dma);
    }
    
    return OS_ERROR;
}

os_err_t soft_dma_deinit(soft_dma_t *dma)
{
    return OS_EOK;
}

os_err_t soft_dma_start_recv(soft_dma_t *dma, os_uint8_t *buff, os_uint32_t size)
{
    soft_dma_buffer_init(dma, buff, size);

    OS_ASSERT(dma->buffer->hard_buff.buff != OS_NULL);
    
    return dma->ops.dma_start(dma, dma->buffer->hard_buff.buff, dma->buffer->hard_buff.size);
}

os_err_t soft_dma_stop_recv(soft_dma_t *dma)
{
    soft_dma_buffer_deinit(dma);
    
    return dma->ops.dma_stop(dma);
}

static void soft_dma_irq_callback(soft_dma_t *dma)
{
    if (dma->irq_mask & dma->irq_status & SOFT_DMA_IRQ_HALF)
    {
        dma->cbs.dma_half_callback(dma);
        dma->irq_status &= ~SOFT_DMA_IRQ_HALF;
    }

    if (dma->irq_mask & dma->irq_status & SOFT_DMA_IRQ_FULL)
    {
        dma->cbs.dma_full_callback(dma);
        dma->irq_status &= ~SOFT_DMA_IRQ_FULL;
    }

    if (dma->irq_mask & dma->irq_status & SOFT_DMA_IRQ_TIMEOUT)
    {
        dma->cbs.dma_timeout_callback(dma);
        dma->irq_status &= ~SOFT_DMA_IRQ_TIMEOUT;
    }
}

static void soft_dma_irq(soft_dma_t *dma)
{
    os_uint32_t head = dma->ops.get_index(dma);

    soft_dma_updata_buffer(dma, head);

    soft_dma_irq_callback(dma);
}

/* irq handle */
void soft_dma_half_irq(soft_dma_t *dma)
{
    soft_dma_irq(dma);
}

void soft_dma_full_irq(soft_dma_t *dma)
{
    soft_dma_irq(dma);
}

void soft_dma_timeout_irq(soft_dma_t *dma)
{
    dma->irq_status |= SOFT_DMA_IRQ_TIMEOUT;
    soft_dma_irq(dma);
}

/* irq mask */
void soft_dma_irq_mask_set(soft_dma_t *dma, os_uint32_t mask)
{
    dma->irq_mask |= mask;
}

void soft_dma_irq_mask_clr(soft_dma_t *dma, os_uint32_t mask)
{
    dma->irq_mask &= ~mask;
}

