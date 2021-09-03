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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <os_stddef.h>
#include <string.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

#define STM32_UART_POLL_TIMEOUT_MS_PER_BYTE (100)

#ifdef OS_USING_SERIAL

struct stm32_uart
{
    struct os_serial_device serial;

    UART_HandleTypeDef *huart;

    os_list_node_t list;

    os_uint8_t *dma_rx_buff;
    os_uint32_t dma_rx_index;

    os_uint8_t *rx_buff;
    os_uint32_t rx_size;
};

static os_list_node_t stm32_uart_list = OS_LIST_INIT(stm32_uart_list);

static UART_HandleTypeDef *console_uart = OS_NULL;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    struct stm32_uart *uart;

    os_list_for_each_entry(uart, &stm32_uart_list, struct stm32_uart, list)
    {
        if (uart->huart == huart)
        {
            os_hw_serial_isr_txdone((struct os_serial_device *)uart);
            break;
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    struct stm32_uart *uart;
    int count;

    os_list_for_each_entry(uart, &stm32_uart_list, struct stm32_uart, list)
    {
        if (uart->huart != huart)
            continue;
        
        if (uart->huart->hdmarx != OS_NULL)
        {
            int head = huart->RxXferSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);
            if (head >= uart->dma_rx_index)
            {
                count = head - uart->dma_rx_index;
            }
            else
            {
                count = head - uart->dma_rx_index + huart->RxXferSize;
            }
        }
        else
        {
            count = huart->RxXferSize - huart->RxXferCount;
        }

        count = min(count, uart->rx_size);

        if (count <= 0)
            break;

        uart->rx_size = 0;

        __HAL_UART_DISABLE_IT(uart->huart, UART_IT_RXNE);
        __HAL_UART_DISABLE_IT(uart->huart, UART_IT_IDLE);

        if (uart->huart->hdmarx != OS_NULL && uart->huart->hdmarx->Init.Mode == DMA_CIRCULAR)
        {
            if (uart->dma_rx_index + count < huart->RxXferSize)
            {
                memcpy(uart->rx_buff, uart->dma_rx_buff + uart->dma_rx_index, count);
                uart->dma_rx_index += count;
                if (uart->dma_rx_index >= huart->RxXferSize)
                    uart->dma_rx_index -= huart->RxXferSize;
            }
            else
            {
                int count_tail = huart->RxXferSize - uart->dma_rx_index;
                memcpy(uart->rx_buff, uart->dma_rx_buff + uart->dma_rx_index, count_tail);
                memcpy(uart->rx_buff + count_tail, uart->dma_rx_buff, count - count_tail);
                uart->dma_rx_index = count - count_tail;
                if (uart->dma_rx_index >= huart->RxXferSize)
                    uart->dma_rx_index -= huart->RxXferSize;
            }
        }
        else
        {
            __HAL_UART_DISABLE_IT(uart->huart, UART_IT_IDLE);
            HAL_UART_AbortReceive(huart);
        }

        os_hw_serial_isr_rxdone((struct os_serial_device *)uart, count);
    }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_RxCpltCallback(huart);
}

int HAL_USART_IDLE_PROCESS(UART_HandleTypeDef *huart)
{
    if ((__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)
    && (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) != RESET))
    {
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        HAL_UART_RxCpltCallback(huart);
        return 1;
    }

    return 0;
}

static int stm32_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size);

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    struct stm32_uart *uart;

    os_list_for_each_entry(uart, &stm32_uart_list, struct stm32_uart, list)
    {
        if (uart->huart != huart)
            continue;

        if (uart->huart->hdmarx != OS_NULL && uart->huart->hdmarx->Init.Mode == DMA_CIRCULAR)
        {
            uart->dma_rx_index = 0;
            HAL_UART_Receive_DMA(uart->huart, uart->dma_rx_buff, uart->serial.config.rx_bufsz);
        }
        
        stm32_uart_start_recv(&uart->serial, uart->rx_buff, uart->rx_size);
    }
}

static os_err_t stm32_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct stm32_uart *uart;
    os_uint32_t        data_bits;
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = os_container_of(serial, struct stm32_uart, serial);

    uart->huart->Init.BaudRate     = cfg->baud_rate;
    uart->huart->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    uart->huart->Init.Mode         = UART_MODE_TX_RX;
    uart->huart->Init.OverSampling = UART_OVERSAMPLING_16;
    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        uart->huart->Init.StopBits = UART_STOPBITS_1;
        break;
    case STOP_BITS_2:
        uart->huart->Init.StopBits = UART_STOPBITS_2;
        break;
    default:
        return OS_EINVAL;
    }
    switch (cfg->parity)
    {
    case PARITY_NONE:
        uart->huart->Init.Parity = UART_PARITY_NONE;
        data_bits                = cfg->data_bits;
        break;
    case PARITY_ODD:
        uart->huart->Init.Parity = UART_PARITY_ODD;
        data_bits                = cfg->data_bits + 1;
        break;
    case PARITY_EVEN:
        uart->huart->Init.Parity = UART_PARITY_EVEN;
        data_bits                = cfg->data_bits + 1;
        break;
    default:
        return OS_EINVAL;
    }

    switch (data_bits)
    {
    case DATA_BITS_8:
        uart->huart->Init.WordLength = UART_WORDLENGTH_8B;
        break;
    case DATA_BITS_9:
        uart->huart->Init.WordLength = UART_WORDLENGTH_9B;
        break;
    default:
        return OS_EINVAL;
    }

    if (HAL_UART_Init(uart->huart) != HAL_OK)
    {
        return OS_ERROR;
    }

    if (uart->huart->hdmarx != OS_NULL && uart->huart->hdmarx->Init.Mode == DMA_CIRCULAR)
    {
        __HAL_UART_DISABLE_IT(uart->huart, UART_IT_IDLE);
        HAL_UART_AbortReceive(uart->huart);
    
        if (uart->dma_rx_buff != OS_NULL)
            os_free(uart->dma_rx_buff);

        int rx_bufsz = min(128, serial->config.rx_bufsz);
        
        uart->dma_rx_buff = os_calloc(1, rx_bufsz);
        if (uart->dma_rx_buff == OS_NULL)
        {
            return OS_ENOMEM;
        }

        uart->dma_rx_index = 0;

        os_base_t level = os_irq_lock();
        HAL_UART_Receive_DMA(uart->huart, uart->dma_rx_buff, rx_bufsz);
        __HAL_UART_DISABLE_IT(uart->huart, UART_IT_RXNE);
        __HAL_UART_DISABLE_IT(uart->huart, UART_IT_IDLE);
        os_irq_unlock(level);
    }

    return OS_EOK;
}

static int stm32_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct stm32_uart *uart;
    HAL_StatusTypeDef  ret;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct stm32_uart, serial);

    if (uart->huart->hdmatx != OS_NULL)
    {
        ret = HAL_UART_Transmit_DMA(uart->huart, (uint8_t *)buff, size);
    }
    else
    {
        ret = HAL_UART_Transmit_IT(uart->huart, (uint8_t *)buff, size);
    }

    return (ret == HAL_OK) ? size : 0;
}

static int stm32_uart_stop_send(struct os_serial_device *serial)
{
    struct stm32_uart *uart;
    HAL_StatusTypeDef  ret;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct stm32_uart, serial);

    ret = HAL_UART_AbortTransmit(uart->huart);

    return (ret == HAL_OK) ? 0 : -1;
}

static int stm32_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    HAL_StatusTypeDef  ret;
    struct stm32_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct stm32_uart, serial);

    uart->rx_buff = buff;
    uart->rx_size = size;

    if (uart->huart->hdmarx != OS_NULL && uart->huart->hdmarx->Init.Mode != DMA_CIRCULAR)
    {
        ret = HAL_UART_Receive_DMA(uart->huart, buff, size);
    }
    else
    {
        ret = HAL_UART_Receive_IT(uart->huart, buff, size);
    }

    __HAL_UART_ENABLE_IT(uart->huart, UART_IT_IDLE);

    return (ret == HAL_OK) ? size : 0;
}

static int stm32_uart_stop_recv(struct os_serial_device *serial)
{
    struct stm32_uart *uart;
    HAL_StatusTypeDef  ret;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct stm32_uart, serial);

    __HAL_UART_DISABLE_IT(uart->huart, UART_IT_IDLE);

    ret = HAL_UART_AbortReceive(uart->huart);

    return (ret == HAL_OK) ? 0 : -1;
}

static int stm32_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct stm32_uart *uart;
    HAL_StatusTypeDef  ret;

    int i;
    os_base_t level;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct stm32_uart, serial);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();
        uart->huart->gState = HAL_UART_STATE_READY;
        ret = HAL_UART_Transmit(uart->huart, (uint8_t *)buff + i, 1, STM32_UART_POLL_TIMEOUT_MS_PER_BYTE);
        os_irq_unlock(level);
    }

    return (ret == HAL_OK) ? size : 0;
}

static const struct os_uart_ops stm32_uart_ops = {
    .init         = stm32_uart_init,

    .start_send   = stm32_uart_start_send,
    .stop_send    = stm32_uart_stop_send,

    .start_recv   = stm32_uart_start_recv,
    .stop_recv    = stm32_uart_stop_recv,
    
    .poll_send    = stm32_uart_poll_send,
};

static void stm32_usart_parse_configs_from_stm32cube(struct stm32_uart *uart)
{
    struct os_serial_device *serial = &uart->serial;
    
    serial->config.baud_rate = uart->huart->Init.BaudRate;

    switch (uart->huart->Init.StopBits)
    {
    case UART_STOPBITS_1:
        serial->config.stop_bits = STOP_BITS_1;
        break;
    case UART_STOPBITS_2:
        serial->config.stop_bits = STOP_BITS_2;
        break;
    }
    switch (uart->huart->Init.Parity)
    {
    case UART_PARITY_NONE:
        serial->config.parity   = PARITY_NONE;
        break;
    case UART_PARITY_ODD:
        serial->config.parity   = PARITY_ODD;
        break;
    case UART_PARITY_EVEN:
        serial->config.parity   = PARITY_EVEN;
        break;
    }

    switch (uart->huart->Init.WordLength)
    {
    case UART_WORDLENGTH_8B:
        serial->config.data_bits = DATA_BITS_8;
        break;
    case UART_WORDLENGTH_9B:
        serial->config.data_bits = DATA_BITS_9;
        break;
    }
}

static int stm32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    os_err_t    result  = 0;
    os_base_t   level;

    struct stm32_uart *uart = os_calloc(1, sizeof(struct stm32_uart));

    OS_ASSERT(uart);

    uart->huart = (UART_HandleTypeDef *)dev->info;

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &stm32_uart_ops;
    serial->config = config;

    stm32_usart_parse_configs_from_stm32cube(uart);

    OS_ASSERT(uart->huart->hdmatx == OS_NULL || uart->huart->hdmatx->Init.Mode == DMA_NORMAL);

    level = os_irq_lock();
    os_list_add_tail(&stm32_uart_list, &uart->list);
    os_irq_unlock(level);
    
    result = os_hw_serial_register(serial, dev->name, NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

#else   /* OS_USING_SERIAL */

static int stm32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    console_uart = (UART_HandleTypeDef *)dev->info;
    return 0;
}

#endif

void __os_hw_console_output(char *str)
{
    if (console_uart == OS_NULL)
        return;

    while (*str)
    {
        HAL_UART_Transmit(console_uart, (os_uint8_t *)str, 1, STM32_UART_POLL_TIMEOUT_MS_PER_BYTE);
        str++;
    }

}

OS_DRIVER_INFO stm32_usart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = stm32_usart_probe,
};

OS_DRIVER_DEFINE(stm32_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

static int stm32_usart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    if(!strcmp(dev->name,OS_CONSOLE_DEVICE_NAME))
        console_uart = (UART_HandleTypeDef *)dev->info;
    return OS_EOK;
}

OS_DRIVER_INFO stm32_usart_early_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = stm32_usart_early_probe,
};

OS_DRIVER_DEFINE(stm32_usart_early_driver, CORE, OS_INIT_SUBLEVEL_MIDDLE);
